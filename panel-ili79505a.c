// SPDX-License-Identifier: GPL-2.0-only
/* ILI79505A TDDI display: DSI video only, vendor initialization over I2C. */
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

/* The Ilitek V3 I2C driver owns the 0x41 client and serializes its packets. */
extern int ili79505a_panel_run_test_sequence(void);
extern int ili79505a_panel_reset_touch(void);
extern int ili79505a_panel_touch_ready(void);

struct ili79505a_panel {
    struct drm_panel panel;
    struct mipi_dsi_device *dsi;
    struct gpio_desc *reset;
    struct gpio_desc *backlight;
    struct gpio_desc *bias;
    bool prepared;
};

static inline struct ili79505a_panel *to_ili79505a(struct drm_panel *panel)
{
    return container_of(panel, struct ili79505a_panel, panel);
}

static int ili79505a_prepare(struct drm_panel *panel)
{
    struct ili79505a_panel *ctx = to_ili79505a(panel);
    int ret;

    if (ctx->prepared)
        return 0;

    gpiod_set_value_cansleep(ctx->backlight, 0);
    gpiod_set_value_cansleep(ctx->bias, 1);
    msleep(20);

    /* The reset polarity comes from reset-gpios in the new overlay. */
    gpiod_set_value_cansleep(ctx->reset, 0);
    usleep_range(1000, 2000);
    gpiod_set_value_cansleep(ctx->reset, 1);
    usleep_range(5000, 6000);
    gpiod_set_value_cansleep(ctx->reset, 0);
    usleep_range(10000, 11000);

    /* P02 is owned by the Ilitek driver; reset touch after LCD reset and
     * before sending any I2C DDI packets. This also covers warm reboots.
     */
    ret = ili79505a_panel_reset_touch();
    if (ret) {
        dev_err(panel->dev, "ILI79505A touch reset failed: %d\n", ret);
        goto reset_failed;
    }

    /* No DSI commands: all TEST register packets go through Ilitek I2C. */
    ret = ili79505a_panel_run_test_sequence();
    if (ret) {
        dev_err(panel->dev, "ILI79505A I2C initialization failed: %d\n", ret);
        goto reset_failed;
    }

    /* Register touch only when post-bias, post-reset FW metadata and the
     * full display initialization are available. Keep video working if
     * touch is temporarily unavailable, and log it for diagnosis.
     */
    ret = ili79505a_panel_touch_ready();
    if (ret)
        dev_warn(panel->dev, "ILI79505A touch not ready: %d\n", ret);

    ctx->prepared = true;
    return 0;

reset_failed:
    gpiod_set_value_cansleep(ctx->reset, 1);
    gpiod_set_value_cansleep(ctx->bias, 0);
    return ret;
}

static int ili79505a_enable(struct drm_panel *panel)
{
    struct ili79505a_panel *ctx = to_ili79505a(panel);

    if (!ctx->prepared)
        return -EINVAL;
    gpiod_set_value_cansleep(ctx->backlight, 1);
    return 0;
}

static int ili79505a_disable(struct drm_panel *panel)
{
    struct ili79505a_panel *ctx = to_ili79505a(panel);

    gpiod_set_value_cansleep(ctx->backlight, 0);
    return 0;
}

static int ili79505a_unprepare(struct drm_panel *panel)
{
    struct ili79505a_panel *ctx = to_ili79505a(panel);

    if (!ctx->prepared)
        return 0;

    gpiod_set_value_cansleep(ctx->backlight, 0);
    gpiod_set_value_cansleep(ctx->reset, 1);
    msleep(10);
    gpiod_set_value_cansleep(ctx->bias, 0);
    ctx->prepared = false;
    return 0;
}

