# fusion — C++20 product core (CUDA arrives at M1; ui/ is the sanctioned exception)
cmake_minimum_required(VERSION 3.26)
project(fusion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(fusion_core STATIC core/config.cpp core/sim.cpp control/policy.cpp)
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

# M2 S18 (D-048): the batched-env runner (fourth consumer of one-dynamics-source, D-011)
add_executable(fusor_train_env apps/fusor_train_env_main.cpp)
target_link_libraries(fusor_train_env PRIVATE fusion_core)

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

add_executable(test_vertical tests/test_vertical.cpp)
target_link_libraries(test_vertical PRIVATE fusion_core)
add_test(NAME vertical_enemy COMMAND test_vertical ${CMAKE_SOURCE_DIR})

add_executable(test_gs tests/test_gs.cpp)
target_link_libraries(test_gs PRIVATE fusion_core)
add_test(NAME gs_solver COMMAND test_gs ${CMAKE_SOURCE_DIR})

# M2 S17a (D-046): the renderer must not silently drop event kinds (feed.cpp needs no
# llama — render_feed is pure). Guards the tokenizer's diet (FORWARD_NOTES_M2 §5).
add_executable(test_feed tests/test_feed.cpp membrane/feed.cpp)
target_link_libraries(test_feed PRIVATE fusion_core)
add_test(NAME feed_render COMMAND test_feed)

# M2 S18 (D-048): the stepwise-plant parity gate + the ghost-fork PoC (relay Q18/Q23)
add_executable(test_parity_env tests/test_parity_env.cpp)
target_link_libraries(test_parity_env PRIVATE fusion_core)
add_test(NAME parity_train_env COMMAND test_parity_env ${CMAKE_SOURCE_DIR})

add_executable(test_ghost tests/test_ghost.cpp)
target_link_libraries(test_ghost PRIVATE fusion_core)
add_test(NAME ghost_fork COMMAND test_ghost ${CMAKE_SOURCE_DIR})

# ---- M0.5: fusor_shadow — the thesis-slice triple (llama.cpp in-process, delay-loaded
# from C:/llama.cpp at runtime; import libs vendored — auricle's proven pattern)
set(LLAMA_TP ${CMAKE_SOURCE_DIR}/third_party/llama.cpp)
if(EXISTS ${LLAMA_TP}/lib/llama.lib)
  message(STATUS "fusion: llama.cpp import libs found — fusor_shadow enabled")
  add_executable(fusor_shadow membrane/shadow.cpp membrane/feed.cpp)
  target_include_directories(fusor_shadow PRIVATE ${LLAMA_TP}/include)
  target_compile_definitions(fusor_shadow PRIVATE "FUSION_LLAMA_DIR=\"C:/llama.cpp\"")
  target_link_libraries(fusor_shadow PRIVATE fusion_core
    ${LLAMA_TP}/lib/llama.lib ${LLAMA_TP}/lib/ggml.lib ${LLAMA_TP}/lib/ggml-base.lib
    delayimp)
  target_link_options(fusor_shadow PRIVATE
    /DELAYLOAD:llama.dll /DELAYLOAD:ggml.dll /DELAYLOAD:ggml-base.dll)
else()
  message(STATUS "fusion: llama.cpp import libs absent — fusor_shadow skipped")
endif()

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  # seedcheck: the trainer's gate-seed guard must refuse eval/gate domains (CTL-12)
  add_test(NAME seedcheck COMMAND ${Python3_EXECUTABLE}
           ${CMAKE_SOURCE_DIR}/trainer/tune_pid.py --selftest-seed-guard)
  # statecheck (CTL-14b, hard M1 gate): the policy TU's include closure cannot reach
  # the true-state headers; token scan backs it; the poison guards must exist.
  add_test(NAME statecheck COMMAND ${Python3_EXECUTABLE}
           ${CMAKE_SOURCE_DIR}/tools/statecheck.py ${CMAKE_SOURCE_DIR})
endif()
