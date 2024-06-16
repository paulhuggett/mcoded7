cmake_minimum_required (VERSION 3.22)
project (mcoded7 LANGUAGES CXX)
option (MCODED7_FUZZTEST "Enable FuzzTest")

include (CheckCXXCompilerFlag)

set (PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

function (setup_gtest)
  if (EXISTS "${PROJECT_ROOT}/googletest/CMakeLists.txt")
    # Tell gtest to link against the "Multi-threaded Debug DLL runtime library"
    # on Windows.
    set (gtest_force_shared_crt On CACHE BOOL "Always use msvcrt.dll")
    # We don't want to install either gtest or gmock.
    set (INSTALL_GTEST Off CACHE BOOL "Disable gtest install")
    set (INSTALL_GMOCK Off CACHE BOOL "Disable gmock install")
    add_subdirectory ("${PROJECT_ROOT}/googletest")

    foreach (target gtest gmock gmock_main gtest_main)
      set (gclang_options -Wno-implicit-int-float-conversion)
      if (ICUBABY_LIBCXX)
        list (APPEND gclang_options -stdlib=libc++)
      endif ()
      set (gintelllvm_options ${gclang_options} -Wno-tautological-constant-compare)

      target_compile_features (${target} PUBLIC cxx_std_20)
      target_compile_definitions (${target} PUBLIC GTEST_REMOVE_LEGACY_TEST_CASEAPI_=1)
      target_compile_options (${target} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${gclang_options}>
        $<$<CXX_COMPILER_ID:IntelLLVM>:${gintelllvm_options}>
      )
      target_link_options (${target} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${gclang_options}>
        $<$<CXX_COMPILER_ID:IntelLLVM>:${gintelllvm_options}>
      )
    endforeach()
  endif()
endfunction (setup_gtest)

option (MCODED7_FUZZTEST "Enable FuzzTest")
if (MCODED7_FUZZTEST)
  set (FUZZTEST_FUZZING_MODE On)
  include (FetchContent)
  set (FUZZTEST_REPO_BRANCH "1635d42" CACHE STRING "FuzzTest repository branch.")
  message ("Building fuzztest at tag " ${FUZZTEST_REPO_BRANCH})
  FetchContent_Declare (
    fuzztest
    GIT_REPOSITORY https://github.com/google/fuzztest.git
    GIT_TAG ${FUZZTEST_REPO_BRANCH}
  )
  FetchContent_MakeAvailable (fuzztest)
  include (GoogleTest)
  fuzztest_setup_fuzzing_flags ()
else ()
  setup_gtest ()
endif (MCODED7_FUZZTEST)
enable_testing ()


set (warning_options -Wall -Wextra -pedantic)
add_library(mcoded7 INTERFACE mcoded7.hpp)
target_include_directories (mcoded7 INTERFACE .)
target_compile_features(mcoded7 INTERFACE cxx_std_20)
target_compile_options(mcoded7 INTERFACE ${warning_options})


add_executable (unittests unittests.cpp)
target_compile_features(unittests PRIVATE cxx_std_20)
target_compile_options(unittests PRIVATE ${warning_options})
target_compile_options(unittests PRIVATE -fsanitize=undefined)
target_compile_definitions (unittests PRIVATE MCODED7_FUZZTEST=$<BOOL:${MCODED7_FUZZTEST}>)
target_link_options(unittests PRIVATE -fsanitize=undefined)
target_link_libraries(unittests PRIVATE mcoded7)
if (MCODED7_FUZZTEST)
  target_link_libraries (unittests PRIVATE gtest gmock)
  link_fuzztest (unittests)
  gtest_discover_tests (unittests)
else ()
  target_link_libraries (unittests PRIVATE gmock_main)
endif (MCODED7_FUZZTEST)
