---
title: Quit from the menus with the close button or Command-Q on macOS
category: fix
release: 0.3.0
targets:
- type: system
  id: leaving-a-match
  effect: changed
credit:
- Piotr Großmann
---

On macOS, closing the window or pressing Command-Q outside a scenario now ends the program the way Exit Game on the main menu does, and closes any open screen first. The close button was previously ignored outside a scenario. During a scenario it still quits the same way as Abort in the options menu, and Windows is unchanged.
