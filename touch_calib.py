#!/usr/bin/env python3
"""GT911 touchscreen calibration — base fix + rotation, no reboot.
Supports I2C (gt911_poll) and USB HID (QinHeng adapter).

  touch_calib set  base <flip-x|flip-y|normal>    Hardware fix (rarely changed)
  touch_calib set  rotate <0|90|180|270>          Match display rotation
  touch_calib reset                                Restore working default
  touch_calib show                                 Show current state
  touch_calib save                                 Persist to udev

The base fix operates on raw touch coordinates before rotation.
"""

import subprocess, sys, os, glob

BASE = {
    "normal": ("1 0 0 0 1 0",   ""),
    "flip-x": ("-1 0 1 0 1 0",  "mirror X raw"),
    "flip-y": ("1 0 0 0 -1 1",  "mirror Y raw"),
}

ROTATE = {
    "0":   "1 0 0 0 1 0",
    "90":  "0 1 0 -1 0 1",
    "180": "-1 0 1 0 -1 1",
    "270": "0 -1 1 1 0 0",
}

STATE_FILE = "/home/xc/.config/touch_calib.state"
UDEV_RULE  = "/etc/udev/rules.d/98-gt911-calibration.rules"


def detect_device():
    """Auto-detect touchscreen: returns (udev_match, dev_id, bind_path)."""
    # I2C GT911
    i2c_bind = "/sys/bus/i2c/drivers/gt911_poll"
    if os.path.exists(f"{i2c_bind}/10-005d"):
        return (
            'ENV{ID_INPUT_TOUCHSCREEN}=="1", ENV{ID_PATH}=="platform-fe205000.i2c"',
            "10-005d",
            i2c_bind,
            "I2C GT911"
        )

    # USB HID (QinHeng 1a86:e5e3 or similar)
    for hid_dev in sorted(glob.glob("/sys/bus/hid/devices/*:1A86:E5E3.*")):
        try:
            drv_link = os.readlink(f"{hid_dev}/driver")
            bind_path = os.path.dirname(drv_link)
            dev_name = os.path.basename(hid_dev)

            # Find ID_PATH via udevadm on the associated input device
            r = subprocess.run(
                "sudo udevadm info --query=property --name=/dev/input/event5 2>/dev/null | grep '^ID_PATH='",
                shell=True, capture_output=True, text=True)
            id_path = r.stdout.strip().split("=", 1)[1] if r.stdout else None
            if not id_path:
                r = subprocess.run(
                    "for f in /dev/input/by-path/*usb*event*; do "
                    "sudo udevadm info --query=property --name=\"$f\" 2>/dev/null; done | grep '^ID_PATH=' | head -1",
                    shell=True, capture_output=True, text=True)
                id_path = r.stdout.strip().split("=", 1)[1] if r.stdout else None

            if id_path:
                match = f'ENV{{ID_INPUT_TOUCHSCREEN}}=="1", ENV{{ID_PATH}}=="{id_path}"'
                return match, dev_name, bind_path, "USB HID"
        except Exception:
            continue

    # Fallback: match any touchscreen by name substring
    for hid_dev in sorted(glob.glob("/sys/bus/hid/devices/*")):
        try:
            with open(f"{hid_dev}/uevent") as f:
                uevent = f.read()
            if "HID_NAME" not in uevent:
                continue
            drv_link = os.readlink(f"{hid_dev}/driver")
            if "hid-multitouch" not in drv_link and "hid-generic" not in drv_link:
                continue
            bind_path = os.path.dirname(drv_link)
            dev_name = os.path.basename(hid_dev)
            # Use name-based match instead of ID_PATH
            name = uevent.split("HID_NAME=")[1].split("\n")[0]
            match = f'ENV{{ID_INPUT_TOUCHSCREEN}}=="1", ATTRS{{name}}=="{name}"'
            return match, dev_name, bind_path, "USB HID (fallback)"
        except Exception:
            continue

    # Search /proc/bus/input/devices for any touchscreen
    try:
        with open("/proc/bus/input/devices") as f:
            data = f.read()
        for block in data.split("\n\n"):
            if "Touchscreen" not in block and "TOUCHSCREEN" not in block:
                continue
            for line in block.split("\n"):
                if line.startswith("N: Name="):
                    name = line.split('"')[1]
                    match = f'ENV{{ID_INPUT_TOUCHSCREEN}}=="1"'
                    return match, "auto", None, f"auto-detect ({name})"
    except Exception:
        pass

    return None, None, None, None


