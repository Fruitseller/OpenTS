---
title: Allow switching away from OpenTS on macOS
category: fix
release: 0.2.0
targets:
- type: key
  id: Fullscreen
  effect: changed
credit:
- Piotr Großmann
---

After the macOS window first receives focus, switching to another application
no longer pulls the game back to the front. A full-screen game stays at the
normal window level so the selected application's windows can appear above it.
Windows behavior is unchanged.
