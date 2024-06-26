#########################################################################
# Copyright (c) 2021, Xilinx Inc. and Contributors. All rights reserved.
# SPDX-License-Identifier: MIT
#########################################################################
# add_subdirectory (graph)

collector_list  (_list PROJECT_INC_DIRS)
include_directories (${_list} ${CMAKE_CURRENT_SOURCE_DIR})

collector_list  (_list PROJECT_LIB_DIRS)
link_directories (${_list})

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set (_deps "dfx" "dl" "pthread" "systemd")

foreach (_app load_accel accel-bzip2 accel-shm accel-aes accel-container dfx-mgr-client dfx-mgrd )
#foreach (_app load_accel dfx-mgr-client dfx-mgrd dfx-mgrd2 graphTest6)
  set (_src ${CMAKE_CURRENT_SOURCE_DIR}/${_app}.c)
  if (WITH_SHARED_LIB)
    add_executable (${_app}-share ${_src})
    if (PROJECT_EC_FLAGS)
      string(REPLACE " " ";" _ec_flgs ${PROJECT_EC_FLAGS})
      target_compile_options (${_app}-share PUBLIC ${_ec_flgs})
    endif (PROJECT_EC_FLAGS)
    target_link_libraries (${_app}-share ${PROJECT_NAME}-shared ${_deps})
    install (TARGETS ${_app}-share RUNTIME DESTINATION bin)
    add_dependencies (${_app}-share ${PROJECT_NAME}-shared)
  endif (WITH_SHARED_LIB)

  if (WITH_STATIC_LIB)
    if (${PROJECT_SYSTEM} STREQUAL "linux")
        add_executable (${_app}-static ${_src})
        if (PROJECT_EC_FLAGS)
          string(REPLACE " " ";" _ec_flgs ${PROJECT_EC_FLAGS})
          target_compile_options (${_app}-static PUBLIC ${_ec_flgs})
        endif (PROJECT_EC_FLAGS)
        target_link_libraries (${_app}-static ${PROJECT_NAME}-static ${_deps})
        install (TARGETS ${_app}-static RUNTIME DESTINATION bin)
    endif (${PROJECT_SYSTEM} STREQUAL "linux")
  endif (WITH_STATIC_LIB)
endforeach (_app)
