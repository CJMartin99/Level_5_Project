Level 5 MSci Project
===========================

This project uses a modified version of the Glasgow Clique Solver to investigate the ways in which we might improve the speed of proof logging.
Full details on the original Glasgow Constraint Solver can be found here: https://github.com/ciaranm/glasgow-subgraph-solver

Compiling
---------
For the code to successfully compile the hardware will require that CMake be installed. 
To run the test set and graphing code an installation of python is required
If running the test set already created there should be no need to manually compile the code.

Running
---------
The entire testing process is run from the python file pipeline.py, running it with python3 (or equivalent) should begin the compilation of the code with CMake and running of each test instance. The code will then recompile for the different test cases and repeat the running of each test instance. 

<!-- vim: set tw=100 spell spelllang=en : -->
