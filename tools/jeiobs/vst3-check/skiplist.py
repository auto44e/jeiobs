"""Read and edit the files of one skip-list check (task 005) for skip-list.sh.
Always run as python3 -I.

  skiplist.py entry SKIP BUNDLE BINARY FAILURES
      SKIP (vst3skip.json) has exactly one entry for BUNDLE; it names BINARY,
      records BINARY's current size and CRC32, `failures` FAILURES and an
      obs_version
  skiplist.py noentry SKIP BUNDLE       no entry for BUNDLE (no SKIP file is none)
  skiplist.py backoff SKIP BUNDLE       BUNDLE's retry_after is 7 days after its
                                        last_failed (in seconds, ms, us, 100 ns or ns)
  skiplist.py past SKIP BUNDLE          move retry_after to 7 days before last_failed
  skiplist.py version SKIP BUNDLE [NEW] print obs_version; with NEW, also set it
  skiplist.py v1list LIST               LIST (vst3list.json) has only version 1 and
                                        plugins, each with exactly the version-1 keys
  skiplist.py at LIST ID BUNDLE         LIST has class ID exactly once, from BUNDLE
  skiplist.py nskipped OUT [N]          one "VST3_SKIPPED <n>" line (n = N if given)
                                        in scanner stdout OUT, before VST3_SCAN_DONE
  skiplist.py loads LOG MODULE N        LOG (loads.log, may be missing) names
                                        MODULE's real path on exactly N lines
  skiplist.py flip MODULE               toggle the last byte of the "vst3chk-flip:"
                                        string, in place
  skiplist.py flipped MODULE ORIGINAL   MODULE differs from ORIGINAL in exactly one
                                        byte but has its size and mtime (ns)
  skiplist.py snap DIR OUT              save names, sizes, mtimes and SHA-256s in DIR
  skiplist.py same DIR SNAP             DIR still matches that snapshot

Prints a short summary for the PASS/FAIL line; exits 0 only if the condition
holds (or the edit was made). Missing or malformed input fails.
"""
import hashlib
import json
import os
import re
import sys
import zlib

V1_CLASS_KEYS = {"name", "id", "path", "pluginName", "discardable", "isFX"}
FLIP = b"vst3chk-flip:"
WEEK_S = 7 * 24 * 3600
UNITS = (("s", 1), ("ms", 10**3), ("us", 10**6), ("100ns", 10**7), ("ns", 10**9))


def load_skip(path):
    with open(path, encoding="utf-8") as f:
        root = json.load(f)
    if root.get("version") != 1 or not isinstance(root.get("bundles"), list):
        raise ValueError(f"{os.path.basename(path)} is not a version 1 skip list with a bundles array")
    return root


def entries(root, bundle):
    return [e for e in root["bundles"] if isinstance(e, dict) and e.get("path") == bundle]


def the_entry(root, bundle):
    hits = entries(root, bundle)
    if len(hits) != 1:
        raise ValueError(f"{len(hits)} entries for the bundle")
    return hits[0]


def save_skip(path, root):
    with open(path + ".vst3check", "w", encoding="utf-8") as f:
        json.dump(root, f, indent=4)
    os.replace(path + ".vst3check", path)


def integer(entry, key):
    value = entry.get(key)
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError(f"{key} is {value!r}, not an integer")
    return value


def crc32_of(path):
    crc = 0
    with open(path, "rb") as f:
        while chunk := f.read(65536):
            crc = zlib.crc32(chunk, crc)
    return crc


def same_crc(stored, want):
    """An integer (also a sign-extended 32-bit one) or a hex or decimal string."""
    if isinstance(stored, bool):
        return False
    if isinstance(stored, int):
        return stored % 2**32 == want
    if isinstance(stored, str):
        for base in (16, 10):
            try:
                if int(stored, base) % 2**32 == want:
                    return True
            except ValueError:
                pass
    return False


