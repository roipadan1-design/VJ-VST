#!/usr/bin/env python3
"""Validate the proposed research-bundle contracts; not a renderer or a DSP test.

Usage: python validate_presets.py [--root DIRECTORY] [--self-test]
Dependency: jsonschema >= 4 (install in your own environment if necessary).
No network requests, external schema resolution, or shader compilation are used.
"""
from __future__ import annotations

import argparse
import copy
import json
import math
import re
import sys
from pathlib import Path
from typing import Any

try:
    from jsonschema import Draft202012Validator, FormatChecker
except ImportError:
    raise SystemExit("Missing dependency 'jsonschema'. Install jsonschema>=4, then run again.")


class ValidationFailure(ValueError):
    """An invalid contract, asset or semantic relationship."""


def reject_constant(value: str) -> None:
    raise ValidationFailure(f"Non-JSON numeric constant is forbidden: {value}")


def unique_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValidationFailure(f"Duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path: Path) -> Any:
    if path.stat().st_size > 8_000_000:
        raise ValidationFailure(f"JSON file too large for this validator: {path}")
    return json.loads(path.read_text(encoding="utf-8-sig"),
                      parse_constant=reject_constant, object_pairs_hook=unique_object)


def assert_finite(value: Any, path: str = "$") -> None:
    if isinstance(value, float) and not math.isfinite(value):
        raise ValidationFailure(f"{path}: non-finite value")
    if isinstance(value, dict):
        for key, child in value.items():
            assert_finite(child, f"{path}.{key}")
    elif isinstance(value, list):
        for index, child in enumerate(value):
            assert_finite(child, f"{path}[{index}]")


def check_schema(data: Any, schema: dict[str, Any]) -> None:
    assert_finite(data)
    errors = sorted(Draft202012Validator(schema, format_checker=FormatChecker()).iter_errors(data),
                    key=lambda e: str(list(e.absolute_path)))
    if errors:
        err = errors[0]
        raise ValidationFailure(f"Schema at {list(err.absolute_path)}: {err.message}")


def index_unique(items: list[dict[str, Any]], label: str, key: str = "id") -> dict[Any, dict[str, Any]]:
    result: dict[Any, dict[str, Any]] = {}
    for item in items:
        value = item[key]
        if value in result:
            raise ValidationFailure(f"Duplicate {label} {key}: {value}")
        result[value] = item
    return result


def safe_asset(root: Path, relative: str) -> Path:
    candidate = (root / relative).resolve()
    try:
        candidate.relative_to(root.resolve())
    except ValueError as exc:
        raise ValidationFailure(f"Asset escapes root: {relative}") from exc
    if not candidate.is_file():
        raise ValidationFailure(f"Missing asset: {relative}")
    return candidate


def validate_preset(data: dict[str, Any], schema: dict[str, Any], root: Path) -> None:
    check_schema(data, schema)
    stages = index_unique(data["stages"], "stage")
    macros = index_unique(data["macros"], "macro")
    slots = index_unique(data["macros"], "macro", "slot")
    if not {0, 1, 2, 3}.issubset(slots):
        raise ValidationFailure("First four stable macro slots 0..3 must be present")
    mods = index_unique(data["modulators"], "modulator")
    index_unique(data["routes"], "route")
    index_unique(data["triggers"], "trigger")
    destinations: set[str] = set()
    prior: set[str] = set()
    for stage_id, stage in stages.items():
        parameters = index_unique(stage["parameters"], f"parameter in {stage_id}", "name")
        for param in parameters.values():
            if not param["min"] < param["max"]:
                raise ValidationFailure(f"Invalid physical range: {stage_id}.{param['name']}")
            if not param["min"] <= param["default"] <= param["max"]:
                raise ValidationFailure(f"Default outside range: {stage_id}.{param['name']}")
            destinations.add(f"stage.{stage_id}.{param['name']}")
        for binding in stage["inputs"].values():
            if binding.startswith("stage.") and binding[6:] not in prior:
                raise ValidationFailure(f"Stage input is missing, forward-referenced or cyclic: {binding}")
            if not binding.startswith("stage.") and binding not in {
                "input.video", "input.camera", "input.spout"
            }:
                raise ValidationFailure(f"Unknown external input: {binding}")
        asset = safe_asset(root, stage["shader"])
        if asset.stat().st_size > 2_000_000:
            raise ValidationFailure("Shader exceeds this validator's inspection limit")
        match = re.match(r"\s*/\*(.*?)\*/", asset.read_text(encoding="utf-8"), re.S)
        if match is None:
            raise ValidationFailure(f"Missing ISF metadata header: {asset.name}")
        header = json.loads(match.group(1), parse_constant=reject_constant, object_pairs_hook=unique_object)
        inputs = {v["NAME"]: v for v in header.get("INPUTS", [])}
        if len(inputs) != len(header.get("INPUTS", [])):
            raise ValidationFailure(f"Duplicate ISF input name in {asset.name}")
        for name, param in parameters.items():
            if name not in inputs or inputs[name].get("TYPE") != "float":
                raise ValidationFailure(f"Scalar parameter lacks float ISF input: {name}")
            for a, b in [("min", "MIN"), ("max", "MAX"), ("default", "DEFAULT")]:
                if b in inputs[name] and not math.isclose(param[a], inputs[name][b], abs_tol=1e-8):
                    raise ValidationFailure(f"Preset / shader metadata mismatch for {name}.{a}")
        prior.add(stage_id)
    fixed_sources = {
        "audio.level.activity", "audio.level.absoluteActivity", "audio.bass.activity",
        "audio.mid.activity", "audio.high.activity", "audio.bass.absoluteActivity",
        "audio.mid.absoluteActivity", "audio.high.absoluteActivity",
        "descriptor.centroid01", "descriptor.flatness", "descriptor.rolloff01",
        "descriptor.flux01", "descriptor.energyTrend", "clock.beatPhase",
        "clock.barPhase", "clock.beatPosition",
    }
    for route in data["routes"]:
        if not route["inputMin"] < route["inputMax"]:
            raise ValidationFailure(f"Invalid route input range: {route['id']}")
        if route["destination"] not in destinations:
            raise ValidationFailure(f"Unknown route destination: {route['destination']}")
        source = route["source"]
        valid = source in fixed_sources
        if source.startswith("macro."):
            valid = source[6:] in macros
        elif source.startswith("env."):
            valid = source[4:] in mods and mods[source[4:]]["type"] == "ad"
        elif source.startswith("lfo."):
            valid = source[4:] in mods and mods[source[4:]]["type"] == "lfo"
        if not valid:
            raise ValidationFailure(f"Unknown or mistyped route source: {source}")
    for trigger in data["triggers"]:
        for action in trigger["actions"]:
            if action["type"] == "envelope":
                target = action["target"]
                if target not in mods or mods[target]["type"] != "ad":
                    raise ValidationFailure(f"Envelope action target is not an AD envelope: {target}")


def validate_message(data: dict[str, Any], schema: dict[str, Any]) -> None:
    check_schema(data, schema)
    if data["kind"] == "features":
        previous = None
        for band in data["bands"]:
            if not 0 <= band["loHz"] < band["hiHz"] <= data["sampleRate"] / 2:
                raise ValidationFailure("Band limits must be ordered and below Nyquist")
            if previous is not None and not math.isclose(previous, band["loHz"]):
                raise ValidationFailure("The six bands must be contiguous")
            previous = band["hiHz"]
        if not 0 <= data["transport"]["beatPhase"] < 1 or not 0 <= data["transport"]["barPhase"] < 1:
            raise ValidationFailure("Phases must lie in [0,1)")
    else:
        midi = data["eventType"] in {"midiNoteOn", "midiNoteOff"}
        if midi and (data["note"] is None or data["channel"] is None):
            raise ValidationFailure("MIDI events need note and channel")
        if not midi and (data["note"] is not None or data["channel"] is not None):
            raise ValidationFailure("Non-MIDI events must have null note/channel")


def run_self_tests(root: Path, preset: dict[str, Any], ps: dict[str, Any],
                   frame: dict[str, Any], event: dict[str, Any], fs: dict[str, Any]) -> int:
    count = 0

    def rejects(base: dict[str, Any], mutate: Any, checker: Any, label: str) -> None:
        nonlocal count
        candidate = copy.deepcopy(base)
        mutate(candidate)
        try:
            checker(candidate)
        except (ValidationFailure, ValueError, KeyError):
            count += 1
        else:
            raise AssertionError(f"Negative test did not reject: {label}")

    cp = lambda d: validate_preset(d, ps, root)
    cm = lambda d: validate_message(d, fs)
    tests = [
        (lambda d: d.update(schemaVersion="1.0"), "wrong major"),
        (lambda d: d.update(undocumented=True), "unknown property"),
        (lambda d: d["macros"][1].update(slot=0), "duplicate slot"),
        (lambda d: d["macros"][1].update(id=d["macros"][0]["id"]), "duplicate macro"),
        (lambda d: d["routes"][0].update(destination="stage.halo.missing"), "missing destination"),
        (lambda d: d["routes"][0].update(source="macro.missing"), "missing macro source"),
        (lambda d: d["routes"][0].update(source="env.breath"), "wrong modulator type"),
        (lambda d: d["routes"][0].update(inputMin=1,inputMax=1), "zero input width"),
        (lambda d: d["routes"][0].update(amount=3), "excess amount"),
        (lambda d: d["routes"][0].update(attackMs=-1), "negative attack"),
        (lambda d: d["routes"][0].update(amount=float("nan")), "NaN"),
        (lambda d: d["stages"][0]["parameters"][0].update(default=99), "bad default"),
        (lambda d: d["stages"][0]["parameters"][0].update(max=.01), "reversed range"),
        (lambda d: d["stages"][0]["parameters"][0].update(default=.44), "shader header mismatch"),
        (lambda d: d["stages"][0].update(shader="../outside.fs"), "traversal path"),
        (lambda d: d["stages"][0].update(shader="shaders/missing.fs"), "missing asset"),
        (lambda d: d["stages"][0].update(inputs={"feedback":"stage.halo"}), "self-cycle"),
        (lambda d: d["triggers"][0]["actions"][0].update(target="breath"), "trigger wrong type"),
        (lambda d: d["modulators"][1].update(periodBeats=0), "zero LFO period"),
        (lambda d: d["macros"].pop(), "too few macros"),
    ]
    for mutate, label in tests:
        rejects(preset, mutate, cp, label)
    for mutate, label in [
        (lambda d:d.update(sequence=-1),"negative sequence"),
        (lambda d:d["spectrum32"].pop(),"short spectrum"),
        (lambda d:d["spectrum32"].__setitem__(0,float("inf")),"infinite spectrum"),
        (lambda d:d["bands"][0].update(hiHz=20),"zero band width"),
        (lambda d:d["bands"][0].update(hiHz=61),"noncontiguous band"),
        (lambda d:d["bands"][-1].update(hiHz=30000),"Nyquist violation"),
        (lambda d:d["transport"].update(beatPhase=1),"phase endpoint"),
        (lambda d:d.update(sessionId="not-a-uuid"),"invalid UUID"),
        (lambda d:d.update(sampleRate=8000),"unsupported rate"),
    ]:
        rejects(frame,mutate,cm,label)
    for mutate, label in [
        (lambda d:d.update(strength=1.1),"event strength"),
        (lambda d:d.update(eventType="midiNoteOn"),"MIDI missing note"),
        (lambda d:d.update(note=36,channel=1),"onset with MIDI fields"),
        (lambda d:d.update(expiresAfterMs=0),"zero event expiry"),
    ]:
        rejects(event,mutate,cm,label)
    # Parser-level negative tests are independent of schemas.
    for text in ['{"a":1,"a":2}', '{"a":NaN}', '{"a":Infinity}']:
        try:
            json.loads(text,parse_constant=reject_constant,object_pairs_hook=unique_object)
        except ValidationFailure:
            count += 1
        else:
            raise AssertionError("Strict JSON parser unexpectedly accepted invalid data")
    return count


def main() -> int:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,default=Path(__file__).resolve().parent)
    parser.add_argument("--self-test",action="store_true")
    args=parser.parse_args()
    root=args.root.resolve()
    try:
        ps=read_json(root/"preset.schema.json");fs=read_json(root/"audio-feature-contract.schema.json")
        Draft202012Validator.check_schema(ps);Draft202012Validator.check_schema(fs)
        presets=sorted((root/"presets").glob("*.json"))
        if not presets: raise ValidationFailure("No preset examples found")
        for path in presets: validate_preset(read_json(path),ps,root)
        frames=sorted((root/"examples").glob("*.json"))
        for path in frames: validate_message(read_json(path),fs)
        print(f"PASS: 2 schemas, {len(presets)} preset(s), {len(frames)} message(s), semantic references and ISF header.")
        if args.self_test:
            count=run_self_tests(root,read_json(presets[0]),ps,read_json(root/"examples/feature-frame.json"),read_json(root/"examples/onset-event.json"),fs)
            print(f"PASS: {count} negative validation tests rejected invalid input.")
        print("NOT TESTED: GLSL compilation, rendering, DSP, C++ build, Live/Max/VST3, MIDI hardware or latency.")
        return 0
    except (ValidationFailure,OSError,ValueError,KeyError,AssertionError) as exc:
        print(f"FAIL: {exc}",file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
