#!/usr/bin/env bash
# Prints the CHANGELOG.md section of one version as GitHub release text (empty subsections left
# out), or exits with an error if there is no such section.
#
#   scripts/release-notes.sh 1.2.3 [CHANGELOG.md]
#   scripts/release-notes.sh Unreleased          # preview what the next release will say
#   scripts/release-notes.sh --latest            # prints the latest released version only
set -euo pipefail

file="${2:-$(dirname "$0")/../CHANGELOG.md}"

if [ "${1:-}" = "--latest" ]; then
  version=$(sed -n 's/^## \[\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\)\].*/\1/p' "$file" | head -n 1)
  [ -n "$version" ] || { echo "no released version in $file" >&2; exit 1; }
  echo "$version"
  exit 0
fi

version="${1:?usage: release-notes.sh <version>|Unreleased|--latest [changelog]}"
notes=$(awk -v ver="$version" '
  /^## \[/ { if (on) exit; on = (index($0, "[" ver "]") == 4); next }
  !on { next }
  /^### / { heading = $0; next }
  /^[[:space:]]*$/ { next }
  { if (heading != "") { if (printed) print ""; print heading; heading = ""; printed = 1 } print }
' "$file")

if [ -z "$notes" ]; then
  echo "no entries for $version in $file" >&2
  exit 1
fi
printf '%s\n' "$notes"
