
from ast import Load
from platform import machine
import tensorflow as tf
import pandas as pd
import numpy as np
from colorama import Fore
import sys
import subprocess
from agent import Agent

def save_csv(agent , path):
    index = 0
    weights_index = 0
    bias_index = 0
    for i in agent.actor.weights:
        if(index % 2 == 0):
            np.savetxt(path + "models_weights/" +agent.name +"_"+ str(weights_index) +"_weights.csv", i.numpy() , delimiter=",")
            weights_index += 1
        else:
            np.savetxt(path + "models_weights/" +agent.name +"_"+ str(bias_index) +"_bias.csv", i.numpy() , delimiter=",")
            bias_index += 1
        index +=1
        
    print(Fore.GREEN + "..." + agent.name+" weights saved successfully..."+Fore.WHITE)


if __name__ == '__main__':
    
    main_path = str(sys.argv[1])
    name = sys.argv[2] 
    load = int(sys.argv[3])

    agent = Agent( name= name ,files_path = main_path)
    agent.load_agent()

    if(load == 1):
        agent.learn()
        agent.load_model()

    agent.learn()
    agent.save_model()
    save_csv(agent,main_path)






