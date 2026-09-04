---
title: Load and redraw isometric terrain safely
category: fix
release: 0.2.0
targets:
- type: format
  id: scenario-terrain
  effect: changed
credit:
- Piotr Großmann
---

The macOS build now finds theater TMP files and reads their Microsoft-layout
records correctly. Empty tile sets no longer trigger invalid arithmetic or
indexing, and a cell whose selected sub-tile has no image record keeps its base
terrain bounds. Existing scenario and theater files require no changes.
