cmake_minimum_required(VERSION 3.20)
project(polyec_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(polyec_core STATIC
  src/sim.cpp
  src/trade.cpp
  src/realm.cpp
  src/realm_court.cpp
)

target_include_directories(polyec_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(polyec_core PUBLIC cxx_std_20)
target_compile_options(polyec_core PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

enable_testing()

add_executable(polyec_tests tests/test_core.cpp)
target_link_libraries(polyec_tests PRIVATE polyec_core)

add_test(NAME core_tests COMMAND polyec_tests)
add_executable(polyec_trade_tests tests/test_trade.cpp)
target_link_libraries(polyec_trade_tests PRIVATE polyec_core)
add_test(NAME trade_tests COMMAND polyec_trade_tests)
add_executable(polyec_trade_replay tests/trade_replay.cpp)
target_link_libraries(polyec_trade_replay PRIVATE polyec_core)
add_executable(polyec_realm_tests tests/test_realm.cpp)
target_link_libraries(polyec_realm_tests PRIVATE polyec_core)
add_test(NAME realm_tests COMMAND polyec_realm_tests)
add_executable(polyec_realm_transition_replay tests/realm_transition_replay.cpp)
target_link_libraries(polyec_realm_transition_replay PRIVATE polyec_core)

add_executable(polyec_court_tests tests/test_realm_court.cpp)
target_link_libraries(polyec_court_tests PRIVATE polyec_core)
add_test(NAME court_tests COMMAND polyec_court_tests)
