
import tensorflow as tf
import pandas as pd
import numpy as np
from colorama import Fore
from agent import Agent
import sys

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
    
def trainer(names,path , load):
    agents = []
                     
    for i in range(10):
        agents.append(Agent( name= names[i],files_path = path))
        agents[i].load_agent()
        if(load == 1):
            agents[i].learn()
            agents[i].load_model()

        agents[i].learn()
        save_csv(agents[i],path)
        
if __name__ == '__main__':
    
    names = ["Amir" , "Diaco" , "Hesam" , "Atredin" ,"Madyar" , "Radmehr", "Kian" , "Poyan" , "Piroz" , "Iran" ]

    path_l = sys.argv[1]
    load = int(sys.argv[3])

    trainer(names,path_l , load) 

    if(sys.argv[2] != ''):
        path_r = sys.argv[2]
        trainer(names,path_r , load)
       

    
    
       
    
    

