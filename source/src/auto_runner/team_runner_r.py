#!/usr/bin/env python
# coding: utf-8

# In[ ]:


from cProfile import run
import subprocess
import pandas as pd
import time


if __name__ == '__main__':
    
    data = pd.read_csv("auto_runner/run.csv")
    while(True):

      while(data.run[0] == 0):
          data = pd.read_csv("auto_runner/run.csv")
          time.sleep(2)
        
      data.run[0] = 0
      data.to_csv("auto_runner/run.csv" , sep=',',index=False)

      subprocess.Popen(["./start.sh","-t" ,"Galaxy_Coders2"])
      

            
            

    

