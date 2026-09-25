# Competitive landscape and live workflow: how VJ VST becomes a finished instrument

*Prepared 26 September 2026. Scope: competitors, live workflow and product. Visual styling is out of scope (see `DESIGN-DOSSIER.md`, written in parallel). This goes beyond `docs/RESEARCH-BRIEF-COMPETITORS.md` / `docs/RESEARCH-REPORT.md` (23 Sept), which covered features and craft. This report is about the performer's journey.*

---

## תקציר מנהלים (לבעלים)

1. **הבעיה היא לא הסצנות, אלא הדרך אליהן.** בפלאגין יש היום כ-29 נובים וכ-38 כפתורים על מסך אחד. אין בו הפרדה בין "להופיע", "לעצב" ו"להגדיר". כל כלי מצליח שבדקתי מפריד בין השלושה.
2. **אין לך מתחרה ישיר.** אף אחד לא משלב את כל אלה: פלאגין בתוך Live 10, מנוע גנרטיבי נפרד, ניתוח לפי תפקיד (קיק/סנר/האט) ועבודה על Iris Xe. ב-Windows, ‏Videosync 2 נתמך רשמית רק על Live 12. ‏Synesthesia ממליץ על כרטיס מסך ייעודי (GTX 1060) ומקבל רק מיקס סטריאו אחד.
3. **מה המתחרים לימדו:** ב-Synesthesia יש שלוש "מגירות" (סצנה / Meta / מדיה) עם Lock ו-Undo לכל אחת. ב-Resolume יש Dashboard שמרכז את מה שחשוב, ואפשר "לבחור" קליפ בלי "להפעיל" אותו. ב-Isadora וב-VDMX יש רשימת קיו עם GO.
4. **ימים: מסך PERFORM כברירת מחדל.** רשת סצנות עם תמונות קטנות, 8 נובים קבועים, וכפתורי GO / SAFE / BLACKOUT / CALM. כל השאר עובר לדפים DESIGN ו-SETUP.
5. **ימים: מצב "NEXT".** לחיצה על סצנה מכינה אותה בלבד. רואים מה יבוא ומתי (מהבהב עד הביט או התיבה, כמו ב-Ableton), ו-GO מפעיל.
6. **ימים: אנלייזרים משניים פשוטים.** אנלייזר על ערוץ קיק/סנר מקבל מסך "מקור" קטן. התפקיד נקבע אוטומטית לפי שם הערוץ, ורק אנלייזר אחד שולח מאקרו, בלי שתצטרך לזכור.
7. **ימים: בטיחות.** כפתור SAFE, ומגביל הבזקים אחד שכולל את כלל ההבזק האדום של WCAG. זה קריטי לאסתטיקה האדומה שלך. בנוסף, מנוע שזוכר את המסך ואת ה-fullscreen ומתחבר מחדש לבד.
8. **ימים: קישור ל-Ableton כבר היום.** תווי MIDI בערוץ שמור בוחרים קיו. זה עובד ב-Live 10 גם ב-Session וגם ב-Arrangement, בלי Max.
9. **שבועות: Cue Bank.** ‏32 מצבים עם שמות, GO ו-Back, בפרמטר אחד שאפשר לאוטומט. זה גם הבסיס למופע המחול.
10. **שבועות: פחות נובים.** ‏14 נובי LOOK מתקפלים לבורר "Look" עם שמות ונוב Amount אחד. הנובים המפורטים עוברים לדף DESIGN.
11. **שבועות: תצוגה מקדימה קטנה בתוך הפלאגין.** ‏Videosync 2.1 ו-Zwobot כבר עושים את זה. ובנוסף, שלט מהטלפון.
12. **הימורים גדולים:** Show Mode, כלומר רשימת קיו נעולה לטיימליין של Live עם שכבה חיה מעליה. "טייס משנה" שמציע את הקיו הבא לפי מבנה השיר. ודף "Mix Map" שהופך את הניתוח לפי תפקיד ליתרון גלוי.
13. **מה לא לעשות:**
    - AI בזמן אמת (דורש כרטיס NVIDIA עם 24GB).
    - מטריצת מודולציה פתוחה, כמו ב-Resolume או ב-VS: היא מקור לתלונות של "איבדתי שליטה".
    - עוד סצנות, לפני שהזרימה מסודרת.
14. **שלוש החלטות שמחכות לך** מופיעות בסוף המסמך (חלק 9).

---

## How to read this report

**Evidence tags** (on every non-obvious claim):
- **[D]** official documentation or manual
- **[M]** marketing / product page
- **[P]** press or editorial
- **[U]** real user report (forum thread)
- **[S]** search-result snippet only; the page was not opened, so treat it as a lead
- **[R]** read from our own repo
- **[I]** my inference

