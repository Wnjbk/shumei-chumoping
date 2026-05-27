
// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2019, Michael Srba

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/backlight.h>
#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct bv050fwm {
    struct drm_panel panel;
    struct mipi_dsi_device *dsi;
    struct regulator_bulk_data supplies[1];
    struct gpio_desc *reset_gpio;
    struct gpio_desc *backlight_gpio;

    bool prepared;
};

static inline struct
bv050fwm *to_bv050fwm(struct drm_panel *panel)
{
    return container_of(panel, struct bv050fwm, panel);
}

static void bv050fwm_reset(struct bv050fwm *ctx)
{
    struct mipi_dsi_device *dsi = ctx->dsi;
    struct device *dev = &dsi->dev;
    gpiod_set_value_cansleep(ctx->backlight_gpio, 1);

    gpiod_set_value_cansleep(ctx->reset_gpio, 0);
    usleep_range(1000, 2000);
    gpiod_set_value_cansleep(ctx->reset_gpio, 1);
    usleep_range(5000, 6000);
    gpiod_set_value_cansleep(ctx->reset_gpio, 0);
    usleep_range(10000, 11000);
}

static int bv050fwm_on(struct bv050fwm *ctx)
{
    struct mipi_dsi_device *dsi = ctx->dsi;
    struct device *dev = &dsi->dev;
    int ret;

    dsi->mode_flags |= MIPI_DSI_MODE_LPM;
    dev_info(dev, "bv050fwm_on\n");

mipi_dsi_dcs_write_seq(dsi, 0xFF, 0x98, 0x81, 0x03);

mipi_dsi_dcs_write_seq(dsi, 0x01, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x02, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x03, 0x73);

mipi_dsi_dcs_write_seq(dsi, 0x04, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x05, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x06, 0x0A);

mipi_dsi_dcs_write_seq(dsi, 0x07, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x08, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x09, 0x61);

mipi_dsi_dcs_write_seq(dsi, 0x0A, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x0B, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x0C, 0x01);

mipi_dsi_dcs_write_seq(dsi, 0x0D, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x0E, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x0F, 0x61);

mipi_dsi_dcs_write_seq(dsi, 0x10, 0x61);

mipi_dsi_dcs_write_seq(dsi, 0x11, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x12, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x13, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x14, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x15, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x16, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x17, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x18, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x19, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x1A, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x1B, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x1C, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x1D, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x1E, 0x40);

mipi_dsi_dcs_write_seq(dsi, 0x1F, 0x80);

mipi_dsi_dcs_write_seq(dsi, 0x20, 0x06);

mipi_dsi_dcs_write_seq(dsi, 0x21, 0x01);

mipi_dsi_dcs_write_seq(dsi, 0x22, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x23, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x24, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x25, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x26, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x27, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x28, 0x33);

mipi_dsi_dcs_write_seq(dsi, 0x29, 0x03);

mipi_dsi_dcs_write_seq(dsi, 0x2A, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x2B, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x2C, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x2D, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x2E, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x2F, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x30, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x31, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x32, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x33, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x34, 0x04);

mipi_dsi_dcs_write_seq(dsi, 0x35, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x36, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x37, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x38, 0x3C);

mipi_dsi_dcs_write_seq(dsi, 0x39, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x3A, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x3B, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x3C, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x3D, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x3E, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x3F, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x40, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x41, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x42, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x43, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x44, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x50, 0x10);

mipi_dsi_dcs_write_seq(dsi, 0x51, 0x32);

mipi_dsi_dcs_write_seq(dsi, 0x52, 0x54);

mipi_dsi_dcs_write_seq(dsi, 0x53, 0x76);

mipi_dsi_dcs_write_seq(dsi, 0x54, 0x98);

mipi_dsi_dcs_write_seq(dsi, 0x55, 0xBA);

mipi_dsi_dcs_write_seq(dsi, 0x56, 0x10);

mipi_dsi_dcs_write_seq(dsi, 0x57, 0x32);

mipi_dsi_dcs_write_seq(dsi, 0x58, 0x54);

mipi_dsi_dcs_write_seq(dsi, 0x59, 0x76);

mipi_dsi_dcs_write_seq(dsi, 0x5A, 0x98);

mipi_dsi_dcs_write_seq(dsi, 0x5B, 0xBA);

mipi_dsi_dcs_write_seq(dsi, 0x5C, 0xDC);

mipi_dsi_dcs_write_seq(dsi, 0x5D, 0xFE);

mipi_dsi_dcs_write_seq(dsi, 0x5E, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x5F, 0x0E);

mipi_dsi_dcs_write_seq(dsi, 0x60, 0x0F);

mipi_dsi_dcs_write_seq(dsi, 0x61, 0x0C);

mipi_dsi_dcs_write_seq(dsi, 0x62, 0x0D);

mipi_dsi_dcs_write_seq(dsi, 0x63, 0x06);

mipi_dsi_dcs_write_seq(dsi, 0x64, 0x07);

mipi_dsi_dcs_write_seq(dsi, 0x65, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x66, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x67, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x68, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x69, 0x01);

mipi_dsi_dcs_write_seq(dsi, 0x6A, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x6B, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x6C, 0x15);

mipi_dsi_dcs_write_seq(dsi, 0x6D, 0x14);

mipi_dsi_dcs_write_seq(dsi, 0x6E, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x6F, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x70, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x71, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x72, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x73, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x74, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x75, 0x0E);

mipi_dsi_dcs_write_seq(dsi, 0x76, 0x0F);

mipi_dsi_dcs_write_seq(dsi, 0x77, 0x0C);

mipi_dsi_dcs_write_seq(dsi, 0x78, 0x0D);

mipi_dsi_dcs_write_seq(dsi, 0x79, 0x06);

mipi_dsi_dcs_write_seq(dsi, 0x7A, 0x07);

mipi_dsi_dcs_write_seq(dsi, 0x7B, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x7C, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x7D, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x7E, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x7F, 0x01);

mipi_dsi_dcs_write_seq(dsi, 0x80, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x81, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x82, 0x14);

mipi_dsi_dcs_write_seq(dsi, 0x83, 0x15);

mipi_dsi_dcs_write_seq(dsi, 0x84, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x85, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x86, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x87, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x88, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x89, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0x8A, 0x02);

mipi_dsi_dcs_write_seq(dsi, 0xFF, 0x98, 0x81, 0x04);

//mipi_dsi_dcs_write_seq(dsi, 0x2d, 0xff);

//mipi_dsi_dcs_write_seq(dsi, 0x2f, 0x01);

mipi_dsi_dcs_write_seq(dsi, 0x00, 0x80);

mipi_dsi_dcs_write_seq(dsi, 0x38, 0x01);

mipi_dsi_dcs_write_seq(dsi, 0x39, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x6C, 0x15);

mipi_dsi_dcs_write_seq(dsi, 0x6E, 0x2A);

mipi_dsi_dcs_write_seq(dsi, 0x6F, 0x33);

mipi_dsi_dcs_write_seq(dsi, 0x3A, 0x94);

mipi_dsi_dcs_write_seq(dsi, 0x8D, 0x14);

mipi_dsi_dcs_write_seq(dsi, 0x87, 0xBA);

mipi_dsi_dcs_write_seq(dsi, 0x26, 0x76);

mipi_dsi_dcs_write_seq(dsi, 0xB2, 0xD1);

mipi_dsi_dcs_write_seq(dsi, 0xB5, 0x06);

mipi_dsi_dcs_write_seq(dsi, 0x3B, 0x98);

mipi_dsi_dcs_write_seq(dsi, 0xFF, 0x98, 0x81, 0x01);

mipi_dsi_dcs_write_seq(dsi, 0x22, 0x0A);

mipi_dsi_dcs_write_seq(dsi, 0x31, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x53, 0x7D);

mipi_dsi_dcs_write_seq(dsi, 0x55, 0x8F);

mipi_dsi_dcs_write_seq(dsi, 0x40, 0x33);

mipi_dsi_dcs_write_seq(dsi, 0x50, 0x96);

mipi_dsi_dcs_write_seq(dsi, 0x51, 0x96);

mipi_dsi_dcs_write_seq(dsi, 0x60, 0x23);

mipi_dsi_dcs_write_seq(dsi, 0xA0, 0x08);

mipi_dsi_dcs_write_seq(dsi, 0xA1, 0x1D);

mipi_dsi_dcs_write_seq(dsi, 0xA2, 0x2A);

mipi_dsi_dcs_write_seq(dsi, 0xA3, 0x10);

mipi_dsi_dcs_write_seq(dsi, 0xA4, 0x15);

mipi_dsi_dcs_write_seq(dsi, 0xA5, 0x28);

mipi_dsi_dcs_write_seq(dsi, 0xA6, 0x1C);

mipi_dsi_dcs_write_seq(dsi, 0xA7, 0x1D);

mipi_dsi_dcs_write_seq(dsi, 0xA8, 0x7E);

mipi_dsi_dcs_write_seq(dsi, 0xA9, 0x1D);

mipi_dsi_dcs_write_seq(dsi, 0xAA, 0x29);

mipi_dsi_dcs_write_seq(dsi, 0xAB, 0x6B);

mipi_dsi_dcs_write_seq(dsi, 0xAC, 0x1A);

mipi_dsi_dcs_write_seq(dsi, 0xAD, 0x18);

mipi_dsi_dcs_write_seq(dsi, 0xAE, 0x4B);

mipi_dsi_dcs_write_seq(dsi, 0xAF, 0x20);

mipi_dsi_dcs_write_seq(dsi, 0xB0, 0x27);

mipi_dsi_dcs_write_seq(dsi, 0xB1, 0x50);

mipi_dsi_dcs_write_seq(dsi, 0xB2, 0x64);

mipi_dsi_dcs_write_seq(dsi, 0xB3, 0x39);
mipi_dsi_dcs_write_seq(dsi, 0xB7, 0x03);

mipi_dsi_dcs_write_seq(dsi, 0xC0, 0x08);

mipi_dsi_dcs_write_seq(dsi, 0xC1, 0x1D);

mipi_dsi_dcs_write_seq(dsi, 0xC2, 0x2A);

mipi_dsi_dcs_write_seq(dsi, 0xC3, 0x10);

mipi_dsi_dcs_write_seq(dsi, 0xC4, 0x15);

mipi_dsi_dcs_write_seq(dsi, 0xC5, 0x28);

mipi_dsi_dcs_write_seq(dsi, 0xC6, 0x1C);

mipi_dsi_dcs_write_seq(dsi, 0xC7, 0x1D);

mipi_dsi_dcs_write_seq(dsi, 0xC8, 0x7E);

mipi_dsi_dcs_write_seq(dsi, 0xC9, 0x1D);

mipi_dsi_dcs_write_seq(dsi, 0xCA, 0x29);

mipi_dsi_dcs_write_seq(dsi, 0xCB, 0x6B);

mipi_dsi_dcs_write_seq(dsi, 0xCC, 0x1A);

mipi_dsi_dcs_write_seq(dsi, 0xCD, 0x18);

mipi_dsi_dcs_write_seq(dsi, 0xCE, 0x4B);

mipi_dsi_dcs_write_seq(dsi, 0xCF, 0x20);

mipi_dsi_dcs_write_seq(dsi, 0xD0, 0x27);

mipi_dsi_dcs_write_seq(dsi, 0xD1, 0x50);

mipi_dsi_dcs_write_seq(dsi, 0xD2, 0x64);

mipi_dsi_dcs_write_seq(dsi, 0xD3, 0x39);

mipi_dsi_dcs_write_seq(dsi, 0xFF, 0x98, 0x81, 0x00);

mipi_dsi_dcs_write_seq(dsi, 0x11);

msleep(120);

mipi_dsi_dcs_write_seq(dsi, 0x35);

mipi_dsi_dcs_write_seq(dsi, 0x29);

msleep(150);
    ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
    if (ret < 0) {
        dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
        return ret;
    }
    msleep(120);

    ret = mipi_dsi_dcs_set_display_on(dsi);
    if (ret < 0) {
        dev_err(dev, "Failed to set display on: %d\n", ret);
        return ret;
    }
    msleep(20);
    dev_info(dev, "bv050fwm_on end\n");
  return 0;
}

