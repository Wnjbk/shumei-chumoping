#!/bin/bash
# BOE BV050FWM + GT911 touchscreen installer for fresh Raspberry Pi OS
# Usage: curl -sSL https://raw.githubusercontent.com/Wnjbk/shumei-chumoping/master/install.sh | sudo bash
# Or:    git clone https://github.com/Wnjbk/shumei-chumoping && cd shumei-chumoping && sudo bash install.sh

set -e

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
KERNEL_VER=$(uname -r)

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: run this installer as root, for example: sudo bash install.sh"
    exit 1
fi

TARGET_USER="${SUDO_USER:-xc}"
TARGET_HOME="$(getent passwd "$TARGET_USER" | cut -d: -f6)"
if [ -z "$TARGET_HOME" ] || [ ! -d "$TARGET_HOME" ]; then
    echo "ERROR: cannot find home directory for user $TARGET_USER"
    exit 1
fi

if [ -d /boot/firmware ]; then
    BOOT_DIR=/boot/firmware
else
    BOOT_DIR=/boot
fi

OVERLAY_DIR=$BOOT_DIR/overlays
MODULES_DIR=/lib/modules/$KERNEL_VER

echo "============================================"
echo " BOE BV050FWM + GT911 Touch Installer"
echo " Kernel: $KERNEL_VER"
echo " Boot dir: $BOOT_DIR"
echo "============================================"

# ---- 1. External-module build dependencies + kernel headers ----
if [ ! -f /lib/modules/$KERNEL_VER/build/Makefile ]; then
    echo "[1/8] Installing build dependencies and kernel headers..."
    apt update
    apt install -y build-essential device-tree-compiler
    apt install -y linux-headers-$KERNEL_VER || apt install -y raspberrypi-kernel-headers
else
    echo "[1/8] Installing build dependencies..."
    apt update
    apt install -y build-essential device-tree-compiler
fi

if [ ! -f /lib/modules/$KERNEL_VER/build/Makefile ]; then
    echo "ERROR: Kernel headers still missing at /lib/modules/$KERNEL_VER/build"
    echo "       Install the matching headers for kernel $KERNEL_VER, then rerun this script."
    exit 1
fi

# ---- 2. config.txt ----
echo "[2/8] Setting up config.txt..."
mkdir -p $OVERLAY_DIR
cp $BOOT_DIR/config.txt $BOOT_DIR/config.txt.bak.$(date +%Y%m%d_%H%M%S)

sed -i \
    -e '/^# === BOE BV050FWM Display + Touch ===$/d' \
    -e '/^camera_auto_detect=/d' \
    -e '/^display_auto_detect=/d' \
    -e '/^dtoverlay=vc4-kms-v3d/d' \
    -e '/^max_framebuffers=/d' \
    -e '/^dtparam=i2c_vc=on$/d' \
    -e '/^dtoverlay=boe-bv050fwm$/d' \
    -e '/^dtoverlay=dummy-csi-sensor,2lanes$/d' \
    $BOOT_DIR/config.txt

cat >> $BOOT_DIR/config.txt << 'CFGEOF'

# === BOE BV050FWM Display + Touch ===
camera_auto_detect=1
display_auto_detect=0
dtoverlay=vc4-kms-v3d,cma-256
max_framebuffers=2
dtparam=i2c_vc=on
dtoverlay=boe-bv050fwm
dtoverlay=dummy-csi-sensor,2lanes
CFGEOF
echo "  config.txt updated"

# ---- 3. DT overlay ----
echo "[3/8] Installing DT overlay..."
if [ -f "$REPO_DIR/boe-bv050fwm-overlay.dts" ]; then
    dtc -@ -I dts -O dtb -o /tmp/boe-bv050fwm.dtbo "$REPO_DIR/boe-bv050fwm-overlay.dts"
    cp /tmp/boe-bv050fwm.dtbo $OVERLAY_DIR/boe-bv050fwm.dtbo
    echo "  DT overlay installed"
else
    echo "  WARN: boe-bv050fwm-overlay.dts not found"
fi

# ---- 4. Panel driver ----
echo "[4/8] Building panel driver..."
PANEL_SRC=/tmp/panel-build-$$
rm -rf $PANEL_SRC
mkdir -p $PANEL_SRC

