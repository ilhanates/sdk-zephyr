/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>

#include "edtt_driver.h"

#define EDTT_TEST

#ifndef EDTT_TEST
#include <target_comm.h>
#include "tca_transport.h"
target_comm_t g_target_comm;
static tca_transport_config_t trans_cfg;
#endif


#define CMD_CUSTOM_ADV_START	 201
#define CMD_CUSTOM_SCANNER_START 202
#define CMD_CUSTOM_SCANNER_STOP  203

#define ADV_INTERVAL 50

static uint8_t adv_data[] = { 0xAA, 0xBB, 0xCC };
static const struct bt_data ad[] = { BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_data, 3), };
struct bt_le_adv_param param = BT_LE_ADV_PARAM_INIT(0, ADV_INTERVAL, ADV_INTERVAL, NULL);

#define CMD_CUSTOM_SCANNER_START 202

struct bt_le_scan_param scan_param = {
	.type       = BT_HCI_LE_SCAN_PASSIVE,
	.options    = BT_LE_SCAN_OPT_NONE,
	.interval   = 0x0010,
	.window     = 0x0010,
};

static void scan_recv_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
		    struct net_buf_simple *buf)
{
	printk("scan_recv_cb:%d\n", k_uptime_get_32());
}


static int adv_start(void)
{
	int err = bt_le_adv_start(&param, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
	}

	return err;
}

static int scan_start(void)
{
	int err = bt_le_scan_start(&scan_param, scan_recv_cb);
	if (err) {
		printk("Starting scanning failed (err %d)\n", err);
	}

	return err;
}

static int scan_stop(void)
{
	int err = bt_le_scan_stop();
	if (err) {
		printk("bt_le_scan_stop failed with %d, resetting\n", err);
	}

	return err;
}

int main(void)
{
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

#if defined(EDTT_TEST)
	enable_edtt_mode();
	set_edtt_autoshutdown(true);
	edtt_start();
#else
	tca_init(&g_target_comm, &trans_cfg);
#endif

	uint16_t command = 0;
	while (1) {
		#if defined(EDTT_TEST)
			uint16_t size;
			edtt_read((uint8_t *)&command, sizeof(command), EDTTT_BLOCK);
			command = sys_le16_to_cpu(command);
			edtt_read((uint8_t *)&size, sizeof(size), EDTTT_BLOCK);
			size = sys_le16_to_cpu(size);
			if (size && size < 256) {
				unsigned char data[256]={};
				edtt_read(data, size, EDTTT_BLOCK);
			}
			// printk("command 0x%04X received (size %u)\n", command, size);
		#else
			tca_send_and_receive(&g_target_comm);
		#endif

		err = 0;
		switch (command) {
			case CMD_CUSTOM_ADV_START:
				err = adv_start();
				break;
			case CMD_CUSTOM_SCANNER_START:
				err = scan_start();
				break;
			case CMD_CUSTOM_SCANNER_STOP:
				err = scan_stop();
				break;
			default:
				break;
		}

		#if defined(EDTT_TEST)
			edtt_write((uint8_t *)&err, sizeof(int), EDTTT_BLOCK);
		#endif
	}
	return 0;
}