static int bv050fwm_off(struct bv050fwm *ctx)
{
    struct mipi_dsi_device *dsi = ctx->dsi;
    struct device *dev = &dsi->dev;
    int ret;

    dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

    ret = mipi_dsi_dcs_set_display_off(dsi);
    if (ret < 0) {
        dev_err(dev, "Failed to set display off: %d\n", ret);
        return ret;
    }
    msleep(20);

    ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
    if (ret < 0) {
        dev_err(dev, "Failed to enter sleep mode: %d\n", ret);
        return ret;
    }
    msleep(120);
    gpiod_set_value_cansleep(ctx->backlight_gpio, 0);


    return 0;
}

static int bv050fwm_prepare(struct drm_panel *panel)
{
    struct bv050fwm *ctx = to_bv050fwm(panel);
    struct device *dev = &ctx->dsi->dev;
    int ret;

    if (ctx->prepared)
        return 0;

    // regulator_bulk_disable(ARRAY_SIZE(ctx->supplies),
    //                      ctx->supplies);
    // msleep(1000);

    ret = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
    if (ret < 0) {
        dev_err(dev, "Failed to enable regulators: %d\n", ret);
        return ret;
    }

    bv050fwm_reset(ctx);

    ret = bv050fwm_on(ctx);
    if (ret < 0) {
        dev_err(dev, "Failed to initialize panel: %d\n", ret);
        gpiod_set_value_cansleep(ctx->reset_gpio, 0);
        regulator_bulk_disable(ARRAY_SIZE(ctx->supplies),
                       ctx->supplies);
        return ret;
    }

    ctx->prepared = true;
    return 0;
}

