import os
import subprocess
import pandas as pd

if __name__ == '__main__':

    data = pd.read_csv("auto_runner/run.csv")
    subprocess.Popen(["rcssmonitor"])

    while(True):
        subprocess.Popen(["python3","auto_runner/team_runner_l.py"])

        data.run[0] = 1
        data.to_csv("auto_runner/run.csv" , sep=',',index=False)

        os.system("rcssserver")
        os.system("python3 Python_files/all_trainer_A2C.py 'team_data_l/' 'team_data_r/' '1'")
