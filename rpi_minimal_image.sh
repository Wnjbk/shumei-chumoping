#!/usr/bin/env bash
# Create a small, first-boot-expandable Raspberry Pi OS image from an SD card.
# Run this on a separate Linux host, not from the card being copied.

set -Eeuo pipefail

PISHRINK_URL="https://raw.githubusercontent.com/Drewsif/PiShrink/master/pishrink.sh"
WORK_DIR=""
SOURCE=""
OUTPUT=""
PISHRINK_SOURCE=""
KEEP_RAW=1

usage() {
    cat <<'EOF'
Usage:
  sudo bash rpi_minimal_image.sh --source /dev/sdX --output /path/pi-min.img

Options:
  --source DEV       Whole SD device, for example /dev/sdb, never /dev/sdb2
  --output FILE      Final shrunk image
  --workdir DIR      Directory for the raw intermediate image
  --pishrink FILE    Use a locally saved PiShrink script
  --delete-raw       Delete the raw intermediate after a successful shrink
  -h, --help         Show this help
EOF
}

die() {
    echo "ERROR: $*" >&2
    exit 1
}

need() {
    command -v "$1" >/dev/null 2>&1 || die "missing command: $1"
}

cleanup() {
    set +e
    if [[ -n "${LOOP_DEV:-}" ]]; then
        udevadm settle >/dev/null 2>&1
        losetup -d "$LOOP_DEV" >/dev/null 2>&1
    fi
}
trap cleanup EXIT

while [[ $# -gt 0 ]]; do
    case "$1" in
        --source) SOURCE="${2:-}"; shift 2 ;;
        --output) OUTPUT="${2:-}"; shift 2 ;;
        --workdir) WORK_DIR="${2:-}"; shift 2 ;;
        --pishrink) PISHRINK_SOURCE="${2:-}"; shift 2 ;;
        --delete-raw) KEEP_RAW=0; shift ;;
        -h|--help) usage; exit 0 ;;
        *) usage >&2; die "unknown option: $1" ;;
    esac
done

[[ "$(id -u)" == 0 ]] || die "run as root"
[[ -n "$SOURCE" && -n "$OUTPUT" ]] || { usage >&2; exit 2; }

for cmd in lsblk dd e2fsck resize2fs losetup udevadm blockdev numfmt; do
    need "$cmd"
done
if [[ -z "$PISHRINK_SOURCE" ]]; then
    need curl
fi

SOURCE="$(readlink -f "$SOURCE")"
OUTPUT="$(readlink -m "$OUTPUT")"
[[ -b "$SOURCE" ]] || die "$SOURCE is not a block device"
[[ "$(lsblk -dnro TYPE "$SOURCE")" == "disk" ]] || die "--source must be a whole disk"
[[ "$SOURCE" != "$(findmnt -no SOURCE / 2>/dev/null || true)" ]] || die "refusing to copy the running root disk"

PARTS="$(lsblk -lnpo NAME,TYPE,FSTYPE "$SOURCE")"
BOOT_PART="$(awk '$2 == "part" && $3 == "vfat" { print $1; exit }' <<< "$PARTS")"
ROOT_PART="$(awk '$2 == "part" && $3 == "ext4" { print $1; exit }' <<< "$PARTS")"
[[ -n "$BOOT_PART" ]] || die "could not find a vfat boot partition"
[[ -n "$ROOT_PART" ]] || die "could not find an ext4 root partition"

MOUNTED_PARTS="$(lsblk -lnpo NAME,MOUNTPOINT "$SOURCE" | awk 'NF > 1 && $2 != "" { print $1 " -> " $2 }')"
[[ -z "$MOUNTED_PARTS" ]] || die "source partitions are mounted:\n$MOUNTED_PARTS\nunmount them before running"

SOURCE_SIZE="$(blockdev --getsize64 "$SOURCE")"
ROOT_START="$(cat "/sys/class/block/$(basename "$ROOT_PART")/start")"
ROOT_SIZE="$(blockdev --getsize64 "$ROOT_PART")"
echo "Source : $SOURCE ($(numfmt --to=iec "$SOURCE_SIZE"))"
echo "Boot   : $BOOT_PART"
echo "Root   : $ROOT_PART (offset $((ROOT_START * 512)) bytes, $(numfmt --to=iec "$ROOT_SIZE"))"
echo "Output : $OUTPUT"
read -r -p "Type the source device path to continue [$SOURCE]: " CONFIRM
[[ "$CONFIRM" == "$SOURCE" ]] || die "source confirmation did not match"

if [[ -z "$WORK_DIR" ]]; then
    WORK_DIR="$(dirname "$OUTPUT")/rpi-image-work"
fi
mkdir -p "$WORK_DIR"
mkdir -p "$(dirname "$OUTPUT")"
RAW="$WORK_DIR/$(basename "$SOURCE").raw.img"
PISHRINK="$WORK_DIR/pishrink.sh"

[[ ! -e "$RAW" ]] || die "raw file already exists: $RAW; remove it or choose another workdir"
[[ ! -e "$OUTPUT" ]] || die "output already exists: $OUTPUT; move it or choose another path"

echo "[1/4] Copying the complete card with dd..."
dd if="$SOURCE" of="$RAW" bs=16M iflag=fullblock status=progress conv=fsync
sync

echo "[2/3] Downloading PiShrink..."
if [[ -n "$PISHRINK_SOURCE" ]]; then
    PISHRINK_SOURCE="$(readlink -f "$PISHRINK_SOURCE")"
    [[ -f "$PISHRINK_SOURCE" ]] || die "PiShrink file not found: $PISHRINK_SOURCE"
    if [[ "$PISHRINK_SOURCE" != "$PISHRINK" ]]; then
        cp -- "$PISHRINK_SOURCE" "$PISHRINK"
    fi
    echo "Using local PiShrink: $PISHRINK_SOURCE"
else
    echo "Downloading PiShrink..."
    curl --fail --location --retry 3 --output "$PISHRINK" "$PISHRINK_URL"
fi
chmod 0755 "$PISHRINK"
bash -n "$PISHRINK"

echo "[3/3] Shrinking the copy and enabling first-boot expansion..."
# PiShrink keeps the image bootable, moves the final partition boundary,
# and installs its resize-on-first-boot service by default.
"$PISHRINK" "$RAW" "$OUTPUT"

echo "Final image: $OUTPUT"
lsblk -f "$OUTPUT" 2>/dev/null || true

if [[ "$KEEP_RAW" == 0 ]]; then
    rm -f -- "$RAW"
    echo "Raw intermediate removed: $RAW"
else
    echo "Raw intermediate kept: $RAW"
fi
echo "Write the final image with Raspberry Pi Imager or: dd if=$OUTPUT of=/dev/sdX bs=16M status=progress conv=fsync"
echo "The first boot expands the root filesystem to fill the new card."