**Limits:**
- Reddit could not be fetched ("unable to fetch from www.reddit.com"), so user voice comes from vendor forums (Resolume, Showsync, Cycling '74, JUCE, Ableton) and editorial pieces.
- Several vendor pages returned 403/404: Resolume forum pages, Magic Music Visuals features, Serato support, Derivative pricing. The affected claims are tagged [S] or "unknown".
- Install-to-first-visual times for competitors are **estimates [I]** built from their documented steps. None were timed hands-on.
- Effort is in **dev-days of one engineer or agent session**, assuming the current code base.

---

# Part 1. Where we stand: the product through the performer's eyes

## 1.1 What exists (from the repo)

| Area | Current state | Source |
|---|---|---|
| Topology | One VJ Analyzer VST3 per track: roles MIX / KICK / SNARE / HAT / BASS / TEXTURE. It sends OSC to a separate VJ Engine process on UDP 9000. | [R] `USAGE.md` |
| Host parameters | About 56 (role, sensitivity, trim, normaliser, send, 8 macros, preset, blackout, 13 look, calm, morph, palette, 5 react, 6 move, 4 media, shots, hud, lookahead, 7 momentary actions) | [R] `plugin/Source/PluginProcessor.cpp` |
| Editor | Fixed 1180 × 838 px | [R] `PluginEditor.cpp:495` |
| Controls on one screen | About 29 rotary knobs, about 38 buttons and toggles, 4 dropdowns, 3 colour chips, plus the scene list | [R] `docs/product-design/current-ui.png` |
| Scene choice | Text list of 16 scenes. Selection waits for the next beat. No thumbnail, no "cued" indication. The Preset parameter runs 0–64, so one MIDI knob sweeps the list about 4 times. | [R] `PresetManager.cpp`, `docs/REVIEW-2026-09-24-HE.md` §5 |
| Recall | Plug-in state is saved with the Live set. The engine re-sends everything every second. Snapshots A–D morph over Cut … 16 bars. | [R] `AnalysisWorker.cpp:384`, `USAGE.md` |
| Engine | Launched by a button in the plug-in or by `Start VJ Session.bat`. It does **not** remember monitor, fullscreen or quality (no settings persistence found). | [R] `EngineLauncher.cpp`; grep of `engine/Source` for `PropertiesFile` / `ApplicationProperties` finds nothing |
| Multi-instance | `sendControls` ("Send macros") defaults to **on** in every instance. The docs tell the user to switch it off on all but one. | [R] `PluginProcessor.cpp:177`, `QUICKSTART-HE.md` |
| Installation | `Build and Install.bat` compiles everything. The user then changes Live's VST3 preference and rescans. | [R] `USAGE.md` |
| Show needs | A dance piece, *Before It Disappears*. Hybrid operation: a cue-locked Arrangement spine plus a live controller layer. It has a luminance budget, and `CONCEPT.md` asks for a Cue Bank and unified flash safety. | [R] `docs/before-it-disappears/CONCEPT.md` §6.2–6.4 |

## 1.2 Why the owner feels lost (diagnosis)

1. **There are no modes.**
   - Setup controls (Trim, Lookahead, Adaptive, role) sit next to show controls (HIT, BLACKOUT) and design controls (14 LOOK knobs).
   - The eye cannot find "what do I touch now".
   - Every successful tool separates these tiers:
     - Synesthesia has Scene / Meta / Media banks [D].
     - Resolume has a Dashboard you fill with what matters [D].
     - Live itself has Configure mode [D].
2. **Every instance shows the full UI.**
   - A KICK analyzer shows the whole instrument, dimmed.
   - The performer has to remember which instance is the master.
3. **Scene switching is blind.**
   - You click a name and something happens "on the next beat".
   - Nothing shows that it is waiting, or what it will look like.
   - Ableton users are trained by blinking launch buttons (a Max forum user: *"'is_triggered' indicates if the scene is blinking"* [U], https://cycling74.com/forums/problem-continually-observing-any-given-scenes-state-firedidle).
4. **Only 4 snapshots, and they are not a set list.**
   - A show with 9+ sections needs named cues. `CONCEPT.md` §6.2 reaches the same conclusion [R].
5. **Recall is incomplete.** After a reboot the engine opens windowed on the wrong monitor, with default quality and no scene until Live sends one [R].
6. **Fail-safe is only BLACKOUT.**
   - There is no "go to a safe look" and no single flash budget.
   - Flash and kick-cuts are counted separately (`CONCEPT.md` §6.4 cites `LookPass.cpp:455`, `PresetManager.cpp:376`) [R].

---

# Part 2. Competitive landscape

## 2.1 Map of the field

| Tier | What it is | Tools |
|---|---|---|
| **A. Inside the DAW, as a plug-in** (closest to us) | A VST3/AU on a track that makes visuals | **Photism**, **VS – Visual Synthesizer**, **Spettro**, VYSOR / Sphere / Orra (not verified) |
| **B. Inside Live, as Max for Live** | Visuals as Live devices; Live's UI is the UI | **Videosync 2.x**, **Zwobot**, EboSuite, T3X2R, Jitter/Vizzie patches |
| **C. External VJ apps fed by audio / MIDI / Link** | A separate app; Live sends audio or MIDI | **Synesthesia**, **Resolume Avenue/Arena**, VDMX6 (Mac), NestDrop, Magic Music Visuals, Arkestra (Mac), Visualz, Karleido (browser) |
| **D. Patching / creative coding** | Build your own instrument | TouchDesigner (+ TDAbleton, tox packs), Resolume Wire, TiXL (Tooll3), Hydra, cables.gl, vvvv, Max/Jitter |
| **E. Show control** | Cue lists for theatre and dance | Isadora, Millumin, VDMX Cue List, QLab (not opened) |
| **F. DJ-software video** | Visuals tied to DJ decks | VirtualDJ (2026 AI visuals), Serato Video |
| **G. AI real-time** | Diffusion video at runtime | Daydream Scope / StreamDiffusion, freebeat AI VJ, VirtualDJ AI shaders, Arkestra Studio AI |
| **Not a visual tool** | | **Envelop for Live** is Ambisonic spatial *audio* for Live 10–11+ (GPLv2) [D] https://github.com/EnvelopSound/EnvelopForLive. **Pixelynx**: search returned only a company listing tied to Web3/music, with no VJ product found [S]. Status: unknown. |

## 2.2 Comparison table

The "Fit" column rates suitability for **our** constraints: Live 10.1, Windows, Iris Xe, solo performer who is also the musician.

| Tool | Platform / format | Fit for Live 10 + Iris Xe | Install → first visual | How audio-reactivity is configured | Scene browsing and switching | Control / MIDI | Live integration | Output | Price | What users say |
|---|---|---|---|---|---|---|---|---|---|---|
| **VJ VST (ours)** | VST3 + separate engine, Windows | Native | About 9 steps incl. compiling and a Live prefs change; ~10–15 min first time [I] | Per-track role instances + global REACT TO gate + Impact / Softness / Reactivity [R] | Text list, beat-quantised, no cue display [R] | Everything is a host param; Live MIDI Map; no in-plugin learn [R] | Automation of all params; MIDI notes as hits; transport [R] | Window / fullscreen / Spout [R] | n/a | Owner: "lost inside the VST", too many knobs |
| **Videosync 2.1** (Showsync) | M4L devices + renderer; Mac / Win | **No**: needs Live 11.1+ / Max 8.3; Windows supported "for use with Live 12 only" [D] https://support.showsync.com/release-notes/videosync/2.0 | Install, open Live, drop device and clip [I] | Via Live's modulators and sidechains; no dedicated hit analyser (see previous report A1) | Live's clips **are** the scenes; Session / Arrangement / Follow Actions [M] https://www.showsync.com/videosync | Live's MIDI mapping, Push [S] | Deepest in the field: warping, arrangement, mixer = opacity [P] https://cdm.link/videosync-1-0-arrives-visuals-integrate-with-ableton-live-session-arrangement-warping/ | Spout / Syphon, multiple outputs, networked mode [M] | 1.0: €79 Intro / €199 [P]; current price unknown | Praise: "12 … screens … running for a month" (testimonial on the vendor page) [M]. Complaints: jerky at 50 Hz vs 60 fps target [U] https://forum.showsync.com/t/videosync-running-slow-in-ableton/2651; laptop battery drain, choppy tempo-automated clips [D] https://support.showsync.com/videosync/troubleshooting/common-pitfalls |
| **Photism** | VST3 / AU / M4L + browser renderer | Partial: VST3 runs in Live 10 [I]; the M4L device needs 11/12 (prior report); Spout on Windows is "beta" [M] | 4 steps: drop plug-in, open visuals, press play, (capture) [M] https://photism.app/ | Fixed: "full frequency spectrum drives every scene"; Sidechain companion in the paid version [M] | 20 scenes, "switch mid-track" [M] | 16 macros, "automate in Live" [M] https://photism.app/lite/ | Automation lanes [M] | Fullscreen, Syphon / Spout (beta) [M] | $19; Lite free [M] | No independent reviews found |
| **VS – Visual Synthesizer** (Imaginando) | VST3 / AU / standalone, all OSes | Runs as a plug-in [M]; GPU demand unknown | Load as instrument or FX [I] | **Mod matrix**: audio bands / peaks / envelopes, 4 LFOs, MIDI envelopes [M] https://www.imaginando.pt/products/vs-visual-synthesizer | Playlists that "advance on their own" [M] | MIDI learn on everything [M] | Plug-in host automation [M] | NDI / Spout / Syphon 4K60 [M] | €129 or €9.90 / month [M] | KVR: 0 reviews [S] |
| **Spettro** | VST3 Windows | Yes (Windows 11 VST3) [M] | Load shader and play [I] | Level / bass / mid / high / beat into shaders [M] https://www.kvraudio.com/product/spettro-by-creature-from-the-black | "up to 2008 MIDI-selectable shaders" [M] | MIDI note selects shader [M] | Plug-in | Spout, NDI [M] | Free (donation) [M] | 0 reviews on KVR [M] |
| **Zwobot** | M4L, Live Suite | Partial: page says Max 7.2.5 minimum; prior report found Live 11/12 required [M] https://www.zwobotmax.com/ | Device on a track [I] | Sound-reactive modules on the host track; global hi / lo ranges (prior report) | Two decks + crossfader [M] | MIDI | Beat clock up to 1/16; **built-in monitor module** [M] | Window / Spout (prior report) | Previously $39 / $69; current unknown | Vendor admits Live audio takes priority, so frames can stick (prior report) |
| **EboSuite** | Live plug-ins (Mac per prior brief) | Unknown | Unknown | Unknown | Session-view video [P] via competitor page https://photism.app/learn/best-ableton-visualizers/ | Unknown | Session / Arrangement | Unknown | Unknown [D] https://www.ebosuite.com/ | — |
| **Max for Live / Jitter / Vizzie** | Max patches in Live | Yes (Max 8 exists on this machine) | Patch it yourself | Anything | Anything | Anything | Full LOM | Window | Included in Suite | Jitter runs in Live's interface thread: "frame updates will be interrupted by the Live application's interface redraws" [D] https://docs.cycling74.com/legacy/max8/vignettes/jitter_and_Max_for_Live |
| **Synesthesia** | App, Mac / Win | **Weak**: "dedicated graphics card … GTX 1060 is a good baseline" [D] https://synesthesia.live/docs/faq/ | Download, run; no sign-up for trial [M] https://synesthesia.live/pricing | Automatic: "you don't have to worry about carefully setting audio levels"; single input, "combine all channels … into one mix" [D] https://synesthesia.live/docs/faq/audio.html | Library with S/M/L thumbnails, tags, stars / colours "by mood, genre, gig", search; playlists with durations and Auto-Transition [D] https://app.synesthesia.live/docs/manual/library.html , https://app.synesthesia.live/docs/manual/presets.html | Global position-based mapping (SLIDER_1, TOGGLE_1…) + per-scene override; LED feedback [D] https://app.synesthesia.live/docs/faq/midi_osc.html | MIDI from Live; no plug-in | Syphon / Spout / NDI **Pro only** [M] | $199 Standard / $399 Pro, perpetual [M] | Search snippets mention learning curve and GPU cost [S]; no forum evidence reachable |
| **Resolume Avenue / Arena** | App, Mac / Win | Runs on Windows; content is clips, not generative scenes | ~5 steps with a bundled example composition [D] https://resolume.com/support/en/quickstart-tutorial | **Per-parameter** FFT: L / M / H, Gain, Fall [D] https://resolume.com/support/en/parameter-animation | Clip grid with thumbnails; *select* (name handle) vs *trigger* (thumbnail); Beat Snap to beat / bar / N bars [D] https://resolume.com/support/en/clips | MIDI / OSC mapping; Dashboard dials fed by drag and drop, with in / out range and invert [D] https://resolume.com/support/en/dashboard | Link; MIDI from Live via loopback [S] | NDI / Syphon / Spout, capture cards [M] https://resolume.com/software/avenue-arena | €299 / €799 [M] | Low-volume FFT jitter; "lose control" with audio-reactive effects (prior report); no autosave [S] |
| **Resolume Wire** | Node patcher for Arena / Avenue | n/a | — | — | Builds sources / effects that appear in Arena [M] https://resolume.com/software/wire | MIDI / OSC | — | — | €399 [M] (lime list says "included", a conflict) | — |
| **VDMX6** | Mac only | No (Mac) | Build your own UI from plug-ins [P] https://cdm.link/vdmx6/ | Audio analysis plug-in | Media bins; **Cue List** timed by index / beats / measures / seconds / SMPTE [D] https://vdmx.vidvox.net/tutorials/introduction-to-the-cue-list-plugin | MIDI / OSC / DMX | Timecode, clock | Syphon | $99 / $249 Plus (CDM) vs $199 / $349 (lime list, 2026) | "Open-ended … semi-modular UI" [P] |
| **TouchDesigner** (+ TDAbleton, tox packs) | App, Mac / Win | Iris Xe will struggle with heavy networks [I] | Hours to weeks [I]; "steepest climb" [P] https://limeartgroup.com/the-mega-list-of-vj-software-and-tools/ | audioAnalysis palette component (prior report) | You build it | Anything | **TDAbleton**: song position, triggered scenes, playing / fired slots, per-track levels, device params; Live 9–10 via TDAbleton 1.x [D] https://docs.derivative.ca/TDAbleton | Anything | Free non-commercial; paid tiers [P] | Community sells tox packs (e.g. Gumroad audio-reactive projects) [S] |
| **NestDrop** (MilkDrop) | Windows | Yes (Windows, GPU-light presets) [I] | Run, pick audio input [I] | Beat detection; "auto-change presets based on beat detection" [M] https://nestimmersion.ca/nestdrop.php | "Live preview and static thumbnails", **queue windows**, stars in 5 colours, search [M] | Multi-controller MIDI (Midnight) [M] | Link (Pro) [M] | Spout, fullscreen, NDI (Pro) [M] | Free / $50 / $75 [M] | — |
| **Magic Music Visuals** | App, Mac / Win | Probably OK [I] | Node patching, "moderate setup effort" (competitor rating) [P] https://photism.app/software/magic-music-visuals/ | Per-node; many audio inputs [S] | Scenes / playlists [S] | MIDI / OSC [S] | Audio via VB-Cable [S] | Spout / Syphon [S] | $44.95 [S] | "ceiling … limited" (competitor opinion) [P] |
| **KaleidoscopeEnhanced** | Win / Linux, MIT | Needs OpenGL 4.3 [D] | — | Beat / onset, key, mood, **song-structure tracking** [D] https://github.com/reneweller-coding/KaleidoscopeEnhanced | 866 scenes, **auto cross-fades by mood** [D] | — | — | — | Free (MIT; some shaders CC BY-NC-SA) [D] | Hobby project; a proof of auto-VJ ideas |
| **Arkestra 3** | Mac only | No | Free tier [M] https://www.arkestra.app/ | Audio-reactivity, vision tracking (prior report) | Scenes + snapshots, timeline [M] | MIDI / OSC / Link [M] | Link | Syphon (Pro) | Free; $199 or $19 / mo × 12 [M] | User: cheaper than Synesthesia [M] (testimonial) |
| **Visualz** (v3, 2026) | Mac / Win | Unknown | Free to start [M] https://visualzstudio.com/ | "band-isolated modulators that bind to any param" [M] | Clip → Layer → Track → Set [M] | MIDI; own hardware deck CVJ-1 [M] | — | Spout / Syphon / NDI / RTMP [M] | Free + PRO [M] | — |
| **Karleido** | Browser | Works anywhere [I] | "plug in, connect your audio, and you are ready" [M] https://karleido.com/ | Beat markers [M] | **Timeline with sections** (studio) + performance mode [M] | MIDI mapping [M] | — | Syphon / Spout [M] | Unknown | — |
| **Isadora** | Mac / Win | Yes (show control) | Patching | Anything | **Scene list; spacebar = next; bumps vs crossfades; Blind Mode** [D] https://troikatronix.com/isadora/cueing-show-control/ | MIDI / OSC / DMX | MIDI from Live [S] | Many | Unknown | Widely used in dance and theatre [S] |
| **Millumin 5** | Mac only | No | — | — | Timelines, cue lists [P] (lime list) | — | — | — | €29 / week to €399 lifetime [P] | Rental suits single show runs [P] |
| **TiXL (Tooll3)** | Windows, open source | Unknown on Iris Xe | Node graph + timeline [P] (lime list) | Audio-reactive VJ content [S] | — | MIDI / OSC / Spout [S] | — | Spout | Free | — |
| **Hydra / cables.gl** | Browser | Runs anywhere | Live coding / patching [P] | Code | Code | — | — | Browser | Free | Not a performer's UI [I] |
| **VirtualDJ (2026)** | App | DJ context | — | AI shaders from text prompts, AILoops "beat-synced" [P] https://www.digitaldjtips.com/virtualdj-2026-brings-fluid-beatgridding-ai-video-visuals/ | Auto-Change of visuals [S] | DJ controllers | — | Video out | One-off or subscription [P] | Editor: "How many DJs actually want AI-generated visuals…" [P] |
| **Serato Video** | Serato plug-in | DJ context | — | **Visual FX linked to audio FX** [S] | — | DJ controller | — | Syphon [S] | "No longer available" per a retailer [S] | — |
| **Daydream Scope** | Python app | **No**: 24 GB NVIDIA recommended [S]; licence CC BY-NC-SA [D] https://github.com/daydreamlive/scope | — | Text / webcam / video input [D] | — | — | — | Spout / NDI [D] | Free, non-commercial | — |
| **freebeat AI VJ** | Browser | — | — | "3–5 second audio-to-visual latency" [P] (lime list) | — | — | — | — | Credits | Latency disqualifies live hits [I] |
| **Notch, Lumen, Modul8, CoGe** | Windows (Notch) / Mac (others) | Notch too heavy; the others are Mac-only | — | — | — | — | — | — | Notch by subscription | Only in the lime list [P]; not examined further |

## 2.3 Per-tool workflow notes (what to copy, what to avoid)

### Videosync 2.1: the "Live is the UI" school
- **Copy.** Its core idea is that Live's clips, scenes, automation and mixer *are* the visual controls ("treat video as audio" [M] https://www.showsync.com/videosync). For a musician who runs both sound and picture from one set, the most familiar gesture is launching a clip or scene. 2.1 added a **Video Monitor** "an essential from VJ software" and a recorder [P] https://cdm.link/videosync-2-1-for-ableton-live/.
- **Avoid.** It is tied to Live 11.1+ / 12. The developer declined to autostart the renderer from M4L "to avoid launching external processes" [U] https://forum.showsync.com/t/videosync-ideas-a-place-of-organization/796.
  - Our plug-in already launches its engine, which is a small but real advantage [R].
  - Showsync's own troubleshooting blames laptop battery mode and refresh-rate mismatches for low fps [D].
  - That is the same class of problem our owner hit, running on battery (REVIEW-2026-09-24 §1).
- **Lesson.** Session/Arrangement linking is the strongest workflow a DAW-native tool has. We cannot have clips *be* video on Live 10, but we can have clips *select cues* (see R8, M5).

### Photism: the "drop it like an EQ" school
- **Copy.**
  - The four-step onboarding [M].
  - A **separate companion device** for sidechain sources, which is the same pattern as our secondary role instances but framed as a different, smaller device [M] https://photism.app/lite/.
  - 16 automatable macros [M].
- **Avoid.**
  - A fixed "spectrum drives every scene" [M]. That gives no performer control over what reacts.
  - Beta Spout on Windows [M].
- **Lesson.** Being small and simple is the product here. The architecture (plug-in → local renderer) is the same as ours [M].

### VS – Visual Synthesizer: the mod-matrix school
- **Copy.** Playlists that advance on their own. Polyphonic "voices" for MIDI [M].
- **Avoid.** An open modulation matrix where "every routing is user-built" (prior report, A3). This is the exact "everything reacts to everything" trap. Our control model rules (one audio source per parameter) are a better design [R].

### Synesthesia: the best-structured *performer* UI in class C
Mechanisms worth stealing, all from docs [D]:
1. **Three banks: Scene / Meta / Media**, each with *Lock, Default, Random, Undo* (https://app.synesthesia.live/docs/manual/controls.html).
   - **Lock** means a preset load will not touch that bank.
   - **Undo** is a live safety net.
2. **Master brightness is "notably not stored in presets"**, so a preset can never blow out the room. That is a pattern for our SAFE / ceiling.
3. **Preset channels:** loading a preset can affect Scene, Meta and/or Media independently (https://app.synesthesia.live/docs/manual/presets.html). This maps directly to "recall only LOOK" for our snapshots.
4. **FavSlots:** a short, MIDI-mappable shortlist of presets for the gig, separate from the full library.
5. **Playlists:** scene + preset items with durations, **Auto-Transition** and shuffle.
6. **Global MIDI mapping by position** (SLIDER_1, TOGGLE_1 …) applies to every scene, with per-scene overrides and LED feedback (https://app.synesthesia.live/docs/faq/midi_osc.html). One controller layout serves all scenes.
7. **Library** with thumbnail sizes, tags, stars and colours "by mood, genre, gig" (https://app.synesthesia.live/docs/manual/library.html).
8. **Transition modes:** Crossfade or **Fade to Black**, which is GPU-cheap because only one scene runs (https://synesthesia.live/docs/faq/). This matters on Iris Xe.
9. **Audio "Presence", "Hits", "Time", `syn_FadeInOut`, `syn_Intensity`** (https://app.synesthesia.live/docs/ssf/audio_uniforms.html). These are named signals, not raw FFT.

**Weaknesses for us:**
- One stereo input only [D].
- A dedicated GPU is recommended [D].
- Spout / NDI cost the $399 Pro tier [M].

### Resolume Avenue / Arena: the clip-grid school
1. **Select ≠ trigger.** Clicking the name handle selects a clip for editing and preview without sending it to output (https://resolume.com/support/en/clips) [D]. That is our missing "NEXT".
2. **Beat Snap** per clip or composition: next beat, bar, 2 or 4 bars [D]. We have this in the engine, but it is invisible.
3. **Autopilot:** next / previous / random ("Any, Other and Bag"), timed by clip end, beats or seconds, with a clear priority rule: "if you trigger a column or clip, then that's what happens" [D] https://resolume.com/support/en/autopilot. **Manual always beats auto.** Adopt that rule verbatim.
4. **Dashboard:** drag any parameter onto a dial, with in / out range, invert and several parameters per dial [D] https://resolume.com/support/en/dashboard. This is the pattern for "8 performance knobs, each a curated macro".
5. **Per-parameter FFT** with Gain and Fall [D] https://resolume.com/support/en/parameter-animation. It is powerful, but the prior report documents jitter and "lose control" complaints. Keep audio conditioning at the source, as we already do.
6. Autosave is absent according to forum snippets [S]. That is a recall gap; our "state lives in the Live set" is stronger [R].

### VDMX6 and Isadora: the cue-list school
- The VDMX **Cue List** holds data and clip-trigger cues on index / beats / measures / seconds / SMPTE time, with per-cue lock, enable and jump-by-name [D] https://vdmx.vidvox.net/tutorials/introduction-to-the-cue-list-plugin.
- **Isadora** covers dance and theatre [D] https://troikatronix.com/isadora/cueing-show-control/:
  - a linear scene list
  - **spacebar = next scene**
  - **bumps** (zero-second) vs **crossfades** with separate in / out times
  - **Blind Mode**: "edit inactive Scenes while your active Scenes are still outputting"
  - background scenes at partial intensity
- **Lesson for the dance show.** We need GO / Back, named cues, per-cue transition and blind editing. That is `CONCEPT.md`'s Cue Bank plus two show-control conventions.

### NestDrop: the preset-deck school on Windows
- It shows "live preview and static thumbnails", has multiple **queue windows**, 5-colour stars and search, and auto-changes on beat [M] https://nestimmersion.ca/nestdrop.php.
- **Lesson.** Thumbnails plus a queue ("what's next") is the standard, even in a free tool.

### TouchDesigner + TDAbleton: proof that Live 10 can drive visuals structurally
TDAbleton exposes:
- song position
- **triggered and last-started scenes**
- **playing / fired slot index per track**
- per-track levels
- device parameters

It still supports Live 9–10 through TDAbleton 1.x [D] https://docs.derivative.ca/TDAbleton. Its pitfalls are duplicate track names and meters that update only when visible [D].
- **Lesson.** A Remote-Script or M4L companion can give us Session linking on Live 10. The Max-forum advice is to observe `playing_slot_index` on a track rather than scene `is_triggered`, which "indicates if the scene is blinking but not if it's actually playing" [U] https://cycling74.com/forums/problem-continually-observing-any-given-scenes-state-firedidle. Also, IDs cannot be set from notifications, so use `deferlow` [U] https://cycling74.com/forums/observing-selected-scene.

### Auto-VJ precedents
- **Resolume Autopilot**: timed, random "bag" [D].
- **NestDrop** changes on beat [M].
- **VS / Synesthesia** playlists auto-advance [M]/[D].
- **KaleidoscopeEnhanced** is the only one found that ties choice to **song structure and mood tags** [D] https://github.com/reneweller-coding/KaleidoscopeEnhanced.
- **VirtualDJ** automix detects song structure for audio [S].
- **Nobody** found proposes the *next* cue for a human to confirm. That is an opening (B2).

### AI (2024–2026)
- Real-time diffusion (Daydream Scope) needs ~24 GB NVIDIA [S] and is non-commercial [D].
- Browser AI VJ has 3–5 s latency [P].
- VirtualDJ ships prompt-to-shader visuals [P]. Arkestra ships prompt-to-ISF generation [M].
- **Verdict.** Irrelevant at runtime on Iris Xe. At most, use offline prompt-to-shader as a *design-time* aid. It does not fit a performer's workflow problem.

---

# Part 3. Workflow analysis by theme

For each theme: how the field does it, where we are, and the verdict.

## 3.1 One instance per track vs. one master device

| Approach | Who | Pros | Cons |
|---|---|---|---|
| Single master input | Synesthesia (one stereo input [D]), Resolume composition FFT [D], Photism basic [M] | Zero routing | Drums have to be guessed from the mix; no instrument roles |
| Master + companion "sidechain" devices | Photism Sidechain [M], Videosync sidechain effects (prior report) | Extra sources only when needed | Companion is a separate product concept |
| Per-parameter source choice | Resolume clip / layer / group FFT [D], VS matrix [M] | Flexible | Jitter and "lost control" complaints (prior report) |
| **Per-track role instances** | **Us** [R], TDAbleton per-track levels [D] | Real kick / snare separation, MIDI-as-hits, low latency | Every instance looks like the whole instrument; "Send macros" must be managed by hand [R] |

**Verdict:** keep per-track roles. They are a genuine differentiator nobody else offers in-DAW. Change the *presentation*:
1. **One "Main" instance and N "Source" instances.**
   - A Source instance shows only: role, input meter, hit LED, Hit Sens, Trim, and "connected to Main ✓".
   - This is the Photism companion pattern, but in one binary.
2. **Automatic leader election.** The engine grants "Main" to the first MIX instance that says hello. Others auto-demote, so `sendControls` stops being a user decision.
3. **Auto-role from track name.** JUCE's `updateTrackProperties` receives the track name in Live 10 VST3, with a quirk: a name change arrives with a blank colour and vice versa [U] https://forum.juce.com/t/audioprocessor-updatetrackproperties-ableton-10-vst3-not-working-as-expected/43049. Map "kick | bd | 808" → KICK and so on, and keep manual override.

## 3.2 Scene browsing

| Pattern | Who | Our state |
|---|---|---|
| Thumbnails (static) | Synesthesia (S/M/L) [D], NestDrop [M], Resolume clip thumbnails [D] | None [R] |
| Live preview of a candidate | NestDrop live preview [M], Resolume preview via select [D], Synesthesia marketplace preview with your audio [D] | None |
| Tags / stars / colours for the gig | Synesthesia "mood, genre, gig" [D], NestDrop 5-colour stars [M] | None |
| Gig shortlist | Synesthesia FavSlots [D] | Snapshots A–D only |
| Queue ("what's next") | NestDrop queue windows [M], Isadora scene list [D] | None |

**Verdict.**
- Static thumbnails are nearly free for us. `engine/preset_regression_test.py --keep <dir>` already renders a snapshot of every preset [R] (`USAGE.md`, *Tests and tools*). Ship those PNGs as the scene grid.
- A 16-scene library fits in a 4×4 grid. No search is needed yet.

## 3.3 Quantised switching and "cued" state

| Pattern | Who |
|---|---|
| Blinking = armed, solid = playing | Ableton clip / scene launch (forum description of `is_triggered` as "blinking") [U] |
| Beat Snap per clip, with a global default | Resolume [D] |
| GO key for the next queued item | Isadora spacebar [D] |
| Bump vs crossfade per cue | Isadora [D], Synesthesia transition modes [D] |

**Ours:** the engine has `pendingIndex`, prebuilds the next scene and switches on the beat [R] `PresetManager.cpp:288–317`. `/v2/status` does not report a pending scene or a countdown, so the plug-in cannot display it [R] `USAGE.md` (status fields).

**Verdict (quick win):**
1. Add `pending`, `beatsToGo` and `quantum` to `/v2/status`.
2. Show the pending tile blinking with a count-in ring.
3. Make the quantum visible and switchable in one place: *Cut · Beat · Bar · 4 Bars*.
4. Add an explicit **GO NOW** modifier for when the performer wants the cut off-grid.

## 3.4 Macro design: how many knobs, and what they mean

| Tool | Knobs | Meaning |
|---|---|---|
| Synesthesia | Scene controls defined per scene; Meta controls global [D] | Mixed: per-scene labels + a fixed global bank |
| Photism | 16 macros [M] | Per-scene meaning unknown |
| Resolume | Dashboard dials filled by the user [D] | User-defined |
| Live racks | 8 macros (Live 10) [D] https://www.ableton.com/en/manual/instrument-drum-and-effect-racks/ | User-defined; Macro Variations arrived only in **Live 11** [D] https://www.ableton.com/en/release-notes/live-11/ |
| Ours | 8 macros with *fixed direction, per-scene label* + 14 LOOK + 4 MOVE + 3 REACT + 3 SIGNAL [R] | Good rules, too many visible knobs |

**Verdict.**
- The control *rules* designed on 24 Sept (SHAPE / MOVE / REACT / LOOK, one rule each) are sound. The problem is **surface area**.
- Adopt a strict **8 + 4 + 4** performance surface:
  - **8 knobs:** Intensity, Speed, Form, Scale, Erode, Detail, Impact, **Look Amount**
  - **4 toggles:** Freeze, Calm, Reverse, Sync
  - **4 actions:** GO, HIT, SAFE, BLACKOUT
- Everything else moves to DESIGN or SETUP pages and into *named looks* (see R10).
- This matches Live's 8-knob banks and Push's 8 encoders. Live shows plug-in parameters in the order set in Configure mode, and that order can be saved inside a Rack [D] https://www.ableton.com/en/manual/working-with-instruments-and-effects/.
- Keep the total host parameter count **≤ 64**. Live opens plug-ins with more than 64 parameters "with an empty panel" [D] (same page). We are at about 56 [R], and new cue / show parameters must not overflow.

## 3.5 Audio→visual mapping UI

| Pattern | Who | Verdict for us |
|---|---|---|
| Per-parameter FFT (source, band, gain, fall) | Resolume [D] | Do **not** copy: it is the jitter source |
| Mod matrix | VS [M] | Do **not** copy for performers; maybe in a hidden "scene author" tool |
| Named semantic signals (presence, hits, time, intensity) | Synesthesia [D] | We have these (`build`, `presence`, band activity) [R]; name them in the UI |
| Global gates | Our REACT TO [R]; Synesthesia Meta "Audio Control" reactivity (Pro) [D] | Keep; this is the right layer for performers |
| Activity visualisation | Our knob modulation rings [R]; Beam's "see exactly what Beam is sending" (prior report) | Keep; extend to a Mix Map (B3) |

**Verdict:** our model (condition at the source, route in the scene, gate globally) is ahead of the field (prior report). Do not add a mapping UI for the performer. Instead offer **React profiles**, one selector: *Still · Breath · Pulse · Cut*. Each is a preset of Impact / Softness / Reactivity / Push / SHOTS. This removes 5 knobs from the perform view.

## 3.6 Ableton integration

| Capability | Field | Ours | Opportunity |
|---|---|---|---|
| Automation of every control | Photism, Videosync, VS [M] | Yes [R] | Make cue index the *one* thing to automate |
| Clips / scenes as visual triggers | Videosync (clips are video) [M]; Resolume / Isadora via MIDI [S] | MIDI notes only as hits [R] | **Reserved-channel MIDI notes select cues** (quick win); M4L companion for clip / scene names (medium) |
| Session scene ↔ visual scene | TDAbleton exposes triggered scenes (Live 9–10 via 1.x) [D] | None | "VJ Link" M4L: observe `playing_slot_index` of a "VJ" track [U] |
| Scene metadata in names | Live 10 used scene names for tempo; Live 11 moved it to controls [D] https://www.ableton.com/en/manual/session-view/ | — | Tag scene or clip names with `[vj 7]` and read them via M4L |
| Push / controller banks | Push displays plug-in parameter names (Live 9.6 notes [D]); Configure mode reorders the panel [D] | Unordered | Order the first 8 parameters; ship a Rack preset |
| Takeover | Live's Pick-Up / Value Scaling [D] https://www.ableton.com/en/manual/midi-and-key-remote-control/ | Docs recommend Value Scaling [R] | Also send soft-takeover hints to our own MIDI learn |
| Tempo / phase | Link, host transport | Host transport [R] | Already better than Link-only tools ("Link … doesn't trigger visual events" [P] https://www.arkestra.app/articles/sync-vj-software-ableton-live) |

## 3.7 Set management and recall

| Step | Field | Ours |
|---|---|---|
| State saved with project | Videosync (Live set), Photism (automation) [M] | Yes: plug-in state + media paths [R] |
| Startup scene | Synesthesia "Default Startup Scene" [D] https://app.synesthesia.live/docs/faq/settings.html | No |
| Autosave / crash recovery | Resolume: none (forum snippets) [S] | The Live set holds the state; the engine is stateless and re-sent every second [R] (**good**) |
| Output placement remembered | Common expectation [I] | **No**: the engine forgets monitor and fullscreen [R] |
| Per-gig shortlist | Synesthesia FavSlots [D] | Snapshots A–D |

## 3.8 Fail-safes on stage

| Fail-safe | Field | Ours | Gap |
|---|---|---|---|
| Blackout | Everyone | Yes [R] | — |
| Master ceiling outside presets | Synesthesia Master "not stored in presets" [D] | No | Add a **ceiling** that no cue can exceed (also the dance show's luminance budget) |
| Safe scene / panic | Isadora bump to any scene [D] | No | **SAFE**: fade (1 bar) to a designated safe cue; digital LOOK to 0; CALM on |
| Flash limiter | WCAG: ≤3 flashes/s, with a separate **red-flash** threshold for saturated red [D] https://www.w3.org/WAI/WCAG21/Understanding/three-flashes-or-below-threshold.html | Flash / cuts capped at 3/s each, not unified [R] | Unify; add a red-flash rule. Our palettes are red-mono. |
| Engine crash | Videosync troubles are in-process with Live [D]; Jitter is in Live's UI thread [D] | Separate process + state re-send [R] (**strong**) | Auto-relaunch and "ENGINE LOST" banner |
| Hardware backup | "better to have something up … than nothing": a spare player with pre-recorded mixes [P] https://vdmx.vidvox.net/tutorials/vj-travel-kit-whats-in-our-bag | None | Record a safety render of each show section (a recorder is on others' lists; see prior report) |
| Undo a mistake | Synesthesia per-bank Undo [D] | No | One-level **Undo last recall** |

## 3.9 Install → first sound-to-image

| Tool | Documented path |
|---|---|
| Photism | 4 steps [M] |
| Resolume | 5 steps, bundled example composition [D] |
| Synesthesia | Download and run, no sign-up [M] |
| Karleido | "plug in, connect your audio" [M] |
| **Ours** | 1) run `Build and Install.bat` (compiles) → 2) Live Preferences, VST3 folders on → 3) Rescan → 4) `Start VJ Session.bat` → 5) drag the plug-in to Master → 6) check role = MIX → 7) press play → 8) move the engine window to the projector → 9) press F → 10) choose a scene [R] |

The target is **3 steps**: install → drag onto Master → press play. The engine opens itself on the last-used output, and a default scene is already running (the demo groove when the transport is stopped).

## 3.10 Monitoring and preview

- A **preview monitor inside the DAW** is standard in class B:
  - Videosync 2.1 Video Monitor [P]
  - Zwobot "built-in monitor module" [M]
  - Resolume preview monitor [D]
- **Ours:** the performer must look at the projector or a second window.
- **Verdict.** Add a small program thumbnail (~5–10 fps) plus a NEXT thumbnail (static) in the perform page. This matters most for the owner's rear-projection show, where he may not see the screen from the operator position [R] (`show-before-it-disappears` notes: rear projection behind dancers).

---

# Part 4. Journey map: current vs. proposed

Friction is scored 1 (smooth) to 5 (blocking) [I].

| Stage | Goal | Current path [R] | Friction | Proposed path | Proven by |
|---|---|---|---|---|---|
| **1. Install** | Plug-in visible in Live | Compile via .bat; change a Live preference; rescan | 4 | Signed installer copies the VST3 + engine and writes the engine path; one-page "Enable VST3 folder" check shown inside the plug-in if missing | Photism, Synesthesia one-step installs [M] |
| **2. First sound → image** | See something react within 1 minute | Start the engine (.bat or button); drag to Master; choose MIX; press play; find the engine window | 3 | Dropping the plug-in auto-launches the engine on the last or second monitor, fullscreen; the default scene is already running; the first instance becomes Main automatically | Photism 4 steps [M]; Videosync declined autostart (gap) [U] |
| **3. Add precision** | Tight kicks and snares | Add instances, set roles, remember "Send macros" off on all but one | 3 | Drop on the kick track and it becomes KICK (from the track name) in Source mode; Main shows "KICK ✓ connected" | JUCE track name in Live 10 [U]; Photism companion [M] |
| **4. Explore scenes** | Find the right look | Text list; each click cuts live after a beat | 3 | Thumbnail grid; click = **cue** (NEXT, blinking); preview thumbnail; GO or auto on the quantum | Resolume select vs trigger [D]; NestDrop thumbnails / queue [M] |
| **5. Shape a look** | Tune a scene | 29 knobs visible together; LOOK is 14 knobs | 4 | DESIGN page: SHAPE / MOVE / REACT / LOOK with one rule each (the existing grouping); named Looks + React profiles; "Store as cue" | Synesthesia banks + Lock / Default / Undo [D] |
| **6. Build the set** | One state per song section | 4 snapshots; scene index + macros automated by hand | 4 | **Cue Bank** (32 named cues: scene + shape + look + palette + react + transition); cue list with notes; recall channels (e.g. "LOOK only") | VDMX Cue List [D]; Synesthesia playlists / channels [D]; CONCEPT §6.2 [R] |
| **7. Link to music** | Visuals follow the arrangement | Automate the preset parameter (0–64 range, hard to read) | 3 | Reserved MIDI channel: note n = cue n (Session clips or Arrangement); later a VJ Link M4L that reads clip / scene names | Isadora / Resolume users drive visuals with MIDI from Live [S]; TDAbleton scenes [D] |
| **8. Rehearse** | Adjust cue timing quickly | Re-automate lanes by hand | 3 | Press GO while Live records automation → the `cueIndex` lane is written; Blind edit of cues not on screen | Live records plug-in parameter changes [D]; Isadora Blind Mode [D] |
| **9. Live show** | Few, big, safe gestures | Hunting among 70 controls; no "next" | 4 | PERFORM page: grid + NEXT + 8 knobs + 4 toggles + GO / HIT / SAFE / BLACKOUT; optional phone remote | Isadora spacebar GO [D]; TouchOSC / Open Stage Control [D] |
| **10. Something goes wrong** | Recover in seconds | BLACKOUT; restart the engine by hand; flashes counted separately | 4 | SAFE (fade to the safe cue), unified flash + red-flash limiter, brightness ceiling outside cues, ENGINE LOST banner + auto-relaunch + full re-send, Undo last recall | Synesthesia Master outside presets [D]; WCAG [D]; our re-send [R] |
| **11. Recall after reboot** | Identical show state | Plug-in state OK; engine forgets output, fullscreen and quality | 3 | Engine remembers output monitor, fullscreen, quality and last cue; startup cue = cue 1 or "safe" | Synesthesia Default Startup Scene [D] |

---

# Part 5. Opportunities: what we could do that nobody does well

| Idea | Does anyone do it well? (evidence) | Our unfair advantage | Value | Effort | Verdict |
|---|---|---|---|---|---|
| **Session scene / clip ↔ visual cue linking on Live 10** | Videosync does it via clips, but needs Live 11.1+ / 12 [D]. TDAbleton exposes scenes (Live 9–10 via 1.x) but you build the visuals yourself [D]. Resolume users hand-roll MIDI loopback [S]. | Our analyzer already receives MIDI in Live 10 [R]; Max 8 is installed | 5 | S (MIDI notes) → M (M4L names) | **Do** (R8 now, M5 later) |
| **Auto-VJ that follows song structure** | Resolume Autopilot is time / random [D]; KaleidoscopeEnhanced uses mood and structure but is not a performer tool [D] | `build`, `presence`, `energyTrend` already computed [R]; host bar position | 4 | L | **Do as "co-pilot"**: it suggests NEXT and the human confirms (B2) |
| **Per-track role instances as a visible feature** | No in-DAW tool exposes instrument roles (Synesthesia: single input [D]) | Already built [R] | 4 | M | **Do**: the Mix Map (B3) turns hidden plumbing into a selling point |
| **Show mode for choreographed pieces** | Isadora, Millumin, VDMX cue lists exist [D]/[P], but none is inside the DAW with audio analysis | Host transport + cue bank + analysis in one | 5 (for the dance show) | L | **Do** (B1), staged after the Cue Bank |
| **Phone / tablet remote** | Generic tools (TouchOSC, Open Stage Control) [D]; Resolume layouts [S] | Engine already speaks OSC [R] | 3 | M | **Do cheaply**: the engine serves one web page (M6) |
| **Preview monitor in the plug-in** | Videosync 2.1, Zwobot [P]/[M] | Engine is separate, so a low-res readback is cheap [I] | 4 | M | **Do** (M3) |
| **Real-time AI visuals** | Daydream needs a 24 GB GPU [S]; freebeat has 3–5 s latency [P] | None on Iris Xe | 1 | L | **Don't** |
| **Clip-as-video (Videosync clone)** | Videosync owns it [M] | None on Live 10 | 2 | L | **Don't** |
| **Visual FX linked to audio FX** (Serato idea [S]) | Serato Video, now unavailable [S] | We could map a Live effect parameter via M4L to a LOOK amount [I] | 2 | M | Later / maybe |
| **Luminance / flash budget as a product feature** | Nobody advertises it; Photism only warns [M] (prior report) | Dance show needs it [R] | 4 | S | **Do** (R5) |

---

# Part 6. Recommendations (prioritised)

Format: **Problem → Solution → Proven by → Effort → Risk.** Effort is in dev-days (one engineer or agent session).

## 6A. Quick wins (days)

**R1. Perform-first layout: three pages plus a "Source" mini-UI.**
- **Problem.** About 70 controls on one screen; the owner is "lost" [R].
- **Solution.**
  - **PERFORM** (default page):
    - scene / cue grid
    - NEXT tile
    - 8 knobs (Intensity, Speed, Form, Scale, Erode, Detail, Impact, Look Amount)
    - 4 toggles (Freeze, Calm, Reverse, Sync)
    - GO / HIT / SAFE / BLACKOUT
    - engine status
  - **DESIGN:** today's SHAPE / MOVE / REACT / LOOK / palette / media panels, unchanged.
  - **SETUP:** role, Hit Sens, Trim, Adaptive / Locked, Lookahead, REACT TO, engine path, output.
  - Non-Main instances show only the **Source** view.
  - **Keep all parameter IDs**, so automation and mappings survive.
- **Proven by.** Synesthesia Scene / Meta / Media banks [D]; Resolume Dashboard [D]; Photism companion device [M].
- **Effort.** 3–4 days (layout only; the DESIGN page reuses current components).
- **Risk.** Low. Coordinate visual styling with `DESIGN-DOSSIER.md`.

**R2. Scene grid with thumbnails and one-line descriptions.**
- **Problem.** Scenes are names only.
- **Solution.**
  - Render one PNG per preset with the existing `preset_regression_test.py --keep` [R], and store it next to the preset JSON.
  - The engine sends the path in `/v2/presets`, and the plug-in shows a 4×4 grid.
  - Add a star to mark a "gig set", like Synesthesia's gig shortlist.
- **Proven by.** Synesthesia library [D]; NestDrop thumbnails [M]; Resolume thumbnails [D].
- **Effort.** 1–2 days.
- **Risk.** Low. Thumbnails of generative scenes can mislead; render at a representative moment (e.g. 8 s into the demo groove).

**R3. Visible cue state: NEXT + GO + quantum.**
- **Problem.** A scene click waits invisibly for the beat.
- **Solution.**
  - `/v2/status` adds `pending`, `beatsToGo` and `quantum`.
  - The tile blinks with a countdown ring.
  - A single quantum selector: *Now · Beat · Bar · 4 Bars*.
  - A click in the grid **cues** (Resolume "select") and GO fires. Holding Shift, or double-clicking, fires immediately.
  - Next / Prev host parameters move the cursor. The GO host parameter fires.
- **Proven by.** Resolume select vs trigger and Beat Snap [D]; Ableton blinking launch buttons [U]; Isadora spacebar GO [D].
- **Effort.** 1–2 days (the engine already has `pendingIndex` [R]).
- **Risk.** Low. Needs one clear rule for when a cue fires: on GO, or immediately once the scene is picked.

**R4. Automatic Main / Source roles.**
- **Problem.** "Send macros" is on by default everywhere [R], and roles are set by hand.
- **Solution.**
  - The engine elects the first MIX instance as Main and tells the others to become Sources; if Main disappears, it re-elects.
  - Default the role from the track name via `updateTrackProperties`, with manual override.
- **Proven by.** Zwobot can only *warn* "Do not use the Zwobot main module twice" (prior report). The JUCE forum confirms track names arrive in Live 10 VST3 [U].
- **Effort.** 2 days.
- **Risk.** Medium-low. Live's name / colour callback quirk [U]; debounce it.

**R5. Safety pack.**
- **Problem.** Only BLACKOUT exists; flash counting is split; the red palette is prone to red flashes.
- **Solution.**
  - **SAFE** (host parameter plus a big button): fade over 1 bar to a designated safe cue, digital LOOK to 0, CALM on. A second press returns.
  - **Brightness ceiling**, not stored in snapshots or cues.
  - **Unified flash budget:**
    - ≤3 per second across Flash, kick-cuts and SHOTS
    - the stricter show rule `CONCEPT.md` asks for (≤2/s, ≤6 per 5 s)
    - a **red-flash clamp** for saturated-red transitions
  - **Undo last recall.**
- **Proven by.** Synesthesia Master outside presets and per-bank Undo [D]; WCAG 2.3.1 [D]; `CONCEPT.md` §6.4 [R].
- **Effort.** 2–3 days.
- **Risk.** Low. Taste: the limiter must never be *audible* in the picture, i.e. it should skip events, not dim.

**R6. Engine persistence and self-healing.**
- **Problem.** After a reboot or crash the output state is lost [R].
- **Solution.**
  - The engine stores output monitor, fullscreen, quality and last preset (e.g. an `ApplicationProperties` file).
  - The plug-in can "Auto-start engine" (default on for Main).
  - A red **ENGINE LOST** banner with one-click relaunch.
  - After reconnect, the existing 1 s full re-send restores the state [R].
  - Add an engine watchdog that respawns if the process dies while Live is playing.
- **Proven by.** Synesthesia Default Startup Scene [D]. The gap is proven by Resolume's missing autosave [S] and Videosync's declined autostart [U].
- **Effort.** 2 days.
- **Risk.** Low. Watch for respawn loops on shader errors: back off after 3 tries.

**R7. Controller-ready defaults.**
- **Problem.** Mapping about 56 parameters by hand. The Preset parameter spans 0–64, so one knob sweeps the list about 4 times [R].
- **Solution.**
  - Order host parameters so the first 8 are the performance knobs and the next 8 are toggles and actions. Live's panel order can be set in Configure mode [D]; that controller banks follow it is an inference to verify on hardware [I].
  - Ship an Audio Effect Rack preset (`VJ Main.adg`) with Configure done and 8 rack macros mapped. Live says to "create a Rack containing the configured plug-in" to keep a parameter set [D].
  - Ship a template Live set with the controller mapped.
  - Replace the Preset 0–64 integer with a Cue / Scene parameter of ≤32 steps.
  - Keep the total ≤64 parameters [D].
- **Proven by.** Live Configure / Rack [D]; Synesthesia position-based global mapping [D].
- **Effort.** 1–2 days.
- **Risk.** Low-medium. Reordering VST3 parameters can break saved automation *if IDs change*. Only the order changes; the IDs stay.

**R8. Cue selection by MIDI notes (Session and Arrangement linking today).**
- **Problem.** Visuals do not follow Live's scenes or arrangement.
- **Solution.**
  - On a reserved MIDI channel (e.g. ch 16), note *n* selects cue / scene *n* and velocity sets the transition (0 = cut).
  - Put one MIDI clip per Session scene on a "VJ" MIDI track routed to the Main analyzer. Launching a Live scene then launches the visual cue on the same quantum.
  - The same notes work in the Arrangement.
  - Notes on that channel never count as hits.
- **Proven by.** Isadora / Resolume + Ableton users trigger scenes with MIDI from Live [S]. Videosync's whole premise is clips = visuals [M].
- **Effort.** 1–2 days.
- **Risk.** Low. Document one rule: role MIDI channels 1–15 are hits, channel 16 is cues.

**R9. Recall channels and bank lock for snapshots.**
- **Problem.** Snapshots recall everything (macros + LOOK + palette) [R].
- **Solution.**
  - Per snapshot or cue, choose what it affects: *Shape/Move · Look · Palette · React*.
  - A per-bank **Lock** so a recall never touches, say, the palette mid-show.
- **Proven by.** Synesthesia preset channels and bank Lock [D].
- **Effort.** 1–2 days.
- **Risk.** Low.

## 6B. Medium (weeks)

**M1. Cue Bank and Cue List (the heart of set building).**
- **Problem.** 4 snapshots cannot hold a show [R] (`CONCEPT.md` §6.2).
- **Solution.** 32 named cues. Each cue holds:
  - scene
  - shape / move macros
  - look + palette
  - react profile
  - transition (bump, crossfade N beats, fade through black)
  - quantum
  - recall channels
  - a note field

  Plus:
  - a list view with **GO / Back / Jump**
  - an automatable `cueIndex` parameter
  - "Store to cue"
  - "Blind edit" of any cue while another is live
- **Proven by.** VDMX Cue List [D]; Isadora bumps / crossfades / Blind Mode [D]; Synesthesia playlists [D].
- **Effort.** 1.5–2 weeks.
- **Risk.** Low-medium. The state tree grows. Fade-through-black keeps the transition cheap on Iris Xe [D] (Synesthesia notes it runs one scene at a time).

**M2. Named Looks and React profiles (fewer knobs, same power).**
- **Problem.** 14 LOOK knobs and 5 react knobs on the perform surface [R].
- **Solution.**
  - **Look** selector: *Clean · 35mm · Worn · Burnt · Broadcast · Data*, each a stored LOOK vector, plus a **Look Amount** knob that scales from clean to the full look.
  - **React** selector: *Still · Breath · Pulse · Cut* (Impact / Softness / Reactivity / Push / SHOTS presets).
  - Detailed knobs stay in DESIGN, and editing them creates "Custom".
- **Proven by.** Photism named palettes [M]; Synesthesia Meta vs Scene separation [D]. The research consensus to "map with restraint" [P] comes from the prior report and ZeroToVJ (page not reachable this time).
- **Effort.** 1 week (mostly curating the looks with the owner).
- **Risk.** Taste: the owner must sign off on the look names and vectors.

**M3. Preview monitor in the plug-in.**
- **Problem.** The performer cannot see the output from Live (rear projection) [R].
- **Solution.**
  - The engine reads back a 320×180 frame every 100–200 ms using an async PBO, so it does not stall the GPU.
  - It writes it to a shared-memory ring or sends it as a small JPEG over localhost.
  - PERFORM shows **PROGRAM** (live) and **NEXT** (the static thumbnail, or a live prebuild frame if cheap).
- **Proven by.** Videosync 2.1 Video Monitor [P]; Zwobot monitor [M]; Resolume preview [D].
- **Effort.** 1 week.
- **Risk.** Medium: readback cost on Iris Xe, and plug-in repaints on Live's UI thread. Cap the repaint rate and allow it to be switched off.

**M4. In-plug-in MIDI learn with a global controller profile.**
- **Problem.** Live's MIDI mappings live per set. Soft takeover depends on Live settings.
- **Solution.**
  - Plug-in-level learn, stored globally so every set uses the same map.
  - LED feedback out.
  - Pick-up / value-scaling behaviour built in.
  - Maps target *performance slots* (Knob 1–8, GO, SAFE), not scenes.
- **Proven by.** Synesthesia global position-based mapping + feedback [D]; VS MIDI learn [M]; Live takeover modes [D].
- **Effort.** 1 week.
- **Risk.** Medium: VST3 MIDI input to an effect plug-in needs the track's MIDI routed. Document it, or pair it with M5.

**M5. "VJ Link" Max for Live companion (Live 10, Max 8).**
- **Problem.** R8 needs a MIDI clip per scene; users think in Live scene names.
- **Solution.**
  - A tiny M4L device observes `playing_slot_index` on a "VJ" track (the forum-recommended route) and / or the selected scene.
  - It reads clip or scene names tagged `[vj 7]` or `[vj Membrane]` and sends `/v2/cue` over OSC.
  - Use `deferlow` for observer ID changes.
- **Proven by.** TDAbleton scene / slot data, including Live 9–10 via 1.x [D]; the Cycling '74 threads [U]; Max's documented Live API [D] https://docs.cycling74.com/userguide/m4l/live_api/.
- **Effort.** 1 week.
- **Risk.** Medium: M4L is Suite-only; `is_triggered` timing quirks [U]. Keep R8 as the fallback.

**M6. Phone / tablet remote served by the engine.**
- **Problem.** The performer may be away from the laptop (dance show, rehearsals in the hall).
- **Solution.**
  - The engine serves one web page on the LAN (WebSocket to its OSC handlers).
  - It shows:
    - the cue list
    - GO / Back
    - SAFE / BLACKOUT / CALM
    - 4 faders
    - a thumbnail
  - PIN-protected, LAN only.
- **Proven by.** TouchOSC (all platforms, bidirectional) [D] https://hexler.net/touchosc; Open Stage Control (free, runs in a browser) [D] https://openstagecontrol.ammd.net/.
- **Alternative.** Ship a TouchOSC / Open Stage Control layout instead (≈1 day).
- **Effort.** 1 week (1 day for the layout alternative).
- **Risk.** Medium: stage Wi-Fi. Provide a USB-tethered / hotspot recipe.

**M7. Installer and first-run.**
- **Problem.** Installation compiles code [R]. Not finished-product level.
- **Solution.**
  - An installer (VST3 + engine + presets + thumbnails).
  - A first-run card in the plug-in:
    - "Engine found ✓"
    - "Output: pick screen"
    - "Press play"
  - The demo groove runs until audio arrives.
- **Proven by.** Photism / Synesthesia / Resolume first-run paths [M]/[D].
- **Effort.** 3–5 days.
- **Risk.** Low.

## 6C. Big bets

**B1. Show Mode: timeline cues + live layer.**
- **Problem.** The dance piece needs a repeatable, cue-locked spine plus live expression [R].
- **Solution.**
  - Cues can carry a **bar position**; the plug-in reads the host song position.
  - Show Mode fires them as the Arrangement plays.
  - **Rule:** any manual touch overrides until the next cue (Resolume's "manual wins" priority [D]).
  - A **HOLD** button freezes cue advancement.
  - Pre-roll: NEXT shows the upcoming cue and the bars left.
  - Rehearsal: "record GO presses as cue positions".
  - Luminance budget per cue (ceiling).
- **Proven by.** VDMX Cue List by beats / measures / SMPTE [D]; Isadora [D]; Millumin timelines [P].
- **Effort.** 3–5 weeks.
- **Risk.** Medium. Two sources of truth (automation vs Show Mode): pick **one** per set. Recommend the host-automated `cueIndex` (R8 / M1) first, and let Show Mode add pre-roll and HOLD on top.

**B2. Co-pilot: structure-aware Auto-VJ that *suggests*.**
- **Problem.** A solo performer cannot play music and pick visuals at every phrase.
- **Solution.**
  - The co-pilot watches `build` / `presence` / `energyTrend` and bar count (8 / 16-bar phrases).
  - When a section boundary is likely, it **pre-cues** a NEXT from the song's *pool* (curated cues, tagged calm / build / peak / release).
  - The performer confirms with GO, or the cue auto-fires if **Auto** is on.
  - Autopilot modes: *Off · Suggest · Auto (bag-random within the pool)*.
- **Proven by.**
  - Resolume Autopilot (bag random, beats) [D].
  - NestDrop beat auto-change [M].
  - KaleidoscopeEnhanced structure / mood selection [D].
  - Nobody pairs suggestion with confirmation. That is the gap.
- **Effort.** 3–6 weeks (including tuning on the owner's tracks with `analyze_wav`).
- **Risk.** Medium-high on taste. Limit it to curated pools; it must never fire mid-phrase.

**B3. Mix Map: make role analysis the product's signature.**
- **Problem.** Our unique multi-role analysis is invisible.
- **Solution.**
  - The Main instance lists every connected Source (role, track name, level, hit LED, latency).
  - A small **5 × 5 grid** of roles × scene slots (impulse / pulse / texture / swell / drift, per the prior report's REACT TO v2).
  - A missing role falls back to MIX and is flagged.
- **Proven by.** No in-DAW tool offers instrument roles (Synesthesia: single mixed input [D]). Beam's "see exactly what Beam is sending" is the monitoring pattern (prior report).
- **Effort.** 2–3 weeks.
- **Risk.** Complexity creep. It lives on the SETUP page, never on PERFORM.

**B4 (conditional). Productise for others.** Licensing, a content pack, docs and a trial. Only if the owner wants to sell (see open question 1). Otherwise stop at M7.

## 6D. What not to do (and why)

| Temptation | Why not |
|---|---|
| An open modulation matrix | VS / Resolume's per-parameter model brings jitter and "lost control" (prior report) |
| More scenes now | The owner says the scenes are good; workflow is the bottleneck |
| Real-time AI | Needs a 24 GB NVIDIA GPU [S] and is non-commercial [D]; web AI has 3–5 s latency [P] |
| Rendering inside Live (M4L / Jitter) | Frame rate tied to Live's UI thread [D]; Videosync 2's Windows support is limited to Live 12, where the newer video engine is "more stable" [D] |
| Exceeding 64 host parameters | Live then opens an empty panel [D] |

## 6E. Suggested sequence

| Order | Items | Why |
|---|---|---|
| Week 1 | R1, R3, R4, R5, R6 | Removes "lost" and "unsafe" first; mostly UI and state work |
| Week 2 | R2, R7, R8, R9, M7 | Recall, controller, Ableton linking; install polish |
| Weeks 3–4 | M1, M2 | Set-building core and fewer knobs; unlocks the dance show |
| Weeks 5–6 | M3, M4, M6 | Monitoring and remote control |
| Later | M5, B1, B2, B3 | Deeper integration and differentiators |

---

# Part 7. Proposed control architecture

This is structure, not styling. `DESIGN-DOSSIER.md` owns the visuals.

```
MAIN instance (on Master)                          SOURCE instance (on kick/snare/... tracks)
┌ PERFORM (default) ─────────────────────────┐     ┌ role [auto from track name]      ┐
│ Cue/scene grid (thumbnails, ★ gig set)     │     │ input meter · hit LED             │
│ NOW ▸ NEXT (blinking, bars-to-go) · GO     │     │ Hit Sens · Trim                   │
│ 8 knobs: Intensity Speed Form Scale        │     │ "connected to Main ✓"             │
│          Erode Detail Impact LookAmount    │     └───────────────────────────────────┘
│ Look ▾   React ▾   Palette ▾               │
│ Freeze Calm Reverse Sync                   │
│ HIT   SAFE   BLACKOUT   [program preview]  │
└────────────────────────────────────────────┘
┌ DESIGN ── today's SHAPE/MOVE/REACT/LOOK/MEDIA panels, "Store to cue", Lock per bank ┐
┌ SETUP ─── role, sens, trim, adaptive, lookahead, REACT TO, engine/output, Mix Map  ┐
```

**Host parameter plan** (≤64, IDs unchanged):
1. Parameters 1–8: performance knobs.
2. Parameters 9–16: GO, Next, Prev, HIT, SAFE, BLACKOUT, CALM, FREEZE.
3. Then `cueIndex`, look / react selectors.
4. Then the remaining detail parameters.

---

# Part 8. Evidence gaps and caveats

- **User voice:** Reddit (r/vjing) was not reachable, and several Resolume forum threads returned 403. Complaints about Resolume (autosave), Magic, Serato and Isadora rest on search snippets [S].
- **Timings:** competitor install-to-first-visual times are inferred from documented steps, not timed.
- **Price conflicts:**
  - VDMX6: CDM ($99 / $249) vs the 2026 lime list ($199 / $349).
  - Resolume Wire: official page (€399) vs lime list ("included").
  - Videosync and Zwobot current prices: unknown.
- **Competitor-authored sources:** Photism's comparison pages are written by a competitor and are partly outdated. They call Videosync "macOS only", which Showsync's own release notes contradict.
- **Our own claims:** they come from the repo and docs. Nothing in this report was tested inside Live (consistent with the Sept 25 state).

---

# Part 9. Open questions for the owner

1. **For whom is the finished product?** Only you (the show + your gigs), or for sale to others? This decides whether B4 (installer polish, licence, content) matters.
2. **How should the dance show advance?** By Live's timeline (automation or MIDI notes fire each cue), by you pressing GO per cue like theatre, or both (timeline with HOLD)?
3. **Where will your eyes be during the show?** On the laptop, the stage, or neither? This decides whether the preview monitor (M3) or a phone / tablet remote (M6) comes first.

---

# Sources (pages actually opened for this report)

**Official docs / manuals [D]**
- https://app.synesthesia.live/docs/ (hub) · https://app.synesthesia.live/docs/manual/controls.html · https://app.synesthesia.live/docs/manual/library.html · https://app.synesthesia.live/docs/manual/presets.html · https://app.synesthesia.live/docs/faq/midi_osc.html · https://app.synesthesia.live/docs/faq/settings.html · https://app.synesthesia.live/docs/ssf/audio_uniforms.html · https://synesthesia.live/docs/faq/ · https://synesthesia.live/docs/faq/audio.html · https://www.synesthesia.live/public-release/changelog/1.20.0.66.html
- https://resolume.com/support/en/autopilot · https://resolume.com/support/en/dashboard · https://resolume.com/support/en/clips · https://resolume.com/support/en/parameter-animation · https://resolume.com/support/en/quickstart-tutorial
- https://support.showsync.com/release-notes/videosync/2.0 · https://support.showsync.com/videosync/troubleshooting/common-pitfalls
- https://vdmx.vidvox.net/tutorials/introduction-to-the-cue-list-plugin
- https://troikatronix.com/isadora/cueing-show-control/
- https://docs.derivative.ca/TDAbleton
- https://docs.cycling74.com/userguide/m4l/live_api/ · https://docs.cycling74.com/legacy/max8/vignettes/jitter_and_Max_for_Live
- https://www.w3.org/WAI/WCAG21/Understanding/three-flashes-or-below-threshold.html
- https://github.com/reneweller-coding/KaleidoscopeEnhanced · https://github.com/daydreamlive/scope · https://github.com/projectM-visualizer/projectm · https://github.com/EnvelopSound/EnvelopForLive
- https://hexler.net/touchosc · https://openstagecontrol.ammd.net/
- Ableton Live manual and release notes (retrieved through Ableton's documentation search): https://www.ableton.com/en/manual/working-with-instruments-and-effects/ · https://www.ableton.com/en/manual/launching-clips/ · https://www.ableton.com/en/manual/instrument-drum-and-effect-racks/ · https://www.ableton.com/en/manual/midi-and-key-remote-control/ · https://www.ableton.com/en/manual/session-view/ · https://www.ableton.com/en/release-notes/live-11/ · https://www.ableton.com/en/release-notes/live-9/

**Marketing / product pages [M]**
- https://synesthesia.live/ · https://synesthesia.live/pricing · https://www.showsync.com/videosync · https://resolume.com/software/avenue-arena · https://resolume.com/software/wire · https://photism.app/ · https://photism.app/lite/ · https://www.imaginando.pt/products/vs-visual-synthesizer · https://www.zwobotmax.com/ · https://nestimmersion.ca/nestdrop.php · https://www.kvraudio.com/product/spettro-by-creature-from-the-black · https://www.arkestra.app/ · https://visualzstudio.com/ · https://karleido.com/ · https://www.ebosuite.com/

**Press / editorial [P]** (competitor-authored pages are marked)
- https://cdm.link/vdmx6/ · https://cdm.link/videosync-2-1-for-ableton-live/ · https://cdm.link/videosync-1-0-arrives-visuals-integrate-with-ableton-live-session-arrangement-warping/ · https://www.digitaldjtips.com/virtualdj-2026-brings-fluid-beatgridding-ai-video-visuals/ · https://limeartgroup.com/the-mega-list-of-vj-software-and-tools/ · https://vdmx.vidvox.net/tutorials/vj-travel-kit-whats-in-our-bag · https://www.arkestra.app/articles/sync-vj-software-ableton-live (vendor article) · https://photism.app/learn/best-ableton-visualizers/ (competitor-authored) · https://photism.app/software/magic-music-visuals/ (competitor-authored)

**User reports [U]**
- https://forum.showsync.com/t/videosync-running-slow-in-ableton/2651 · https://forum.showsync.com/t/videosync-ideas-a-place-of-organization/796 · https://forum.showsync.com/t/videosync-2-open-beta-launch-for-windows-and-macos/1768 · https://forum.ableton.com/viewtopic.php?f=1&t=238698 · https://cycling74.com/forums/problem-continually-observing-any-given-scenes-state-firedidle · https://cycling74.com/forums/scene-is-triggered-state · https://cycling74.com/forums/observing-selected-scene · https://forum.juce.com/t/audioprocessor-updatetrackproperties-ableton-10-vst3-not-working-as-expected/43049

**Search snippets only [S], not opened; treat as leads**
- Resolume forum on autosave (e.g. https://resolume.com/forum/viewtopic.php?t=15530) and on triggering from Live via MIDI (https://resolume.com/forum/viewtopic.php?t=17213)
- Daydream Scope VRAM (https://topvidtools.com/2026/03/17/daydream-scope-review/)
- Magic Music Visuals pricing (https://magicmusicvisuals.com/)
- Serato Video (https://www.digitaldjtips.com/reviews/serato-video-plugin/)
- Isadora + Ableton MIDI (https://support.troikatronix.com/support/solutions/articles/13000081452-control-isadora-from-ableton-live-with-midi)
- Videosync Push support (https://cdm.link/videosync-1-3-visual-tool-for-ableton-live/)

**Earlier in-repo research this builds on:** `docs/RESEARCH-REPORT.md` (23 Sept), `docs/REVIEW-2026-09-24-HE.md`, `docs/before-it-disappears/CONCEPT.md`.
