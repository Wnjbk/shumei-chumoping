// SPDX-License-Identifier: GPL-2.0
/*
 * GT911 touchscreen driver (polling mode)
 *
 * For panels where the GT911 INT pin is routed through an I2C GPIO expander
 * (e.g. PCA9555) and cannot generate CPU interrupts directly.
 *
 * Polls the GT911 status register (0x814E) at a configurable interval.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#define GT911_REG_CMD            0x8040
#define GT911_REG_CFG_VER        0x8047
#define GT911_REG_CFG_START      0x8047
#define GT911_REG_TOUCH_STATUS   0x814E
#define GT911_REG_TOUCH_DATA     0x814F
#define GT911_REG_PRODUCT_ID     0x8140

#define GT911_CMD_READ_COORDS    0x00
#define GT911_CMD_SLEEP          0x05

#define GT911_MAX_TOUCH          5
#define GT911_POLL_MS            10
#define GT911_CFG_SIZE           186

static const u8 gt911_default_cfg[GT911_CFG_SIZE] = {
	0x00,              // [0]   config version
	0xD0, 0x02,        // [1-2] X max = 720
	0x00, 0x05,        // [3-4] Y max = 1280
	0x05,              // [5]   touch number
	0x01,              // [6]   module switch 1
	0x00,              // [7]   module switch 2 (0=single sided)
	0x01,              // [8]   shake count / debounce
	0x00,              // [9]   filter
	0x00,              // [10]  large touch
	0x00,              // [11]  noise reduction
	0x50,              // [12]  screen touch level
	0x30,              // [13]  screen leave level
	0x00,              // [14]  low power control
	0x5A,              // [15]  refresh rate
	0x32, 0x00,        // [16-17] X threshold group 1
	0x19, 0x00,        // [18-19] X threshold group 2
	0x32, 0x00,        // [20-21] Y threshold group 1
	0x19, 0x00,        // [22-23] Y threshold group 2
	0x00, 0x00,        // [24-25] soft key map
	0x14,              // [26]  sensor channel num X
	0x22,              // [27]  sensor channel num Y
	0x00, 0x00, 0x00, 0x00, // [28-31]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [32-39]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [40-47]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [48-55]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [56-63]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [64-71]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [72-79]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [80-87]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [88-95]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [96-103]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [104-111]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [112-119]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [120-127]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [128-135]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [136-143]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [144-151]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [152-159]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [160-167]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [168-175]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [176-183]
	0x99, 0x01,        // [184-185] checksum placeholder (computed at init)
};

struct gt911_data {
	struct i2c_client *client;
	struct input_dev *input;
	struct gpio_desc *reset_gpio;
	struct gpio_desc *irq_gpio;
	struct delayed_work poll_work;
	unsigned int poll_ms;
	bool suspended;
};

static int gt911_i2c_read(struct i2c_client *client, u16 reg, u8 *buf, int len)
{
	struct i2c_msg msgs[2];
	u8 reg_buf[2];
	int ret;

	reg_buf[0] = (reg >> 8) & 0xFF;
	reg_buf[1] = reg & 0xFF;

	msgs[0].addr = client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 2;
	msgs[0].buf = reg_buf;

	msgs[1].addr = client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = len;
	msgs[1].buf = buf;

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret < 0)
		return ret;
	if (ret != 2)
		return -EIO;
	return 0;
}

static int gt911_i2c_write(struct i2c_client *client, u16 reg, const u8 *buf, int len)
{
	struct i2c_msg msg;
	u8 *tx_buf;
	int ret;

	tx_buf = kmalloc(2 + len, GFP_KERNEL);
	if (!tx_buf)
		return -ENOMEM;

	tx_buf[0] = (reg >> 8) & 0xFF;
	tx_buf[1] = reg & 0xFF;
	memcpy(tx_buf + 2, buf, len);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 2 + len;
	msg.buf = tx_buf;

	ret = i2c_transfer(client->adapter, &msg, 1);
	kfree(tx_buf);
	if (ret < 0)
		return ret;
	if (ret != 1)
		return -EIO;
	return 0;
}

static int gt911_reset(struct gt911_data *ts)
{
	if (!ts->reset_gpio)
		return 0;

	gpiod_set_value_cansleep(ts->reset_gpio, 0);
	usleep_range(5000, 10000);
	gpiod_set_value_cansleep(ts->reset_gpio, 1);
	msleep(55);
	return 0;
}

static int gt911_write_config(struct gt911_data *ts, u8 *cfg, int len)
{
	return gt911_i2c_write(ts->client, GT911_REG_CFG_START, cfg, len);
}

static int gt911_init_config(struct gt911_data *ts)
{
	u8 cfg[GT911_CFG_SIZE];
	int ret, i;
	u16 sum;
	u8 ver[2];

	/* Check if GT911 already has valid config */
	ret = gt911_i2c_read(ts->client, GT911_REG_CFG_VER, ver, 2);
	if (ret == 0 && (ver[0] != 0 || ver[1] != 0)) {
		dev_info(&ts->client->dev, "GT911 config v%02x%02x, keeping\n",
			 ver[1], ver[0]);
		goto skip_config;
	}

	dev_info(&ts->client->dev, "Writing GT911 config for %ux%u\n",
		 ts->input->absinfo[ABS_MT_POSITION_X].maximum,
		 ts->input->absinfo[ABS_MT_POSITION_Y].maximum);
	memcpy(cfg, gt911_default_cfg, GT911_CFG_SIZE);

	/* Config layout at 0x8047: bytes 0-1=version, 2-3=X, 4-5=Y */
	cfg[1] = 0x00;
	cfg[2] = ts->input->absinfo[ABS_MT_POSITION_X].maximum & 0xFF;
	cfg[3] = (ts->input->absinfo[ABS_MT_POSITION_X].maximum >> 8) & 0xFF;
	cfg[4] = ts->input->absinfo[ABS_MT_POSITION_Y].maximum & 0xFF;
	cfg[5] = (ts->input->absinfo[ABS_MT_POSITION_Y].maximum >> 8) & 0xFF;

	/* Checksum: sum of bytes 0..183 + stored at [184-185] = 0 mod 65536 */
	sum = 0;
	for (i = 0; i < GT911_CFG_SIZE - 2; i++)
		sum += cfg[i];
	sum = (u16)(0 - sum);
	cfg[GT911_CFG_SIZE - 2] = sum & 0xFF;
	cfg[GT911_CFG_SIZE - 1] = (sum >> 8) & 0xFF;

	dev_info(&ts->client->dev, "Config checksum = 0x%04x\n", sum);

	ret = gt911_write_config(ts, cfg, GT911_CFG_SIZE);
	if (ret) {
		dev_err(&ts->client->dev, "Failed to write config: %d\n", ret);
		return ret;
	}
	msleep(100);

