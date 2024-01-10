#!/usr/bin/env bash
# Copyright 2019 Oticon A/S
# SPDX-License-Identifier: Apache-2.0
# Run in ${ZEPHYR_BASE}/tests/bsim/bluetooth/ll/edtt/test_app

source ${ZEPHYR_BASE}/tests/bsim/sh_common.source

export CCACHE_DISABLE=1
export EDTT_PATH=${ZEPHYR_BASE}/../tools/edtt/

SIM_ID="test_app"
VERBOSE=2
APP_DIR="${ZEPHYR_BASE}/tests/bsim/bluetooth/ll/edtt/test_app"
APP_NAME_PREFIX="bs_nrf52_bsim_tests_bsim_bluetooth_ll_edtt_test_app"

CWD="$(cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)"

: "${EDTT_PATH:?EDTT_PATH must be defined}"

cd ${EDTT_PATH}

Execute ./src/edttool.py -s=${SIM_ID} -d=0 -t bsim  -T ${APP_DIR}/edtt_test_suite -v=${VERBOSE} -D=2 -devs 1 2 -RxWait=5e3

cd ${BSIM_OUT_PATH}/bin

Execute ./${APP_NAME_PREFIX}_target_app_prj_conf -s=${SIM_ID} -d=1 -v=${VERBOSE}

Execute ./${APP_NAME_PREFIX}_target_app_prj_conf -s=${SIM_ID} -d=2 -v=${VERBOSE}

Execute ./bs_2G4_phy_v1 -v=${VERBOSE} -s=${SIM_ID} -D=3  $@
