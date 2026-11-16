---
title: Ship the macOS build as an application bundle
category: feature
release: 0.3.0
targets:
- type: command
  id: launch:data-directory
  effect: changed
- type: command
  id: launch:user-directory
  effect: changed
- type: system
  id: ui-files
  effect: changed
credit:
- Piotr Großmann
---

The macOS build now produces `GameD.app` or `Game.app` instead of a loose executable. A bundle reads game data from the folder that contains the `.app` and keeps the UI in `Contents/Resources/ui`; `-DATADIR=` replaces the data folder.

A bundle writes the player's files, debug log, out-of-sync reports and crash reports to `~/Library/Application Support/OpenTS/`. `-USERDIR=` replaces the folder for the player's files, while the debug log, out-of-sync reports and crash reports stay in the default folder.
