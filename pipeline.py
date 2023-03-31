import os
import sys

small_tests = ["brock200_2.clq","brock200_3.clq","brock200_4.clq"]

laptop_tests = [
    "brock200_1.clq","brock200_2.clq","brock200_3.clq","brock200_4.clq",
    "C125.9.clq", 
    "c-fat200-1.clq", "c-fat200-2.clq", "c-fat200-5.clq", "c-fat500-1.clq","c-fat500-2.clq","c-fat500-5.clq","c-fat500-10.clq",
    "gen200_p0.9_55.clq",
    "hamming6-2.clq", "hamming6-4.clq", "hamming8-2.clq", "hamming8-4.clq", "hamming10-2.clq",
    "johnson8-2-4.clq", "johnson8-4-4.clq", "johnson16-2-4.clq",
    "keller4.clq",
    "MANN_a9.clq",
    "p_hat300-1.clq", "p_hat300-2.clq",
    "p_hat500-1.clq", "p_hat500-2.clq",
    "p_hat700-1.clq", "p_hat1000-1.clq",
    "san200_0.7_1.clq", "san200_0.7_2.clq", "san200_0.9_1.clq",
    "san400_0.5_1.clq", "san400_0.7_1.clq",
    "sanr200_0.7.clq", 
    "sanr400_0.5.clq" 
]

test_cases = [
    "brock200_1.clq","brock200_2.clq","brock200_3.clq","brock200_4.clq",
    "brock400_1.clq","brock400_2.clq","brock400_3.clq","brock400_4.clq",
    "brock800_1.clq","brock800_2.clq","brock800_3.clq","brock800_4.clq",
    "C125.9.clq", "C250.9.clq", "C500.9.clq", "C1000.9.clq", "C2000.5.clq", "C2000.9.clq", "C4000.5.clq",
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

def run_instances(hardware, run_type):
    # Control how many times the instances are run for each code test case
    temp_cwd = os.getcwd()
    for i in range (5):
        os.chdir(temp_cwd + "/results/results_" + str(i))
        
        output_filename = hardware + "_tests_" + run_type + ".csv"
        #create file and populate first row
        with open(output_filename, "w") as f:
            f.write("hostname,commandline,started_at,file,proof_model,proof_log,status,nodes,omega,clique,runtime\n")

        os.chdir(temp_cwd)
        output_filename = "results/results_" + str(i) + "/" + output_filename
        
        print("Starting run " + str(i+1) + " on code type: " + run_type)
        for filename in laptop_tests: # CHANGE
            print("#",end='',flush=True) #outputs a hash character per instance run
            proofname = "proof_outputs/" + filename[:-4] + "_proof"
            os.system('./glasgow_clique_solver --prove ' + proofname + ' test-instances/DIMACS_all_ascii/' + filename + ' >> ' + output_filename)
        print("")

def main():
    
    hardware = sys.argv[1]
    cwd = os.getcwd()

    # # compile code for 1st test - unaltered code
    # os.chdir(cwd + '/build')
    # os.system('cmake .. -DCODETYPE:STRING=original')
    # os.system('make')
    # os.chdir(cwd)
    # # for each test instance record runtime
    # run_instances(hardware, "Original")

    # # compile code for 2nd test - /n code
    # os.chdir(cwd + '/build')
    # os.system('cmake .. -DCODETYPE:STRING=newline')
    # os.system('make')
    # os.chdir(cwd)
    # # for each test instance record runtime
    # run_instances(hardware, "Newline")

    # # compile code for 3rd test - fmt lib
    # os.chdir(cwd + '/build')
    # os.system('cmake .. -DCODETYPE:STRING=fmt')
    # os.system('make')
    # os.chdir(cwd)
    # # for each test instance record runtime
    # run_instances(hardware, "FMT")

    # # compile code for 4th test - colour classes fix
    # os.chdir(cwd + '/build')
    # os.system('cmake .. -DCODETYPE:STRING=colour')
    # os.system('make')
    # os.chdir(cwd)
    # # for each test instance record runtime
    # run_instances(hardware, "Colour_Class")

    # compile code for 5th test - vector vs map
    os.chdir(cwd + '/build')
    os.system('cmake .. -DCODETYPE:STRING=vector')
    os.system('make')
    os.chdir(cwd)
    # for each test instance record runtime
    run_instances(hardware, "Vector")

    # compile code for 6th test - no comments included
    os.chdir(cwd + '/build')
    os.system('cmake .. -DCODETYPE:STRING=comment')
    os.system('make')
    os.chdir(cwd)
    # for each test instance record runtime
    run_instances(hardware, "Comment")

    # compile code for 7th test - max improvement attempt (newline w/ colour class fix, no comments)
    os.chdir(cwd + '/build')
    os.system('cmake .. -DCODETYPE:STRING=max')
    os.system('make')
    os.chdir(cwd)
    # for each test instance record runtime
    run_instances(hardware, "Max")

if __name__ == "__main__":
    main()