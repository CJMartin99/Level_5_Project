import pandas as pd
from matplotlib import pyplot as plt
import seaborn as sb
import os

def text_strip(input_text):
    # file = test-instances/DIMACS_all_ascii/ = length 39
    if len(input_text) > 30:
        stripped_text = input_text[39:]
    else:
        stripped_text = input_text
    return stripped_text

def calculate_avg_runtime(df,code_type):
    df['avg_runtime'] = df[['runtime','runtime1','runtime2','runtime3','runtime4']].mean(axis=1)
    df['code_type'] = code_type

def create_comparison_graph(df,hardware):
    cwd = os.getcwd()

    sb.set_theme(style="whitegrid")
    #ax = sb.boxplot(x="code_type",y="avg_runtime",data=df_laptop_original_newline)
    ax = sb.swarmplot(x="code_type",y="runtimes",data=df,hue="code_type",size=2.5,legend=False)#,jitter=0.4)
    ax.set_title("Test Instances Runtimes (" + hardware + ")")
    ax.set_xlabel("Code Test Cases")
    ax.set_ylabel("Runtime (milliseconds)")
    plt.yscale("log")
    #plt.ylim(ymin=0.1)
    plt.xticks(rotation=0)

    os.chdir(cwd+"/graphs")
    plt.savefig(hardware + "_Runtimes.png")
    plt.clf()
    os.chdir(cwd)

def create_speed_up_graph(df,hardware):
    cwd = os.getcwd()

    sb.set_theme(style="whitegrid")
    ax = sb.stripplot(x="file",y="speed_ups",data=df,hue="code_type",size=3,jitter=0.3)
    ax.set_title(hardware + " speed-ups vs Original")
    ax.set_xlabel("Test instances")
    ax.set_ylabel("Speed-Up")
    plt.xticks(rotation=270)
    plt.ylim(ymin=0)

    os.chdir(cwd+"/graphs")
    plt.savefig(hardware + "_speedups_vs_original.png")
    plt.clf()
    os.chdir(cwd)

