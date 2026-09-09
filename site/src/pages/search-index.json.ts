import { getDocs } from '@/lib/docs';
export async function GET() {
  return new Response(
    JSON.stringify(
      (await getDocs()).map(({ data, body }) => ({
        slug: data.slug,
        title: data.title,
        group: data.group,
        description: data.description,
        text: (body || '')
          .replace(/```[\s\S]*?```/g, ' ')
          .replace(/[#*`\[\]()>|]/g, ' ')
          .replace(/\s+/g, ' '),
      })),
    ),
    { headers: { 'Content-Type': 'application/json' } },
  );
}