skip_config:
	/* Set read-coordinates mode */
	{
		u8 cmd = 0x00;
		gt911_i2c_write(ts->client, GT911_REG_CMD, &cmd, 1);
	}
	return 0;
}

static void gt911_poll_work(struct work_struct *work)
{
	struct gt911_data *ts = container_of(work, struct gt911_data,
					      poll_work.work);
	u8 buf[GT911_MAX_TOUCH * 8 + 1];
	int ret, i, npoints;

	if (ts->suspended)
		goto reschedule;

	ret = gt911_i2c_read(ts->client, GT911_REG_TOUCH_STATUS,
			     buf, sizeof(buf));

	if (ret)
		goto reschedule;

	if (!(buf[0] & 0x80))
		goto reschedule;

	npoints = buf[0] & 0x0F;
	if (npoints > GT911_MAX_TOUCH)
		goto reschedule;

	for (i = 0; i < npoints; i++) {
		u8 *p = &buf[1 + i * 8];
		unsigned int id, x, y;

		id = p[0] & 0x0F;
		x = p[1] | (p[2] << 8);
		y = p[3] | (p[4] << 8);

		input_mt_slot(ts->input, id);
		input_mt_report_slot_state(ts->input, MT_TOOL_FINGER, true);
		input_report_abs(ts->input, ABS_MT_POSITION_X, x);
		input_report_abs(ts->input, ABS_MT_POSITION_Y, y);
	}

	input_mt_sync_frame(ts->input);
	input_sync(ts->input);

	/* Clear status register to acknowledge */
	buf[0] = 0x00;
	gt911_i2c_write(ts->client, GT911_REG_TOUCH_STATUS, buf, 1);

reschedule:
	schedule_delayed_work(&ts->poll_work,
			      msecs_to_jiffies(ts->poll_ms));
}

