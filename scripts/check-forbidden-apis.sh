#!/usr/bin/env bash
#
# Forbidden-API guard.
#
# Scans every file we own for the process-access and input-injection APIs listed
# in SAFETY.md and exits non-zero if any of them appears. SAFETY.md is the single
# source of truth for the list. See the "Forbidden-API guard" section there.
#
# Usage: scripts/check-forbidden-apis.sh [repo-root]
#
# The repo root defaults to the repository that contains this script. Files are
# the ones git knows about (tracked, plus untracked files that are not ignored).
# The game/ submodule and the list block inside SAFETY.md are skipped.
#
# Exit codes: 0 clean, 1 forbidden API found, 2 usage or setup error.

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="${1:-"$script_dir/.."}"

if ! root="$(cd "$root" 2>/dev/null && pwd)"; then
    echo "forbidden-api guard: cannot open repository root '${1:-}'" >&2
    exit 2
fi

safety="$root/SAFETY.md"
begin_marker='<!-- forbidden-api-list:begin -->'
end_marker='<!-- forbidden-api-list:end -->'

if [ ! -f "$safety" ]; then
    echo "forbidden-api guard: $safety not found" >&2
    exit 2
fi

if ! git -C "$root" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "forbidden-api guard: $root is not a git work tree" >&2
    exit 2
fi

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

if ! grep -qxF "$begin_marker" "$safety" || ! grep -qxF "$end_marker" "$safety"; then
    echo "forbidden-api guard: list markers missing from SAFETY.md" >&2
    exit 2
fi

# Pull the backticked entries out of the list block in SAFETY.md.
awk -v b="$begin_marker" -v e="$end_marker" '
    $0 == b { inside = 1; next }
    $0 == e { inside = 0; next }
    inside && match($0, /`[^`]+`/) { print substr($0, RSTART + 1, RLENGTH - 2) }
' "$safety" >"$work/patterns"

count="$(grep -c . "$work/patterns" || true)"
if [ "$count" -eq 0 ]; then
    echo "forbidden-api guard: no entries found in the SAFETY.md list" >&2
    exit 2
fi

# Scan every file we own: tracked files plus untracked files that are not
# ignored. The game submodule and SAFETY.md are left out here. git grep does not
# descend into submodules, and -I skips binary files.
: >"$work/hits"
status=0
git -C "$root" grep -n -I -i -F --untracked -f "$work/patterns" -- \
    . ':(exclude)game' ':(exclude)SAFETY.md' >>"$work/hits" || status=$?
if [ "$status" -gt 1 ]; then
    echo "forbidden-api guard: git grep failed while scanning files" >&2
    exit 2
fi

# Scan SAFETY.md with the list block blanked out, keeping line numbers intact.
awk -v b="$begin_marker" -v e="$end_marker" '
    $0 == b { inside = 1; print ""; next }
    $0 == e { inside = 0; print ""; next }
    { print inside ? "" : $0 }
' "$safety" | { grep -n -i -F -f "$work/patterns" || true; } |
    sed 's/^/SAFETY.md:/' >>"$work/hits"

if [ -s "$work/hits" ]; then
    echo "forbidden-api guard: FAILED. Forbidden APIs found (see SAFETY.md):" >&2
    cat "$work/hits" >&2
    exit 1
fi

echo "forbidden-api guard: OK ($count entries checked)"
