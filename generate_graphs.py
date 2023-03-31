import pandas as pd
from matplotlib import pyplot as plt
import seaborn as sb
import os

def text_strip(input_text):
    stripped_text = input_text[10:]
    int_val = int(stripped_text)
    return int_val

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
    plt.savefig(hardware + "_Runtimes_2.png")
    plt.clf()
    os.chdir(cwd)

def create_speed_up_graph(df,hardware,instance):
    cwd = os.getcwd()

    sb.set_theme(style="whitegrid")
    ax = sb.stripplot(x="file",y="speed_up",data=df,size=6,jitter=0.3)
    ax.set_title(instance + " speed-up vs Original (" + hardware + ")")
    ax.set_xlabel("Test instances")
    ax.set_ylabel("Speed-Up")
    plt.xticks(rotation=270)
    plt.ylim(ymin=0)

    os.chdir(cwd+"/graphs")
    plt.savefig(instance + "_speedup_vs_original_" + hardware + ".png")
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
    df_laptop_comment =  pd.read_csv('Laptop_tests_Comment.csv')

    # final version will be set to 10
    for i in range(1,5):
        os.chdir(cwd + "/results/results_" + str(i))

        laptop_original = pd.read_csv('Laptop_tests_Original.csv')
        laptop_original = laptop_original['runtime']
        df_laptop_original['runtime'+str(i)] = laptop_original

        cluster_original = pd.read_csv('Cluster_tests_Original.csv')
        cluster_original = cluster_original['runtime']
        df_cluster_original['runtime'+str(i)] = cluster_original

        #epyc_original = pd.read_csv('Epyc_tests_Original.csv')
        #epyc_original = epyc_original['runtime']
        #df_epyc_original['runtime'+str(i)] = epyc_original

        laptop_newline = pd.read_csv('Laptop_tests_Newline.csv')
        laptop_newline = laptop_newline['runtime']
        df_laptop_newline['runtime'+str(i)] = laptop_newline
        
        cluster_newline = pd.read_csv('Cluster_tests_Newline.csv')
        cluster_newline = cluster_newline['runtime']
        df_cluster_newline['runtime'+str(i)] = cluster_newline
        
        #epyc_newline = pd.read_csv('Epyc_tests_Newline.csv')
        #epyc_newline = epyc_newline['runtime']
        #df_epyc_newline['runtime'+str(i)] = epyc_newline

        laptop_fmt = pd.read_csv('Laptop_tests_FMT.csv')
        laptop_fmt = laptop_fmt['runtime']
        df_laptop_fmt['runtime'+str(i)] = laptop_fmt
        
        cluster_fmt = pd.read_csv('Cluster_tests_FMT.csv')
        cluster_fmt = cluster_fmt['runtime']
        df_cluster_fmt['runtime'+str(i)] = cluster_fmt
        
        #epyc_fmt = pd.read_csv('Epyc_tests_FMT.csv')
        #epyc_fmt = epyc_fmt['runtime']
        #df_epyc_fmt['runtime'+str(i)] = epyc_fmt

        laptop_cc = pd.read_csv('Laptop_tests_Colour_Class.csv')
        laptop_cc = laptop_cc['runtime']
        df_laptop_cc['runtime'+str(i)] = laptop_cc
        
        cluster_cc = pd.read_csv('Cluster_tests_Colour_Class.csv')
        cluster_cc = cluster_cc['runtime']
        df_cluster_cc['runtime'+str(i)] = cluster_cc
        
        #epyc_cc = pd.read_csv('Epyc_tests_Colour_Class.csv')
        #epyc_cc = epyc_cc['runtime']
        #df_epyc_cc['runtime'+str(i)] = epyc_cc

        laptop_vector = pd.read_csv('Laptop_tests_Vector.csv')
        laptop_vector = laptop_vector['runtime']
        df_laptop_vector['runtime'+str(i)] = laptop_vector

        laptop_comment = pd.read_csv('Laptop_tests_Comment.csv')
        laptop_comment = laptop_comment['runtime']
        df_laptop_comment['runtime'+str(i)] = laptop_comment

    os.chdir(cwd)

    print(df_cluster_original.head())

    # Average the runtimes for each test case on each hardware
    # code for expanding back to 10 runs
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

    # create merged dfs for each graph
    df_laptop = pd.concat([df_laptop_original,df_laptop_newline,df_laptop_fmt,df_laptop_cc])
    df_laptop = pd.melt(df_laptop,
                            id_vars=["file","code_type"],
                            value_vars=["runtime","runtime1","runtime2","runtime3","runtime4"],
                            var_name = "test_run",
                            value_name="runtimes")
    
    df_cluster = pd.concat([df_cluster_original,df_cluster_newline,df_cluster_fmt,df_cluster_cc])
    df_cluster = pd.melt(df_cluster,
                            id_vars=["file","code_type"],
                            value_vars=["runtime","runtime1","runtime2","runtime3","runtime4"],
                            var_name = "test_run",
                            value_name="runtimes")

    df_laptop_newline["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_newline['avg_runtime']
    df_laptop_fmt["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_fmt['avg_runtime']
    df_laptop_cc["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_cc['avg_runtime']
    df_laptop_vector["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_vector['avg_runtime']
    df_laptop_comment["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_comment['avg_runtime']

    df_cluster_newline["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_newline['avg_runtime']
    df_cluster_fmt["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_fmt['avg_runtime']
    df_cluster_cc["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_cc['avg_runtime']
    
    # generate graphs for each hardware comparing the different test cases - CHANGE to all runtimes rather than average
    #create_comparison_graph(df_laptop,"Laptop")
    #create_comparison_graph(df_cluster,"Cluster")
    #plt.clf()
    #create_speed_up_graph(df_laptop_newline,"Laptop","Newline")
    #create_speed_up_graph(df_laptop_fmt,"Laptop","FMT")
    #create_speed_up_graph(df_laptop_cc,"Laptop","Colour_Class")
    create_speed_up_graph(df_laptop_vector,"Laptop","Vector")
    create_speed_up_graph(df_laptop_comment,"Laptop","Comment")

    #create_speed_up_graph(df_cluster_newline,"Cluster","Newline")
    #create_speed_up_graph(df_cluster_fmt,"Cluster","FMT")
    #create_speed_up_graph(df_cluster_cc,"Cluster","Colour_Class")
    
if __name__ == "__main__":
    main()
