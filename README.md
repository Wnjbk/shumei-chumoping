# ILI79505A HNH07 屏幕驱动（树莓派）

这是基于原项目独立整理的 ILI79505A/TDDI 分支；原 BOE BV050FWM + GT911 方案保留在 `master`，不要用 `master` 的安装脚本安装本屏。

在 Raspberry Pi 4B、64 位 Raspberry Pi OS、内核 `6.18.34+rpt-rpi-v8` 实测：清理已安装的旧驱动后，在板上重新编译、安装、重启，用户确认显示与触摸均正常。其他内核需有与当前运行内核完全匹配的 headers，并自行验证兼容性。

## 接口与工作方式

| 项目 | 实测设置 |
| --- | --- |
| 面板 | ILI79505A HNH07，DRM 模式 720×1280，桌面横屏由 kanshi `transform 270` 实现 |
| 传图 | Raspberry Pi DSI-1，2-lane RGB888；DSI 不发送面板初始化指令 |
| 控制 | I²C-10，TDDI 芯片地址 `0x41`；背光开启前通过 Ilitek Linux I²C 驱动执行 TEST 初始化表 182 步 |
| 电源 | TCA9555（`0x20`）P04 使能正负 5.8 V，P00 复位，P01 背光，P02 触摸复位，P03 触摸信号 |
| 触摸 | `ILITEK_TDDI`，直接 I²C 轮询；校准 `base=normal`、`rotate=0`，矩阵 `1 0 0 0 1 0` |

驱动编译需要 `ilitek_v3/firmware/` 中随项目提供的 `.ili` 数据；安装脚本不会执行 Flash 烧录或固件擦除，运行时使用屏上现有固件。

## 在树莓派上安装

不要 `curl | sudo bash`：脚本需要同目录的内核驱动、设备树和固件头文件。源码须放在板上持久目录，不能放 `/tmp`。

```bash
git clone --branch ili79505a-hnh07 https://github.com/Wnjbk/shumei-chumoping.git
cd shumei-chumoping
bash install.sh --check --user "$(id -un)"
sudo bash install.sh
sudo reboot
```

从 root shell 安装时显式指定桌面账号，如 `sudo bash install.sh --user wnk`（将 `wnk` 换成实际用户名）。脚本先编译 `ilitek_v3_driver.ko` 和 `Module.symvers`，再编译依赖该符号表的 `panel-ili79505a.ko`，生成 overlay 并配置开机加载顺序和 `polling=1`。编译产物留在项目目录；安装过程对被替换文件备份到项目 `backups/install-时间戳-PID/`，不会自动重启。

安装不会覆盖已经保存的触摸方向或已存在的 kanshi 配置。全新配置才默认采用上表的横屏与单位矩阵。模块与设备树的修改在下一次重启生效。

## 安装后核查

```bash
lsmod | grep -E 'ilitek_v3_driver|panel_ili79505a'
cat /sys/module/ilitek_v3_driver/parameters/polling  # Y
cat /sys/class/drm/card*-DSI-1/status             # connected
cat /sys/class/drm/card*-DSI-1/modes              # 720x1280
touch_calib show
journalctl -b -k --no-pager | grep -E 'P03 polling|HNH07 page 6|TEST sequence transferred'
```

`touch_calib show` 应识别 `ILITEK_TDDI`。实测校准为 `base=normal rotate=0`；如更换装配方向，可用 `touch_calib set base ...` / `touch_calib set rotate ...` 调整。不要用旧 GT911 脚本的 `reset`：显示与触摸共享一颗 TDDI 芯片。

回退时先找到最近的 `backups/install-*/`，检查其中的相对目录结构，按原路径恢复所需的 `config.txt`、`.ko`、`.dtbo` 和开机配置，再运行 `sudo depmod -a`、重启；备份文件不存在表示原路径此前不存在。若从旧项目迁移，旧文件可能另存于 `backups/redeploy-*/removed/`。

## 文件索引

- `panel-ili79505a.c`：DRM 面板，仅使用 DSI 承载视频。
- `ili79505a-overlay.dts`：DSI、TCA9555、I²C TDDI 连线。
- `ili79505a_test_cmds.h`：供应商 TEST 面板初始化表。
- `ilitek_v3/`：触摸轮询、I²C DDI 命令与构建必需的内嵌固件数据。
- `install.sh`、`ilitek-v3-polling.conf`、`touch_calib.py`：安装、开机轮询参数、触摸映射工具。
- `config.txt`：参考开机配置；安装脚本不会整文件覆盖现有配置。

原仓库 BOE/GT911 源码仍可在旧版历史中查阅，但不属于本屏的安装步骤。
