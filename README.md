# 树莓派触摸屏

BOE BV050FWM MIPI-DSI 面板 + GT911 触摸屏，树莓派 4B/CM4 完整驱动方案。

## 文件说明

| 文件 | 说明 |
|------|------|
| `gt911_poll.c` | GT911 轮询模式触摸驱动 |
| `Makefile` | GT911 驱动编译脚本 |
| `boe-bv050fwm-overlay.dts` | DT overlay（PCA9555 + GT911 + DSI panel） |
| `ili9881c_panel-ilitek-ili9881c.c` | ILI9881C MIPI-DSI 面板驱动（主文件） |
| `ili9881c_panel-ili9881c.c` | ILI9881C 面板驱动（辅助文件） |
| `ili9881c_Makefile` | ILI9881C 驱动编译脚本 |
| `config.txt` | Pi 4B `/boot/firmware/config.txt` 参考配置 |

## GT911 轮询模式触摸驱动

适用于 GT911 INT 引脚通过 I2C GPIO 扩展器（PCA9555）连接、无法产生 CPU 中断的场景。

### 工作原理

- GT911 INT 引脚始终拉低（GPIOD_OUT_LOW），I2C 地址稳定为 0x5D
- 通过轮询 GT911 状态寄存器 (0x814E) 获取触摸数据
- 默认轮询间隔 10ms，分辨率 1280x720

### 编译

```bash
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
```

### 部署

1. 编译 DT overlay 放入 `/boot/firmware/overlays/`
2. 在 `/boot/firmware/config.txt` 添加 `dtoverlay=boe-bv050fwm`
3. 安装驱动模块到 `/lib/modules/$(uname -r)/kernel/drivers/input/touchscreen/`
4. 运行 `depmod -a`，加载 `modprobe gt911_poll`
5. 开机自启：`echo "gt911_poll" > /etc/modules-load.d/gt911.conf`

### 注意事项

- 需黑名单加载原生 goodix_ts 驱动：
  ```bash
  echo "blacklist goodix_ts" > /etc/modprobe.d/blacklist-goodix.conf
  ```

## ILI9881C MIPI-DSI 面板驱动

BOE BV050FWM 面板使用 ILI9881C 控制器，通过 DSI1 接口连接。

### 编译

```bash
cd ili9881c/
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
```

### 部署

```bash
sudo cp panel-ili9881c.ko /lib/modules/$(uname -r)/kernel/drivers/gpu/drm/panel/
sudo depmod -a
```
