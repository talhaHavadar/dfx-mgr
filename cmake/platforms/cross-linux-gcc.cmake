#########################################################################
# Copyright (c) 2021, Xilinx Inc. and Contributors. All rights reserved.
# SPDX-License-Identifier: MIT
#########################################################################

set (CMAKE_SYSTEM_NAME  "Linux"              CACHE STRING "")
set (CMAKE_C_COMPILER   "${CROSS_PREFIX}gcc" CACHE STRING "")
set (CMAKE_CXX_COMPILER "${CROSS_PREFIX}g++" CACHE STRING "")
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER CACHE STRING "")
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER CACHE STRING "")
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER CACHE STRING "")

# vim: expandtab:ts=2:sw=2:smartindent
