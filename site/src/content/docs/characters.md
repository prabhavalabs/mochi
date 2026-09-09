---
slug: 'characters'
title: 'Characters & states'
group: 'SDK'
order: 20
description: 'Four distinct silhouettes, ten shared emotions, and colors you can adapt to any application.'
---

## Meet the cast

![Mochi, Sprout, Peach, and Nimbus](/media/characters.png)

| Character           | Appearance                               | Portal personality   |
| ------------------- | ---------------------------------------- | -------------------- |
| `Character::Mochi`  | Cream buddy with a smile and warm cheeks | Quietly cheerful     |
| `Character::Sprout` | Mint seedling with two leaves            | Endlessly curious    |
| `Character::Peach`  | Peach cat with ears and whiskers         | A playful sweetheart |
| `Character::Nimbus` | Blue cloud with a scalloped outline      | A gentle daydreamer  |

Mochi is the project's original mascot and brand mark. All four designs use procedural geometry; the core needs no sprite sheets.

## Select a character

```cpp
mochi::Mascot buddy;
buddy.setCharacter(mochi::Character::Peach);
buddy.setState(mochi::State::Thinking);
```

Selection preserves state, elapsed time, speed, pause, and speech input. Choosing a different character loads its default palette. Choosing the current character preserves any palette edits. Invalid enum values return false and leave the selection unchanged. `Character::Count` is a sentinel, not a character.

Use `character_name()`, `character_detail()`, and `kCharacterCount` to build your own selector. Each instance has its own selection; you do not need to include the firmware's menu in your program.

## Every feeling

| Enum               | Motion                              |
| ------------------ | ----------------------------------- |
| `State::Idle`      | Gentle breathing and wandering gaze |
| `State::Blinking`  | Repeating paired blinks             |
| `State::Happy`     | Cheerful bounce and smiling eyes    |
| `State::Listening` | Attentive gaze and raised hand      |
| `State::Thinking`  | Upward glance and thoughtful pose   |
| `State::Speaking`  | Rhythmic mouth and hand movement    |
| `State::Sleeping`  | Closed eyes and slow breathing      |
| `State::Surprised` | Wide eyes and a round mouth         |
| `State::Sad`       | Lowered pose and concerned brows    |
| `State::Waving`    | A friendly waving hand              |

`State::Count` is a sentinel. Use `state_name()` and `state_detail()` for labels. Speaking can use `setSpeechLevel(0..1)` to drive mouth opening from your application's audio analysis; the SDK itself does not capture or play audio.

## Customize the palette

```cpp
buddy.setCharacter(mochi::Character::Sprout);
auto colors = mochi::default_palette(mochi::Character::Sprout);
colors.body = mochi::rgb565(0xB8E8D0);
colors.blush = mochi::rgb565(0xF5AFA0);
buddy.setPalette(colors);
```

`Palette` contains body, face, blush, crease, accent, and detail colors. Accent/detail color leaves and ear details. Colors are native-endian RGB565 values. Keep enough contrast between the face and body at the size you actually render.

## Add a new design

Extend the enum, name/detail helpers, default palette, and renderer geometry together. Test every state and scale, clipping near screen edges, and the full extent of moving hands. The portable test suite covers all built-in character/state combinations. See [Contributing](/docs/contributing/) before proposing additions.

For a portal-only personality change, edit the character registry and theme tokens described in the [website guide](/docs/website/). Website selection does not change firmware settings.
