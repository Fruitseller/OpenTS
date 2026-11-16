---
title: Treat Command as Control on macOS
category: fix
release: 0.3.0
targets:
- type: format
  id: keyboard-ini
  effect: changed
credit:
- Piotr Großmann
---

On macOS, the Command key now acts as Control. A binding stored for Control runs when Command is pressed, a Command-click carries the same modifier as a Control-click, and the keyboard dialog shows the binding as Ctrl. The Control key itself keeps working.
