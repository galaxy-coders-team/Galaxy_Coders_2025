
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
    
    
def A2C(names,path_l,path_r,load):
    agents_l = []
    agents_r = []

    for i in range(10):

        agents_l.append(Agent( name= names[i],files_path = path_l))
        agents_l[i].load_agent()

        agents_r.append(Agent( name= names[i],files_path = path_r))
        agents_r[i].load_agent()

        if(load == 1):
            agents_l[i].learn()
            agents_l[i].load_model()
            
            agents_r[i].learn()
            agents_r[i].load_model()

        agents_l[i].learn()
        agents_r[i].learn()


    weights_a = []
    for index, weight in enumerate(agents_l[0].actor.weights):

        weights_sum = weight + agents_r[0].actor.weights[index]

        for i in range(1,10):
            weights_sum += agents_l[i].actor.weights[index] + agents_r[i].actor.weights[index]

        weights_a.append( weights_sum / 20 )

    weights_c = []
    for index, weight in enumerate(agents_l[0].critic.weights):

        weights_sum = weight + agents_r[0].critic.weights[index]

        for i in range(1,10):
            weights_sum += agents_l[i].critic.weights[index] + agents_r[i].critic.weights[index]

        weights_c.append( weights_sum / 20 )

    weights_t_a = []
    for index, weight in enumerate(agents_l[0].target_actor.weights):

        weights_sum = weight + agents_r[0].target_actor.weights[index]

        for i in range(1,10):
            weights_sum += agents_l[i].target_actor.weights[index] + agents_r[i].target_actor.weights[index]

        weights_t_a.append( weights_sum / 20 )

    weights_t_c = []
    for index, weight in enumerate(agents_l[0].target_critic.weights):

        weights_sum = weight + agents_r[0].target_critic.weights[index]

        for i in range(1,10):
            weights_sum += agents_l[i].target_critic.weights[index] + agents_r[i].target_critic.weights[index]

        weights_t_c.append( weights_sum / 20 )

    for i in range(10):
        
        agents_l[i].actor.set_weights(weights_a)
        agents_r[i].actor.set_weights(weights_a)

        agents_l[i].critic.set_weights(weights_c)
        agents_r[i].critic.set_weights(weights_c)
        
        agents_l[i].target_actor.set_weights(weights_t_a)
        agents_r[i].target_actor.set_weights(weights_t_a)

        agents_l[i].target_critic.set_weights(weights_t_c)
        agents_r[i].target_critic.set_weights(weights_t_c)

        agents_l[i].save_model()
        agents_r[i].save_model()

        save_csv(agents_l[i],path_l)
        save_csv(agents_r[i],path_r)

            
            


def A2C_l(names,path_l,load):
    agents_l = []

    for i in range(10):

        agents_l.append(Agent( name= names[i],files_path = path_l))
        agents_l[i].load_agent()

        if(load == 1):
            agents_l[i].learn()
            agents_l[i].load_model()

        agents_l[i].learn()


    weights_a = []
    for index, weight in enumerate(agents_l[0].actor.weights):

        weights_sum = weight 

        for i in range(1,10):
            weights_sum += agents_l[i].actor.weights[index] 

        weights_a.append( weights_sum / 10 )

    weights_c = []
    for index, weight in enumerate(agents_l[0].critic.weights):

        weights_sum = weight 

        for i in range(1,10):
            weights_sum += agents_l[i].critic.weights[index] 

        weights_a.append( weights_sum / 10 )

    weights_t_a = []
    for index, weight in enumerate(agents_l[0].target_actor.weights):

        weights_sum = weight

        for i in range(1,10):
            weights_sum += agents_l[i].target_actor.weights[index] 

        weights_a.append( weights_sum / 10 )

    weights_t_c = []
    for index, weight in enumerate(agents_l[0].target_critic.weights):

        weights_sum = weight 

        for i in range(1,10):
            weights_sum += agents_l[i].target_critic.weights[index] 

        weights_a.append( weights_sum / 10 )

    for i in range(10):
        
        agents_l[i].actor.set_weights(weights_a)

        agents_l[i].critic.set_weights(weights_c)
        
        agents_l[i].target_actor.set_weights(weights_t_a)

        agents_l[i].target_critic.set_weights(weights_t_c)

        agents_l[i].save_model()

        save_csv(agents_l[i],path_l)
               

        
if __name__ == '__main__':
    
    names = ["Amir" , "Diaco" , "Hesam" , "Atredin" ,"Madyar" , "Radmehr", "Kian" , "Poyan" , "Piroz" , "Iran" ]

    path_l = sys.argv[1]
    load = int(sys.argv[3])

    if(sys.argv[2] != ''):
        path_r = sys.argv[2]
        A2C(names,path_l,path_r,load)
    else:
        A2C_l(names,path_l,load)

       

    
    
       
    
    

