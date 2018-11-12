cmake_minimum_required(VERSION 3.5.0)
project(rbggamemanager)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pipe")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native")

add_subdirectory(../rbgParser ./rbgParser/.)
include_directories(../rbgParser/src)

find_package(Threads)

# see https://cmake.org/cmake/help/latest/module/FindBoost.html
find_package(Boost REQUIRED COMPONENTS system program_options)
include_directories(${Boost_INCLUDE_DIR})

set(PLAYING_FILES ${PLAYING_FILES} src/game_nfa/automaton.h src/game_components/name_resolver.h src/actions/action.cc src/actions/action.h src/game_components/game_description.h src/game_nfa/game_moves_description.h src/game_components/game_state.cc src/game_components/game_state.h src/actions/arithmetic_operation.cc src/actions/arithmetic_operation.h src/parser_connection/game_nfa_creator.cc src/parser_connection/game_nfa_creator.h src/parser_connection/parser_connection.cc src/parser_connection/parser_connection.h src/game_components/search_context.cc src/game_components/search_context.h src/utils/fast_bit_vector.h src/utils/utilities.h src/actions/action_application.h src/actions/action_types.h src/game_components/move.h
        src/parser_connection/arithmetic_creator.cpp src/parser_connection/arithmetic_creator.h src/game_components/graph_board.h src/utils/global_config.cpp)
add_executable(benchmark ${PLAYING_FILES} src/benchmark.cc)
target_link_libraries(benchmark rbgParser)
target_link_libraries(benchmark ${Boost_LIBRARIES})
target_link_libraries(benchmark ${CMAKE_THREAD_LIBS_INIT})

add_executable(host ${PLAYING_FILES} src/networking/server.h src/networking/server.cc src/networking/message.h src/networking/networking_utils.cc src/networking/networking_utils.h src/run_server.cc)
target_link_libraries(host rbgParser)
target_link_libraries(host ${Boost_LIBRARIES})
target_link_libraries(host ${CMAKE_THREAD_LIBS_INIT})

add_executable(random_client ${PLAYING_FILES} src/networking/message.h src/networking/networking_utils.cc src/networking/networking_utils.h src/networking/client.h src/random_client.cc)
target_link_libraries(random_client rbgParser)
target_link_libraries(random_client ${Boost_LIBRARIES})
target_link_libraries(random_client ${CMAKE_THREAD_LIBS_INIT})

add_executable(material_client ${PLAYING_FILES} src/networking/message.h src/networking/networking_utils.cc src/networking/networking_utils.h src/networking/client.h src/material_heuristic_client.cc)
target_link_libraries(material_client rbgParser)
target_link_libraries(material_client ${Boost_LIBRARIES})
target_link_libraries(material_client ${CMAKE_THREAD_LIBS_INIT})

