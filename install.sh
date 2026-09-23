#!/usr/bin/env bash
# ILI79505A DSI display + ILITEK_TDDI I2C touch installer for Raspberry Pi OS.
# From a persistent checkout: sudo bash install.sh [--user LOGIN]
# Read-only prerequisites check: bash install.sh --check
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$PWD/install.sh}")" && pwd)"
KERNEL_VER="$(uname -r)"
KERNEL_BUILD="/lib/modules/$KERNEL_VER/build"
BOOT_DIR=/boot
[[ -d /boot/firmware ]] && BOOT_DIR=/boot/firmware
MODULES_DIR="/lib/modules/$KERNEL_VER"
CHECK_ONLY=0
TARGET_USER="${SUDO_USER:-}"

while (($#)); do
    case "$1" in
        --check) CHECK_ONLY=1 ;;
        --user)
            [[ $# -ge 2 ]] || { echo 'ERROR: --user requires a login name' >&2; exit 2; }
            TARGET_USER="$2"; shift ;;
        -h|--help)
            echo 'Usage: sudo bash install.sh [--user LOGIN] | bash install.sh --check [--user LOGIN]'
            exit 0 ;;
        *) echo "ERROR: unknown option: $1" >&2; exit 2 ;;
    esac
    shift
done

for source in "$REPO_DIR/Makefile" "$REPO_DIR/panel-ili79505a.c" \
              "$REPO_DIR/ili79505a_test_cmds.h" "$REPO_DIR/ili79505a-overlay.dts" \
              "$REPO_DIR/ilitek_v3/Makefile" "$REPO_DIR/ilitek_v3/ilitek_v3.c" \
              "$REPO_DIR/touch_calib.py" "$REPO_DIR/ilitek-v3-polling.conf"; do
    [[ -f "$source" ]] || { echo "ERROR: missing source: $source" >&2; exit 1; }
done
[[ -f "$BOOT_DIR/config.txt" ]] || { echo "ERROR: missing $BOOT_DIR/config.txt" >&2; exit 1; }
[[ -d "$MODULES_DIR" ]] || { echo "ERROR: missing $MODULES_DIR" >&2; exit 1; }

if [[ $(uname -m) == aarch64 ]] && command -v dpkg >/dev/null && \
   [[ $(dpkg --print-architecture) != arm64 ]]; then
    echo 'ERROR: 64-bit kernel needs a 64-bit Raspberry Pi OS userland' >&2
    exit 1
fi

if [[ -z "$TARGET_USER" || "$TARGET_USER" == root ]] && [[ $CHECK_ONLY == 0 ]]; then
    echo 'ERROR: run via sudo from the desktop user, or pass --user LOGIN' >&2
    exit 1
fi
TARGET_HOME=''
if [[ -n "$TARGET_USER" ]]; then
    TARGET_HOME="$(getent passwd "$TARGET_USER" | cut -d: -f6)"
    [[ -n "$TARGET_HOME" && -d "$TARGET_HOME" ]] || {
        echo "ERROR: home directory not found for $TARGET_USER" >&2; exit 1;
    }
fi

echo "ILI79505A: kernel=$KERNEL_VER, boot=$BOOT_DIR, source=$REPO_DIR"
if [[ $CHECK_ONLY == 1 ]]; then
    for tool in make gcc dtc python3; do
        command -v "$tool" >/dev/null && echo "OK: $tool" || echo "MISSING: $tool"
    done
    [[ -f "$KERNEL_BUILD/Makefile" ]] && echo 'OK: matching kernel headers' || echo 'MISSING: matching kernel headers'
    echo "Target user: ${TARGET_USER:-unspecified (pass --user LOGIN to install)}"
    echo 'Check only: no build, installation, firmware write, or reboot performed.'
    exit 0
fi
[[ $EUID -eq 0 ]] || { echo 'ERROR: run as root with sudo' >&2; exit 1; }

# Only install dependencies if absent; do not upgrade a working kernel.
packages=()
command -v make >/dev/null && command -v gcc >/dev/null || packages+=(build-essential)
command -v dtc >/dev/null || packages+=(device-tree-compiler)
command -v python3 >/dev/null || packages+=(python3)
if [[ ! -f "$KERNEL_BUILD/Makefile" ]]; then
    packages+=("linux-headers-$KERNEL_VER")
