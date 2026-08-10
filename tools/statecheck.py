#!/usr/bin/env python3
# tools/statecheck.py — THE FENCE CTEST (CTL-14b, D-043; hard M1 gate).
# (1) Walks the include closure of the policy TU (control/policy.cpp + its headers)
#     and asserts it cannot reach a true-state header (physics_tier0.h, sim.h,
#     transport.h, vertical.h, gs.h, gs_free.h).
# (2) Token-scans control/ sources for true-state identifiers.
# (3) Asserts the poison guards exist (the compile-time arm of the fence).
import os, re, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else "."
FENCE_TUS = ["control/policy.cpp", "control/policy.h", "control/obs.h"]
FORBIDDEN_HEADERS = ["physics_tier0.h", "sim.h", "sim_env.h", "transport.h", "vertical.h",
                     "gs.h", "gs_free.h", "philox.h"]   # sim_env.h = the plant (D-048)
ALLOWED_PROJECT = {"control/obs.h", "control/policy.h", "core/config.h"}
FORBIDDEN_TOKENS = [r"\bState\b", r"\bxvert\b", r"\bTier1Burn\b", r"\bvs_truth\b",
                    r"\brun_sim\b", r"\bVerticalModel\b", r"\bGsFreeSolver\b"]

def resolve(inc, cur_dir):
    for base in (cur_dir, ROOT, os.path.join(ROOT, "core"), os.path.join(ROOT, "control")):
        p = os.path.normpath(os.path.join(base, inc))
        if os.path.isfile(p):
            return os.path.relpath(p, ROOT).replace("\\", "/")
    return None

def closure(start):
    seen, stack = set(), [start]
    while stack:
        f = stack.pop()
        if f in seen:
            continue
        seen.add(f)
        try:
            text = open(os.path.join(ROOT, f), encoding="utf-8").read()
        except OSError:
            continue
        for mm in re.finditer(r'#include\s+"([^"]+)"', text):
            r = resolve(mm.group(1), os.path.dirname(os.path.join(ROOT, f)))
            if r:
                stack.append(r)
    return seen

fail = 0
for tu in FENCE_TUS:
    cl = closure(tu)
    for f in sorted(cl):
        base = os.path.basename(f)
        if base in FORBIDDEN_HEADERS:
            print(f"STATECHECK RED: {tu} closure reaches {f}")
            fail = 1
        if f.startswith("core/") and f not in ALLOWED_PROJECT:
            print(f"STATECHECK RED: {tu} closure reaches non-allowed {f}")
            fail = 1

for tu in FENCE_TUS:
    text = open(os.path.join(ROOT, tu), encoding="utf-8").read()
    body = re.sub(r"//[^\n]*", "", text)          # comments may NAME things; code may not
    for tok in FORBIDDEN_TOKENS:
        for mm in re.finditer(tok, body):
            print(f"STATECHECK RED: token {tok} in {tu}")
            fail = 1

for hdr, guard in [("core/physics_tier0.h", "FUSION_OBS_FENCE"),
                   ("core/sim.h", "FUSION_OBS_FENCE")]:
    text = open(os.path.join(ROOT, hdr), encoding="utf-8").read()
    if f"#ifdef {guard}" not in text or "#error" not in text:
        print(f"STATECHECK RED: poison guard missing in {hdr}")
        fail = 1

if fail:
    sys.exit(1)
print("STATECHECK GREEN (the policy runtime cannot see the true state; poison armed)")
