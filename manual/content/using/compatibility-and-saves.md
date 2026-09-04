---
title: Compatibility and save games
summary: How OpenTS lists and loads save games, and which save games a build accepts.
category: compatibility-migration
source_files:
  - README.md
  - code/loaddlg.cpp
  - code/saveload.cpp
  - code/savever.cpp
related:
  - type: using
    id: project-status
---

OpenTS uses the English Tiberian Sun 2.03 release as its inherited data and behavior baseline.

A build lists and loads only saves written by the same OpenTS version. Every save records a version stamp: the major, minor, and patch numbers of the version shown in the corner of the title screen. A prerelease label such as `-beta1` and the build details in parentheses after the number are not part of the stamp.

- The Win32 and x64 builds share the stamp. Load a save with the platform that wrote it, and play a network game with every player on the same platform; a mixed network game can go out of sync.

Saving and loading on macOS has not received runtime verification.