static int bv050fwm_unprepare(struct drm_panel *panel)
{
    struct bv050fwm *ctx = to_bv050fwm(panel);
    struct device *dev = &ctx->dsi->dev;
    int ret;

    if (!ctx->prepared)
        return 0;

    ret = bv050fwm_off(ctx);
    if (ret < 0)
        dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

    gpiod_set_value_cansleep(ctx->reset_gpio, 0);
    regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);

    ctx->prepared = false;
    return 0;
}

/* Backlight control code */

static int bv050fwm_bl_update_status(struct backlight_device *bl)
{
    struct mipi_dsi_device *dsi = bl_get_data(bl);
    struct device *dev = &bl->dev;
    u8 brightness = backlight_get_brightness(bl) & 0xff;

    //mipi_dsi_dcs_write_seq(dsi, 0x15, 0x00, 0x02, 0x51, 0xEF);
    //mipi_dsi_dcs_write_seq(dsi, 0x51, brightness);
    //mipi_dsi_dcs_write(dsi, 0x51, &brightness, 1);
    mipi_dsi_dcs_set_display_brightness(dsi, brightness);
    dev_info(dev, "boe_bl_update_status: brightness = %u\n", brightness);

    return 0;
}

static int bv050fwm_bl_get_brightness(struct backlight_device *bl)
{
    struct mipi_dsi_device *dsi = bl_get_data(bl);
        struct device *dev = &bl->dev;
    u16 brightness = 0;
    int ret;
    ret = mipi_dsi_dcs_get_display_brightness(dsi, &brightness);
        if (ret < 0) {
             dev_err(&bl->dev, "Failed to get display brightness: %d\n", ret);
             return ret;
        }
        dev_info(&bl->dev, "Read brightness: %u (0x%x)\n", brightness, brightness);
    return brightness & 0xff;
}

