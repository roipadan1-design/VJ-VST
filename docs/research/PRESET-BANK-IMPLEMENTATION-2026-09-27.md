# Preset bank ("Looks") - implementation notes

Date: 2026-09-27. Built from `docs/product-design/PRESET-BANK-PROPOSAL.md`, Option B, with scene defaults (owner approved).
Status: **built, compiles clean (Release), harness-tested against the real engine. Not yet tried in Ableton Live.**
Nothing is committed; everything is in the working tree for review.

## What you get

- **PLAY grid switch** `SCENES · LOOKS · ★` at the top right of the SCENE section (where the grid hint used to be). The choice is remembered per plug-in instance.
- **Look tiles** (same 109 x 30 size and colours as scene tiles): name, a star, a small strip of the look's 3 colours, the scene it is built on. Badges: `NEXT` (cued), `SAVED` (just saved), `DEF` (it is its scene's default), `START` (start-up look). A red bar/outline = the look now on screen. A scene that no longer exists shows as "Scene (missing)" in amber.
- **Saving:** right-click the **live scene's name** (big title), the **live scene tile**, or an **empty spot in LOOKS** > *Save look...* > a name field opens where the cue bar is, pre-filled with e.g. "Hot Blobs · Film · Ice" > Enter (or SAVE) > the grid switches to LOOKS and the new tile flashes SAVED. Esc / CANCEL = don't save.
- **Using:** click a look = cue (NEXT, shown in the cue bar); GO (button, MIDI, automation) = switch; double-click = both. On GO the scene changes (by name, on the beat as usual) and the knobs blend to the look over the **Snapshot Morph** time (same as Moments).
- **Star:** click the star on a tile. Favourites sort first in LOOKS; ★ shows only favourites.
- **Right-click a look tile:** Update with current settings (asks once, in a submenu) · Rename... · Duplicate · Delete (asks once; the file goes to the Recycle Bin) · Add/Remove favourite · **Set as this scene's default** (ticked when set; click again to clear) · **Set as start-up look** (same) · Show the looks folder.
- **Right-click a scene tile:** the live one offers *Save look...* and *Save look as <scene>'s default...*; any scene with a default shows "Opens on: <look>" and *Clear <scene>'s default*. Scenes with a default have a small dot in the tile's corner.
- **Moments A-D:** unchanged, plus right-click > *Save to library...* (enabled when the slot holds a moment).

## Storage (what I chose and why)

- One JSON file per look in **`Documents\VJ VST\Looks\`** (`juce::File::userDocumentsDirectory`, so a OneDrive-redirected Documents works). Created on the first save. Outside the repo and build tree: `build_instrument_presets.py` can never see it.
- **No manifest - the folder is the library.** It is rescanned when its listing changes (names / sizes / dates, checked at most twice a second while the plug-in window is open, and before any recall). Reason: nothing can go out of sync; copying a `.json` in or deleting one in Explorer just works; backup = copy the folder. One library is shared by every plug-in instance in the process.
- The **scene-default and start-up flags are stored inside the look files** (`"sceneDefault"`, `"startup"`). The library keeps them unique: setting one clears it on the other look(s). If hand-copied files ever disagree, the most recently saved wins.
- Scenes are referenced **by name** (`"scene": "Hot Blobs"`, matched case-insensitively), never by position. Palette, LOOK preset and react style are stored by name and index (name wins).
- A look holds exactly what a Moment holds (reuses `snapshotParamIds()`): 8 macros; the 13 LOOK knobs (this includes **Cut Rate** and **Reactivity**, as Moments do); drift, push, softness, shots, hud; look amount; the 5 follow toggles; palette + 3 custom colours; LOOK preset; react style. Not included (as with Moments): freeze, reverse, sync, STILL/calm, media, blackout, role/input settings.

Example (trimmed):
```json
{ "format": "vjvst-look", "version": 1, "name": "Test Ice", "scene": "Hot Blobs",
  "favourite": true, "sceneDefault": true, "startup": false,
  "palette": "Ice", "paletteIndex": 3, "look": "Film", "lookIndex": 2, "reactStyle": "Pulse", "reactStyleIndex": 1,
  "colours": ["ff050000", "ffe01008", "ffff9a86"],
  "values": { "macro1": 0.21, "macro2": 0.5, "grain": 0.3, "...": "..." },
  "created": "2026-09-27T...", "modified": "2026-09-27T..." }
