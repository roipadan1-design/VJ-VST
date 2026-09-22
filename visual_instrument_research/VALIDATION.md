# Validation record

**Date:** 22 September 2026. Checks ran in the research container, not the target Windows/Live environment.

## Executed checks

Command:

```text
python validate_presets.py --self-test
```

Result:

```text
PASS: 2 schemas, 1 preset(s), 2 message(s), semantic references and ISF header.
PASS: 36 negative validation tests rejected invalid input.
NOT TESTED: GLSL compilation, rendering, DSP, C++ build, Live/Max/VST3, MIDI hardware or latency.
```

The validator used Python and jsonschema4.26.0. It checked both schemas against Draft2020-12, the complete preset and two messages, UUID formats, finite values, unique IDs/slots, parameter bounds/defaults, source/destination references, trigger-envelope types, contained/existing shader paths, ISF scalar input metadata, band ordering/Nyquist and event field consistency.

The 36 negative tests cover malformed/unknown schema fields, duplicate IDs/slots, missing or mistyped references, zero/reversed ranges, excessive route values, NaN/Infinity, shader metadata mismatch, traversal/missing paths, graph self-reference, invalid LFO timing, insufficient macros, invalid source/message fields, band gaps/Nyquist violations, phase endpoints, wrong UUID/sample rate and duplicate JSON keys.

These tests establish consistency of the supplied **design artifacts**. They do not establish musical quality, transport accuracy, runtime performance or complete ISF conformance. Shader checking reads metadata only; it is not a GLSL parser/compiler.

The HTML edition was also rendered in the installed Chromium browser at 1440×1100 and 390×844 viewports. Desktop and Hebrew-summary screenshots were visually inspected; document width matched each viewport without page-level horizontal overflow. Wide tables retain their own scroll containers.

The reading edition is generated from the Markdown sources. Internal links/anchors and referenced local files are checked during packaging. Source links retain their real URLs; external sites are not mirrored or assumed permanently available.

## Not executed

No MSVC/CMake build, Max patch execution, plugin hosting, GPU compilation/rendering, audio analysis benchmark, visual golden-image test, Spout exchange, controller/SysEx test, photon-latency measurement or Windows soak test was performed. These are explicit roadmap gates, not implied successes.