def main():
    cwd = os.getcwd()

    os.chdir(cwd + "/results/results_0")
    df_laptop_original = pd.read_csv('Laptop_tests_Original.csv')
    df_cluster_original = pd.read_csv('Cluster_tests_Original.csv')
    #df_epyc_original = pd.read_csv('Epyc_tests_Original.csv')

    df_laptop_newline = pd.read_csv('Laptop_tests_Newline.csv')
    df_cluster_newline = pd.read_csv('Cluster_tests_Newline.csv')
    #df_epyc_newline = pd.read_csv('Epyc_tests_Newline.csv')

    df_laptop_fmt = pd.read_csv('Laptop_tests_FMT.csv')
    df_cluster_fmt = pd.read_csv('Cluster_tests_FMT.csv')
    #df_epyc_fmt = pd.read_csv('Epyc_tests_FMT.csv')

    df_laptop_cc = pd.read_csv('Laptop_tests_Colour_Class.csv')
    df_cluster_cc = pd.read_csv('Cluster_tests_Colour_Class.csv')
    #df_epyc_cc = pd.read_csv('Epyc_tests_Colour_Class.csv')

    df_laptop_vector = pd.read_csv('Laptop_tests_Vector.csv')
    #df_cluster_vector = pd.read_csv('Cluster_tests_Vector.csv')
    #df_epyc_vector = pd.read_csv('Epyc_tests_Vector.csv')

    df_laptop_comment =  pd.read_csv('Laptop_tests_Comment.csv')
    #df_cluster_comment = pd.read_csv('Cluster_tests_Comment.csv')
    #df_epyc_comment = pd.read_csv('Epyc_tests_Comment.csv')

    df_laptop_max = pd.read_csv('Laptop_tests_Max.csv')
    #df_cluster_max = pd.read_csv('Cluster_tests_Max.csv')
    #df_epyc_max = pd.read_csv('Epyc_tests_Max.csv')

    df_file_pairs = [
        [df_laptop_original,'Laptop_tests_Original.csv'], [df_cluster_original,'Cluster_tests_Original.csv'], # [df_epyc_original,'Epyc_tests_Original.csv'],
        [df_laptop_newline,'Laptop_tests_Newline.csv'], [df_cluster_newline,'Cluster_tests_Newline.csv'], # [df_epyc_newline,'Epyc_tests_Newline.csv'],
        [df_laptop_fmt,'Laptop_tests_FMT.csv'], [df_cluster_fmt,'Cluster_tests_FMT.csv'], # [df_epyc_fmt,'Epyc_tests_FMT.csv'],
        [df_laptop_cc,'Laptop_tests_Colour_Class.csv'], [df_cluster_cc,'Cluster_tests_Colour_Class.csv'], # [df_epyc_cc,'Epyc_tests_Colour_Class.csv'],
        [df_laptop_vector,'Laptop_tests_Vector.csv'], # [df_cluster_vector,'Cluster_tests_Vector.csv'], # [df_epyc_vector,'Epyc_tests_Vector.csv'],
        [df_laptop_comment,'Laptop_tests_Comment.csv'], # [df_cluster_comment,'Cluster_tests_Comment.csv'], # [df_epyc_comment,'Epyc_tests_Comment.csv'],
        [df_laptop_max,'Laptop_tests_Max.csv'], # [df_cluster_max,'Cluster_tests_Max.csv'], # [df_epyc_max,'Epyc_tests_Max.csv']
    ]

    # final version may be set to 10
    for df_file_pair in df_file_pairs:
        
        df = df_file_pair[0]
        df["file"] = df["file"].apply(text_strip)
        filename = df_file_pair[1]

        for i in range(1,5):
            os.chdir(cwd + "/results/results_" + str(i))
            temp_df = pd.read_csv(filename)
            temp_df = temp_df['runtime']
            df['runtime'+str(i)] = temp_df

    os.chdir(cwd)

    # Average the runtimes for each test case on each hardware
    # code for expanding to 10 runs
    # ,'runtime5','runtime6','runtime7','runtime8','runtime9'
    for df in [df_laptop_original,df_cluster_original]:
        calculate_avg_runtime(df,"original")

    for df in [df_laptop_newline,df_cluster_newline]:
        calculate_avg_runtime(df,"newline")

    for df in [df_laptop_fmt,df_cluster_fmt]:
        calculate_avg_runtime(df,"fmt")

    for df in [df_laptop_cc,df_cluster_cc]:
        calculate_avg_runtime(df,"colour_class")

    calculate_avg_runtime(df_laptop_vector, "vector")
    calculate_avg_runtime(df_laptop_comment, "comment")
    calculate_avg_runtime(df_laptop_max, "max")

    df_laptop_newline["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_newline['avg_runtime']
    df_laptop_fmt["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_fmt['avg_runtime']
    df_laptop_cc["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_cc['avg_runtime']
    df_laptop_vector["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_vector['avg_runtime']
    df_laptop_comment["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_comment['avg_runtime']
    df_laptop_max["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_max['avg_runtime']

    #df_cluster_newline["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_newline['avg_runtime']
    #df_cluster_fmt["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_fmt['avg_runtime']
    #df_cluster_cc["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_cc['avg_runtime']
    
    # create merged dfs for each graph
    df_laptop = pd.concat([df_laptop_original,df_laptop_newline,df_laptop_fmt,df_laptop_cc,df_laptop_vector,df_laptop_comment,df_laptop_max])
    df_laptop_speed_up = pd.concat([df_laptop_newline,df_laptop_fmt,df_laptop_cc,df_laptop_vector,df_laptop_comment,df_laptop_max])

    df_laptop_runs = pd.melt(df_laptop,
                            id_vars=["file","code_type"],
                            value_vars=["runtime","runtime1","runtime2","runtime3","runtime4"],
                            var_name = "test_run",
                            value_name="runtimes")
    
    df_laptop_speed_ups = pd.melt(df_laptop_speed_up,
                            id_vars=["file","code_type"],
                            value_vars=["speed_up"],
                            var_name = "speed_up",
                            value_name="speed_ups")
    
    print(df_laptop_speed_ups.head())

    #df_cluster = pd.concat([df_cluster_original,df_cluster_newline,df_cluster_fmt,df_cluster_cc])
    #df_cluster_runs = pd.melt(df_cluster,
    #                        id_vars=["file","code_type"],
    #                        value_vars=["runtime","runtime1","runtime2","runtime3","runtime4"],
    #                        var_name = "test_run",
    #                        value_name="runtimes")

    # generate graphs for each hardware comparing the different test cases - CHANGE to all runtimes rather than average
    create_comparison_graph(df_laptop_runs,"Laptop")
    #create_comparison_graph(df_cluster_runs,"Cluster")
    #create_comparison_graph(df_epyc_runs,"Epyc")
    
    create_speed_up_graph(df_laptop_speed_ups,"Laptop")
    #create_speed_up_graph(df_cluster,"Cluster")
    #create_speed_up_graph(df_epyc,"Epyc")
    
if __name__ == "__main__":
    main()