def describe(e):
    return (f"kind {e.get('kind')!r}, failures {e.get('failures')!r}, reason {e.get('reason')!r}, "
            f"obs_version {e.get('obs_version')!r}")


def entry(skip, bundle, binary, failures):
    e = the_entry(load_skip(skip), bundle)
    st = os.stat(binary)
    stored = e.get("binary")
    problems = []
    if not isinstance(stored, str) or os.path.normpath(os.path.join(bundle, stored)) != os.path.normpath(binary):
        problems.append(f"binary {stored!r}")
    if e.get("size") != st.st_size or isinstance(e.get("size"), bool):
        problems.append(f"size {e.get('size')!r} != {st.st_size}")
    crc = crc32_of(binary)
    if not same_crc(e.get("crc32"), crc):
        problems.append(f"crc32 {e.get('crc32')!r} != {crc}")
    if e.get("failures") != int(failures) or isinstance(e.get("failures"), bool):
        problems.append(f"failures {e.get('failures')!r} != {failures}")
    if not isinstance(e.get("obs_version"), str) or not e.get("obs_version"):
        problems.append(f"obs_version {e.get('obs_version')!r}")
    if problems:
        return False, "entry wrong: " + "; ".join(problems) + f" ({describe(e)})"
    return True, f"entry ({describe(e)}; binary, size and crc32 match)"


def noentry(skip, bundle):
    if not os.path.exists(skip):
        return True, "no entry (no skip file)"
    root = load_skip(skip)
    hits = entries(root, bundle)
    if hits:
        return False, f"{len(hits)} entry/entries ({describe(hits[0])})"
    return True, f"no entry ({len(root['bundles'])} other(s))"


def week(e):
    """retry_after - last_failed and the unit in which that is 7 days (or None)."""
    delta = integer(e, "retry_after") - integer(e, "last_failed")
    for name, per_second in UNITS:
        if abs(delta - WEEK_S * per_second) <= 120 * per_second:
            return delta, name
    return delta, None


def backoff(skip, bundle):
    delta, unit = week(the_entry(load_skip(skip), bundle))
    if unit is None:
        return False, f"retry_after - last_failed = {delta}, not 7 days in any unit"
    return True, f"retry_after = last_failed + 7 d ({unit})"


def past(skip, bundle):
    root = load_skip(skip)
    e = the_entry(root, bundle)
    delta, unit = week(e)
    if unit is None:
        return False, f"no 7-day backoff to move (retry_after - last_failed = {delta})"
    e["retry_after"] = integer(e, "last_failed") - delta
    save_skip(skip, root)
    return True, "retry_after moved to last_failed - 7 d"


def version(skip, bundle, *new):
    root = load_skip(skip)
    e = the_entry(root, bundle)
    old = e.get("obs_version")
    if not isinstance(old, str) or not old:
        return False, f"obs_version {old!r}"
    if new:
        e["obs_version"] = new[0]
        save_skip(skip, root)
    return True, old


def v1list(path):
    with open(path, encoding="utf-8") as f:
        root = json.load(f)
    keys = sorted(root)
    if keys != ["plugins", "version"] or root["version"] != 1 or not isinstance(root["plugins"], list):
        return False, f"vst3list.json has keys {keys}, version {root.get('version')!r}"
    odd = [sorted(p) for p in root["plugins"] if not isinstance(p, dict) or set(p) != V1_CLASS_KEYS]
    if odd:
        return False, f"vst3list.json has {len(odd)} class(es) with other keys, e.g. {odd[0]}"
    return True, f"vst3list.json: version 1 and plugins only, {len(root['plugins'])} classes with v1 keys"


def at(path, class_id, bundle):
    with open(path, encoding="utf-8") as f:
        paths = [p.get("path") for p in json.load(f)["plugins"] if p.get("id") == class_id]
    return paths == [bundle], f"class {class_id[:8]} from {paths}"


