"""Read one vst3-check run's output for run.sh (always run as python3 -I).

  results.py ids LIST_JSON WANT...          class ids in a scanner list.json
  results.py names OBS_LOG WANT...          effect names OBS listed in its log
  results.py skips LOG BUNDLE N [timeout|killed]
                                            "[VST3 Scanner] Skipped BUNDLE: <reason>"

Prints a short summary for the PASS/FAIL line. Exits 0 only if the ids or
names are exactly WANT (as a multiset), or if exactly N skip lines name BUNDLE,
each with a timeout reason when "timeout" is given, or with exactly the
reason for a probe the scanner had to kill itself when "killed" is given.
Unreadable input fails.
"""
import collections
import json
import sys

LISTED = "[VST3 Scanner]   "
SKIPPED = "[VST3 Scanner] Skipped "
KILLED = "probe timed out (killed)"
REASONS = {"timeout": ("probe timed out", KILLED), "killed": (KILLED,)}


def compare(what, got, want):
    got, want = collections.Counter(got), collections.Counter(want)
    if got == want:
        return True, f"the {sum(want.values())} expected {what}"
    missing = ", ".join(sorted((want - got).elements())) or "none"
    extra = ", ".join(sorted((got - want).elements())) or "none"
    return False, f"{sum(got.values())} {what}, missing [{missing}], unexpected [{extra}]"


def lines(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read().splitlines()


def ids(path, *want):
    with open(path, encoding="utf-8") as f:
        plugins = json.load(f)["plugins"]
    return compare("classes", [p["id"] for p in plugins], want)


def names(path, *want):
    return compare("names listed", [l.split(LISTED, 1)[1] for l in lines(path) if LISTED in l], want)


def skips(path, bundle, count, *mode):
    if len(mode) > 1 or (mode and mode[0] not in REASONS):
        raise ValueError(f"unknown mode {mode}")
    allowed = REASONS[mode[0]] if mode else None
    reasons = []
    for line in lines(path):
        rest = line.split(SKIPPED, 1)[1] if SKIPPED in line else ""
        if rest.startswith(bundle + ": "):
            reasons.append(rest[len(bundle) + 2 :])
    ok = len(reasons) == int(count) and (allowed is None or all(r in allowed for r in reasons))
    return ok, f"skipped x{len(reasons)}" + (f" ({'; '.join(reasons)})" if reasons else "")


def main(argv):
    try:
        ok, summary = {"ids": ids, "names": names, "skips": skips}[argv[1]](*argv[2:])
    except Exception as e:  # missing or malformed input never passes
        ok, summary = False, f"unreadable ({type(e).__name__}: {e})"
    print(summary)
    return 0 if ok else 1


sys.exit(main(sys.argv))
