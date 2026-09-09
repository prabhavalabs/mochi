#include "mochi/Renderer.h"
#include <algorithm>
#include <cmath>

namespace mochi {
namespace {
constexpr float pi=3.14159265358979323846f;
struct CircleTable {
  Point points[64];
  CircleTable() {
    for(int i=0;i<64;i++) points[i]={std::cos(2*pi*i/64),std::sin(2*pi*i/64)};
  }
};
const CircleTable circle;
Point bezier(Point a,Point b,Point c,Point d,float t) {
  const float u=1-t;
  return {u*u*u*a.x+3*u*u*t*b.x+3*u*t*t*c.x+t*t*t*d.x,
          u*u*u*a.y+3*u*u*t*b.y+3*u*t*t*c.y+t*t*t*d.y};
}
}
Palette default_palette(Character character) {
  switch (character) {
    case Character::Sprout:
      return {rgb565(0xBCE4B4),rgb565(0x203A2B),rgb565(0xF2BEA0),rgb565(0x79AD7F),rgb565(0x4F9567),rgb565(0xCDECAA)};
    case Character::Peach:
      return {rgb565(0xF7C89F),rgb565(0x3B2A24),rgb565(0xED9E8B),rgb565(0xC79678),rgb565(0xECA595),rgb565(0xFFE1BD)};
    case Character::Nimbus:
      return {rgb565(0xC8DFF5),rgb565(0x26354A),rgb565(0xD9B7CF),rgb565(0x93AFCB),rgb565(0xABC7E5),rgb565(0xE5F0FC)};
    default: return {};
  }
}
Renderer::Renderer(Surface surface, Palette palette)
    : pixels_(surface.pixels),width_(surface.width),height_(surface.height),
      stride_(surface.stride == 0 ? surface.width : surface.stride),palette_(palette) {
  // The coordinate bound also keeps raster conversions safely inside int.
  if (!pixels_ || width_ <= 0 || height_ <= 0 || width_ > 16384 || height_ > 16384 || stride_ < width_) return;
  const uint64_t required = uint64_t(height_ - 1) * uint64_t(stride_) + uint64_t(width_);
  valid_ = required <= surface.capacity;
}
bool Renderer::setCharacter(Character character) {
  if (static_cast<unsigned>(character) >= kCharacterCount) return false;
  if (character != character_) {
    character_ = character;
    palette_ = default_palette(character);
  }
  return true;
}
bool Renderer::clear(uint16_t color) {
  if (!valid_) return false;
  for (int y=0;y<height_;y++) std::fill(pixels_+size_t(y)*stride_,pixels_+size_t(y)*stride_+width_,color);
  return true;
}
Point Renderer::transform(Point p) const {
  const float x=p.x*pose_.sx,y=p.y*pose_.sy;
  return {cx_+scale_*(x*cos_-y*sin_),cy_+scale_*(x*sin_+y*cos_)};
}
void Renderer::pixel(int x,int y,uint16_t color,float alpha) {
  if(x<0||x>=width_||y<0||y>=height_||alpha<=0) return;
  uint16_t& dest=pixels_[size_t(y)*stride_+x];
  if(alpha>=.99f){dest=color;return;}
  const unsigned a=static_cast<unsigned>(alpha*256),b=256-a;
  const unsigned r=(((color>>11)*a+(dest>>11)*b)>>8);
  const unsigned g=((((color>>5)&63)*a+((dest>>5)&63)*b)>>8);
  const unsigned blue=(((color&31)*a+(dest&31)*b)>>8);
  dest=static_cast<uint16_t>((r<<11)|(g<<5)|blue);
}
void Renderer::polygon(const Point* source,size_t count,uint16_t color) {
  if(count<3||count>128) return;
  Point pts[128]; float min_y=height_,max_y=-1;
  for(size_t i=0;i<count;i++){pts[i]=transform(source[i]);min_y=std::min(min_y,pts[i].y);max_y=std::max(max_y,pts[i].y);}
  const int start=std::max(0,static_cast<int>(std::floor(min_y)));
  const int end=std::min(height_-1,static_cast<int>(std::ceil(max_y)));
  for(int y=start;y<=end;y++) {
    float xs[128];size_t n=0;const float scan=y+.5f;
    for(size_t i=0,j=count-1;i<count;j=i++) {
      const auto a=pts[j],b=pts[i];
      if((a.y<=scan&&b.y>scan)||(b.y<=scan&&a.y>scan))
        xs[n++]=a.x+(scan-a.y)*(b.x-a.x)/(b.y-a.y);
    }
    std::sort(xs,xs+n);
    for(size_t i=0;i+1<n;i+=2) {
      const float left=xs[i],right=xs[i+1];
      const int x0=static_cast<int>(std::floor(left)),x1=static_cast<int>(std::floor(right));
      if(x0==x1){pixel(x0,y,color,right-left);continue;}
      pixel(x0,y,color,x0+1-left);
      // Only the two boundary pixels need alpha. Fill the solid span directly.
      const int start_x=std::max(0,x0+1),end_x=std::min(width_,x1);
      if(end_x>start_x) std::fill(pixels_+size_t(y)*stride_+start_x,pixels_+size_t(y)*stride_+end_x,color);
      pixel(x1,y,color,right-x1);
    }
  }
}
void Renderer::ellipse(float x,float y,float rx,float ry,float angle,uint16_t color) {
  Point pts[32];const float ca=angle==0?1:std::cos(angle),sa=angle==0?0:std::sin(angle);
  const int count=std::max(rx,ry)<6?16:32;
  for(int i=0;i<count;i++){
    const Point unit=circle.points[i*64/count];const float dx=rx*unit.x,dy=ry*unit.y;
    pts[i]={x+dx*ca-dy*sa,y+dx*sa+dy*ca};
  }
  polygon(pts,count,color);
}
void Renderer::curve(Point a,Point b,Point c,Point d,float width,uint16_t color) {
  Point points[17];
  for(int i=0;i<=16;i++)points[i]=bezier(a,b,c,d,float(i)/16);
  stroke(points,17,width,color);
}
void Renderer::stroke(const Point* points,size_t count,float width,uint16_t color) {
  if(count<2||count>64)return;
  Point outline[128];const float radius=width*.5f;
  for(size_t i=0;i<count;i++){
    const Point prev=points[i==0?0:i-1],next=points[i+1<count?i+1:count-1];
    const float dx=next.x-prev.x,dy=next.y-prev.y,len=std::max(.001f,std::hypot(dx,dy));
    const float nx=-dy/len*radius,ny=dx/len*radius;
    outline[i]={points[i].x+nx,points[i].y+ny};
    outline[count*2-1-i]={points[i].x-nx,points[i].y-ny};
  }
  polygon(outline,count*2,color);
  ellipse(points[0].x,points[0].y,radius,radius,0,color);
  ellipse(points[count-1].x,points[count-1].y,radius,radius,0,color);
}
void Renderer::hand(float x,float y,float angle,int side) {
  ellipse(x,y,20,28,angle,palette_.body);
  Point points[13];const float start=side<0?.1f:1.4f,end=side<0?1.75f:3.05f;
  const float ca=std::cos(angle),sa=std::sin(angle);
  for(int i=0;i<=12;i++){
    const float a=start+(end-start)*i/12,dx=17*std::cos(a),dy=25*std::sin(a);
    points[i]={x+dx*ca-dy*sa,y+dx*sa+dy*ca};
  }
  stroke(points,13,2,palette_.crease);
}
void Renderer::eye(float x,float openness,float happy) {
  x+=pose_.gaze_x;const float y=-15+pose_.gaze_y;
  if(openness<.12f){curve({x-9,y+2},{x-4,y+7},{x+4,y+7},{x+9,y+2},4.5f,palette_.face);return;}
  if(happy>.65f){curve({x-10,y},{x-4,y-10},{x+4,y-10},{x+10,y},5.5f,palette_.face);return;}
  const float h=34*openness,r=std::min(8.5f,h*.5f);
  Point pts[36];size_t n=0;
  for(int corner=0;corner<4;corner++){
    const float cx=x+(corner==0||corner==3 ? 8.5f-r : -8.5f+r);
    const float cy=y+(corner<2 ? h/2-r : -h/2+r);
    for(int i=0;i<9;i++){
      const Point unit=circle.points[(corner*16+i*2)%64];pts[n++]={cx+r*unit.x,cy+r*unit.y};
    }
  }
  polygon(pts,n,palette_.face);
}
void Renderer::cat_ear(int side) {
  Point outline[49]; size_t n=0;
  Point prev{side*120.f,-46}; outline[n++]=prev;
  const Point curves[][3]={
    {{side*122.f,-74},{side*116.f,-131},{side*107.f,-138}},
    {{side*98.f,-140},{side*67.f,-110},{side*52.f,-86}},
    {{side*61.f,-61},{side*92.f,-49},{side*120.f,-46}}};
  for(const auto& c:curves){for(int i=1;i<=12;i++)outline[n++]=bezier(prev,c[0],c[1],c[2],float(i)/12);prev=c[2];}
  polygon(outline,n,palette_.body);
  const Point inner[]={{side*106.f,-117},{side*112.f,-77},{side*79.f,-89}};
  polygon(inner,3,palette_.accent);
}
void Renderer::body() {
  if (character_ == Character::Nimbus) {
    // Overlapping lobes make a cloud silhouette, including at icon sizes.
    ellipse(0,30,147,94,0,palette_.body);
    ellipse(-94,-13,58,70,0,palette_.body);
    ellipse(-27,-58,67,66,0,palette_.body);
    ellipse(68,-30,68,73,0,palette_.body);
    ellipse(117,24,38,55,0,palette_.body);
    return;
  }
  if (character_ == Character::Peach) { cat_ear(-1); cat_ear(1); }
  const float top = character_ == Character::Mochi ? -128.f : -100.f;
  Point outline[80];size_t n=0;Point prev{0,top};outline[n++]=prev;
  const Point curves[][3]={{{79,top-2},{123,-69},{145,-3}},{{161,42},{171,80},{146,101}},
    {{124,122},{59,124},{0,124}},{{-62,124},{-127,121},{-148,103}},
    {{-174,83},{-162,42},{-145,-5}},{{-122,-75},{-72,top-1},{0,top}}};
  for(const auto& c:curves){for(int i=1;i<=12;i++)outline[n++]=bezier(prev,c[0],c[1],c[2],float(i)/12);prev=c[2];}
  polygon(outline,n,palette_.body);
  if (character_ == Character::Sprout) {
    curve({0,-91},{-2,-104},{0,-114},{4,-121},6,palette_.accent);
    for (int side:{-1,1}) {
      Point leaf[25];size_t count=0;const Point root{2,-113};leaf[count++]=root;
      const Point tip{side*46.f,side<0 ? -127.f : -138.f};
      for(int i=1;i<=12;i++)leaf[count++]=bezier(root,{side*5.f,-139},{side*28.f,-140},tip,float(i)/12);
      for(int i=1;i<=12;i++)leaf[count++]=bezier(tip,{side*34.f,-111},{side*17.f,-103},root,float(i)/12);
      polygon(leaf,count,palette_.accent);
      curve({side*8.f,-116},{side*17.f,-119},{side*24.f,-122},{side*31.f,-126},1.8f,palette_.detail);
    }
  }
}
bool Renderer::draw(const Pose& p,float x,float y,float scale) {
  if (!valid_ || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(scale) ||
      std::fabs(x)>1000000 || std::fabs(y)>1000000 || scale<=0 || scale>100) return false;
  const float fields[]={p.y,p.sx,p.sy,p.tilt,p.gaze_x,p.gaze_y,p.eye_l,p.eye_r,p.happy_eyes,
      p.smile,p.mouth_open,p.mouth_round,p.mouth_x,p.brow_l,p.brow_r,p.brow_sad,
      p.left_x,p.left_y,p.left_angle,p.right_x,p.right_y,p.right_angle,p.cheeks};
  for (float value:fields) if (!std::isfinite(value) || std::fabs(value)>10000) return false;
  // Custom poses share the same bounded transform contract as built-in poses.
  if (p.sx<=0 || p.sx>10 || p.sy<=0 || p.sy>10 || p.eye_l<0 || p.eye_l>10 || p.eye_r<0 || p.eye_r>10) return false;
  pose_=p;cx_=x;cy_=y+p.y*scale;scale_=scale;cos_=std::cos(p.tilt);sin_=std::sin(p.tilt);
  body();
  hand(p.left_x,p.left_y,p.left_angle,-1);hand(p.right_x,p.right_y,p.right_angle,1);
  ellipse(-68+p.gaze_x,17+p.gaze_y,13,8.5f,.07f,palette_.blush);
  ellipse(68+p.gaze_x,17+p.gaze_y,13,8.5f,.07f,palette_.blush);
  eye(-48,p.eye_l,p.happy_eyes);eye(48,p.eye_r,p.happy_eyes);
  if (character_ == Character::Peach) {
    for(int side:{-1,1}) for(int line=0;line<2;line++) {
      const float y=5+line*10+p.gaze_y;
      curve({side*93.f+p.gaze_x,y},{side*97.f+p.gaze_x,y-1},
            {side*102.f+p.gaze_x,y-2},{side*106.f+p.gaze_x,y-3},2.2f,palette_.crease);
    }
    const Point nose[]={{-4+p.gaze_x,8+p.gaze_y},{4+p.gaze_x,8+p.gaze_y},{p.gaze_x,12+p.gaze_y}};
    polygon(nose,3,palette_.face);
  }
  for(int side:{-1,1}){
    const float amount=side<0?p.brow_l:p.brow_r;
    if(amount<.15f)continue;
    const float bx=side*48+p.gaze_x,by=p.gaze_y;
    curve({bx-9,-47-side*4*p.brow_sad+by},{bx-3,-51+by},{bx+3,-51+by},
          {bx+9,-47+side*4*p.brow_sad+by},4.5f*amount,palette_.face);
  }
  const float mx=p.mouth_x+p.gaze_x,my=19+p.gaze_y;
  if(p.mouth_open>.06f){const float w=14-p.mouth_round*5,h=3+p.mouth_open*11;ellipse(mx,my+3+h/2,w,h,0,palette_.face);}
  else curve({mx-12,my},{mx-6,my+p.smile*12},{mx+6,my+p.smile*12},{mx+12,my-1},5,palette_.face);
  return true;
}
}
