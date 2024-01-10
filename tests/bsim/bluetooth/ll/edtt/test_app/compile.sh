 # Run in ${ZEPHYR_BASE}/tests/bsim/bluetooth/ll/edtt/test_app

set -ue
: "${BSIM_COMPONENTS_PATH:?BSIM_COMPONENTS_PATH must be defined}"
: "${ZEPHYR_BASE:?ZEPHYR_BASE must be set to point to the zephyr root\
 directory}"

export CCACHE_DISABLE=1

WORK_DIR="${WORK_DIR:-${ZEPHYR_BASE}/bsim_out}"
APP_DIR="tests/bsim/bluetooth/ll/edtt/test_app"
BOARD_ROOT="${BOARD_ROOT:-${ZEPHYR_BASE}}"

rm -rf ${WORK_DIR}
mkdir -p ${WORK_DIR}

source ${ZEPHYR_BASE}/tests/bsim/compile.source

app=${APP_DIR}/target_app conf_file=prj.conf compile

wait_for_background_jobs
