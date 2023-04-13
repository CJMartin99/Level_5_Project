cmake_minimum_required(VERSION 3.1)

project(glasgow-clique-solver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

SET(CODETYPE "option_value" CACHE STRING "Some user-specified option")

if (CODETYPE STREQUAL "original")
    message(STATUS "matched original")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DORIGINAL -pthread")
elseif(CODETYPE STREQUAL "newline")
    message(STATUS "matched newline")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNEWLINE -pthread")
elseif(CODETYPE STREQUAL "fmt")
    message(STATUS "matched fmt")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFMT -pthread")
elseif(CODETYPE STREQUAL "colour")
    message(STATUS "matched colour")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DORIGINAL -DCOLOUR -pthread")
elseif(CODETYPE STREQUAL "vector")
    message(STATUS "matched vector")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DORIGINAL -DVECTOR -pthread")
elseif(CODETYPE STREQUAL "comment")
    message(STATUS "matched comment")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DORIGINAL -DCOMMENTS -pthread")
elseif(CODETYPE STREQUAL "max")
    message(STATUS "matched max")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNEWLINE -DMAX -pthread")
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)


include_directories(src src/formats)

set(graph_formats src/formats/csv.cc src/formats/csv.hh
                  src/formats/dimacs.cc src/formats/dimacs.hh
                  src/formats/graph_file_error.cc src/formats/graph_file_error.hh
                  src/formats/input_graph.cc src/formats/input_graph.hh
                  src/formats/lad.cc src/formats/lad.hh
                  src/formats/read_file_format.cc src/formats/read_file_format.hh
                  src/formats/vfmcs.cc src/formats/vfmcs.hh)

set(main_files src/clique.cc src/clique.hh
          src/configuration.cc src/configuration.hh
          src/glasgow_clique_solver.cc 
          src/graph_traits.cc src/graph_traits.hh
          src/do_not_print.cc src/do_not_print.hh
          src/proof.cc src/proof.hh src/proof-fwd.hh
          src/restarts.cc src/restarts.hh
          src/svo_bitset.cc src/svo_bitset.hh
          src/timeout.cc src/timeout.hh
          src/watches.cc src/watches.hh)
          
add_executable(glasgow_clique_solver ${main_files})

find_package(Boost REQUIRED COMPONENTS iostreams program_options)
if(Boost_FOUND)
    message("Boost Found")
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(glasgow_clique_solver ${Boost_LIBRARIES})
elseif(NOT Boost_FOUND)
    error("Boost Not Found")
endif()

add_subdirectory(fmt)
add_library(formats STATIC ${graph_formats})

target_link_libraries(glasgow_clique_solver formats)
target_link_libraries(glasgow_clique_solver fmt)