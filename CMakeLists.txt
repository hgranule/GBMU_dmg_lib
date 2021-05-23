cmake_minimum_required(VERSION 3.15)
project(GBMU)

set(COMPILE_FLAGS                   -Wall -Werror -Wextra)
set(CMAKE_CXX_STANDARD              17)
set(EXTERNAL_DIR                    ${CMAKE_BINARY_DIR}/external)
set(TEST_DIR                        ${CMAKE_SOURCE_DIR}/test)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin) 
execute_process(COMMAND mkdir -p    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})


################################################################################
# GameBoy emulation library                                                    #
################################################################################
set(GBMU_LIB_INCLUDE_DIR "include")
set(GBMU_LIB_SOURCES
        "include/GB_config.h"

        "include/common/GB_clock.h"
        "include/common/GB_dbuffer.h"
        "include/common/GB_macro.h"
        "include/common/GB_types.h"

        "include/memory/GB_vaddr.h"

        "include/device/GB_interrupt.h"
        "include/device/GB_wram.h"
        "include/device/GB_joypad.h"
        "include/device/GB_oram.h"
        "include/device/GB_vram.h"

        "sources/interrupt.cc"
        "sources/wram.cc"
        "sources/joypad.cc"
)
add_library(gbmu STATIC ${GBMU_LIB_SOURCES})
target_include_directories(gbmu PUBLIC ${GBMU_LIB_INCLUDE_DIR})
target_compile_options(gbmu PUBLIC ${COMPILE_FLAGS})


################################################################################
# SPDlog library                                                               #
################################################################################
set(SPDLOG_SDIR ${EXTERNAL_DIR}/spdlog)
set(SPDLOG_INCLUDE ${SPDLOG_SDIR}/include)
execute_process(
    COMMAND             mkdir -p ${EXTERNAL_DIR}
    COMMAND             git clone https://github.com/gabime/spdlog.git -b v1.x ${SPDLOG_SDIR}
    WORKING_DIRECTORY   ${CMAKE_SOURCE_DIR}
)
add_subdirectory(${SPDLOG_SDIR})


################################################################################
# Google Test Framework configure                                              #
################################################################################
enable_testing()
set(GOOGLETEST_SDIR ${EXTERNAL_DIR}/gt)
set(GOOGLETEST_INCLUDE ${GOOGLETEST_SDIR}/googletest/include)
execute_process(
    COMMAND             mkdir -p ${EXTERNAL_DIR}
    COMMAND             git clone https://github.com/google/googletest.git -b release-1.10.0 ${GOOGLETEST_SDIR}
    WORKING_DIRECTORY   ${CMAKE_SOURCE_DIR}
)
add_subdirectory(${GOOGLETEST_SDIR})

macro(ADD_GBMU_LIB_TEST TNAME)
    add_executable(${TNAME} ${ARGN})
    target_link_libraries(${TNAME} gtest gtest_main gbmu)
    target_include_directories(${TNAME} PUBLIC ${GOOGLETEST_INCLUDE})
    target_compile_definitions(${TNAME} PUBLIC UNIT_TEST_)
    add_test(NAME ${TNAME}
            COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TNAME} --gtest_print_time=0
            WORKING_DIRECTORY ${TEST_DIR})
endmacro()


################################################################################
# GameBoy library unit tests                                                   #
################################################################################
ADD_GBMU_LIB_TEST(interrupt_ctrl_test   "test/interrupt.cc")
ADD_GBMU_LIB_TEST(wram_test             "test/wram.cc")
ADD_GBMU_LIB_TEST(joypad_test           "test/joypad.cc")
ADD_GBMU_LIB_TEST(oram_test             "test/oram.cc")
ADD_GBMU_LIB_TEST(vram_test             "test/vram.cc")
