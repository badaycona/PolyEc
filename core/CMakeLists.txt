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
  src/campaign.cpp
  src/finance_manager.cpp
  src/employment_manager.cpp
  src/household_index.cpp
  src/project_manager.cpp
  src/activity.cpp
  src/schedule.cpp
  src/realm_clock.cpp
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

add_executable(polyec_campaign_tests tests/test_campaign.cpp)
target_link_libraries(polyec_campaign_tests PRIVATE polyec_core)
add_test(NAME campaign_tests COMMAND polyec_campaign_tests)

add_executable(polyec_manager_tests tests/test_managers.cpp)
target_link_libraries(polyec_manager_tests PRIVATE polyec_core)
add_test(NAME manager_tests COMMAND polyec_manager_tests)

add_executable(polyec_activity_tests tests/test_activity.cpp)
target_link_libraries(polyec_activity_tests PRIVATE polyec_core)
add_test(NAME activity_tests COMMAND polyec_activity_tests)
add_executable(polyec_schedule_tests tests/test_schedule.cpp)
target_link_libraries(polyec_schedule_tests PRIVATE polyec_core)
add_test(NAME schedule_tests COMMAND polyec_schedule_tests)

add_executable(polyec_realm_clock_tests tests/test_realm_clock.cpp)
target_link_libraries(polyec_realm_clock_tests PRIVATE polyec_core)
add_test(NAME realm_clock_tests COMMAND polyec_realm_clock_tests)

add_executable(polyec_realm_clock_replay tests/realm_clock_replay.cpp)
target_link_libraries(polyec_realm_clock_replay PRIVATE polyec_core)
add_test(NAME realm_clock_replay COMMAND polyec_realm_clock_replay)
