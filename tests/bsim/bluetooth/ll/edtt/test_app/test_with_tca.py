import os
import time
from ctf.targetcomm2 import TargetCommander, BabbleSimCommunication, BabbleSimPipesTransport, concurrent_utils

NUM_OF_DEVICES = 2
RX_WAIT_S = 0.01
MAX_RESYNC_OFFSET_US = 5000  # -mro

tcas = []
APP_DIR = os.getenv('ZEPHYR_BASE') + '/bsim_out/tests/bsim/bluetooth/ll/edtt/test_app/target_app/'
target_file = APP_DIR + 'bs_nrf52_bsim_tests_bsim_bluetooth_ll_edtt_test_app_target_app_prj_conf/zephyr/zephyr.exe'
assert os.path.exists(target_file), 'zephyr app (with nrf52_bsim) could not found!'
print('\u001b[33m. !!!! Keep in mind to compile target_app for tca with commenting out #define EDTT_TEST\u001b[0m')


def start_simulation():
    # cleanup
    os.system('rm -rf /tmp/bs_azure/*')
    os.system('killall -9 bs_2G4_phy_v1 > /dev/null')
    os.system('killall -9 zephyr.exe > /dev/null')

    # Run phy + devices
    os.system(f'cd /bsim/bsim/bin && /bsim/bsim/bin/bs_2G4_phy_v1 -s=test -D={NUM_OF_DEVICES + 1} -v=4 -sim_length=60e6 &')
    for idx in range(NUM_OF_DEVICES):
        os.system(target_file + f' -s=test -d={idx + 1} -v=4 -mro={MAX_RESYNC_OFFSET_US} &')

    time.sleep(0.1)


def start_tca():
    fifo_path = '/tmp/bs_azure/test/'

    phy_fifo_tx_path = fifo_path + '2G4.d0.dtp'
    phy_fifo_rx_path = fifo_path + '2G4.d0.ptd'
    phy_transport = BabbleSimPipesTransport(phy_fifo_tx_path, phy_fifo_rx_path)

    for idx in range(NUM_OF_DEVICES):
        device_id = idx + 1
        fifo_tx_path = fifo_path + f'Device{device_id}.PTTin'
        fifo_rx_path = fifo_path + f'Device{device_id}.PTTout'
        target_transport = BabbleSimPipesTransport(fifo_tx_path, fifo_rx_path)
        transport = BabbleSimCommunication(target_transport, phy_transport)
        transport.rx_wait = RX_WAIT_S

        TargetCommander.unit_path_filter = ['*/target_app/src/*']
        tca = TargetCommander(elf_file_name=target_file, transport=transport, autostart=False, name=f'dev_{device_id}')
        tcas.append(tca)

    concurrent_utils.joinall([tca.astart() for tca in tcas], timeout=5)


def test_adv_scanner():
    tcas[0].t.main.adv_start()

    tcas[1].t.main.scan_start()
    tcas[1].sleep(0.5)
    tcas[1].t.main.scan_stop()


if __name__ == '__main__':
    start_simulation()
    start_tca()

    test_adv_scanner()
