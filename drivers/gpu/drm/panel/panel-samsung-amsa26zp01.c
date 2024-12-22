// SPDX-License-Identifier: GPL-2.0-only
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2015-2017, The Linux Foundation. All rights reserved.
//   Copyright (c) 2024 chalkin Deng <chalkin@chalkin@yeah.net>

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>

#include <video/mipi_display.h>

#include <drm/display/drm_dsc.h>
#include <drm/display/drm_dsc_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct samsung_amsa26zp01 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct drm_dsc_config dsc;
	struct gpio_desc *reset_gpio;
	struct regulator_bulk_data supplies[2];
};

static inline
struct samsung_amsa26zp01 *to_samsung_amsa26zp01(struct drm_panel *panel)
{
	return container_of(panel, struct samsung_amsa26zp01, panel);
}

static void samsung_amsa26zp01_reset(struct samsung_amsa26zp01 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(2000, 3000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(400);
}

static void samsung_amsa26zp01_on(struct mipi_dsi_multi_context *dsi_ctx)
{

	mipi_dsi_generic_write_seq_multi(dsi_ctx, 0xf0, 0x5a, 0x5a);
	mipi_dsi_generic_write_seq_multi(dsi_ctx, 0xf1, 0x5a, 0x5a);
	mipi_dsi_generic_write_seq_multi(dsi_ctx, 0xfc, 0x5a, 0x5a);

	mipi_dsi_generic_write_seq_multi(dsi_ctx, 0xf8, 0x58, 0x00, 0x10, 0x54);
	mipi_dsi_generic_write_seq_multi(dsi_ctx, 0xf9, 0x04, 0x85);
	/* refresh rate Transition */
	/* 60 Hz */
	// mipi_dsi_generic_write_seq_multi(dsi_ctx, 0x60, 0x00);
	/* 120 Hz */
	mipi_dsi_generic_write_seq_multi(dsi_ctx, 0x60, 0x20);
	mipi_dsi_msleep(dsi_ctx, 50);
	// #define SAMSUNG_BRIGHTNESS_MODE	0x53
	// mipi_dsi_generic_write_seq_multi(dsi_ctx, SAMSUNG_BRIGHTNESS_MODE, 0xE0);
	mipi_dsi_generic_write_seq_multi(dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x28);  /* 0x20:Normal mode + Smooth dimming off; 0x28: Normal mode + Smooth dimming on */
	mipi_dsi_generic_write_seq_multi(dsi_ctx, 0xf8, 0x58, 0x00, 0x30, 0x35);
	mipi_dsi_msleep(dsi_ctx, 100);
	mipi_dsi_generic_write_seq_multi(dsi_ctx, 0xf9, 0xc0, 0x2b);
	mipi_dsi_msleep(dsi_ctx, 100);
	mipi_dsi_dcs_set_display_on_multi(dsi_ctx);
}

static int samsung_amsa26zp01_disable(struct drm_panel *panel)
{
	struct samsung_amsa26zp01 *ctx = to_samsung_amsa26zp01(panel);
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = dsi };

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);

	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 100);

	return dsi_ctx.accum_err;
}

static int samsung_amsa26zp01_prepare(struct drm_panel *panel)
{
	struct samsung_amsa26zp01 *ctx = to_samsung_amsa26zp01(panel);
	struct drm_dsc_picture_parameter_set pps;
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	dsi_ctx.accum_err = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (dsi_ctx.accum_err)
		return dsi_ctx.accum_err;

	samsung_amsa26zp01_reset(ctx);

	samsung_amsa26zp01_on(&dsi_ctx);

	drm_dsc_pps_payload_pack(&pps, &ctx->dsc);
	// print_hex_dump_bytes("pps: ", DUMP_PREFIX_OFFSET, &pps, sizeof(pps));
	mipi_dsi_picture_parameter_set_multi(&dsi_ctx, &pps);
	mipi_dsi_compression_mode_ext_multi(&dsi_ctx, true, MIPI_DSI_COMPRESSION_DSC, 0);
	mipi_dsi_msleep(&dsi_ctx, 28);

	if (dsi_ctx.accum_err) {
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	}

	return dsi_ctx.accum_err;
}

