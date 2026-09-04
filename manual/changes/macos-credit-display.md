---
title: Display credits correctly on macOS
category: fix
release: 0.2.0
targets: []
credit:
- Piotr Großmann
---

The macOS build now formats the current credit total with its actual integer
width instead of reading an incompatible variadic argument. The Windows
readout is unchanged.