static int gt911_probe(struct i2c_client *client)
{
	struct gt911_data *ts;
	struct input_dev *input;
	struct device *dev = &client->dev;
	int ret;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	ts = devm_kzalloc(dev, sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;

	ts->client = client;
	i2c_set_clientdata(client, ts);

	ts->irq_gpio = devm_gpiod_get_optional(dev, "irq", GPIOD_OUT_LOW);
	if (IS_ERR(ts->irq_gpio))
		return dev_err_probe(dev, PTR_ERR(ts->irq_gpio),
				     "Failed to get IRQ GPIO\n");

	ts->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(ts->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ts->reset_gpio),
				     "Failed to get reset GPIO\n");

	/*
	 * Hardware reset for stable 0x5D address:
	 * INT stays output-low permanently (polling mode, no IRQ needed).
	 * RST: low → 1ms → high. INT=low at RST rising → addr 0x5D.
	 */
	if (ts->reset_gpio && ts->irq_gpio) {
		usleep_range(1000, 2000);
		gpiod_set_value_cansleep(ts->reset_gpio, 1);
		msleep(55);
	}

	/* Verify chip is alive by reading product ID */
	{
		u8 pid[4];
		ret = gt911_i2c_read(ts->client, GT911_REG_PRODUCT_ID, pid, 4);
		if (ret || pid[0] == 0) {
			dev_err(dev, "GT911 not responding (ret=%d pid=%02x%02x%02x%02x)\n",
				ret, pid[0], pid[1], pid[2], pid[3]);
			return -ENODEV;
		}
		dev_info(dev, "GT911 product ID: %02x%02x%02x%02x\n",
			 pid[0], pid[1], pid[2], pid[3]);
	}

	input = devm_input_allocate_device(dev);
	if (!input)
		return -ENOMEM;

	ts->input = input;
	ts->poll_ms = GT911_POLL_MS;

	input->name = "GT911 Touchscreen (polling)";
	input->phys = "input/ts";
	input->id.bustype = BUS_I2C;
	input->dev.parent = dev;

	input_set_abs_params(input, ABS_MT_POSITION_X, 0, 1280, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, 720, 0, 0);

	ret = input_mt_init_slots(input, GT911_MAX_TOUCH,
				  INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);
	if (ret)
		return ret;

	ret = input_register_device(input);
	if (ret)
		return ret;

	gt911_init_config(ts);

	INIT_DELAYED_WORK(&ts->poll_work, gt911_poll_work);
	schedule_delayed_work(&ts->poll_work,
			      msecs_to_jiffies(ts->poll_ms));

	dev_info(dev, "GT911 polling driver registered (poll=%ums)\n",
		 ts->poll_ms);
	return 0;
}

static void gt911_remove(struct i2c_client *client)
{
	struct gt911_data *ts = i2c_get_clientdata(client);

	cancel_delayed_work_sync(&ts->poll_work);
}

static int gt911_suspend(struct device *dev)
{
	struct gt911_data *ts = dev_get_drvdata(dev);
	u8 cmd = GT911_CMD_SLEEP;

	ts->suspended = true;
	cancel_delayed_work_sync(&ts->poll_work);

	gt911_i2c_write(ts->client, GT911_REG_CMD, &cmd, 1);
	return 0;
}

static int gt911_resume(struct device *dev)
{
	struct gt911_data *ts = dev_get_drvdata(dev);

	ts->suspended = false;
	gt911_reset(ts);
	schedule_delayed_work(&ts->poll_work,
			      msecs_to_jiffies(ts->poll_ms));
	return 0;
}

static DEFINE_SIMPLE_DEV_PM_OPS(gt911_pm_ops, gt911_suspend, gt911_resume);

#ifdef CONFIG_OF
static const struct of_device_id gt911_of_match[] = {
	{ .compatible = "goodix,gt911" },
	{ .compatible = "goodix,gt9110" },
	{ .compatible = "goodix,gt9271" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, gt911_of_match);
#endif

static const struct i2c_device_id gt911_i2c_ids[] = {
	{ "gt911", 0 },
	{ "gt9110", 0 },
	{ "gt9271", 0 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, gt911_i2c_ids);

static struct i2c_driver gt911_driver = {
	.driver = {
		.name = "gt911_poll",
		.of_match_table = of_match_ptr(gt911_of_match),
		.pm = pm_sleep_ptr(&gt911_pm_ops),
	},
	.probe = gt911_probe,
	.remove = gt911_remove,
	.id_table = gt911_i2c_ids,
};

module_i2c_driver(gt911_driver);

MODULE_AUTHOR("XC");
MODULE_DESCRIPTION("GT911 touchscreen driver (polling mode)");
MODULE_LICENSE("GPL");
