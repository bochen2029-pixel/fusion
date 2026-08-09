# fusion — C++20 product core (CUDA arrives at M1; ui/ is the sanctioned exception)
cmake_minimum_required(VERSION 3.26)
project(fusion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(fusion_core STATIC core/config.cpp core/sim.cpp)
target_include_directories(fusion_core PUBLIC ${CMAKE_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}/third_party/tomlplusplus)
if(MSVC)
  # /fp:strict — the determinism law (no FMA contraction variance, no fast-math)
  target_compile_options(fusion_core PUBLIC /W4 /EHsc /fp:strict /utf-8)
endif()

add_executable(fusor apps/fusor_main.cpp)
target_link_libraries(fusor PRIVATE fusion_core)

add_executable(fusor_mc apps/fusor_mc_main.cpp)
target_link_libraries(fusor_mc PRIVATE fusion_core)

enable_testing()

add_executable(test_replay tests/test_replay.cpp)
target_link_libraries(test_replay PRIVATE fusion_core)
add_test(NAME replay_oracle COMMAND test_replay ${CMAKE_SOURCE_DIR})

add_executable(test_physics_sanity tests/test_physics_sanity.cpp)
target_link_libraries(test_physics_sanity PRIVATE fusion_core)
add_test(NAME physics_sanity COMMAND test_physics_sanity ${CMAKE_SOURCE_DIR})

add_executable(test_budget tests/test_budget.cpp)
target_link_libraries(test_budget PRIVATE fusion_core)
add_test(NAME budget_events COMMAND test_budget ${CMAKE_SOURCE_DIR})

add_executable(test_golden tests/test_golden.cpp)
target_link_libraries(test_golden PRIVATE fusion_core)
add_test(NAME golden_check COMMAND test_golden ${CMAKE_SOURCE_DIR})

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  # seedcheck: the trainer's gate-seed guard must refuse eval/gate domains (CTL-12)
  add_test(NAME seedcheck COMMAND ${Python3_EXECUTABLE}
           ${CMAKE_SOURCE_DIR}/trainer/tune_pid.py --selftest-seed-guard)
endif()
