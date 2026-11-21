#!/usr/bin/env bash
#
# Self-test for scripts/check-forbidden-apis.sh.
#
# Builds throwaway git repositories that hold a copy of SAFETY.md and checks that
# the guard passes a clean tree, fails on every planted list entry, skips the
# game/ submodule path, and still catches a match in SAFETY.md outside the list.
#
# The list entries are read from SAFETY.md at run time so this file never holds
# a forbidden name itself.
#
# Usage: tests/safety/forbidden_apis_selftest.sh [repo-root]

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "${1:-"$script_dir/../.."}" && pwd)"
guard="$root/scripts/check-forbidden-apis.sh"
safety="$root/SAFETY.md"

# The list in SAFETY.md has this many entries. A change to the list should be a
# deliberate edit here too, so an entry cannot vanish unnoticed.
expected_entries=30

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

failures=0

fail() {
    echo "FAIL: $*" >&2
    failures=$((failures + 1))
}

pass() {
    echo "ok:   $*"
}

# Make a fresh repository that holds only a copy of SAFETY.md and a clean file.
new_repo() {
    local dir="$1"
    mkdir -p "$dir"
    git -C "$dir" init -q
    cp "$safety" "$dir/SAFETY.md"
    printf 'int add(int a, int b) { return a + b; }\n' >"$dir/clean.c"
}

# Run the guard and return its exit code without tripping set -e.
run_guard() {
    local code=0
    bash "$guard" "$1" >"$work/last.out" 2>&1 || code=$?
    return "$code"
}

guard_code() {
    local code=0
    run_guard "$1" || code=$?
    echo "$code"
}

entries="$work/entries"
awk '
    $0 == "<!-- forbidden-api-list:begin -->" { inside = 1; next }
    $0 == "<!-- forbidden-api-list:end -->" { inside = 0; next }
    inside && match($0, /`[^`]+`/) { print substr($0, RSTART + 1, RLENGTH - 2) }
' "$safety" >"$entries"

count="$(grep -c . "$entries" || true)"
if [ "$count" -eq "$expected_entries" ]; then
    pass "SAFETY.md lists $expected_entries entries"
else
    fail "SAFETY.md lists $count entries, expected $expected_entries"
fi

# 1. The real repository is clean.
code="$(guard_code "$root")"
if [ "$code" -eq 0 ]; then
    pass "guard passes on this repository"
else
    fail "guard exit $code on this repository"
    cat "$work/last.out" >&2
fi

# 2. A clean throwaway tree passes.
new_repo "$work/clean"
code="$(guard_code "$work/clean")"
if [ "$code" -eq 0 ]; then
    pass "guard passes on a clean tree"
else
    fail "guard exit $code on a clean tree"
    cat "$work/last.out" >&2
fi

# 3. Every entry fails the guard when planted in an owned file.
n=0
while IFS= read -r entry; do
    [ -n "$entry" ] || continue
    n=$((n + 1))
    repo="$work/plant-$n"
    new_repo "$repo"
    mkdir -p "$repo/src"
    printf 'void f(void) { %s(); }\n' "$entry" >"$repo/src/planted.c"
    code="$(guard_code "$repo")"
    if [ "$code" -eq 1 ] && grep -q 'src/planted.c:1:' "$work/last.out"; then
        pass "guard fails on planted entry $n"
    else
        fail "guard exit $code on planted entry $n ($entry)"
        cat "$work/last.out" >&2
    fi
done <"$entries"

first="$(head -n 1 "$entries")"

# 4. Matching ignores case.
repo="$work/case"
new_repo "$repo"
printf '%s\n' "$first" | tr '[:lower:]' '[:upper:]' >"$repo/upper.txt"
code="$(guard_code "$repo")"
if [ "$code" -eq 1 ]; then
    pass "guard ignores case"
else
    fail "guard exit $code on an upper-case match"
fi

# 5. A match under game/ is skipped because that path is the upstream submodule.
repo="$work/game"
new_repo "$repo"
mkdir -p "$repo/game"
printf '%s\n' "$first" >"$repo/game/upstream.cpp"
code="$(guard_code "$repo")"
if [ "$code" -eq 0 ]; then
    pass "guard skips the game/ submodule path"
else
    fail "guard exit $code with a match under game/"
    cat "$work/last.out" >&2
fi

# 6. A match in SAFETY.md outside the list block is still caught.
repo="$work/safety"
new_repo "$repo"
printf '\nStray mention: %s\n' "$first" >>"$repo/SAFETY.md"
code="$(guard_code "$repo")"
if [ "$code" -eq 1 ] && grep -q '^SAFETY.md:' "$work/last.out"; then
    pass "guard catches a match in SAFETY.md outside the list"
else
    fail "guard exit $code with a stray match in SAFETY.md"
    cat "$work/last.out" >&2
fi

# 7. Ignored files are not ours to scan, untracked ones are.
repo="$work/ignore"
new_repo "$repo"
printf 'build/\n' >"$repo/.gitignore"
mkdir -p "$repo/build"
printf '%s\n' "$first" >"$repo/build/generated.txt"
code="$(guard_code "$repo")"
if [ "$code" -eq 0 ]; then
    pass "guard skips ignored files"
else
    fail "guard exit $code with a match in an ignored file"
fi

# 8. A list with no entries is a setup error, not a silent pass.
repo="$work/empty"
new_repo "$repo"
awk '
    $0 == "<!-- forbidden-api-list:begin -->" { print; inside = 1; next }
    $0 == "<!-- forbidden-api-list:end -->" { inside = 0 }
    !inside { print }
' "$safety" >"$repo/SAFETY.md"
code="$(guard_code "$repo")"
if [ "$code" -eq 2 ]; then
    pass "guard refuses an empty list"
else
    fail "guard exit $code with an empty list, expected 2"
fi

if [ "$failures" -gt 0 ]; then
    echo "forbidden-api self-test: $failures check(s) failed" >&2
    exit 1
fi

echo "forbidden-api self-test: all checks passed"
