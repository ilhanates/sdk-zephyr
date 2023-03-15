# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(tester)

target_sources(app PRIVATE
	../common.c
	main.c
	tester.c
)

zephyr_include_directories(..)
