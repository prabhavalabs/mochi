import { getCollection } from 'astro:content';
export async function getDocs() {
  return (await getCollection('docs')).sort((a, b) => a.data.order - b.data.order);
}
export async function getNavigation() {
  return (await getDocs()).map(({ data }) => ({
    slug: data.slug,
    title: data.title,
    group: data.group,
  }));
}
