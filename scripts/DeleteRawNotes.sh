#!/usr/bin/env bash

set -euo pipefail

print_usage() {
    echo "Usage: $0 --eval <course/lab>" >&2
}

if [[ $# -ne 2 || $1 != "--eval" ]]; then
    print_usage
    exit 1
fi

evaluation=$2
if [[ -z $evaluation || $evaluation == /* ]]; then
    echo "Error: The evaluation must be a relative path such as prog2/lab1." >&2
    exit 1
fi

case "/$evaluation/" in
    *"/../"*|*"/./"*|*"//"*)
        echo "Error: The evaluation path contains an unsafe component." >&2
        exit 1
        ;;
esac

script_directory=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
repository_root=$(cd -- "$script_directory/.." && pwd -P)
data_directory=$(cd -- "$repository_root/data" && pwd -P)
lab_directory="$repository_root/data/$evaluation"

if [[ ! -d $lab_directory ]]; then
    echo "Error: Evaluation folder does not exist: $lab_directory" >&2
    exit 1
fi

lab_directory=$(cd -- "$lab_directory" && pwd -P)
case "$lab_directory/" in
    "$data_directory/"*) ;;
    *)
        echo "Error: The evaluation folder resolves outside data/." >&2
        exit 1
        ;;
esac

projects_directory="$lab_directory/projects"
if [[ ! -d $projects_directory ]]; then
    echo "Error: Projects folder does not exist: $projects_directory" >&2
    exit 1
fi

projects_directory=$(cd -- "$projects_directory" && pwd -P)
case "$projects_directory/" in
    "$lab_directory/"*) ;;
    *)
        echo "Error: The projects folder resolves outside the evaluation." >&2
        exit 1
        ;;
esac

mapfile -d '' raw_notes < <(
    find "$projects_directory" -type f -name 'raw_note.txt' -print0
)

raw_note_count=${#raw_notes[@]}
if (( raw_note_count == 0 )); then
    echo "No raw_note.txt files found for \"$evaluation\"."
    exit 0
fi

echo "Found $raw_note_count raw_note.txt file(s) under:"
echo "  $projects_directory"

read -r -p "Delete all of these raw notes? [y/N] " first_confirmation
case ${first_confirmation,,} in
    y|yes) ;;
    *)
        echo "Deletion cancelled."
        exit 0
        ;;
esac

read -r -p "This cannot be undone. Type DELETE to confirm: " second_confirmation
if [[ $second_confirmation != "DELETE" ]]; then
    echo "Deletion cancelled."
    exit 0
fi

for raw_note in "${raw_notes[@]}"; do
    rm -- "$raw_note"
done

echo "Deleted $raw_note_count raw_note.txt file(s). This cannot be recovered."
