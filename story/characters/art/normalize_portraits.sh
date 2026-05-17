#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROFILE_PATH="${SCRIPT_DIR}/portrait_normalization_profile.json"
VALIDATE_ONLY=0

usage() {
    cat <<'EOF'
usage: normalize_portraits.sh [--validate-only]
  --validate-only   Validate profile schema only; do not process images.
EOF
}

if [[ $# -gt 1 ]]; then
    usage >&2
    exit 1
fi
if [[ $# -eq 1 ]]; then
    case "$1" in
        --validate-only)
            VALIDATE_ONLY=1
            ;;
        *)
            usage >&2
            exit 1
            ;;
    esac
fi

if ! command -v jq >/dev/null 2>&1; then
    echo "error: 'jq' not found in PATH." >&2
    exit 1
fi
if [[ ${VALIDATE_ONLY} -eq 0 ]] && ! command -v magick >/dev/null 2>&1; then
    echo "error: ImageMagick 'magick' not found in PATH." >&2
    exit 1
fi

validate_profile_schema() {
    if ! jq -e '(keys | sort) == ["default","overrides","version"]' "${PROFILE_PATH}" >/dev/null; then
        echo "error: profile must contain exactly top-level keys: version, default, overrides" >&2
        exit 1
    fi
    if ! jq -e '.default | type == "object"' "${PROFILE_PATH}" >/dev/null; then
        echo "error: profile.default must be an object" >&2
        exit 1
    fi
    if ! jq -e '(.default | (keys - ["fuzz_percent"] | length) == 0)' "${PROFILE_PATH}" >/dev/null; then
        echo "error: profile.default contains unsupported keys (only fuzz_percent is allowed)" >&2
        exit 1
    fi
    if ! jq -e '.default.fuzz_percent | type == "number"' "${PROFILE_PATH}" >/dev/null; then
        echo "error: profile.default.fuzz_percent must be numeric" >&2
        exit 1
    fi
    if ! jq -e '.overrides | type == "object"' "${PROFILE_PATH}" >/dev/null; then
        echo "error: profile.overrides must be an object" >&2
        exit 1
    fi
    if ! jq -e '.overrides | keys | all(test("^[a-z0-9_]+_portrait\\.png$"))' "${PROFILE_PATH}" >/dev/null; then
        echo "error: override keys must be canonical portrait filenames (*_portrait.png)" >&2
        exit 1
    fi
    if ! jq -e \
        '.overrides | to_entries | all(
            .value | type == "object" and
            ((keys - ["fuzz_percent"]) | length) == 0 and
            ((has("fuzz_percent") | not) or (.fuzz_percent | type == "number"))
        )' \
        "${PROFILE_PATH}" >/dev/null; then
        echo "error: overrides contain unsupported keys or non-numeric fuzz_percent values" >&2
        exit 1
    fi
}

validate_profile_schema
if [[ ${VALIDATE_ONLY} -eq 1 ]]; then
    echo "portrait profile schema valid"
    exit 0
fi

tmp_dir="$(mktemp -d)"
cleanup() {
    rm -rf "${tmp_dir}"
}
trap cleanup EXIT

mapfile -t portrait_files < <(jq -r '.overrides | keys[]' "${PROFILE_PATH}" | sort)

for filename in "${portrait_files[@]}"; do
    input_path="${SCRIPT_DIR}/${filename}"
    if [[ ! -f "${input_path}" ]]; then
        echo "error: portrait not found: ${input_path}" >&2
        exit 1
    fi

    fuzz_percent="$(jq -r --arg k "${filename}" '.overrides[$k].fuzz_percent // .default.fuzz_percent' "${PROFILE_PATH}")"

    width="$(identify -format '%w' "${input_path}")"
    height="$(identify -format '%h' "${input_path}")"
    max_x="$((width - 1))"
    max_y="$((height - 1))"

    output_path="${tmp_dir}/${filename}"

    magick "${input_path}" \
        -alpha set \
        -fuzz "${fuzz_percent}%" \
        -fill none \
        -draw "color 0,0 floodfill" \
        -draw "color ${max_x},0 floodfill" \
        -draw "color 0,${max_y} floodfill" \
        -draw "color ${max_x},${max_y} floodfill" \
        -strip \
        -define png:compression-level=9 \
        -define png:compression-filter=5 \
        -define png:compression-strategy=1 \
        "${output_path}"

    mv "${output_path}" "${input_path}"
    echo "normalized ${filename}"
done

echo "portrait normalization complete: ${#portrait_files[@]} files"
