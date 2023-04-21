import os
import sys
from tokenize import Number


def clean_dir(path,end_of_the_name):

    names = ["Amir" , "Diaco" , "Hesam" , "Atredin" ,"Madyar" , "Radmehr", "Kian" , "Poyan" , "Piroz" , "Iran" ]

    for name in names:

        file_adress = path + name + end_of_the_name

        if(os.path.exists(file_adress)):
            os.remove(file_adress)

    print("... " + path + "NAME" + end_of_the_name + " Cleared ...")

def clean_dir_n(path,end_of_the_name,number):

    names = ["Amir" , "Diaco" , "Hesam" , "Atredin" ,"Madyar" , "Radmehr", "Kian" , "Poyan" , "Piroz" , "Iran" ]

    for name in names:

        file_adress = path + name + end_of_the_name

        for i in range(number):

            file_adress = path + name + "_" + str(i) + end_of_the_name

            if(os.path.exists(file_adress)):
                os.remove(file_adress)

    print("... " + path + "NAME_I" + end_of_the_name + " Cleared ( " + str(number) + " files ) ...")



if __name__ == '__main__':

    main_path = sys.argv[1]

    clean_dir(main_path + "agents/" , "_data.csv")

    clean_dir(main_path + "agents_reward_avg/" , "_rewards_mean.csv")

    clean_dir(main_path + "memory/states/"      , "_states.csv")
    clean_dir(main_path + "memory/next_states/" , "_next_states.csv")
    clean_dir(main_path + "memory/actions/"     , "_actions.csv")
    clean_dir(main_path + "memory/done/"        , "_done_memory.csv")
    clean_dir(main_path + "memory/rewards/"     , "_rewards.csv")

    clean_dir(main_path + "models/" , "_actor.h5")
    clean_dir(main_path + "models/" , "_critic.h5")
    clean_dir(main_path + "models/" , "_target_actor.h5")
    clean_dir(main_path + "models/" , "_target_critic.h5")

    clean_dir_n(main_path + "models_weights/" , "_bias.csv" , 7)
    clean_dir_n(main_path + "models_weights/" , "_weights.csv" , 7)