static int samsung_amsa26zp01_unprepare(struct drm_panel *panel)
{
	struct samsung_amsa26zp01 *ctx = to_samsung_amsa26zp01(panel);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode samsung_2k_dsc_60_mode = {
	.clock = (2560 + 12 + 12 + 20) * (1600 + 12 + 2 + 16) * 60 / 1000,
	.hdisplay = 2560,
	.hsync_start = 2560 + 12,
	.hsync_end = 2560 + 12 + 12,
	.htotal = 2560 + 12 + 12 + 20,
	.vdisplay = 1600,
	.vsync_start = 1600 + 12,
	.vsync_end = 1600 + 12 + 2,
	.vtotal = 1600 + 12 + 2 + 16,
	.width_mm = 271,
	.height_mm = 170,
	.type = DRM_MODE_TYPE_DRIVER,
};

static const struct drm_display_mode samsung_2k_dsc_120_mode = {
	.clock = (2560 + 20 + 12 + 20) * (1600 + 8 + 2 + 16) * 120 / 1000,
	.hdisplay = 2560,
	.hsync_start = 2560 + 20,
	.hsync_end = 2560 + 20 + 12,
	.htotal = 2560 + 20 + 12 + 20,
	.vdisplay = 1600,
	.vsync_start = 1600 + 8,
	.vsync_end = 1600 + 8 + 2,
	.vtotal = 1600 + 8 + 2 + 16,
	.width_mm = 271,
	.height_mm = 170,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int samsung_amsa26zp01_get_modes(struct drm_panel *panel,
				     struct drm_connector *connector)
{
	int count = 0;

	/* Initialize 120Hz first */
	count += drm_connector_helper_get_modes_fixed(connector, &samsung_2k_dsc_120_mode);
	count += drm_connector_helper_get_modes_fixed(connector, &samsung_2k_dsc_60_mode);

	return count;

}

static const struct drm_panel_funcs samsung_amsa26zp01_panel_funcs = {
	.prepare = samsung_amsa26zp01_prepare,
	.unprepare  = samsung_amsa26zp01_unprepare,
	.disable = samsung_amsa26zp01_disable,
	.get_modes = samsung_amsa26zp01_get_modes,
};

static int samsung_amsa26zp01_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	ret = mipi_dsi_dcs_set_display_brightness(dsi, brightness);
	if (ret < 0)
		return ret;

	return 0;
}

static const struct backlight_ops samsung_amsa26zp01_bl_ops = {
	.update_status = samsung_amsa26zp01_bl_update_status,
};

static struct backlight_device *
samsung_amsa26zp01_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 1024,
		.max_brightness = 2047,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &samsung_amsa26zp01_bl_ops, &props);
}

static int samsung_amsa26zp01_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct samsung_amsa26zp01 *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->supplies[0].supply = "vddio";
	ctx->supplies[1].supply = "vdd";

	ret = devm_regulator_bulk_get(dev, ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret)
		return dev_err_probe(dev, ret,
				     "Failed to get vddio regulator\n");

	ret = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret)
		dev_err(dev, "Failed to enable regulators: %d\n", ret);

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	drm_panel_init(&ctx->panel, dev, &samsung_amsa26zp01_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);
	ctx->panel.prepare_prev_first = true;

	drm_panel_add(&ctx->panel);

	ctx->panel.backlight = samsung_amsa26zp01_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				     "Failed to create backlight\n");


	/* This panel only supports DSC; unconditionally enable it */
	dsi->dsc = &ctx->dsc;

	ctx->dsc.dsc_version_major = 1;
	ctx->dsc.dsc_version_minor = 1;

	/* TODO: Pass slice_per_pkt = 2 */
	ctx->dsc.slice_height = 100;
	ctx->dsc.slice_width = 1280;
	/*
	 * TODO: hdisplay should be read from the selected mode once
	 * it is passed back to drm_panel (in prepare?)
	 */
	WARN_ON(2560 % ctx->dsc.slice_width);
	ctx->dsc.slice_count = 2560 / ctx->dsc.slice_width;
	ctx->dsc.bits_per_component = 8;
	ctx->dsc.bits_per_pixel = 8 << 4; /* 4 fractional bits */
	ctx->dsc.block_pred_enable = true;

	ret = devm_mipi_dsi_attach(dev, dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void samsung_amsa26zp01_remove(struct mipi_dsi_device *dsi)
{
	struct samsung_amsa26zp01 *ctx = mipi_dsi_get_drvdata(dsi);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id samsung_amsa26zp01_of_match[] = {
	{ .compatible = "samsung,amsa26zp01" }, //amsa26zp01
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, samsung_amsa26zp01_of_match);

static struct mipi_dsi_driver samsung_amsa26zp01_driver = {
	.probe = samsung_amsa26zp01_probe,
	.remove = samsung_amsa26zp01_remove,
	.driver = {
		.name = "panel-samsung-amsa26zp01",
		.of_match_table = samsung_amsa26zp01_of_match,
	},
};
module_mipi_dsi_driver(samsung_amsa26zp01_driver);

MODULE_AUTHOR("Chalkin <chalkin@yeah.net>");
MODULE_DESCRIPTION("DRM driver for Samsung AMSA26ZP01 DSC DSI panel");
MODULE_LICENSE("GPL");