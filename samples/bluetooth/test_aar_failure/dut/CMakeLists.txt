# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dut)

target_sources(app PRIVATE
	../common.c
	main.c
	dut.c
)

zephyr_include_directories(..)
