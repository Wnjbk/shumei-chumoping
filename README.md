# 树莓派驱动

树莓派相关外设驱动程序。

## GT911 轮询模式触摸驱动 (gt911_poll)

适用于 GT911 INT 引脚通过 I2C GPIO 扩展器（如 PCA9555）连接、无法产生 CPU 中断的场景。

### 工作原理

- GT911 INT 引脚始终拉低，I2C 地址稳定为 0x5D
- 通过轮询 GT911 状态寄存器 (0x814E) 获取触摸数据
- 默认轮询间隔 10ms

### 文件说明

| 文件 | 说明 |
|------|------|
| `gt911_poll.c` | GT911 轮询模式驱动源码 |
| `Makefile` | 内核模块编译文件 |
| `boe-bv050fwm-overlay.dts` | BOE BV050FWM 面板 DT overlay（含 GT911 + PCA9555） |

### 编译

```bash
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
```

### 部署

1. 编译 DT overlay 并放入 `/boot/firmware/overlays/`
2. 在 `/boot/firmware/config.txt` 中添加 `dtoverlay=boe-bv050fwm`
3. 编译驱动模块并安装到 `/lib/modules/$(uname -r)/kernel/drivers/input/touchscreen/`
4. 运行 `depmod -a`
5. 加载模块：`modprobe gt911_poll`
6. 可选：在 `/etc/modules-load.d/gt911.conf` 中添加 `gt911_poll` 实现开机自启

### 注意事项

- 需黑名单加载原生 goodix_ts 驱动，避免冲突：
  ```bash
  echo "blacklist goodix_ts" > /etc/modprobe.d/blacklist-goodix.conf
  ```
