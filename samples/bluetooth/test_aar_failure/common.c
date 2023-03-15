/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common.h"


DEFINE_FLAG(flag_is_connected);
DEFINE_FLAG(flag_is_resolved);
struct bt_conn *g_conn;
bt_addr_le_t peer_identity;
uint8_t peer_sid;

void wait_connected(void)
{
	WAIT_FOR_FLAG(flag_is_connected);
}

void wait_disconnected(void)
{
	WAIT_FOR_FLAG_UNSET(flag_is_connected);
}

void wait_resolved(void)
{
	WAIT_FOR_FLAG(flag_is_resolved);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected\n");
	UNSET_FLAG(flag_is_connected);
}

// BUILD_ASSERT(CONFIG_BT_MAX_CONN == 1, "This test assumes a single link.");
static void connected(struct bt_conn *conn, uint8_t err)
{
	ASSERT((!g_conn || (conn == g_conn)), "Unexpected new connection.");

	if (!g_conn) {
		g_conn = bt_conn_ref(conn);
	}

	if (err != 0) {
		clear_g_conn();
		return;
	}

	printk("Connected\n");
	SET_FLAG(flag_is_connected);
}

static void identity_resolved(struct bt_conn *conn, const bt_addr_le_t *rpa,
			      const bt_addr_le_t *identity)
{
	char addr_identity[BT_ADDR_LE_STR_LEN];
	char addr_rpa[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(identity, addr_identity, sizeof(addr_identity));
	bt_addr_le_to_str(rpa, addr_rpa, sizeof(addr_rpa));

	printk("BT Identity resolved %s -> %s.\n", addr_rpa, addr_identity);

	peer_identity = *identity;

	SET_FLAG(flag_is_resolved);
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err == 0) {
		printk("BT Security changed: %s level %u.\n", addr, level);
	} else {
		printk("BT Security failed: %s level %u err %d.\n", addr, level, err);
	}
	printk("security_changed\n");
}

// BT_CONN_CB_DEFINE(conn_callbacks) = {
static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
	.identity_resolved = identity_resolved,
	.security_changed = security_changed,
};

static void scan_recv(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf)
{
	peer_sid = info->sid;

	printk("s=%d ", info->sid);
}

static struct bt_le_scan_cb scan_callbacks = {
	.recv = scan_recv,
};



void clear_g_conn(void)
{
	struct bt_conn *conn;

	conn = g_conn;
	g_conn = NULL;
	ASSERT(conn, "Test error: No g_conn!\n");
	bt_conn_unref(conn);
}

/* The following flags are raised by events and lowered by test code. */
DEFINE_FLAG(flag_pairing_complete);
DEFINE_FLAG(flag_pairing_failed);

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	printk("pairing_failed\n");
	SET_FLAG(flag_pairing_failed);
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	printk("pairing_complete\n");
	SET_FLAG(flag_pairing_complete);
}

static struct bt_conn_auth_info_cb bt_conn_auth_info_cb = {
	.pairing_failed = pairing_failed,
	.pairing_complete = pairing_complete,
};

void bs_bt_utils_setup(void)
{
	int err;

	err = bt_enable(NULL);
	ASSERT(err == 0, "bt_enable failed failed (err %d)\n", err);
	err = bt_conn_auth_info_cb_register(&bt_conn_auth_info_cb);
	ASSERT(err == 0, "bt_conn_auth_info_cb_register failed (err %d)\n", err);
}

static void scan_connect_to_first_result__device_found(const bt_addr_le_t *addr, int8_t rssi,
						       uint8_t type, struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;

	if (g_conn != NULL) {
		return;
	}

	/* We're only interested in connectable events */
	if (type != BT_HCI_ADV_IND && type != BT_HCI_ADV_DIRECT_IND) {
		printk("Unexpected advertisement type.");
		return;
	}

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	printk("Got scan result, connecting.. dst %s, RSSI %d\n", addr_str, rssi);
	if (rssi < -50){
		printk("Wont connect as RSSI is low:%d\n", rssi);
		return;
	}

	err = bt_le_scan_stop();
	ASSERT(err == 0, "bt_le_scan_stop failed (%d)", err);

	err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, BT_LE_CONN_PARAM_DEFAULT, &g_conn);
	ASSERT(err == 0, "bt_conn_le_create failed (%d)", err);
}


static void sync_cb(struct bt_le_per_adv_sync *sync,
		    struct bt_le_per_adv_sync_synced_info *info)
{
	printk("sync_cb\n");
}

