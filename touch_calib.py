#!/usr/bin/env python3
"""GT911 touchscreen calibration — base fix + rotation, no reboot.

  touch_calib set  base <flip-x|flip-y|normal>    Hardware fix (rarely changed)
  touch_calib set  rotate <0|90|180|270>          Match display rotation
  touch_calib reset                                Restore working default
  touch_calib show                                 Show current state
  touch_calib save                                 Persist to udev

The base fix operates on raw touch coordinates before rotation.
This screen's hardware needs base=flip-y.
"""

import subprocess, sys, os

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
UDEV_MATCH = 'ENV{ID_INPUT_TOUCHSCREEN}=="1", ENV{ID_PATH}=="platform-fe205000.i2c"'
GT911_BIND = "/sys/bus/i2c/drivers/gt911_poll"


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
    # rotate applied AFTER base:  rot x base
    return mat_mul(ROTATE[rot], BASE[base][0])


def run(cmd):
    subprocess.run(cmd, shell=True, capture_output=True, text=True)


def apply_matrix(m):
    rule = f'{UDEV_MATCH}, ENV{{LIBINPUT_CALIBRATION_MATRIX}}="{m}"\n'
    with open("/tmp/gt911-calib.rules", "w") as f:
        f.write(rule)
    run("sudo cp /tmp/gt911-calib.rules " + UDEV_RULE)
    run("sudo udevadm control --reload-rules")
    run(f"echo -n 10-005d | sudo tee {GT911_BIND}/unbind 2>/dev/null")
    run("sleep 0.3")
    run(f"echo -n 10-005d | sudo tee {GT911_BIND}/bind 2>/dev/null")
    run("sleep 0.5")
    run("killall labwc 2>/dev/null; sleep 0.3")
    env = os.environ.copy()
    env.update({"DISPLAY": ":0", "XAUTHORITY": "/home/xc/.Xauthority"})
    subprocess.Popen(["labwc"], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"applied: {m}")


def main():
    if len(sys.argv) < 2:
        st = load_state()
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