```

## The behaviour change: scene defaults (read this)

**Before:** switching scenes never touched the knobs; they carried over.
**Now:** exactly the same, **except** for a scene that has a default look. Firing such a scene blends the knobs, LOOK, colours and react style to that look, over the Snapshot Morph time (Cut = instantly; default 1 bar) - i.e. firing the scene behaves exactly like GO on its default look.

It applies when the scene is fired from: a scene tile (double-click, or click + GO), `<` / `>` + GO, the MIDI-mapped GO / Previous / Next, **Live automation or a MIDI-mapped "Preset" parameter**, and the automatic switch to a media scene when you load an image. Re-firing the scene that is already live also re-applies it (a quick "reset to my default").

It does **not** apply (knobs carry over as before) when:
- the scene has no default (the dot is absent);
- a **Live set is opened** (the set's saved knobs win);
- the **engine switches by itself** (MOTION > Auto scene cuts, the engine's own keyboard) - applying a default on every automatic cut would make the knobs jump on each cut. Say if you want it there too;
- you GO on a **look**: that look's own values win over its scene's default.

**Start-up look:** a new plug-in instance that did not load a saved state (i.e. a new set / a freshly inserted device), and is the only VJ Analyzer running, opens on it (cut, no blend) about 1.5 s after it is created. Saved Live sets always open as saved; a second VJ Analyzer added to a set never takes it. If the engine is not running yet, the knobs are set at once and the look's scene is fired as soon as the engine lists its scenes.

## Decisions I made where the proposal was open

1. **Moment > Save to library** builds the look on the **scene playing now** (a Moment stores no scene). The name prompt says so.
2. Scene defaults blend over the **Snapshot Morph** time (not an instant cut), consistent with GO on a look. Set Morph to Cut for instant.
3. **Update with current settings** and **Delete** both ask once (submenu "Yes, ..."); Delete moves the file to the Recycle Bin (fallback: a `Deleted` subfolder), never a hard delete.
4. "Update" re-bases the look on the current scene; if that differs from its old scene, it stops being the old scene's default (the info line says so) rather than silently taking over another scene's default.
5. "Reset to factory values" on scene tiles (mentioned in the proposal) was **not** built - it was not in the approved list and the plug-in does not know each scene's factory knob values.
6. Sorting in LOOKS: favourites first, then by name (natural order) - predictable positions for live use. A newly saved look scrolls into view.
7. A look whose scene no longer exists still recalls its knobs onto the current scene, and says so.

## Files

New:
- `plugin/Source/LookLibrary.h`, `plugin/Source/LookLibrary.cpp` - the storage layer.

Changed (plug-in only; no engine, preset, shader or build-script change):
- `plugin/Source/PluginProcessor.h/.cpp` - `recallSnapshot` morph generalised (`startMorph`); looks capture / apply / cue; `fireScene` applies scene defaults; "preset" automation watcher; pending scene by name; start-up look.
- `plugin/Source/PluginEditor.h/.cpp` - `LookGrid`, grid switch, name field, menus (scene tile, scene name, look tile, empty LOOKS area, Moments), star drawn as a path, dot on scene tiles.
- `plugin/Source/LeadRegistry.h` - `anyOtherRunning()` (start-up look ignores deleted devices Live keeps for undo).
- `plugin/CMakeLists.txt` - adds `LookLibrary.cpp` to the plug-in and the harness.
- `plugin/tools/PluginHarness.cpp` - 28 new `--selftest` checks and a `--lookstest out.png [seconds]` click-path mode (both use a scratch folder in %TEMP%, never your Documents).

## Verified

- Full Release build per `Build and Install.bat` (analysis + tests, engine, plug-in): clean; the only warnings (C4459 `height`) were there before. **The VST3 was built but NOT copied into `C:\Program Files\Common Files\VST3`** - run `Build and Install.bat` (with Live closed) to install it.
- `PluginHarness --selftest`: 47/47 pass (19 existing + 28 new: storage, JSON round trip, unique names, favourites order, cue + GO, carry-over unchanged without a default, default applied from fireScene and from automation, one default per scene, clear default, rename/duplicate/delete, Moment > library, hand-copied files, start-up look for a fresh instance only / not for a loaded set / not for a second instance).
- `PluginHarness --lookstest` with the real engine running: switch to LOOKS by clicking it; right-click on the scene name opens a menu; name field pre-filled; typing + Enter saves and flashes; star click favourites and re-sorts; click a look tile = cued, scene unchanged; GO = engine switched to the look's scene and knobs/palette recalled; right-click on a look tile opens its menu; scene without default keeps knobs; double-click the defaulted scene tile = knobs back to the default; ★ view shows only favourites; Moment A > library. Screenshots looked right. `--cuetest` still passes.

## NOT verified - needs your hands in Live

- **Typing the name inside Live.** The harness sends the Enter key directly; in Live, Live may keep some keys for itself (e.g. the computer MIDI keyboard). If Enter or letters don't reach the field, click in the field first; the SAVE button always works.
- **Popup menus and the right-click itself inside Live's plug-in window** (the harness proves the menus open, not that you can pick items with the mouse there).
- **Start-up look on File > New Live Set**, and that Live really restores saved sets before the 1.5 s window (it should: it restores synchronously while loading).
- **Automation of "Preset" in a real arrangement** triggering the scene default.
- Where Windows puts "Documents" on your laptop (OneDrive) - *Show the looks folder* in any look menu opens it.
