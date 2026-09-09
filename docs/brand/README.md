# Mochi visual identity

The original cream-colored **Mochi** is the primary mascot and brand mark for
the Mochi project. Its rounded body, black pill-shaped eyes, small smile, rosy
cheeks, and friendly wave are the recognizable features of the mark.

![Original Mochi mark](mochi.png)

## Assets

| File                                         | Use                                                                                       |
| -------------------------------------------- | ----------------------------------------------------------------------------------------- |
| [mochi.png](mochi.png)                       | Original square master artwork; use for the project mark and square thumbnails            |
| [repository-cover.jpg](repository-cover.jpg) | Landscape cover for the repository README and GitHub social preview                       |
| [social-preview.jpg](social-preview.jpg)     | 1200 × 630 default Open Graph image for the website and LinkedIn-style landscape previews |
| [social-preview-x.jpg](social-preview-x.jpg) | 1200 × 600 landscape image for X large-image cards                                        |
| [social-square.jpg](social-square.jpg)       | 1200 × 1200 optional square Open Graph alternative and manual social-posting asset        |

The PNG master remains the source artwork and should not be replaced by a
compressed derivative. The JPEG covers pair it with the Mochi wordmark and a
short description. Sprout, Peach, and Nimbus remain part of the SDK cast; the
original Mochi leads the project's visual identity. The three social JPEGs are
each under 55 KB at their full dimensions for quick crawler downloads.

## Website sharing

The production site uses absolute image URLs under
`https://mochi.prabhavalabs.com/brand/`. The website build copies these files
from `docs/brand/` into the static output; they need no runtime image service or
server-side generation.

| Metadata or use                                 | Asset                                          | Notes                                                                                               |
| ----------------------------------------------- | ---------------------------------------------- | --------------------------------------------------------------------------------------------------- |
| `og:image`                                      | `social-preview.jpg`                           | Primary 1.91:1 website preview, with 1200 × 630 dimensions, JPEG type, and descriptive alt metadata |
| `twitter:card` / `twitter:image`                | `summary_large_image` / `social-preview-x.jpg` | X-specific 2:1 large card and matching alt metadata                                                 |
| Optional second Open Graph image or manual post | `social-square.jpg`                            | Square alternative for clients or posts that favor 1:1 artwork                                      |
| GitHub repository settings                      | `repository-cover.jpg`                         | Keep the existing 1280 × 640 repository cover separate from website metadata                        |

Open Graph consumers use the first declared image when they need to choose
between multiple images, so keep the 1200 × 630 landscape image first. Include
the image URL, width, height, MIME type, and alt text together as described by
the [Open Graph protocol](https://ogp.me/). LinkedIn recommends a 1.91:1 image
at least 1200 × 627 pixels; see its
[website sharing guidance](https://www.linkedin.com/help/linkedin/answer/a521928).

After changing a share image or its metadata, deploy the site, confirm the
public HTTPS image URL responds, and ask each platform's preview inspector or
debugger to fetch the official page again. Social platforms cache metadata and
may crop, delay, or select images differently, so previews are not guaranteed
to look identical in every client. For a substantial artwork revision, a new
versioned filename is more reliable than replacing an image at an already
cached URL.

## Visual consistency

- Keep Mochi's cream body, peach cheeks, black face, and original proportions.
- Scale uniformly and leave breathing room around the body and waving hand.
- Prefer the supplied dark background so the silhouette stays clear.
- Keep text outside the character and avoid cropping its face or hands.
- Use the primary mark for project identity and the cast image when explaining
  the SDK's character choices.

## Repository preview

The repository cover uses GitHub's recommended 1280 × 640 canvas and is under
1 MB. Repository administrators can set it in **Settings → General → Social
preview → Edit → Upload an image**. See [GitHub's preview instructions](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/customizing-your-repositorys-social-media-preview).

The source and included artwork follow the project's [MIT License](../../LICENSE).