fi
if ((${#packages[@]})); then
    apt-get update
    apt-get install -y "${packages[@]}" || {
        [[ -f "$KERNEL_BUILD/Makefile" ]] || apt-get install -y raspberrypi-kernel-headers
    }
fi
[[ -f "$KERNEL_BUILD/Makefile" ]] || { echo 'ERROR: matching kernel headers unavailable' >&2; exit 1; }
for tool in make gcc dtc python3; do
    command -v "$tool" >/dev/null || { echo "ERROR: $tool unavailable after dependency install" >&2; exit 1; }
done

echo '[1/5] Build both drivers in the persistent source directory'
make -C "$KERNEL_BUILD" M="$REPO_DIR/ilitek_v3" modules
[[ -s "$REPO_DIR/ilitek_v3/Module.symvers" ]] || { echo 'ERROR: Ilitek symbol table missing' >&2; exit 1; }
make -C "$REPO_DIR" modules
mkdir -p "$REPO_DIR/build"
dtc -@ -I dts -O dtb -o "$REPO_DIR/build/ili79505a.dtbo" "$REPO_DIR/ili79505a-overlay.dts"
[[ -s "$REPO_DIR/ilitek_v3/ilitek_v3_driver.ko" && \
   -s "$REPO_DIR/panel-ili79505a.ko" && \
   -s "$REPO_DIR/build/ili79505a.dtbo" ]] || { echo 'ERROR: incomplete build' >&2; exit 1; }

# Unique, persistent, user-visible backup; never delete old source or modules.
BACKUP_DIR="$REPO_DIR/backups/install-$(date +%Y%m%d-%H%M%S)-$$"
mkdir -p "$BACKUP_DIR"
backup() {
    local src="$1" dest="$BACKUP_DIR/${1#/}"
    if [[ -e "$src" || -L "$src" ]]; then
        mkdir -p "$(dirname "$dest")"
        cp -a -- "$src" "$dest"
    fi
}
PANEL_DEST="$MODULES_DIR/extra/panel-ili79505a.ko"
TOUCH_DEST="$MODULES_DIR/kernel/drivers/input/touchscreen/ilitek_v3_driver.ko"
OVERLAY_DEST="$BOOT_DIR/overlays/ili79505a.dtbo"
MOD_LOAD=/etc/modules-load.d/panel-ili79505a.conf
MOD_OPTIONS=/etc/modprobe.d/ilitek-v3-polling.conf
CALIB_DEST=/usr/local/lib/ili79505a/touch_calib.py
LABWC_RC="$TARGET_HOME/.config/labwc/rc.xml"
KANSHI_CONFIG="$TARGET_HOME/.config/kanshi/config"
STATE_FILE="$TARGET_HOME/.config/touch_calib.state"
for existing in "$BOOT_DIR/config.txt" "$PANEL_DEST" "$TOUCH_DEST" \
    "$OVERLAY_DEST" "$MOD_LOAD" "$MOD_OPTIONS" \
    /etc/modules-load.d/gt911.conf /etc/modules-load.d/panel-boe.conf \
    /usr/local/bin/touch_calib "$CALIB_DEST" "$LABWC_RC" \
    "$KANSHI_CONFIG" "$STATE_FILE"; do
    backup "$existing"
done
echo "[2/5] Backup ready: $BACKUP_DIR"

install -Dm0644 "$REPO_DIR/ilitek_v3/ilitek_v3_driver.ko" "$TOUCH_DEST"
install -Dm0644 "$REPO_DIR/panel-ili79505a.ko" "$PANEL_DEST"
install -Dm0644 "$REPO_DIR/build/ili79505a.dtbo" "$OVERLAY_DEST"
install -Dm0644 "$REPO_DIR/ilitek-v3-polling.conf" "$MOD_OPTIONS"
printf 'ilitek_v3_driver\npanel-ili79505a\n' > "$MOD_LOAD"
# Archive obsolete autoload declarations rather than removing driver files.
for old in /etc/modules-load.d/gt911.conf /etc/modules-load.d/panel-boe.conf; do
    if [[ -f "$old" ]]; then
        mkdir -p "$BACKUP_DIR/disabled/$(dirname "${old#/}")"
        mv -- "$old" "$BACKUP_DIR/disabled/${old#/}"
    fi
done
depmod -a "$KERNEL_VER"
echo '[3/5] Modules and overlay installed (next boot will load Ilitek first)'

# Raspberry Pi config sections are scoped: append a single known-good [all]
# stanza after removing only settings that conflict with this panel.
sed -i -E \
    -e '/^# === ILI79505A display ===$/,/^# === END ILI79505A ===$/d' \
    -e '/^[[:space:]]*dtoverlay=(boe-bv050fwm|ili79505a)([[:space:]]*(#.*)?)?$/d' \
    -e '/^[[:space:]]*dtoverlay=vc4-kms-v3d(,.*)?([[:space:]]*#.*)?$/d' \
    -e '/^[[:space:]]*dtoverlay=dummy-csi-sensor,2lanes([[:space:]]*#.*)?$/d' \
    -e '/^[[:space:]]*dtparam=i2c_vc=on([[:space:]]*#.*)?$/d' \
    "$BOOT_DIR/config.txt"
printf '%s\n' '[all]' '# === ILI79505A display ===' \
    'dtoverlay=vc4-kms-v3d,cma-256' 'dtparam=i2c_vc=on' \
    'dtoverlay=ili79505a' 'dtoverlay=dummy-csi-sensor,2lanes' \
    '# === END ILI79505A ===' >> "$BOOT_DIR/config.txt"
echo '[4/5] Boot overlay configured; firmware/Flash left untouched'

# The calibration tool never needs to unbind the shared display/touch IC.
install -Dm0755 "$REPO_DIR/touch_calib.py" "$CALIB_DEST"
ln -sfn "$CALIB_DEST" /usr/local/bin/touch_calib
for config_dir in "$TARGET_HOME/.config" "$TARGET_HOME/.config/kanshi" "$TARGET_HOME/.config/labwc"; do
    if [[ ! -d "$config_dir" ]]; then
        mkdir -p "$config_dir"
        chown "$TARGET_USER:" "$config_dir"
    fi
done
printf 'base=normal\nrotate=0\n' > "$STATE_FILE"
chown "$TARGET_USER:" "$STATE_FILE"
printf 'profile {\n    output DSI-1 enable scale 1.000000 mode 720x1280 position 0,0 transform 270\n}\n' > "$KANSHI_CONFIG"
chown "$TARGET_USER:" "$KANSHI_CONFIG"

# Replace only this panel's calibration; keep unrelated labwc settings.
LABWC_RC="$LABWC_RC" TARGET_USER="$TARGET_USER" python3 - <<'PY'
import os
import pathlib
import pwd
import re

path = pathlib.Path(os.environ['LABWC_RC'])
if path.exists():
    text = path.read_text(encoding='utf-8')
else:
    text = '<openbox_config xmlns="http://openbox.org/3.4/rc">\n</openbox_config>\n'
if not re.search(r'</openbox_config\s*>', text):
    raise SystemExit('ERROR: labwc rc.xml has no closing openbox_config tag')
add = ''
touch = '<touch deviceName="ILITEK_TDDI" mapToOutput="DSI-1" mouseEmulation="yes" />'
touch_pattern = r'<touch\b(?=[^>]*\bdeviceName=["\']ILITEK_TDDI["\'])[^>]*>'
if re.search(touch_pattern, text):
    text = re.sub(touch_pattern, touch, text, count=1)
else:
    add += '  ' + touch + '\n'
matrix = '<calibrationMatrix>1 0 0 0 1 0</calibrationMatrix>'
device_pattern = r'<device\b(?=[^>]*\bcategory=["\']ILITEK_TDDI["\'])[^>]*>.*?</device\s*>'
device_match = re.search(device_pattern, text, flags=re.S)
if device_match:
    device = device_match.group()
    matrix_pattern = r'<calibrationMatrix\b[^>]*>.*?</calibrationMatrix\s*>'
    if re.search(matrix_pattern, device, flags=re.S):
        device = re.sub(matrix_pattern, matrix, device, count=1, flags=re.S)
    else:
        device = re.sub(r'</device\s*>', '      ' + matrix + '\n    </device>', device, count=1)
    text = text[:device_match.start()] + device + text[device_match.end():]
else:
    device = ('    <device category="ILITEK_TDDI">\n'
              '      ' + matrix + '\n'
              '    </device>\n')
    if re.search(r'</libinput\s*>', text):
        text = re.sub(r'</libinput\s*>', lambda m: device + '  ' + m.group(), text, count=1)
    else:
        add += '  <libinput>\n' + device + '  </libinput>\n'
if add:
    text = re.sub(r'</openbox_config\s*>', lambda m: add + m.group(), text, count=1)
path.parent.mkdir(parents=True, exist_ok=True)
path.write_text(text, encoding='utf-8')
owner = pwd.getpwnam(os.environ['TARGET_USER'])
os.chown(path, owner.pw_uid, owner.pw_gid)
PY
echo '[5/5] Calibration tool installed; DSI rotation and ILITEK_TDDI mapping reset to the tested defaults'
echo "Done. Backup: $BACKUP_DIR"
echo 'Reboot when convenient: sudo reboot (installer does not reboot or flash firmware).'
