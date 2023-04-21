import os
import random
import subprocess
import pandas as pd


if __name__ == '__main__':

    data = pd.read_csv("team_data_l/num.csv")
    subprocess.Popen(["rcssmonitor"])
    while(True):
        data.num[0] = random.randint(2,11)
        data.to_csv("team_data_l/num.csv" , sep=',',index=False)

        subprocess.Popen(["python3", "auto_runner/team_runner_l.py"])

        os.system("rcssserver")
        os.system("python3 Python_files/agent_trainer.py 'team_data_l/' 'Atredin' '1'")

