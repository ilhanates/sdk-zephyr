/**
 * Common functions and helpers for BSIM GATT tests
 *
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// #include "bs_tracing.h"
// #include "bs_types.h"
// #include "bstests.h"
// #include "time_machine.h"
#include "zephyr/sys/__assert.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>


// extern enum bst_result_t bst_result;

#define DECLARE_FLAG(flag) extern atomic_t flag
#define DEFINE_FLAG(flag)  atomic_t flag = (atomic_t) false
#define SET_FLAG(flag)	   (void)atomic_set(&flag, (atomic_t) true)
#define UNSET_FLAG(flag)   (void)atomic_set(&flag, (atomic_t) false)
#define WAIT_FOR_FLAG(flag)                                                                        \
	while (!(bool)atomic_get(&flag)) {                                                         \
		(void)k_sleep(K_MSEC(1));                                                          \
	}
#define WAIT_FOR_FLAG_UNSET(flag)                                                                  \
	while ((bool)atomic_get(&flag)) {                                                          \
		(void)k_sleep(K_MSEC(1));                                                          \
	}
#define TAKE_FLAG(flag)                                                                            \
	while (!(bool)atomic_cas(&flag, true, false)) {                                            \
		(void)k_sleep(K_MSEC(1));                                                          \
	}

#define ASSERT(expr, ...)                                                                          \
	do {                                                                                       \
		if (!(expr)) {                                                                     \
			printk("Assert !!!!!!\n");                                                         \
		}                                                                                  \
	} while (0)

extern struct bt_conn *g_conn;
void wait_connected(void);
void wait_disconnected(void);
void wait_resolved(void);
void clear_g_conn(void);
void bs_bt_utils_setup(void);
void scan_connect_to_first_result(void);
void disconnect(void);
void set_security_l2(void);
void advertise_connectable(void);
void advertise_periodic(void);
void start_periodic_sync(void);