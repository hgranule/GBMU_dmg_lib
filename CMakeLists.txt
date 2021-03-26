cmake_minimum_required(VERSION 3.15)
project(GBMU)

set(COMPILE_FLAGS -Wall -Werror -Wextra)
set(CMAKE_CXX_STANDARD "17")
set(EXTERNAL_DIR "build/external")
set(TEST_DIR "test")

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

macro(ADD_GOOGLE_TEST TNAME)
    add_executable(${TNAME} ${ARGN})
    target_link_libraries(${TNAME} gtest gtest_main)
    target_include_directories(${TNAME} PUBLIC ${GOOGLETEST_INCLUDE})
    add_test(NAME ${TNAME}
            WORKING_DIRECTORY ${TEST_DIR}
            COMMAND ${TNAME})
endmacro()

################################################################################
# GameBoy emulation library                                                    #
################################################################################
set(GBMU_LIB_SOURCES
    "sources/interrupts.cc"
)
add_library(gbmu STATIC ${GBMU_LIB_SOURCES})
target_include_directories(gbmu PUBLIC "include/")
target_compile_options(gbmu PUBLIC ${COMPILE_FLAGS})

################################################################################
# GameBoy library unit tests                                                   #
################################################################################

# ADD_GOOGLE_TEST(test12 "test/test1.cc" "test/test2.cc")