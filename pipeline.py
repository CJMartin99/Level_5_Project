import os

test_cases = [
    "brock200_1.clq","brock200_2.clq","brock200_3.clq","brock200_4.clq",
    "brock400_1.clq","brock400_2.clq","brock400_3.clq","brock400_4.clq",
    "brock800_1.clq","brock800_2.clq","brock800_3.clq","brock800_4.clq",
    "C125.9.clq", "250.9.clq", "C500.9.clq", "C1000.9.clq", "C2000.5.clq", "C2000.9.clq", "C4000.5.clq",
    "c-fat200-1.clq", "c-fat200-2.clq", "c-fat200-5.clq",
    "c-fat500-1.clq","c-fat500-2.clq","c-fat500-5.clq","c-fat500-10.clq",
    "DSJC500_5.clq", "DSJC1000_5.clq",
    "gen200_p0.9_44.clq", "gen200_p0.9_55.clq", "gen400_p0.9_55.clq", "gen400_p0.9_65.clq", "gen400_p0.9_75.clq",
    "hamming6-2.clq", "hamming6-4.clq", "hamming8-2.clq", "hamming8-4.clq", "hamming10-2.clq", "hamming10-4.clq",
    "johnson8-2-4.clq", "johnson8-4-4.clq", "johnson16-2-4.clq", "johnson32-2-4.clq",
    "keller4.clq", "keller5.clq", "keller6.clq",
    "MANN_a9.clq", "MANN_a27.clq", "MANN_a45.clq", "MANN_a81.clq",
    "p_hat300-1.clq", "p_hat300-2.clq", "p_hat300-3.clq",
    "p_hat500-1.clq", "p_hat500-2.clq", "p_hat500-3.clq",
    "p_hat700-1.clq", "p_hat700-2.clq", "p_hat700-3.clq",
    "p_hat1000-1.clq", "p_hat1000-2.clq", "p_hat1000-3.clq",
    "p_hat1500-1.clq", "p_hat1500-2.clq", "p_hat1500-3.clq",
    "san200_0.7_1.clq", "san200_0.7_2.clq",
    "san200_0.9_1.clq", "san200_0.9_2.clq", "san200_0.9_3.clq",
    "san400_0.5_1.clq", "san400_0.7_1.clq", "san400_0.7_2.clq", "san400_0.7_3.clq",
    "san400_0.9_1.clq", "san1000.clq", "sanr200_0.7.clq", "sanr200_0.9.clq",
    "sanr400_0.5.clq", "sanr400_0.7.clq" 
]

def run_instances(run_type):
    for filename in test_cases:
        output_string = "Running test instance: " + filename + " on code version " + run_type + "\n"
        print(output_string)
        proofname = filename[:-4] + "_proof"
        os.system('./glasgow_clique_solver --prove ' + proofname + ' test-instances/DIMACS_all_ascii/' + filename + ' >> laptop_tests.csv')

def main():
    # compile code for 1st test - unaltered code
    os.system('g++ -o glasgow_clique_solver -pthread -lstdc++fs intermediate/glasgow_clique_solver/src/glasgow_clique_solver.o libcommon.a -lboost_thread -lboost_system -lboost_program_options -lboost_iostreams -L/usr/local/lib -lfmt -lstdc++fs')
    # for each test instance record runtime
    run_instances("Original")

    # compile code for 2nd test - /n code
    #os.system('g++ -o glasgow_clique_solver -DEND -pthread -lstdc++fs intermediate/glasgow_clique_solver/src/glasgow_clique_solver.o libcommon.a -lboost_thread -lboost_system -lboost_program_options -lboost_iostreams -L/usr/local/lib -lfmt -lstdc++fs')
    # for each test instance record runtime
    #run_instances("Newline non-flushing")

    # compile code for 3rd test - fmt lib
    #os.system('g++ -o glasgow_clique_solver -DFMT -pthread -lstdc++fs intermediate/glasgow_clique_solver/src/glasgow_clique_solver.o libcommon.a -lboost_thread -lboost_system -lboost_program_options -lboost_iostreams -L/usr/local/lib -lfmt -lstdc++fs')
    # for each test instance record runtime
    #run_instances("FMT Library")

    # compile code for 4th test - colour classes
    #os.system('g++ -o glasgow_clique_solver -DCOLOUR -pthread -lstdc++fs intermediate/glasgow_clique_solver/src/glasgow_clique_solver.o libcommon.a -lboost_thread -lboost_system -lboost_program_options -lboost_iostreams -L/usr/local/lib -lfmt -lstdc++fs')
    # for each test instance record runtime
    #run_instances("Colour Classes")

    # graphing code - only run after each hardware has been run?
    # graphs for each test - grouped by hardware

if __name__ == "__main__":
    main()