/* Bridge_79505A_HKC7: 69 MHz, 720/25/6/25, 1280/150/2/50. */
static const struct drm_display_mode ili79505a_mode = {
    .clock = 69000,
    .hdisplay = 720,
    .hsync_start = 720 + 25,
    .hsync_end = 720 + 25 + 6,
    .htotal = 720 + 25 + 6 + 25,
    .vdisplay = 1280,
    .vsync_start = 1280 + 150,
    .vsync_end = 1280 + 150 + 2,
    .vtotal = 1280 + 150 + 2 + 50,
    .width_mm = 90,
    .height_mm = 151,
};

static int ili79505a_get_modes(struct drm_panel *panel,
                               struct drm_connector *connector)
{
    struct drm_display_mode *mode;

    mode = drm_mode_duplicate(connector->dev, &ili79505a_mode);
    if (!mode)
        return -ENOMEM;
    drm_mode_set_name(mode);
    mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
    connector->display_info.width_mm = mode->width_mm;
    connector->display_info.height_mm = mode->height_mm;
    drm_mode_probed_add(connector, mode);
    return 1;
}

static const struct drm_panel_funcs ili79505a_funcs = {
    .prepare = ili79505a_prepare,
    .enable = ili79505a_enable,
    .disable = ili79505a_disable,
    .unprepare = ili79505a_unprepare,
    .get_modes = ili79505a_get_modes,
};

static int ili79505a_probe(struct mipi_dsi_device *dsi)
{
    struct ili79505a_panel *ctx;
    int ret;

    ctx = devm_kzalloc(&dsi->dev, sizeof(*ctx), GFP_KERNEL);
    if (!ctx)
        return -ENOMEM;

    ctx->reset = devm_gpiod_get(&dsi->dev, "reset", GPIOD_OUT_LOW);
    if (IS_ERR(ctx->reset))
        return dev_err_probe(&dsi->dev, PTR_ERR(ctx->reset), "reset GPIO\n");
    ctx->backlight = devm_gpiod_get(&dsi->dev, "backlight", GPIOD_OUT_LOW);
    if (IS_ERR(ctx->backlight))
        return dev_err_probe(&dsi->dev, PTR_ERR(ctx->backlight), "backlight GPIO\n");
    ctx->bias = devm_gpiod_get(&dsi->dev, "power", GPIOD_OUT_LOW);
    if (IS_ERR(ctx->bias))
        return dev_err_probe(&dsi->dev, PTR_ERR(ctx->bias), "bias GPIO\n");

    ctx->dsi = dsi;
    mipi_dsi_set_drvdata(dsi, ctx);
    dsi->lanes = 2;
    dsi->format = MIPI_DSI_FMT_RGB888;
    dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST;

    drm_panel_init(&ctx->panel, &dsi->dev, &ili79505a_funcs,
                   DRM_MODE_CONNECTOR_DSI);
    ctx->panel.prepare_prev_first = true;
    drm_panel_add(&ctx->panel);

    ret = mipi_dsi_attach(dsi);
    if (ret) {
        drm_panel_remove(&ctx->panel);
        return ret;
    }
    return 0;
}

static void ili79505a_remove(struct mipi_dsi_device *dsi)
{
    struct ili79505a_panel *ctx = mipi_dsi_get_drvdata(dsi);

    mipi_dsi_detach(dsi);
    drm_panel_remove(&ctx->panel);
}

static const struct of_device_id ili79505a_of_match[] = {
    { .compatible = "ilitek,ili79505a-panel" },
    { }
};
MODULE_DEVICE_TABLE(of, ili79505a_of_match);

static struct mipi_dsi_driver ili79505a_driver = {
    .probe = ili79505a_probe,
    .remove = ili79505a_remove,
    .driver = {
        .name = "panel-ili79505a",
        .of_match_table = ili79505a_of_match,
    },
};
module_mipi_dsi_driver(ili79505a_driver);

MODULE_SOFTDEP("pre: ilitek_v3_driver");
MODULE_DESCRIPTION("ILI79505A TDDI panel with I2C-only DDI initialization");
MODULE_LICENSE("GPL");
