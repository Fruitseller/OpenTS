---
title: Tactical view controls
summary: How the view over the tactical map moves, including two-finger scrolling on macOS.
category: interface-controls
keys:
  - AutoScroll
  - ScrollMethod
  - ScrollMultiplier
  - ScrollRate
related:
  - type: system
    id: sidebar
---

The tactical view is the part of the screen that shows the map. The sidebar, with its radar pane and controls, sits beside it. The view stops at the edge of the playable area however it is moved.

## What moves the view

| Input | Result |
| --- | --- |
| The pointer against the edge of the screen | Scrolls toward that edge while [`AutoScroll`](/keys/autoscroll/) is on. |
| A right-button drag across the tactical view | Coast scrolling, as [`ScrollMethod`](/keys/scrollmethod/) and [`ScrollRate`](/keys/scrollrate/) set it. |
| A [keyboard scroll command](/commands/) | Moves the view by a fixed step, once [a key is bound](/formats/keyboard-ini/) to the command. |
| A left click on the radar where no object is drawn | Centers the view on that spot. |
| A two-finger scroll over the tactical view, on macOS | Pans the view on both axes, as described below. |

A notched mouse wheel does not move the view. It runs Sidebar Up or Sidebar Down wherever the pointer is, which [the sidebar page](/systems/sidebar/#scrolling-the-strips) describes.

## Two-finger scrolling on macOS

macOS builds pan the view while a scenario runs, when a trackpad or a Magic Mouse scrolls with the pointer over the tactical view. Over the sidebar the same gesture scrolls the strips.

A trackpad and a Magic Mouse report fractions of a scroll notch, while a notched wheel reports whole numbers. A device that has just reported a fraction stays precise for a moment after it starts reporting whole notches, so slowing to a stop does not hand the gesture to the sidebar.

The map moves the way a document scrolls under the system's **Natural scrolling** setting: with it on, the map follows the fingers; with it off, it moves the other way.

The view moves in proportion to the scroll, and the fractions accumulate, so a slow scroll still moves it.

None of the scrolling settings change two-finger scrolling. [`ScrollRate`](/keys/scrollrate/), [`ScrollMultiplier`](/keys/scrollmultiplier/), [`ScrollMethod`](/keys/scrollmethod/) and [`AutoScroll`](/keys/autoscroll/) govern edge scrolling and right-button scrolling.
