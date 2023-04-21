#!/usr/bin/env python
# coding: utf-8

# In[ ]:


from cProfile import run
import os
import pandas as pd
import subprocess
import sys



if __name__ == '__main__':
    data = pd.read_csv("Python_files/team_runner_conf.csv")

    side = sys.argv[1]

    if(side == 'l'):
        data.run_numbers_l[0] += 1

    if(side == 'r'):
        data.run_numbers_r[0] += 1


    if(data.run_numbers_l[0] == 10 & side == 'r'):

        data.run_numbers_l[0] = 0
        data.games_played[0] += 1

        subprocess.Popen(["./start.sh"])

    if(data.run_numbers_r[0] == 10 & side == 'r'):
        data.run_numbers_r[0] = 0
        subprocess.Popen(["./start.sh -t Galaxy_Coders2"])
        
    data.to_csv("Python_files/team_runner_conf.csv" , sep=',',index=False)

            
            

    

