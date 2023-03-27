import pandas as pd
from matplotlib import pyplot as plt
import seaborn as sb
import os

def text_strip(input_text):
    stripped_text = input_text[10:]
    int_val = int(stripped_text)
    return int_val

def main():
    cwd = os.getcwd()

    os.chdir(cwd + "/results/results_0")
    df_laptop_original = pd.read_csv('Laptop_tests_Original.csv')
    #df_cluster_original = pd.read_csv('Cluster_tests_Original.csv')
    #df_epyc_original = pd.read_csv('Epyc_tests_Original.csv')

    df_laptop_newline = pd.read_csv('Laptop_tests_Newline.csv')
    #df_cluster_newline = pd.read_csv('Cluster_tests_Newline.csv')
    #df_epyc_newline = pd.read_csv('Epyc_tests_Newline.csv')

    df_laptop_fmt = pd.read_csv('Laptop_tests_FMT.csv')
    #df_cluster_fmt = pd.read_csv('Cluster_tests_FMT.csv')
    #df_epyc_fmt = pd.read_csv('Epyc_tests_FMT.csv')

    df_laptop_cc = pd.read_csv('Laptop_tests_Colour_Class.csv')
    #df_cluster_cc = pd.read_csv('Cluster_tests_Colour_Class.csv')
    #df_epyc_cc = pd.read_csv('Epyc_tests_Colour_Class.csv')

    # final version will be set to 10
    for i in range(1,5):
        os.chdir(cwd + "/results/results_" + str(i))

        laptop_original = pd.read_csv('Laptop_tests_Original.csv')
        laptop_original = laptop_original['runtime']
        df_laptop_original['runtime'+str(i)] = laptop_original

        #cluster_original = pd.read_csv('Cluster_tests_Original.csv')
        #cluster_original = cluster_original['runtime']
        #df_cluster_original['runtime'+str(i)] = cluster_original

        #epyc_original = pd.read_csv('Epyc_tests_Original.csv')
        #epyc_original = epyc_original['runtime']
        #df_epyc_original['runtime'+str(i)] = epyc_original

        laptop_newline = pd.read_csv('Laptop_tests_Newline.csv')
        laptop_newline = laptop_newline['runtime']
        df_laptop_newline['runtime'+str(i)] = laptop_newline
        
        #cluster_newline = pd.read_csv('Cluster_tests_Newline.csv')
        #cluster_newline = cluster_newline['runtime']
        #df_cluster_newline['runtime'+str(i)] = cluster_newline
        
        #epyc_newline = pd.read_csv('Epyc_tests_Newline.csv')
        #epyc_newline = epyc_newline['runtime']
        #df_epyc_newline['runtime'+str(i)] = epyc_newline

        laptop_fmt = pd.read_csv('Laptop_tests_FMT.csv')
        laptop_fmt = laptop_fmt['runtime']
        df_laptop_fmt['runtime'+str(i)] = laptop_fmt
        
        #cluster_fmt = pd.read_csv('Cluster_tests_FMT.csv')
        #cluster_fmt = cluster_fmt['runtime']
        #df_cluster_fmt['runtime'+str(i)] = cluster_fmt
        
        #epyc_fmt = pd.read_csv('Epyc_tests_FMT.csv')
        #epyc_fmt = epyc_fmt['runtime']
        #df_epyc_fmt['runtime'+str(i)] = epyc_fmt

        laptop_cc = pd.read_csv('Laptop_tests_Colour_Class.csv')
        laptop_cc = laptop_cc['runtime']
        df_laptop_cc['runtime'+str(i)] = laptop_cc
        
        #cluster_cc = pd.read_csv('Cluster_tests_Colour_Class.csv')
        #cluster_cc = cluster_cc['runtime']
        #df_cluster_cc['runtime'+str(i)] = cluster_cc
        
        #epyc_cc = pd.read_csv('Epyc_tests_Colour_Class.csv')
        #epyc_cc = epyc_cc['runtime']
        #df_epyc_cc['runtime'+str(i)] = epyc_cc

    os.chdir(cwd)

    print(df_laptop_original.head())

    # Average the runtimes for each test case on each hardware
    # code for expanding back to 10 runs
    # ,'runtime5','runtime6','runtime7','runtime8','runtime9'

    df_laptop_original['avg_runtime'] = df_laptop_original[['runtime','runtime1','runtime2','runtime3','runtime4']].mean(axis=1)
    df_laptop_original['code_type'] = 'original'
    #df_cluster_original_mean = df_cluster_original[['runtime','runtime1','runtime2','runtime3','runtime4','runtime5','runtime6','runtime7','runtime8','runtime9']].mean(axis=1)
    #df_epyc_original_mean = df_epyc_original[['runtime','runtime1','runtime2','runtime3','runtime4','runtime5','runtime6','runtime7','runtime8','runtime9']].mean(axis=1)
     
    df_laptop_newline['avg_runtime'] = df_laptop_newline[['runtime','runtime1','runtime2','runtime3','runtime4']].mean(axis=1)
    df_laptop_newline['code_type'] = 'newline'
    #df_cluster_newline_mean = df_cluster_newline[['runtime','runtime1','runtime2','runtime3','runtime4','runtime5','runtime6','runtime7','runtime8','runtime9']].mean(axis=1)
    #df_epyc_newline_mean = df_epyc_newline[['runtime','runtime1','runtime2','runtime3','runtime4','runtime5','runtime6','runtime7','runtime8','runtime9']].mean(axis=1)
    
    df_laptop_fmt['avg_runtime'] = df_laptop_fmt[['runtime','runtime1','runtime2','runtime3','runtime4']].mean(axis=1)
    df_laptop_fmt['code_type'] = 'fmt'
    #df_cluster_fmt_mean = df_cluster_fmt[['runtime','runtime1','runtime2','runtime3','runtime4','runtime5','runtime6','runtime7','runtime8','runtime9']].mean(axis=1)
    #df_epyc_fmt_mean = df_epyc_fmt[['runtime','runtime1','runtime2','runtime3','runtime4','runtime5','runtime6','runtime7','runtime8','runtime9']].mean(axis=1)
    
    df_laptop_cc['avg_runtime'] = df_laptop_cc[['runtime','runtime1','runtime2','runtime3','runtime4']].mean(axis=1)
    df_laptop_cc['code_type'] = 'colour_class'
    #df_cluster_cc_mean = df_cluster_cc[['runtime','runtime1','runtime2','runtime3','runtime4','runtime5','runtime6','runtime7','runtime8','runtime9']].mean(axis=1)
    #df_epyc_cc_mean = df_epyc_cc[['runtime','runtime1','runtime2','runtime3','runtime4','runtime5','runtime6','runtime7','runtime8','runtime9']].mean(axis=1)

    # create merged dfs for each graph
    df_laptop_original_newline = pd.concat([df_laptop_original, df_laptop_newline])
    df_laptop_original_newline = pd.melt(df_laptop_original_newline,
                                         id_vars=["file","code_type"],
                                         value_vars=["runtime","runtime1","runtime2","runtime3","runtime4"],
                                         var_name = "test_run",
                                         value_name="runtimes")

    df_laptop_newline["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_newline['avg_runtime']

    df_laptop_original_fmt = pd.concat([df_laptop_original, df_laptop_fmt])
    df_laptop_fmt["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_fmt['avg_runtime']

    df_laptop_newline_fmt = pd.concat([df_laptop_newline, df_laptop_fmt])
    df_laptop_newline["speed_up_fmt"] = df_laptop_fmt['avg_runtime'] / df_laptop_newline['avg_runtime']

    df_laptop_original_cc = pd.concat([df_laptop_original, df_laptop_cc])
    df_laptop_cc["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_cc['avg_runtime']
    
    # generate graphs for each hardware comparing the different test cases - CHANGE to all runtimes rather than average
    sb.set_theme(style="whitegrid")
    #ax = sb.boxplot(x="code_type",y="avg_runtime",data=df_laptop_original_newline)
    ax = sb.stripplot(x="code_type",y="runtimes",data=df_laptop_original_newline,hue="code_type",size=6,jitter=0.3)
    ax.set_title("Test Instances Average Runtimes")
    ax.set_xlabel("Test instances")
    ax.set_ylabel("Runtime (milliseconds)")
    plt.yscale("log")
    plt.ylim(ymin=0.0001)
    plt.xticks(rotation=90)
    plt.savefig("original_vs_newline_2.png")
    #plt.show()
    plt.clf()
    # generate speed up graphs
    sb.set_theme(style="whitegrid")
    ax = sb.stripplot(x="file",y="speed_up",data=df_laptop_cc,size=6,jitter=0.3)
    ax.set_title("Speed-up per Instance on avg runtimes")
    ax.set_xlabel("Test instances")
    ax.set_ylabel("Speed-Up")
    plt.xticks(rotation=90)
    plt.savefig("newline_speedup_vs_original.png")
    #plt.show()

    # generate graphs for special cases to examine

    
if __name__ == "__main__":
    main()
