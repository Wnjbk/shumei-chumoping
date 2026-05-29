# 树莓派触摸屏

BOE BV050FWM MIPI-DSI 面板 + GT911 触摸屏，树莓派 4B/CM4 完整驱动方案。

## 一键安装

```bash
curl -sSL https://raw.githubusercontent.com/Wnjbk/shumei-chumoping/master/install.sh | sudo bash
```

## 硬件连接

BV050FWM 面板通过 40-pin FPC 连接，I2C 总线（i2c-10）上挂载：

| 芯片 | I2C地址 | 功能 |
|------|---------|------|
| PCA9555 | 0x20 | GPIO 扩展器（LCD_RST, LCD_EN, TP_RST, TP_INT） |
| GT911 | 0x5D | 触摸控制器 |

PCA9555 引脚分配：

| 引脚 | 标签 | 功能 |
|------|------|------|
| 0 | LCD_RST | 面板复位 |
| 1 | LCD_EN | 背光使能 |
| 2 | TP_RST | 触摸复位 |
| 3 | TP_INT | 触摸中断（轮询模式下始终拉低） |

## 新板子完整部署流程

### 1. 烧录系统

树莓派 OS（Bookworm，64-bit），确保内核版本 ≥ 6.6。

### 2. 编译并安装 DT overlay

```bash
# 编译
dtc -@ -I dts -O dtb -o boe-bv050fwm.dtbo boe-bv050fwm-overlay.dts

# 安装
sudo cp boe-bv050fwm.dtbo /boot/firmware/overlays/
```

### 3. 配置 /boot/firmware/config.txt

添加以下内容：

```ini
# DSI 显示
dtoverlay=vc4-kms-v3d
dtoverlay=boe-bv050fwm

# I2C
dtparam=i2c_vc=on
```

### 4. 编译并安装 GT911 轮询驱动

```bash
# 编译
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules

# 安装
sudo cp gt911_poll.ko /lib/modules/$(uname -r)/kernel/drivers/input/touchscreen/
sudo depmod -a
```

### 5. 编译并安装 ILI9881C 面板驱动（如果需要）

内核通常已内置 `panel-ilitek-ili9881c` 模块。如需使用自定义版本：

```bash
cd ili9881c/
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
sudo cp panel-ili9881c.ko /lib/modules/$(uname -r)/kernel/drivers/gpu/drm/panel/
sudo depmod -a
```

### 6. 禁用原生 Goodix 驱动

```bash
sudo sh -c 'echo "blacklist goodix_ts" > /etc/modprobe.d/blacklist-goodix.conf'
sudo sh -c 'echo "install goodix_ts /bin/false" >> /etc/modprobe.d/blacklist-goodix.conf'
```

### 7. 配置开机自动加载模块

```bash
sudo sh -c 'echo "gt911_poll" > /etc/modules-load.d/gt911.conf'
```

### 8. 重启

```bash
sudo reboot
```

### 9. 验证

```bash
# 检查 I2C 设备
sudo i2cdetect -y 10
# 应看到 0x20 (PCA9555) 和 0x5D (GT911)

# 检查 GPIO 状态
cat /sys/kernel/debug/gpio | grep TP
# TP_RST=out hi, TP_INT=out lo

# 检查触摸输入设备
cat /proc/bus/input/devices | grep -A5 GT911

# 测试触摸事件
od -x /dev/input/$(cat /proc/bus/input/devices | grep -A1 GT911 | grep -o 'event[0-9]*')
```

## 文件说明

| 文件 | 说明 |
|------|------|
| `gt911_poll.c` | GT911 轮询模式触摸驱动源码 |
| `Makefile` | GT911 驱动编译脚本 |
| `boe-bv050fwm-overlay.dts` | DT overlay（PCA9555 + GT911 + DSI panel） |
| `ili9881c_panel-ilitek-ili9881c.c` | ILI9881C MIPI-DSI 面板驱动 |
| `ili9881c_panel-ili9881c.c` | ILI9881C 面板驱动（辅助） |
| `ili9881c_Makefile` | ILI9881C 驱动编译脚本 |
| `config.txt` | 树莓派 `/boot/firmware/config.txt` 参考配置 |

## 触摸校准

`touch_calib.py` 提供触摸旋转与翻转的即时配置，无需重启。

### 安装

```bash
sudo ln -s $(pwd)/touch_calib.py /usr/local/bin/touch_calib
```

### 两层结构

| 层 | 作用 | 选项 |
|----|------|------|
| **base** | 硬件修正（换屏才改） | `normal`, `flip-x`, `flip-y` |
| **rotate** | 匹配显示方向 | `0`, `90`, `180`, `270` |

矩阵乘积顺序：`rotate × base`（先修正硬件，再旋转匹配显示）。

### 常用命令

```bash
touch_calib show                   # 查看当前状态
touch_calib set rotate 270         # 横屏（匹配 kanshi transform 270）
touch_calib set rotate 0           # 竖屏
touch_calib set base flip-y        # Y 轴翻转（硬件修正）
touch_calib reset                  # 恢复出厂默认
touch_calib save                   # 持久化（重启不丢）
```

每次 `set` 即时生效：写入 udev 规则 → 重绑 GT911 驱动 → 重启 labwc。

### 状态文件

- 运行时状态：`~/.config/touch_calib.state`
- udev 规则：`/etc/udev/rules.d/98-gt911-calibration.rules`

## GT911 驱动技术说明

- **地址稳定方案**：INT 引脚始终输出低电平（GPIOD_OUT_LOW），使 GT911 在 RST 上升沿采样到 INT=low，地址固定为 0x5D
- **触摸检测**：轮询状态寄存器 0x814E，bit7=1 表示有触摸数据
- **触摸上报**：此 GT911 变体不设置 p[0] bit7（触摸/释放标志），因此始终上报 `MT_TOOL_FINGER, true`
- **配置写入**：驱动启动时检测 GT911 配置版本号（0x8047），若版本 ≠ 0 则保留现有配置，否则写入新配置
- 轮询间隔默认 10ms，分辨率 1280x720

## 替代文件

在 `config/` 目录下提供了所有系统配置文件的参考副本，可直接使用：

```
config/config.txt                         → /boot/firmware/config.txt
config/modules-load_gt911.conf            → /etc/modules-load.d/gt911.conf
config/modprobe_blacklist-goodix.conf     → /etc/modprobe.d/blacklist-goodix.conf
```