def nskipped(out, *want):
    with open(out, encoding="utf-8", errors="replace") as f:
        lines = [l.rstrip("\r") for l in f.read().splitlines()]
    found = [(i, m.group(1)) for i, l in enumerate(lines) if (m := re.fullmatch(r"VST3_SKIPPED (\d+)", l))]
    done = [i for i, l in enumerate(lines) if l == "VST3_SCAN_DONE"]
    if len(found) != 1:
        return False, f"{len(found)} VST3_SKIPPED lines"
    at, n = found[0]
    if not done or done[-1] < at:
        return False, f"VST3_SKIPPED {n} not before VST3_SCAN_DONE"
    if want and n != want[0]:
        return False, f"VST3_SKIPPED {n}, not {want[0]}"
    return True, f"VST3_SKIPPED {n}"


def loads(log, module, count):
    real = os.path.realpath(module)
    try:
        with open(log, encoding="utf-8", errors="replace") as f:
            n = sum(1 for l in f.read().splitlines() if l == real)
    except FileNotFoundError:
        n = 0
    return n == int(count), f"loaded x{n}"


def flip(module):
    with open(module, "r+b") as f:
        data = f.read()
        if data.count(FLIP) != 1:
            return False, f"{data.count(FLIP)} flip markers"
        at = data.index(FLIP) + len(FLIP)
        f.seek(at)
        f.write(b"1" if data[at:at + 1] == b"0" else b"0")
    return True, f"byte {at} flipped"


def flipped(module, original):
    a, b = os.stat(module), os.stat(original)
    with open(module, "rb") as f, open(original, "rb") as g:
        x, y = f.read(), g.read()
    differing = sum(1 for p, q in zip(x, y) if p != q)
    ok = a.st_size == b.st_size and a.st_mtime_ns == b.st_mtime_ns and len(x) == len(y) and differing == 1
    return ok, (f"{differing} byte(s) differ, size {a.st_size} vs {b.st_size}, mtime_ns "
                f"{a.st_mtime_ns} vs {b.st_mtime_ns}, crc32 {crc32_of(module):08x} vs {crc32_of(original):08x}")


def listing(directory):
    result = {}
    for name in sorted(os.listdir(directory)):
        path = os.path.join(directory, name)
        st = os.lstat(path)
        digest = None
        if os.path.isfile(path) and not os.path.islink(path):
            with open(path, "rb") as f:
                digest = hashlib.sha256(f.read()).hexdigest()
        result[name] = [st.st_mode, st.st_size, st.st_mtime_ns, digest]
    return result


def snap(directory, out):
    files = listing(directory)
    with open(out, "w", encoding="utf-8") as f:
        json.dump(files, f)
    return True, f"{len(files)} entries"


def same(directory, saved):
    with open(saved, encoding="utf-8") as f:
        before = json.load(f)
    now = listing(directory)
    added = sorted(set(now) - set(before))
    removed = sorted(set(before) - set(now))
    changed = sorted(n for n in set(now) & set(before) if now[n] != before[n])
    if added or removed or changed:
        return False, f"cache dir changed: added {added}, removed {removed}, modified {changed}"
    return True, f"cache dir unchanged ({len(now)} entries: names, sizes, mtimes, contents)"


COMMANDS = {
    "entry": entry, "noentry": noentry, "backoff": backoff, "past": past, "version": version,
    "v1list": v1list, "at": at, "nskipped": nskipped, "loads": loads, "flip": flip, "flipped": flipped,
    "snap": snap, "same": same,
}


def main(argv):
    try:
        ok, summary = COMMANDS[argv[1]](*argv[2:])
    except OSError as e:  # missing or malformed input never passes
        ok, summary = False, f"unreadable ({os.path.basename(e.filename or '?')}: {e.strerror})"
    except Exception as e:
        ok, summary = False, f"unreadable ({type(e).__name__}: {e})"
    print(summary)
    return 0 if ok else 1


sys.exit(main(sys.argv))
