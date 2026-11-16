---
title: Show save times in local time on macOS
category: fix
release: 0.3.0
targets: []
credit:
- Piotr Großmann
---

The save list on macOS now shows each save's date and time in local time. Previously it showed them as UTC, which differed from the local clock wherever the zone offset is not zero. The Windows list is unchanged.
