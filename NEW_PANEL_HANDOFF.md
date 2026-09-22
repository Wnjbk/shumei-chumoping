# New Panel Adaptation Handoff

Last updated: 2026-09-22

This document is the starting point for adapting a new Raspberry Pi display.
It records the known BOE BV050FWM baseline and deliberately separates verified
facts from assumptions.

## Repository Baseline

- Repository: `https://github.com/Wnjbk/shumei-chumoping.git`
- Current baseline commit: `93eb9bf` (`Fix panel driver for Linux 6.18`)
- Board used for the latest compile check: Raspberry Pi 4B at `192.168.2.16`
- Login user on that board: `wnk` (credentials are intentionally not stored in Git)
- OS requirement for the current installer: 64-bit Raspberry Pi OS.

Before installing, verify both values. The installer now rejects a mixed
32-bit userland with a 64-bit kernel because it cannot build loadable modules.

```bash
uname -m                    # expected: aarch64
dpkg --print-architecture   # expected: arm64
```

Do not run the installer through `curl | sudo bash`. It needs the DTS, C
sources, and Makefiles next to `install.sh`.

```bash
git clone https://github.com/Wnjbk/shumei-chumoping.git
cd shumei-chumoping
sudo bash install.sh
sudo reboot
```

## Known Working Hardware Baseline

This repository targets the BOE BV050FWM MIPI-DSI panel with an ILI9881C
controller and a GT911 touchscreen.

| Item | Verified value |
| --- | --- |
| DSI connection | `DSI-1`, two lanes |
| Preferred panel mode | 720x1280 at about 60 Hz |
| Panel reset | PCA9555 GPIO 0, active low |
| Backlight enable | PCA9555 GPIO 1 |
| Touch reset | PCA9555 GPIO 2 |
| Touch interrupt strap | PCA9555 GPIO 3 |
| I2C bus | `i2c-10` |
| PCA9555 address | `0x20` |
| GT911 address | `0x5d` |
| Touch driver | custom `gt911_poll.ko` polling driver |

The overlay is `boe-bv050fwm-overlay.dts`. It creates the PCA9555 and GT911
nodes under `/soc/i2c0mux/i2c@1` and attaches the panel to `&dsi1`.

The installer writes these effective display settings to the selected boot
partition (`/boot/firmware` on current Raspberry Pi OS):

```ini
camera_auto_detect=1
display_auto_detect=0
dtoverlay=vc4-kms-v3d,cma-256
max_framebuffers=2
dtparam=i2c_vc=on
dtoverlay=boe-bv050fwm
dtoverlay=dummy-csi-sensor,2lanes
```

## Kernel Compatibility Status

The panel driver originally used `mipi_dsi_dcs_write_seq`, which is not
available in Linux 6.18 headers. Commit `93eb9bf` added a guarded replacement
using `mipi_dsi_dcs_write_buffer` in both panel source copies.

Verified on the new 64-bit board:

```text
Kernel: 6.18.34+rpt-rpi-v8
Panel module: panel-ili9881c.ko
Result: compiled successfully
```

Only unused-variable warnings were reported. The full installer and a physical
post-reboot panel test were not completed after this compile check. Treat the
current state as "module compile verified", not "new board fully verified".

The installer was also changed to report full panel/GT911 build failures rather
than hiding them behind `tail` pipelines.

## Touch Configuration Status

The custom GT911 driver writes a 186-byte controller configuration table
(`0x41`) during probe. This is controller configuration, not a full firmware
image. The Linux touch orientation is separate and is applied by `touch_calib`.

Current script default:

```text
base=flip-y
rotate=270
matrix=0 1 0 1 0 0
```

The desktop orientation written by the installer is:

```text
DSI-1 transform 270
```

This combination was inherited from an older card, but the new board was
reported to have touch orientation off by 90 or 270 degrees. It has not been
resolved. `labwc` output mapping can already account for output rotation, so
the explicit touch rotation may be a double transform.

On the actual target screen, test one setting at a time while leaving display
rotation unchanged:

```bash
touch_calib set rotate 0
touch_calib set rotate 90
touch_calib set rotate 180
touch_calib set rotate 270
```

Each command rebinds the input driver and restarts `labwc`. Record the first
correct value before changing installer defaults. A second historical HDMI HID
touchscreen (`9LQ0172005164 HID Transfer`) was mapped to `HDMI-A-1`; the user
recalls that screen needing rotate 180, but that value was not verified from a
stored calibration rule.

## New Screen Adaptation Procedure

Do not overwrite the BOE overlay or panel command table before collecting the
new panel facts. Work from a separate overlay/source copy until the new panel
is accepted.

1. Identify the physical connection: DSI port, lane count, FPC direction, and
   whether a GPIO expander controls reset/backlight.
2. Boot with only standard KMS enabled and collect:

   ```bash
   dmesg | grep -Ei 'dsi|drm|panel|i2c|gpio|pca|goodix'
   ls -l /sys/class/drm
   for f in /sys/class/drm/*/status; do echo "$f=$(cat "$f")"; done
   sudo i2cdetect -y 10
   ```

3. Identify the LCD controller and obtain its documented DSI initialization
   table, timing, pixel format, lane count, reset sequence, and backlight
   polarity. Do not reuse the ILI9881C sequence for a different controller.
4. Create a new overlay with a unique name and a new panel-compatible string.
   Keep the BOE overlay unchanged as the rollback baseline.
5. Build the panel module against the exact running kernel before installing:

   ```bash
   make -C /lib/modules/$(uname -r)/build M=$PWD modules
   ```

6. Install only after module compilation succeeds. Reboot, then verify the
   connector, panel probe messages, mode list, backlight, and touch input.
7. Only after display output is stable, calibrate touch orientation and write
   the final setting into the new screen's installer path.

## Operational Notes And Recovery

- CRLF shell scripts fail on Linux with `$'\r': command not found`. The repo
  contains `.gitattributes` to force LF for `.sh` and `.py`; verify with
  `file install.sh` if a clone behaves unexpectedly.
- A fresh Raspberry Pi OS image may have `packagekitd` temporarily holding the
  APT lock. Wait for it; never remove APT lock files manually.
- Some Raspbian mirror redirects returned HTTP 403 during the earlier setup.
  The installer now only installs the external-module prerequisites plus the
  exact kernel headers; it no longer pulls bison/flex/libssl-dev unnecessarily.
- The installer modifies `config.txt` before building modules and creates a
  timestamped backup. If a build fails, restore the backup or finish the fixed
  build before rebooting.
- Raspberry Pi 2B is not yet supported by this overlay. It needs a pure 32-bit
  `armhf + rpi-v7` OS, a Pi 2 DSI target (likely `dsi0`), and its own overlay
  validation. The current overlay is Pi 4/CM4 oriented (`bcm2711`, `dsi1`).

## Image Backup Script

`rpi_minimal_image.sh` creates a full `dd` image and then runs PiShrink to
make a smaller image that expands on first boot. It supports local PiShrink:

```bash
sudo ./rpi_minimal_image.sh \
  --source /dev/sdX \
  --output /path/rpi-min.img \
  --pishrink /path/pishrink.sh
```

This image workflow was not boot-validated after a user-reported failure.
Do not treat it as a production backup path until it has been verified with a
fresh write, boot, and root-filesystem expansion check.
