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
    df['std_dev'] = df[['runtime','runtime1','runtime2','runtime3','runtime4']].std(axis=1)
    df['rsd'] = ( 100 * (df['std_dev'] / abs(df['avg_runtime'])) )

    df['code_type'] = code_type

def format_decimals(input_val):
    return round(input_val, 2)
    
def strip_file_ext(input_text):
    return input_text[:-4]

def tex_underscore(input_text):
    split_string = input_text.split('_')
    new_string = "{\_}".join(split_string)
    return new_string

def create_comparison_graph(df,hardware):
    cwd = os.getcwd()

    sb.set_theme(style="whitegrid")
    ax = sb.swarmplot(x="code_type",y="runtimes",data=df,hue="code_type",size=2,legend=False)
    ax.set_title("Test Instances Runtimes (" + hardware + ")")
    ax.set_xlabel("Code Test Cases")
    ax.set_ylabel("Runtime (milliseconds)")
    plt.yscale("log")
    #plt.ylim(ymin=0.1)
    plt.xticks(rotation=0)

    hardware_filename = hardware.split()
    hardware_filename = "".join(hardware_filename)

    os.chdir(cwd+"/graphs")
    plt.savefig(hardware_filename + "_Runtimes.png", bbox_inches="tight")
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
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5), ncol=1, fancybox=True, shadow=True)

    hardware_filename = hardware.split()
    hardware_filename = "".join(hardware_filename)

    os.chdir(cwd+"/graphs")
    plt.savefig(hardware_filename + "_speedups_vs_original.png", bbox_inches="tight")
    plt.clf()
    os.chdir(cwd)

def create_table_entry(df,hardware):
    df["file"] = df["file"].apply(strip_file_ext)
    df["file"] = df["file"].apply(tex_underscore)
    df["rsd"] = df["rsd"].apply(format_decimals)
    df["newline_speedup"] = df["newline_speedup"].apply(format_decimals)
    df["fmt_speedup"] = df["fmt_speedup"].apply(format_decimals)
    df["cc_speedup"] = df["cc_speedup"].apply(format_decimals)
    df["vector_speedup"] = df["vector_speedup"].apply(format_decimals)
    df["comment_speedup"] = df["comment_speedup"].apply(format_decimals)
    df["max_speedup"] = df["max_speedup"].apply(format_decimals)
    df = df.sort_values('avg_runtime')

    hardware_filename = hardware.split()
    hardware_filename = "".join(hardware_filename)
    filename = hardware_filename + "_table_entry.csv"
    df.to_csv(filename,sep='&',mode='w',index=False)

def print_speed_up_stats(df, hardware):
    print("speed up stats for " + hardware)
    newline_avg = round(df['newline_speedup'].mean(), 2)
    newline_stddev = round(df['newline_speedup'].std(), 2)
    newline_rsd = ( 100 * (newline_stddev / newline_avg) )
    print("newline: mean = " + str(newline_avg) + ", std_dev = " + str(newline_stddev) + ", RSD = " + str(newline_rsd))

    newline_avg = round(df['fmt_speedup'].mean(), 2)
    newline_stddev = round(df['fmt_speedup'].std(), 2)
    newline_rsd = ( 100 * (newline_stddev / newline_avg) )
    print("fmt: mean = " + str(newline_avg) + ", std_dev = " + str(newline_stddev) + ", RSD = " + str(newline_rsd))

    newline_avg = round(df['cc_speedup'].mean(), 2)
    newline_stddev = round(df['cc_speedup'].std(), 2)
    newline_rsd = ( 100 * (newline_stddev / newline_avg) )
    print("cc: mean = " + str(newline_avg) + ", std_dev = " + str(newline_stddev) + ", RSD = " + str(newline_rsd))

    newline_avg = round(df['vector_speedup'].mean(), 2)
    newline_stddev = round(df['vector_speedup'].std(), 2)
    newline_rsd = ( 100 * (newline_stddev / newline_avg) )
    print("vector: mean = " + str(newline_avg) + ", std_dev = " + str(newline_stddev) + ", RSD = " + str(newline_rsd))

    newline_avg = round(df['comment_speedup'].mean(), 2)
    newline_stddev = round(df['comment_speedup'].std(), 2)
    newline_rsd = ( 100 * (newline_stddev / newline_avg) )
    print("comment: mean = " + str(newline_avg) + ", std_dev = " + str(newline_stddev) + ", RSD = " + str(newline_rsd))

    newline_avg = round(df['max_speedup'].mean(), 2)
    newline_stddev = round(df['max_speedup'].std(), 2)
    newline_rsd = ( 100 * (newline_stddev / newline_avg) )
    print("max: mean = " + str(newline_avg) + ", std_dev = " + str(newline_stddev) + ", RSD = " + str(newline_rsd))
    print("")

