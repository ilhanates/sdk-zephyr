# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2019 Oticon A/S

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(target_app)


set(TCA_TRANSPORT "PIPES" CACHE STRING "TCA transport type")

execute_process(COMMAND ctf-targetcomm -r lib OUTPUT_VARIABLE TCA_DIR)
string(STRIP ${TCA_DIR} TCA_DIR)

#zephyr_library_compile_definitions(-Wno-error)

include(${TCA_DIR}/CMakeLists.txt)
zephyr_library_compile_definitions(CONFIG_TCA_NON_BLOCKING_READ=0)
zephyr_library_compile_definitions(CONFIG_TCA_TRANSPORT_SLEEP_US=5000)


FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  ../../common/edtt_driver_bsim.c
)

zephyr_library_include_directories(${ZEPHYR_BASE}/samples/bluetooth
  ../../common/
  src/
  ${BSIM_COMPONENTS_PATH}/libUtilv1/src/
  ${BSIM_COMPONENTS_PATH}/libPhyComv1/src/
  ${TCA_DIR}
)

target_link_libraries(app PRIVATE ctfTargetComm)