static void term_cb(struct bt_le_per_adv_sync *sync,
		    const struct bt_le_per_adv_sync_term_info *info)
{
	printk("term_cb\n");
}

static void recv_cb(struct bt_le_per_adv_sync *sync,
		    const struct bt_le_per_adv_sync_recv_info *info,
		    struct net_buf_simple *buf)
{
	printk("recv_cb\n");
}

static struct bt_le_per_adv_sync_cb sync_callbacks = {
	.synced = sync_cb,
	.term = term_cb,
	.recv = recv_cb,
};

void scan_connect_to_first_result(void)
{
	int err;
	bt_conn_cb_register(&conn_callbacks);
	bt_le_scan_cb_register(&scan_callbacks);

	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, scan_connect_to_first_result__device_found);
	ASSERT(err == 0, "bt_le_scan_start failed (%d)", err);
}

void disconnect(void)
{
	int err;

	err = bt_conn_disconnect(g_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
	ASSERT(err == 0, "bt_conn_disconnect failed (%d)", err);
}

void set_security_l2(void)
{
	int err;

	err = bt_conn_set_security(g_conn, BT_SECURITY_L2);
	ASSERT(err == 0, "bt_conn_set_security failed (%d)", err);
}

void advertise_connectable(void)
{
	int err;
	int id;
	bt_conn_cb_register(&conn_callbacks);
	struct bt_le_adv_param param = {};

	id = bt_id_create(NULL, NULL);

	param.id = id;
	param.interval_min = 0x0020;
	param.interval_max = 0x4000;
	param.options |= BT_LE_ADV_OPT_ONE_TIME;
	param.options |= BT_LE_ADV_OPT_CONNECTABLE;

	err = bt_le_adv_start(&param, NULL, 0, NULL, 0);
	ASSERT(err == 0, "bt_le_adv_start failed (err %d)\n", err);

	printk("advertise_connectable OK");
}

void advertise_periodic(void)
{
	int err;
	struct bt_le_ext_adv *adv;
	static uint8_t periodic_adv_data[] = {0xff, 0xff, 0x00};
	static const struct bt_data ad[] = {
		BT_DATA(BT_DATA_MANUFACTURER_DATA, periodic_adv_data, 3),
	};

	err = bt_le_ext_adv_create(BT_LE_EXT_ADV_NCONN, NULL, &adv);
	ASSERT(err == 0, "bt_le_ext_adv_create failed (err %d)\n", err);

	err = bt_le_per_adv_set_param(adv, BT_LE_PER_ADV_DEFAULT);
	ASSERT(err == 0, "bt_le_per_adv_set_param failed (err %d)\n");

	err = bt_le_per_adv_set_data(adv, ad, ARRAY_SIZE(ad));
	ASSERT(err == 0, "bt_le_per_adv_set_data failed (err %d)\n");

	err = bt_le_per_adv_start(adv);
	ASSERT(err == 0, "bt_le_per_adv_start failed (err %d)\n");

	err = bt_le_ext_adv_start(adv, BT_LE_EXT_ADV_START_DEFAULT);
	ASSERT(err == 0, "bt_le_ext_adv_start failed (err %d)\n", err);

	printk("advertise_periodic OK");
}

void start_periodic_sync(void)
{
	int err;
	char addr_identity[BT_ADDR_LE_STR_LEN];
	struct bt_le_per_adv_sync *sync = NULL;
	struct bt_le_per_adv_sync_param sync_create_param = {
		.options = BT_LE_PER_ADV_SYNC_OPT_USE_PER_ADV_LIST,
		.skip = 0,
		.timeout = 200,
	};

	bt_addr_le_to_str(&peer_identity, addr_identity, sizeof(addr_identity));
	printk("peer_identity= %s\n", addr_identity);
	printk("peer_sid = %d\n", peer_sid);

	err = bt_le_per_adv_list_clear();
	ASSERT(err == 0, "bt_le_per_adv_list_clear failed (err %d)\n", err);

	err = bt_le_per_adv_list_add(&peer_identity, peer_sid);
	ASSERT(err == 0, "bt_le_per_adv_list_add failed (err %d)\n", err);

	bt_le_per_adv_sync_cb_register(&sync_callbacks);
	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, NULL);
	ASSERT(err == 0, "bt_le_scan_start failed (err %d)", err);
	printk("bt_le_scan_startd\n");

	k_sleep(K_MSEC(2000));

	err = bt_le_per_adv_sync_create(&sync_create_param, &sync);
	ASSERT(err == 0, "bt_le_per_adv_sync_create failed (err %d)", err);
	printk("bt_le_per_adv_sync_create\n");
}