def main():
    cwd = os.getcwd()

    os.chdir(cwd + "/results/results_0")
    df_laptop_original = pd.read_csv('Laptop_tests_Original.csv')
    df_cluster_original = pd.read_csv('Cluster_tests_Original.csv')
    df_epyc_original = pd.read_csv('Epyc_tests_Original.csv')
    df_clusterNFS_original = pd.read_csv('ClusterNFS_tests_Original.csv')
    df_epycNFS_original = pd.read_csv('EpycNFS_tests_Original.csv')

    df_laptop_newline = pd.read_csv('Laptop_tests_Newline.csv')
    df_cluster_newline = pd.read_csv('Cluster_tests_Newline.csv')
    df_epyc_newline = pd.read_csv('Epyc_tests_Newline.csv')
    df_clusterNFS_newline = pd.read_csv('ClusterNFS_tests_Newline.csv')
    df_epycNFS_newline = pd.read_csv('EpycNFS_tests_Newline.csv')

    df_laptop_fmt = pd.read_csv('Laptop_tests_FMT.csv')
    df_cluster_fmt = pd.read_csv('Cluster_tests_FMT.csv')
    df_epyc_fmt = pd.read_csv('Epyc_tests_FMT.csv')
    df_clusterNFS_fmt = pd.read_csv('ClusterNFS_tests_FMT.csv')
    df_epycNFS_fmt = pd.read_csv('EpycNFS_tests_FMT.csv')

    df_laptop_cc = pd.read_csv('Laptop_tests_Colour_Class.csv')
    df_cluster_cc = pd.read_csv('Cluster_tests_Colour_Class.csv')
    df_epyc_cc = pd.read_csv('Epyc_tests_Colour_Class.csv')
    df_clusterNFS_cc = pd.read_csv('ClusterNFS_tests_Colour_Class.csv')
    df_epycNFS_cc = pd.read_csv('EpycNFS_tests_Colour_Class.csv')

    df_laptop_vector = pd.read_csv('Laptop_tests_Vector.csv')
    df_cluster_vector = pd.read_csv('Cluster_tests_Vector.csv')
    df_epyc_vector = pd.read_csv('Epyc_tests_Vector.csv')
    df_clusterNFS_vector = pd.read_csv('ClusterNFS_tests_Vector.csv')
    df_epycNFS_vector = pd.read_csv('EpycNFS_tests_Vector.csv')

    df_laptop_comment =  pd.read_csv('Laptop_tests_Comment.csv')
    df_cluster_comment = pd.read_csv('Cluster_tests_Comment.csv')
    df_epyc_comment = pd.read_csv('Epyc_tests_Comment.csv')
    df_clusterNFS_comment = pd.read_csv('ClusterNFS_tests_Comment.csv')
    df_epycNFS_comment = pd.read_csv('EpycNFS_tests_Comment.csv')

    df_laptop_max = pd.read_csv('Laptop_tests_Max.csv')
    df_cluster_max = pd.read_csv('Cluster_tests_Max.csv')
    df_epyc_max = pd.read_csv('Epyc_tests_Max.csv')
    df_clusterNFS_max = pd.read_csv('ClusterNFS_tests_Max.csv')
    df_epycNFS_max = pd.read_csv('EpycNFS_tests_Max.csv')

    df_file_pairs = [
        [df_laptop_original,'Laptop_tests_Original.csv'], [df_cluster_original,'Cluster_tests_Original.csv'], [df_epyc_original,'Epyc_tests_Original.csv'], [df_clusterNFS_original,'Cluster_tests_Original.csv'], [df_epycNFS_original,'Epyc_tests_Original.csv'],
        [df_laptop_newline,'Laptop_tests_Newline.csv'], [df_cluster_newline,'Cluster_tests_Newline.csv'], [df_epyc_newline,'Epyc_tests_Newline.csv'], [df_clusterNFS_newline,'Cluster_tests_Newline.csv'], [df_epycNFS_newline,'Epyc_tests_Newline.csv'],
        [df_laptop_fmt,'Laptop_tests_FMT.csv'], [df_cluster_fmt,'Cluster_tests_FMT.csv'], [df_epyc_fmt,'Epyc_tests_FMT.csv'], [df_clusterNFS_fmt,'Cluster_tests_FMT.csv'], [df_epycNFS_fmt,'Epyc_tests_FMT.csv'],
        [df_laptop_cc,'Laptop_tests_Colour_Class.csv'], [df_cluster_cc,'Cluster_tests_Colour_Class.csv'], [df_epyc_cc,'Epyc_tests_Colour_Class.csv'], [df_clusterNFS_cc,'Cluster_tests_Colour_Class.csv'], [df_epycNFS_cc,'Epyc_tests_Colour_Class.csv'],
        [df_laptop_vector,'Laptop_tests_Vector.csv'], [df_cluster_vector,'Cluster_tests_Vector.csv'], [df_epyc_vector,'Epyc_tests_Vector.csv'], [df_clusterNFS_vector,'Cluster_tests_Vector.csv'], [df_epycNFS_vector,'Epyc_tests_Vector.csv'],
        [df_laptop_comment,'Laptop_tests_Comment.csv'], [df_cluster_comment,'Cluster_tests_Comment.csv'], [df_epyc_comment,'Epyc_tests_Comment.csv'], [df_clusterNFS_comment,'Cluster_tests_Comment.csv'], [df_epycNFS_comment,'Epyc_tests_Comment.csv'],
        [df_laptop_max,'Laptop_tests_Max.csv'], [df_cluster_max,'Cluster_tests_Max.csv'], [df_epyc_max,'Epyc_tests_Max.csv'], [df_clusterNFS_max,'Cluster_tests_Max.csv'], [df_epycNFS_max,'Epyc_tests_Max.csv']
    ]

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
    for df in [df_laptop_original,df_cluster_original,df_epyc_original,df_clusterNFS_original,df_epycNFS_original]:
        calculate_avg_runtime(df,"original")

    for df in [df_laptop_newline,df_cluster_newline,df_epyc_newline,df_clusterNFS_newline,df_epycNFS_newline]:
        calculate_avg_runtime(df,"newline")

    for df in [df_laptop_fmt,df_cluster_fmt,df_epyc_fmt,df_clusterNFS_fmt,df_epycNFS_fmt]:
        calculate_avg_runtime(df,"fmt")

    for df in [df_laptop_cc,df_cluster_cc,df_epyc_cc,df_clusterNFS_cc,df_epycNFS_cc]:
        calculate_avg_runtime(df,"colour_class")

    for df in [df_laptop_vector,df_cluster_vector,df_epyc_vector,df_clusterNFS_vector,df_epycNFS_vector]:
        calculate_avg_runtime(df, "vector")
    
    for df in [df_laptop_comment,df_cluster_comment,df_epyc_comment,df_clusterNFS_comment,df_epycNFS_comment]:
        calculate_avg_runtime(df, "comment")
    
    for df in [df_laptop_max,df_cluster_max,df_epyc_max,df_clusterNFS_max,df_epycNFS_max]:
        calculate_avg_runtime(df, "max")

    df_laptop_newline["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_newline['avg_runtime']
    df_laptop_fmt["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_fmt['avg_runtime']
    df_laptop_cc["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_cc['avg_runtime']
    df_laptop_vector["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_vector['avg_runtime']
    df_laptop_comment["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_comment['avg_runtime']
    df_laptop_max["speed_up"] = df_laptop_original['avg_runtime'] / df_laptop_max['avg_runtime']

    df_cluster_newline["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_newline['avg_runtime']
    df_cluster_fmt["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_fmt['avg_runtime']
    df_cluster_cc["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_cc['avg_runtime']
    df_cluster_vector["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_vector['avg_runtime']
    df_cluster_comment["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_comment['avg_runtime']
    df_cluster_max["speed_up"] = df_cluster_original['avg_runtime'] / df_cluster_max['avg_runtime']

    df_epyc_newline["speed_up"] = df_epyc_original['avg_runtime'] / df_epyc_newline['avg_runtime']
    df_epyc_fmt["speed_up"] = df_epyc_original['avg_runtime'] / df_epyc_fmt['avg_runtime']
    df_epyc_cc["speed_up"] = df_epyc_original['avg_runtime'] / df_epyc_cc['avg_runtime']
    df_epyc_vector["speed_up"] = df_epyc_original['avg_runtime'] / df_epyc_vector['avg_runtime']
    df_epyc_comment["speed_up"] = df_epyc_original['avg_runtime'] / df_epyc_comment['avg_runtime']
    df_epyc_max["speed_up"] = df_epyc_original['avg_runtime'] / df_epyc_max['avg_runtime']

    df_clusterNFS_newline["speed_up"] = df_clusterNFS_original['avg_runtime'] / df_clusterNFS_newline['avg_runtime']
    df_clusterNFS_fmt["speed_up"] = df_clusterNFS_original['avg_runtime'] / df_clusterNFS_fmt['avg_runtime']
    df_clusterNFS_cc["speed_up"] = df_clusterNFS_original['avg_runtime'] / df_clusterNFS_cc['avg_runtime']
    df_clusterNFS_vector["speed_up"] = df_clusterNFS_original['avg_runtime'] / df_clusterNFS_vector['avg_runtime']
    df_clusterNFS_comment["speed_up"] = df_clusterNFS_original['avg_runtime'] / df_clusterNFS_comment['avg_runtime']
    df_clusterNFS_max["speed_up"] = df_clusterNFS_original['avg_runtime'] / df_clusterNFS_max['avg_runtime']

    df_epycNFS_newline["speed_up"] = df_epycNFS_original['avg_runtime'] / df_epycNFS_newline['avg_runtime']
    df_epycNFS_fmt["speed_up"] = df_epycNFS_original['avg_runtime'] / df_epycNFS_fmt['avg_runtime']
    df_epycNFS_cc["speed_up"] = df_epycNFS_original['avg_runtime'] / df_epycNFS_cc['avg_runtime']
    df_epycNFS_vector["speed_up"] = df_epycNFS_original['avg_runtime'] / df_epycNFS_vector['avg_runtime']
    df_epycNFS_comment["speed_up"] = df_epycNFS_original['avg_runtime'] / df_epycNFS_comment['avg_runtime']
    df_epycNFS_max["speed_up"] = df_epycNFS_original['avg_runtime'] / df_epycNFS_max['avg_runtime']
    
    # create merged dfs for each graph
    df_laptop = pd.concat([df_laptop_original,df_laptop_newline,df_laptop_fmt,df_laptop_cc,df_laptop_vector,df_laptop_comment,df_laptop_max])
    df_laptop_speed_up = pd.concat([df_laptop_newline,df_laptop_fmt,df_laptop_cc,df_laptop_vector,df_laptop_comment,df_laptop_max])
    df_laptop_table = df_laptop_original
    df_laptop_table['newline_speedup'] = df_laptop_newline['speed_up']
    df_laptop_table['fmt_speedup'] = df_laptop_fmt['speed_up']
    df_laptop_table['cc_speedup'] = df_laptop_cc['speed_up']
    df_laptop_table['vector_speedup'] = df_laptop_vector['speed_up']
    df_laptop_table['comment_speedup'] = df_laptop_comment['speed_up']
    df_laptop_table['max_speedup'] = df_laptop_max['speed_up']
    df_laptop_table = df_laptop_table[['file','avg_runtime','rsd','newline_speedup','fmt_speedup','cc_speedup','vector_speedup','comment_speedup','max_speedup']].copy()
    
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
    
    df_cluster = pd.concat([df_cluster_original,df_cluster_newline,df_cluster_fmt,df_cluster_cc,df_cluster_vector,df_cluster_comment,df_cluster_max])
    df_cluster_speed_up = pd.concat([df_cluster_newline,df_cluster_fmt,df_cluster_cc,df_cluster_vector,df_cluster_comment,df_cluster_max])
    df_cluster_table = df_cluster_original
    df_cluster_table['newline_speedup'] = df_cluster_newline['speed_up']
    df_cluster_table['fmt_speedup'] = df_cluster_fmt['speed_up']
    df_cluster_table['cc_speedup'] = df_cluster_cc['speed_up']
    df_cluster_table['vector_speedup'] = df_cluster_vector['speed_up']
    df_cluster_table['comment_speedup'] = df_cluster_comment['speed_up']
    df_cluster_table['max_speedup'] = df_cluster_max['speed_up']
    df_cluster_table = df_cluster_table[['file','avg_runtime','rsd','newline_speedup','fmt_speedup','cc_speedup','vector_speedup','comment_speedup','max_speedup']].copy()

    df_cluster_runs = pd.melt(df_cluster,
                            id_vars=["file","code_type"],
                            value_vars=["runtime","runtime1","runtime2","runtime3","runtime4"],
                            var_name = "test_run",
                            value_name="runtimes")
    
    df_cluster_speed_ups = pd.melt(df_cluster_speed_up,
                            id_vars=["file","code_type"],
                            value_vars=["speed_up"],
                            var_name = "speed_up",
                            value_name="speed_ups")
    
    df_epyc = pd.concat([df_epyc_original,df_epyc_newline,df_epyc_fmt,df_epyc_cc,df_epyc_vector,df_epyc_comment,df_epyc_max])
    df_epyc_speed_up = pd.concat([df_epyc_newline,df_epyc_fmt,df_epyc_cc,df_epyc_vector,df_epyc_comment,df_epyc_max])
    df_epyc_table = df_epyc_original
    df_epyc_table['newline_speedup'] = df_epyc_newline['speed_up']
    df_epyc_table['fmt_speedup'] = df_epyc_fmt['speed_up']
    df_epyc_table['cc_speedup'] = df_epyc_cc['speed_up']
    df_epyc_table['vector_speedup'] = df_epyc_vector['speed_up']
    df_epyc_table['comment_speedup'] = df_epyc_comment['speed_up']
    df_epyc_table['max_speedup'] = df_epyc_max['speed_up']
    df_epyc_table = df_epyc_table[['file','avg_runtime','rsd','newline_speedup','fmt_speedup','cc_speedup','vector_speedup','comment_speedup','max_speedup']].copy()

    df_epyc_runs = pd.melt(df_epyc,
                            id_vars=["file","code_type"],
                            value_vars=["runtime","runtime1","runtime2","runtime3","runtime4"],
                            var_name = "test_run",
                            value_name="runtimes")
    
    df_epyc_speed_ups = pd.melt(df_epyc_speed_up,
                            id_vars=["file","code_type"],
                            value_vars=["speed_up"],
                            var_name = "speed_up",
                            value_name="speed_ups")
    
    df_clusterNFS = pd.concat([df_clusterNFS_original,df_clusterNFS_newline,df_clusterNFS_fmt,df_clusterNFS_cc,df_clusterNFS_vector,df_clusterNFS_comment,df_clusterNFS_max])
    df_clusterNFS_speed_up = pd.concat([df_clusterNFS_newline,df_clusterNFS_fmt,df_clusterNFS_cc,df_clusterNFS_vector,df_clusterNFS_comment,df_clusterNFS_max])
    df_clusterNFS_table = df_clusterNFS_original
    df_clusterNFS_table['newline_speedup'] = df_clusterNFS_newline['speed_up']
    df_clusterNFS_table['fmt_speedup'] = df_clusterNFS_fmt['speed_up']
    df_clusterNFS_table['cc_speedup'] = df_clusterNFS_cc['speed_up']
    df_clusterNFS_table['vector_speedup'] = df_clusterNFS_vector['speed_up']
    df_clusterNFS_table['comment_speedup'] = df_clusterNFS_comment['speed_up']
    df_clusterNFS_table['max_speedup'] = df_clusterNFS_max['speed_up']
    df_clusterNFS_table = df_clusterNFS_table[['file','avg_runtime','rsd','newline_speedup','fmt_speedup','cc_speedup','vector_speedup','comment_speedup','max_speedup']].copy()

    df_clusterNFS_runs = pd.melt(df_clusterNFS,
                            id_vars=["file","code_type"],
                            value_vars=["runtime","runtime1","runtime2","runtime3","runtime4"],
                            var_name = "test_run",
                            value_name="runtimes")
    
    df_clusterNFS_speed_ups = pd.melt(df_clusterNFS_speed_up,
                            id_vars=["file","code_type"],
                            value_vars=["speed_up"],
                            var_name = "speed_up",
                            value_name="speed_ups")

    df_epycNFS = pd.concat([df_epycNFS_original,df_epycNFS_newline,df_epycNFS_fmt,df_epycNFS_cc,df_epycNFS_vector,df_epycNFS_comment,df_epycNFS_max])
    df_epycNFS_speed_up = pd.concat([df_epycNFS_newline,df_epycNFS_fmt,df_epycNFS_cc,df_epycNFS_vector,df_epycNFS_comment,df_epycNFS_max])
    df_epycNFS_table = df_epycNFS_original
    df_epycNFS_table['newline_speedup'] = df_epycNFS_newline['speed_up']
    df_epycNFS_table['fmt_speedup'] = df_epycNFS_fmt['speed_up']
    df_epycNFS_table['cc_speedup'] = df_epycNFS_cc['speed_up']
    df_epycNFS_table['vector_speedup'] = df_epycNFS_vector['speed_up']
    df_epycNFS_table['comment_speedup'] = df_epycNFS_comment['speed_up']
    df_epycNFS_table['max_speedup'] = df_epycNFS_max['speed_up']
    df_epycNFS_table = df_epycNFS_table[['file','avg_runtime','rsd','newline_speedup','fmt_speedup','cc_speedup','vector_speedup','comment_speedup','max_speedup']].copy()

    df_epycNFS_runs = pd.melt(df_epycNFS,
                            id_vars=["file","code_type"],
                            value_vars=["runtime","runtime1","runtime2","runtime3","runtime4"],
                            var_name = "test_run",
                            value_name="runtimes")
    
    df_epycNFS_speed_ups = pd.melt(df_epycNFS_speed_up,
                            id_vars=["file","code_type"],
                            value_vars=["speed_up"],
                            var_name = "speed_up",
                            value_name="speed_ups")

    # generate graphs for each hardware comparing the different test cases - CHANGE to all runtimes rather than average
    create_comparison_graph(df_laptop_runs,"Laptop")
    create_comparison_graph(df_cluster_runs,"Fatanode SSD")
    create_comparison_graph(df_epyc_runs,"Fataepyc SSD")
    create_comparison_graph(df_clusterNFS_runs,"Fatanode NFS")
    create_comparison_graph(df_epycNFS_runs,"Fataepyc NFS")
    
    #create_speed_up_graph(df_laptop_speed_ups,"Laptop")
    #create_speed_up_graph(df_cluster_speed_ups,"Fatanode SSD")
    #create_speed_up_graph(df_epyc_speed_ups,"Fataepyc SSD")
    #create_speed_up_graph(df_clusterNFS_speed_ups,"Fatanode NFS")
    #create_speed_up_graph(df_epycNFS_speed_ups,"Fataepyc NFS")

    #create_table_entry(df_laptop_table,"Laptop")
    #create_table_entry(df_cluster_table,"Fatanode SSD")
    #create_table_entry(df_epyc_table,"Fataepyc SSD")
    #create_table_entry(df_clusterNFS_table,"Fatanode NFS")
    #create_table_entry(df_epycNFS_table,"Fataepyc NFS")

    #print_speed_up_stats(df_laptop_table, "Laptop")
    #print_speed_up_stats(df_cluster_table, "Fatanode SSD")
    #print_speed_up_stats(df_epyc_table, "Fataepyc SSD")
    #print_speed_up_stats(df_clusterNFS_table, "Fatanode NFS")
    #print_speed_up_stats(df_epycNFS_table, "Fataepyc NFS")
    
if __name__ == "__main__":
    main()
