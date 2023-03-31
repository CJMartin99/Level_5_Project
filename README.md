Level 5 MSci Project
===========================

This project uses a modified version of the Glasgow Clique Solver to investigate the ways in which we might improve the speed of proof logging.
Full details on the original Glasgow Constraint Solver can be found here: https://github.com/ciaranm/glasgow-subgraph-solver

Set-up
---------
Upon cloning this repository you will need to run both 'git submodule init' and 'git submodule update' to ensure that the FMT library is correctly set up.
Following this a folder to store the CMake auto generated files is required, I recommend doing 'mkdir build'
To run CMake cd into the new folder and run 'cmake .. -DCODETYPE=###' where ### is one of the code types for the project.
The current code types are: 'original', 'newline', 'fmt', 'colour', 'vector', 'comment', 'max'
Once cmake has finished, run the make command to build the project
cd back to the main folder and then into test-instances and unzip the benchmark
cd back to the main folder and run the glasgow clique solver as normal

Pipeline
---------
To run the pipeline you will need to create the proof_outputs folder
Then run the pipeline with 'python3 (or equivalent) pipeline.py ###' where ### is a string identifier for naming the results generated (usually the type of hardware being run on) 

<!-- vim: set tw=100 spell spelllang=en : -->
