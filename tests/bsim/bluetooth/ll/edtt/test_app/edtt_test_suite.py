import struct
import time;

from components.test_spec import TestSpec;

CMD_CUSTOM_ADV_START = 201
CMD_CUSTOM_SCANNER_START = 202
CMD_CUSTOM_SCANNER_STOP = 203

def check_return(transport, idx):
    RxWait = 5
    timeout = RxWait + 1
    packet = transport.recv(idx, 4, timeout)
    ret = int.from_bytes(packet, byteorder='little')
    assert ret == 0, 'Operation failed'


def adv_scanner(transport):
    packet = transport.recv(1, 4, 200)
    cmd = struct.pack('<HH', CMD_CUSTOM_ADV_START, 0)
    transport.send(1, cmd)
    check_return(transport, 1)

    cmd = struct.pack('<HH', CMD_CUSTOM_SCANNER_START, 0)
    transport.send(0, cmd)
    check_return(transport, 0)

    transport.wait(500)

    cmd = struct.pack('<HH', CMD_CUSTOM_SCANNER_STOP, 0)
    transport.send(0, cmd)
    check_return(transport, 0)

    return 0


def test_adv_scanner(transport, trace):
    trace.trace(3, "Starting test_adv_scanner")
    try:
        adv_scanner(transport)
    except Exception as e:
        trace.trace(1, "test_adv_scanner failed: %s" %str(e))
        return 1

    return 0

_spec = {}
_spec["test_app"] = TestSpec(name = "Test_app", number_devices = 2, description = "Test adv scanner", test_private = test_adv_scanner)

def get_tests_specs():
    return _spec


def run_a_test(args, transport, trace, test_spec, dumps):
    print('\u001b[33m. !!!! Keep in mind to compile target_app for EDTT with #define EDTT_TEST !!!!\u001b[0m')
    return test_spec.test_private(transport, trace)