def mat_mul(a, b):
    """a x b — a applied AFTER b."""
    A = [float(x) for x in a.split()]
    B = [float(x) for x in b.split()]
    return (
        f"{A[0]*B[0] + A[1]*B[3]:g} {A[0]*B[1] + A[1]*B[4]:g} "
        f"{A[0]*B[2] + A[1]*B[5] + A[2]:g} "
        f"{A[3]*B[0] + A[4]*B[3]:g} {A[3]*B[1] + A[4]*B[4]:g} "
        f"{A[3]*B[2] + A[4]*B[5] + A[5]:g}"
    ).replace("-0 ", "0 ").replace("-0", "0")


def load_state():
    st = {"base": "flip-y", "rotate": "270"}
    try:
        with open(STATE_FILE) as f:
            for line in f:
                k, v = line.strip().split("=", 1)
                st[k] = v
    except FileNotFoundError:
        pass
    return st


def save_state(st):
    os.makedirs(os.path.dirname(STATE_FILE), exist_ok=True)
    with open("/tmp/touch_calib.state", "w") as f:
        for k, v in st.items():
            f.write(f"{k}={v}\n")
    subprocess.run(["cp", "/tmp/touch_calib.state", STATE_FILE])


def combined(base, rot):
    return mat_mul(ROTATE[rot], BASE[base][0])


def run(cmd):
    subprocess.run(cmd, shell=True, capture_output=True, text=True)


def apply_matrix(m):
    udev_match, dev_id, bind_path, driver = detect_device()
    if not udev_match:
        print("ERROR: no touchscreen detected!")
        return

    rule = f'{udev_match}, ENV{{LIBINPUT_CALIBRATION_MATRIX}}="{m}"\n'
    with open("/tmp/gt911-calib.rules", "w") as f:
        f.write(rule)
    run("sudo cp /tmp/gt911-calib.rules " + UDEV_RULE)
    run("sudo udevadm control --reload-rules")

    # Rebind driver to re-apply udev properties
    if bind_path and dev_id:
        run(f"echo -n {dev_id} | sudo tee {bind_path}/unbind 2>/dev/null")
        run("sleep 0.3")
        run(f"echo -n {dev_id} | sudo tee {bind_path}/bind 2>/dev/null")
    else:
        run("sudo udevadm trigger --subsystem-match=input 2>/dev/null")

    run("sleep 0.5")
    run("killall labwc 2>/dev/null; sleep 0.3")
    env = os.environ.copy()
    env.update({"DISPLAY": ":0", "XAUTHORITY": "/home/xc/.Xauthority"})
    subprocess.Popen(["labwc"], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"  device={driver}  matrix={m}")


def main():
    if len(sys.argv) < 2:
        st = load_state()
        udev_match, dev_id, bind_path, driver = detect_device()
        print(f"  device={driver}")
        print(f"  base={st['base']}  rotate={st['rotate']}  ->  {combined(st['base'], st['rotate'])}")
        print()
        print("Commands:")
        print("  touch_calib set  base <normal|flip-x|flip-y>")
        print("  touch_calib set  rotate <0|90|180|270>")
        print("  touch_calib reset")
        print("  touch_calib show")
        print("  touch_calib save")
        return

    cmd = sys.argv[1]
    st = load_state()

    if cmd == "show":
        udev_match, dev_id, bind_path, driver = detect_device()
        print(f"  device={driver}")
        print(f"  base={st['base']}  rotate={st['rotate']}  ->  {combined(st['base'], st['rotate'])}")
        return

    if cmd == "reset":
        st = {"base": "flip-y", "rotate": "270"}
        save_state(st)
        m = combined(st["base"], st["rotate"])
        print(f"  base={st['base']}  rotate={st['rotate']}  ->  {m}")
        apply_matrix(m)
        return

    if cmd == "set":
        if len(sys.argv) < 4:
            print("Usage: touch_calib set base <preset>")
            print("       touch_calib set rotate <deg>")
            return
        layer, val = sys.argv[2], sys.argv[3]
        if layer == "base" and val in BASE:
            st["base"] = val
        elif layer == "rotate" and val in ROTATE:
            st["rotate"] = val
        else:
            print(f"Invalid: {layer}={val}")
            return
        save_state(st)
        m = combined(st["base"], st["rotate"])
        print(f"  base={st['base']}  rotate={st['rotate']}  ->  {m}")
        apply_matrix(m)
        return

    if cmd == "save":
        m = combined(st["base"], st["rotate"])
        apply_matrix(m)
        return

    print(f"Unknown: {cmd}")


if __name__ == "__main__":
    main()
