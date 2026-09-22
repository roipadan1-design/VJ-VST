# Audio-reactive visual instrument — research and build specification

**Research date:** 22 September 2026. **Fixed target:** Windows 11 Pro / Ableton Live 10 Suite / Max 8.

Open **Visual_Instrument_Research.html** for the complete offline reading edition. It contains the Hebrew executive summary, English A–I report, six-product comparison matrix, engineering specification, twelve preset designs, validation notes and linked bibliography. No external font or JavaScript is required.

## Start here

Read the Hebrew summary at the beginning of REPORT.md, then ENGINEERING_SPEC.md sections1–4 and11. The recommended path keeps the current external C++ renderer, improves musical analysis/mapping, ships three good generative presets, and postpones a thin VST3 wrapper until the interaction/state contracts are stable.

## Contents

| File | Purpose |
|---|---|
| REPORT.md | Full A–I research report, six-product teardown/matrix, additional landscape, DSP/graphics/architecture/licensing findings and open questions |
| ENGINEERING_SPEC.md | Proposed implementation contract: responsibilities, threads, IPC, DSP defaults, modulation, MIDI, rendering, state, roadmap and acceptance gates |
| STARTER_PRESETS.md | Twelve original visual designs; each has parameter ranges, four macros, audio/MIDI mappings and a performance/test allocation |
| preset.schema.json | Strict Draft2020-12 schema for the **new** preset format |
| presets/01_pulse_halo.json | Complete worked preset in the new format |
| shaders/pulse_halo.fs | Original ISF-style starter shader; **not GPU-compiled here** |
| audio-feature-contract.schema.json | Logical feature/event debug/archive contract; OSC wire semantics are in the spec |
| examples/ | Valid feature-frame and onset-event examples |
| controller-profile.json | Proposed custom Launch Control XL MK1/MK2 assignments, **not a factory/SysEx template** |
| validate_presets.py | Structural and cross-reference validator with negative tests |
| VALIDATION.md | Actual local check results and the untested boundaries |
| sources.md / sources.csv / sources.json |105 source entries with URLs, publication/update evidence and access dates |

## Run the checks

```bash
python -m pip install -r requirements-validation.txt
python validate_presets.py --self-test
```

The validator is development tooling, not a proposed audio-thread dependency. The examples are a specification starter and are **not drop-in files for the existing prototype**. No C++ engine patch, compiled M4L device, VST3 binary, hardware controller template or vendor installer is included.

## Evidence boundaries

The existing implementation was described by the supplied user brief; its source code was not uploaded for inspection. Primary documentation, release histories, repositories, developer material and selected official stills were researched. Demo videos were not played/frame-reviewed. Vendor compatibility, local execution and inferred architecture are distinguished in the report. Missing prices, benchmark data and exact repository last-commit dates remain marked unknown.

GPU/CPU/latency figures proposed by the specification are qualification targets. Only the JSON/schema/asset relationships and document links were tested locally. Live10/Max8, Windows, VST3, GLSL compilation, DSP quality, MIDI hardware and on-stage recovery still require the acceptance tests in the specification.
