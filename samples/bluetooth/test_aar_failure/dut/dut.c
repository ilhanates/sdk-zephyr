/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common.h"

#include <stdint.h>

#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/bluetooth.h>

#include <sdc_hci_cmd_le.h>

DECLARE_FLAG(flag_connected);

void dut_procedure(void)
{
	int err;

	err = bt_enable(NULL);
	ASSERT(!err, "bt_enable failed. (%d)\n", err);

	scan_connect_to_first_result();
	wait_connected();

	set_security_l2();
	wait_resolved();

	disconnect();
	wait_disconnected();
	k_sleep(K_MSEC(2000));

	start_periodic_sync();

	k_sleep(K_MSEC(10000));
}
