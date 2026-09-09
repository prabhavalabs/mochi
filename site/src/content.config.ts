import { defineCollection } from 'astro:content';
import { z } from 'astro/zod';
import { glob } from 'astro/loaders';
const docs = defineCollection({
  loader: glob({ pattern: '**/*.md', base: './src/content/docs' }),
  schema: z.object({
    slug: z.string(),
    title: z.string(),
    description: z.string(),
    group: z.string(),
    order: z.number(),
    source: z.string().optional(),
  }),
});
export const collections = { docs };