static const struct backlight_ops bv050fwm_bl_ops = {
    .update_status = bv050fwm_bl_update_status,
    .get_brightness = bv050fwm_bl_get_brightness,
};

static struct backlight_device *
bv050fwm_create_backlight(struct mipi_dsi_device *dsi)
{
    struct device *dev = &dsi->dev;
    const struct backlight_properties props = {
        .type = BACKLIGHT_RAW,
        .brightness = 255,
        .max_brightness = 255,
    };

    return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
                          &bv050fwm_bl_ops, &props);
}

static const struct drm_display_mode bv050fwm_mode_42hz = {
    .clock = 25200,              // 时钟频率，单位是 kHz
    .hdisplay = 480,              // 水平可视区域
    .hsync_start = 480 + 92,      // 水平同步起始 = hactive + hfront-porch
    .hsync_end = 480 + 92 + 12,   // 水平同步结束 = hsync_start + hsync-len
    .htotal = 480 + 92 + 12 + 88, // 水平总长度 = hactive + hfront + hsync-len + hback-porch
    .vdisplay = 854,              // 垂直可视区域
    .vsync_start = 854 + 18,      // 垂直同步起始 = vactive + vfront-porch
    .vsync_end = 854 + 18 + 4,    // 垂直同步结束 = vsync_start + vsync-len
    .vtotal = 854 + 18 + 4 + 18,  // 垂直总长度 = vactive + vfront-porch + vsync-len + vback-porch
    .width_mm = 62,
    .height_mm = 110,
};

static const struct drm_display_mode bv050fwm_mode_60hz = {
	.clock          = 83330,

	.hdisplay	= 720,
	.hsync_start	= 720 + 239,
	.hsync_end	= 720 + 239 + 33,
	.htotal		= 720 + 239 + 33 + 50,

	.vdisplay	= 1280,
	.vsync_start	= 1280 + 20,
	.vsync_end	= 1280 + 20 + 2,
	.vtotal		= 1280 + 20 + 2 + 30,

	.width_mm	= 90,
	.height_mm	= 151,
};

static int bv050fwm_get_modes(struct drm_panel *panel,
                    struct drm_connector *connector)
{
    struct drm_display_mode *mode;

