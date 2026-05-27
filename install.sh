#!/bin/bash
# BOE BV050FWM + GT911 touchscreen auto-setup script
# For Raspberry Pi 4B/CM4
# Usage: chmod +x install.sh && sudo ./install.sh

set -e

REPO_DRIVERS="https://github.com/Wnjbk/shumei-chumoping.git"
REPO_MISC="https://github.com/Wnjbk/shumei-zawu.git"
WORK_DIR="/tmp/rpi-touch-setup"
BOOT_DIR="/boot/firmware"
OVERLAY_NAME="boe-bv050fwm"
KERNEL_SRC="/lib/modules/$(uname -r)/build"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log()  { echo -e "${GREEN}[+]${NC} $1"; }
warn() { echo -e "${YELLOW}[!]${NC} $1"; }
err()  { echo -e "${RED}[x]${NC} $1"; exit 1; }

# ---------- privilege check ----------
if [ "$(id -u)" -ne 0 ]; then
    err "Please run as root: sudo ./install.sh"
fi

# ---------- dependency check ----------
log "Checking dependencies..."
for cmd in git dtc make gcc; do
    command -v $cmd >/dev/null 2>&1 || err "$cmd not found. Install: apt install git device-tree-compiler make gcc"
done

[ -d "$KERNEL_SRC" ] || err "Kernel headers not found at $KERNEL_SRC. Install: apt install linux-headers-$(uname -r)"

# ---------- download ----------
log "Downloading drivers from GitHub..."
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

git clone --depth 1 "$REPO_DRIVERS" drivers 2>/dev/null || err "Failed to clone $REPO_DRIVERS"
git clone --depth 1 "$REPO_MISC" misc 2>/dev/null || warn "Failed to clone $REPO_MISC (non-critical)"

cd "$WORK_DIR/drivers"

# ---------- build DT overlay ----------
log "Building DT overlay ($OVERLAY_NAME)..."
dtc -@ -I dts -O dtb -o "${OVERLAY_NAME}.dtbo" boe-bv050fwm-overlay.dts || err "DT compile failed"

log "Installing DT overlay to $BOOT_DIR/overlays/"
cp "${OVERLAY_NAME}.dtbo" "$BOOT_DIR/overlays/"

# ---------- build GT911 driver ----------
log "Building GT911 polling driver..."
make -C "$KERNEL_SRC" M="$WORK_DIR/drivers" modules 2>&1 | tail -5 || err "GT911 driver build failed"

log "Installing GT911 driver..."
cp gt911_poll.ko "$KERNEL_SRC/kernel/drivers/input/touchscreen/" 2>/dev/null || \
cp gt911_poll.ko "/lib/modules/$(uname -r)/kernel/drivers/input/touchscreen/"

# ---------- build ILI9881C panel driver ----------
log "Building ILI9881C panel driver..."
cd "$WORK_DIR/drivers"
ILI_BUILD_DIR="$WORK_DIR/build-ili9881c"
mkdir -p "$ILI_BUILD_DIR"
cp ili9881c_Makefile "$ILI_BUILD_DIR/Makefile"
cp ili9881c_panel-ilitek-ili9881c.c "$ILI_BUILD_DIR/"
cp ili9881c_panel-ili9881c.c "$ILI_BUILD_DIR/"

cd "$ILI_BUILD_DIR"
make -C "$KERNEL_SRC" M="$ILI_BUILD_DIR" modules 2>&1 | tail -5 || warn "ILI9881C build failed — kernel may already have it"

if [ -f panel-ili9881c.ko ]; then
    log "Installing ILI9881C driver..."
    cp panel-ili9881c.ko "/lib/modules/$(uname -r)/kernel/drivers/gpu/drm/panel/"
fi

# ---------- update module deps ----------
log "Running depmod..."
depmod -a

# ---------- blacklist goodix_ts ----------
log "Blacklisting original goodix_ts driver..."
cat > /etc/modprobe.d/blacklist-goodix.conf << 'EOF'
blacklist goodix_ts
install goodix_ts /bin/false
EOF

# ---------- autoload GT911 ----------
log "Configuring autoload for GT911..."
echo "gt911_poll" > /etc/modules-load.d/gt911.conf

# ---------- config.txt ----------
log "Updating $BOOT_DIR/config.txt..."
CONFIG="$BOOT_DIR/config.txt"

# Backup original
[ -f "$CONFIG.bak" ] || cp "$CONFIG" "$CONFIG.bak"

# Add required lines if missing
add_if_missing() {
    local line="$1"
    if ! grep -qF "$line" "$CONFIG"; then
        echo "$line" >> "$CONFIG"
        log "  Added: $line"
    fi
}

add_if_missing "dtoverlay=vc4-kms-v3d"
add_if_missing "dtoverlay=${OVERLAY_NAME}"
add_if_missing "dtparam=i2c_vc=on"

# ---------- summary ----------
echo
log "============================================"
log "Setup complete!"
log "============================================"
echo
echo "  DT overlay  : ${BOOT_DIR}/overlays/${OVERLAY_NAME}.dtbo"
echo "  GT911 driver: /lib/modules/$(uname -r)/kernel/drivers/input/touchscreen/gt911_poll.ko"
echo "  Blacklist   : /etc/modprobe.d/blacklist-goodix.conf"
echo "  Autoload    : /etc/modules-load.d/gt911.conf"
echo "  Config      : ${BOOT_DIR}/config.txt (backup: ${BOOT_DIR}/config.txt.bak)"
echo
echo "  Verifying GT911 I2C..."
modprobe gt911_poll 2>/dev/null || true
sleep 1

if i2cdetect -y 10 0x5d 0x5d 2>/dev/null | grep -q "5d"; then
    log "GT911 detected at 0x5D!"
elif i2cdetect -y 10 0x14 0x14 2>/dev/null | grep -q "14"; then
    warn "GT911 at 0x14 instead of 0x5D. Try rebooting first."
else
    warn "GT911 not detected on i2c-10. Check hardware connection."
fi

echo
log "Please reboot: sudo reboot"
echo
