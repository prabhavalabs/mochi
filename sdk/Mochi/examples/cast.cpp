#include <Mochi.h>
#include <cstdio>
#include <vector>

// A standalone consumer: render the same waving pose with every character.
int main(int argc, char** argv) {
  constexpr int cell=320, width=cell*mochi::kCharacterCount, height=320;
  std::vector<uint16_t> pixels(width*height);
  mochi::Renderer renderer({pixels.data(),pixels.size(),width,height});
  const auto pose=mochi::sample_pose(mochi::State::Waving,1.0);
  if (!renderer.clear()) return 1;
  for (int i=0;i<mochi::kCharacterCount;i++) {
    if (!renderer.setCharacter(static_cast<mochi::Character>(i)) ||
        !renderer.draw(pose,cell*i+160,165,.78f)) return 1;
  }
  const char* filename=argc>1 ? argv[1] : "cast.ppm";
  FILE* file=std::fopen(filename,"wb");
  if (!file) { std::perror(filename); return 1; }
  std::fprintf(file,"P6\n%d %d\n255\n",width,height);
  for (const uint16_t pixel:pixels) {
    const unsigned char rgb[]={static_cast<unsigned char>((pixel>>11)*255/31),
      static_cast<unsigned char>(((pixel>>5)&63)*255/63),static_cast<unsigned char>((pixel&31)*255/31)};
    if (std::fwrite(rgb,1,3,file)!=3) { std::fclose(file); return 1; }
  }
  if (std::fclose(file)!=0) return 1;
  std::printf("Rendered Mochi, Sprout, Peach, Nimbus to %s\n",filename);
}
