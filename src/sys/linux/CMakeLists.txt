#########################################################################
# Copyright (c) 2021, Xilinx Inc. and Contributors. All rights reserved.
# SPDX-License-Identifier: MIT
#########################################################################

collect (PROJECT_LIB_HEADERS accel.h)
collect (PROJECT_LIB_HEADERS dma.h)
collect (PROJECT_LIB_HEADERS acapd-vfio-common.h)
collect (PROJECT_LIB_HEADERS generic-device.h)

collect (PROJECT_LIB_SOURCES accel.c)
collect (PROJECT_LIB_SOURCES acapd-vfio-common.c)
collect (PROJECT_LIB_SOURCES dma-axidma-vfio.c)
collect (PROJECT_LIB_SOURCES dma-mcdma-vfio.c)
collect (PROJECT_LIB_SOURCES device.c)
collect (PROJECT_LIB_SOURCES generic-device.c)
collect (PROJECT_LIB_SOURCES json-config.c)
collect (PROJECT_LIB_SOURCES print.c)
collect (PROJECT_LIB_SOURCES shm.c)
collect (PROJECT_LIB_SOURCES shell.c)