    mode = drm_mode_duplicate(connector->dev, &bv050fwm_mode_42hz);
    if (!mode) {
        dev_err(panel->dev, "failed to add mode\n");
        return -ENOMEM;
    }
    drm_mode_set_name(mode);

    mode->type = DRM_MODE_TYPE_DRIVER;
    connector->display_info.width_mm = mode->width_mm;
    connector->display_info.height_mm = mode->height_mm;
    drm_mode_probed_add(connector, mode);

    mode = drm_mode_duplicate(connector->dev, &bv050fwm_mode_60hz);
    if (!mode) {
        dev_err(panel->dev, "failed to add mode\n");
        return -ENOMEM;
    }
    drm_mode_set_name(mode);

    mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
    connector->display_info.width_mm = mode->width_mm;
    connector->display_info.height_mm = mode->height_mm;
    drm_mode_probed_add(connector, mode);

    return 1;
}

static const struct drm_panel_funcs bv050fwm_panel_funcs = {
    .unprepare = bv050fwm_unprepare,
    .prepare = bv050fwm_prepare,
    .get_modes = bv050fwm_get_modes,
};

static int bv050fwm_probe(struct mipi_dsi_device *dsi)
{
    struct device *dev = &dsi->dev;
    struct bv050fwm *ctx;
    int ret;

// dev_info(dev, "bv050fwm module compiled on: %s %s\n", __DATE__, __TIME__);

    ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
    if (!ctx)
        return -ENOMEM;

    ctx->supplies[0].supply = "power";

    ret = devm_regulator_bulk_get(dev, ARRAY_SIZE(ctx->supplies),
                      ctx->supplies);
    if (ret < 0) {
        dev_err(dev, "Failed to get regulators: %d\n", ret);
        return ret;
    }


    ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
    if (IS_ERR(ctx->reset_gpio)) {
        ret = PTR_ERR(ctx->reset_gpio);
        dev_err(dev, "Failed to get reset-gpios: %d\n", ret);
        return ret;
    }

    ctx->backlight_gpio = devm_gpiod_get(dev, "backlight", GPIOD_OUT_HIGH);
    if (IS_ERR(ctx->backlight_gpio)) {
        ret = PTR_ERR(ctx->backlight_gpio);
        dev_err(dev, "Failed to get backlight-gpios: %d\n", ret);
        return ret;
    }

    ctx->dsi = dsi;
    mipi_dsi_set_drvdata(dsi, ctx);

    dsi->lanes = 2;
    dsi->format = MIPI_DSI_FMT_RGB888;
    dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST | MIPI_DSI_MODE_LPM;

    drm_panel_init(&ctx->panel, dev, &bv050fwm_panel_funcs,
               DRM_MODE_CONNECTOR_DSI);

    dev_info(dev, "drm_panel_init ok\n");

    ctx->panel.prepare_prev_first = true;

    ret = drm_panel_of_backlight(&ctx->panel);
    if (ret)
    {
        dev_err(dev, "Failed to set backlight: %d\n", ret);
        return ret;
    }
    // 使用mipi dcs调光
    ctx->panel.backlight = bv050fwm_create_backlight(dsi);
    if (IS_ERR(ctx->panel.backlight))
        return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
                     "Failed to create backlight\n");

    drm_panel_add(&ctx->panel);
    dev_info(dev, "drm_panel_add ok\n");

    ret = mipi_dsi_attach(dsi);
    if (ret < 0) {
        dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
        drm_panel_remove(&ctx->panel);
        return ret;
    }
    dev_info(dev, "mipi_dsi_attach ok\n");

    return 0;
}

static void bv050fwm_remove(struct mipi_dsi_device *dsi)
{
    struct bv050fwm *ctx = mipi_dsi_get_drvdata(dsi);
    int ret;

    ret = mipi_dsi_detach(dsi);
    if (ret < 0)
        dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

    drm_panel_remove(&ctx->panel);
}

static const struct of_device_id bv050fwm_of_match[] = {
    { .compatible = "boe,bv050fwm" },
    { /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, bv050fwm_of_match);

static struct mipi_dsi_driver bv050fwm_driver = {
    .probe = bv050fwm_probe,
    .remove = bv050fwm_remove,
    .driver = {
        .name = "panel-boe-bv050fwm",
        .of_match_table = bv050fwm_of_match,
    },
};
module_mipi_dsi_driver(bv050fwm_driver);

MODULE_DESCRIPTION("MIPI-DSI based Panel Driver for bv050fwm LCD Display Module");
MODULE_LICENSE("GPL");

