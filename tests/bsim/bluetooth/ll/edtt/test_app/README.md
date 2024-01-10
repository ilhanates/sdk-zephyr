# Simple bt scanner / advertiser test apps to run with EDTT

## The commands are run in ncs-drgn: 5.36.0 and higher toolchain.

## Compile
Run `./compile.sh` in test_app folder after selecting test harness either EDTT or TCA in `main.c`. See `#define EDTT_TEST` and comment it out for TCA

## Run the test
for EDTT, Run `./test_with_edtt.sh` in test_app folder
for TCA, Run `python test_with_tca.py`in test_app folder


