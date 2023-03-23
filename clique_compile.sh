#!/bin/bash

# simple script to execute the compilation of the clique solver without needing to alter the makefile
# with additional arguments

g++ -o intermediate/libcommon.a/src/proof.o -c -MD -O3 -march=native -std=c++17 -Isrc/ -W -Wall -g -ggdb3 -pthread src/proof.cc
g++ -o glasgow_clique_solver -pthread -lstdc++fs intermediate/glasgow_clique_solver/src/glasgow_clique_solver.o libcommon.a -lboost_thread -lboost_system -lboost_program_options -lboost_iostreams -L/usr/local/lib -lfmt -lstdc++fs
