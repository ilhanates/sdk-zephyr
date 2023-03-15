/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common.h"

#include <stdint.h>
#include <string.h>

#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/toolchain/gcc.h>
#include <zephyr/bluetooth/addr.h>

#include <sdc_hci_cmd_le.h>

void tester_procedure(void)
{
	int err;

	err = bt_enable(NULL);
	ASSERT(!err, "bt_enable failed. (%d)\n", err);

	advertise_connectable();
	wait_connected();

	wait_resolved();

	wait_disconnected();
	k_sleep(K_MSEC(1000));

	advertise_periodic();

	k_sleep(K_MSEC(10000));
}