# Copy sources (try both naming conventions)
for prefix in "" "ili9881c_"; do
    for name in panel-ili9881c.c panel-ilitek-ili9881c.c; do
        if [ -f "$REPO_DIR/${prefix}${name}" ]; then
            cp "$REPO_DIR/${prefix}${name}" "$PANEL_SRC/$name"
        fi
    done
    if [ -f "$REPO_DIR/${prefix}Makefile" ]; then
        cp "$REPO_DIR/${prefix}Makefile" "$PANEL_SRC/Makefile"
    fi
done

if [ -f $PANEL_SRC/Makefile ] && [ -f $PANEL_SRC/panel-ili9881c.c ]; then
    cd $PANEL_SRC
    make -C /lib/modules/$KERNEL_VER/build M=$PANEL_SRC modules 2>&1 | tail -5
    cp panel-ili9881c.ko $MODULES_DIR/kernel/drivers/gpu/drm/panel/
    echo "  Panel driver installed"
else
    echo "  WARN: panel source not found, using built-in"
fi
cd "$REPO_DIR"

# ---- 5. Clean old + blacklist ----
echo "[5/8] Cleaning old drivers..."
find /lib/modules -name "panel-ilitek-ili9881c.ko*" -delete 2>/dev/null || true
rm -rf /var/lib/dkms/ili9881c /usr/src/ili9881c-* 2>/dev/null || true

cat > /etc/modprobe.d/blacklist-goodix.conf << 'EOF'
blacklist goodix_ts
install goodix_ts /bin/false
EOF
echo "  goodix_ts blacklisted"

# ---- 6. GT911 touch driver ----
echo "[6/8] Building GT911 driver..."
if [ -f "$REPO_DIR/gt911_poll.c" ]; then
    cp "$REPO_DIR/Makefile" /tmp/gt911_build_Makefile 2>/dev/null || true
    cd "$REPO_DIR"
    make clean 2>/dev/null || true
    make 2>&1 | tail -3
    cp gt911_poll.ko $MODULES_DIR/kernel/drivers/input/touchscreen/
    echo "  GT911 driver installed"
else
    echo "  WARN: gt911_poll.c not found"
fi

# ---- 7. Auto-load ----
echo "[7/8] Configuring auto-load..."
echo "gt911_poll" > /etc/modules-load.d/gt911.conf
echo "panel-ili9881c" > /etc/modules-load.d/panel-boe.conf
depmod -a
echo "  Done"

# ---- 8. Default display rotation (Raspberry Pi OS / labwc) ----
echo "[8/9] Setting default DSI rotation..."
KANSHI_DIR="$TARGET_HOME/.config/kanshi"
KANSHI_CONFIG=$KANSHI_DIR/config
mkdir -p "$KANSHI_DIR"
if [ -f "$KANSHI_CONFIG" ]; then
    cp "$KANSHI_CONFIG" "$KANSHI_CONFIG.bak.$(date +%Y%m%d_%H%M%S)"
fi
cat > "$KANSHI_CONFIG" << 'KANSHIEOF'
profile {
    output DSI-1 enable scale 1.000000 mode 720x1280@60.038 position 0,0 transform 270
}
KANSHIEOF
chown -R "$TARGET_USER:$TARGET_USER" "$TARGET_HOME/.config"
echo "  DSI-1 set to right rotation (transform 270)"

# ---- 9. touch_calib ----
echo "[9/9] Installing touch_calib..."
if [ -f "$REPO_DIR/touch_calib.py" ]; then
    mkdir -p "$TARGET_HOME/.config"
    mkdir -p "$TARGET_HOME/shumei-chumoping"
    if [ "$REPO_DIR/touch_calib.py" != "$TARGET_HOME/shumei-chumoping/touch_calib.py" ]; then
        cp "$REPO_DIR/touch_calib.py" "$TARGET_HOME/shumei-chumoping/touch_calib.py"
    fi
    chmod +x "$TARGET_HOME/shumei-chumoping/touch_calib.py"
    ln -sf "$TARGET_HOME/shumei-chumoping/touch_calib.py" /usr/local/bin/touch_calib
    "$TARGET_HOME/shumei-chumoping/touch_calib.py" reset 2>/dev/null || true
    "$TARGET_HOME/shumei-chumoping/touch_calib.py" save 2>/dev/null || true
    chown -R "$TARGET_USER:$TARGET_USER" "$TARGET_HOME/.config" "$TARGET_HOME/shumei-chumoping"
    echo "  touch_calib ready"
fi

# ---- Final ----
echo ""
echo "Updating initramfs..."
update-initramfs -u 2>&1 | tail -2

echo ""
echo "============================================"
echo " Install complete!  sudo reboot"
echo "============================================"
