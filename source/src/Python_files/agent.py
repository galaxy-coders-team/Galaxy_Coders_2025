#Socer Simulation Actor Class 
#{ 
#   programmed BY Amir mahdy yari
#   Email : yarya524@gmail.com
#   Telegram : @a_m_yari
#}

import tensorflow as tf
import tensorflow.keras as keras
from tensorflow.keras.optimizers import Adam
from colorama import Fore

from actor_model import ActorNetwork
from critic_model import CriticNetwork

import random
import numpy as np
import pandas as pd

class Agent:
    def __init__(self, name = "agent" , files_path = "data/" , max_mem = 3000 , 
                 batch_size = 64 , epsilon = 0.05 , eps_decay = 0.98 , eps_min = 0.005 , gamma = 0.98,
                 soft_update_rate = 0.005, learning_rate_alpha = 0.001 , learning_rate_beta = 0.002):
        
        #file managment params
        self.name = name
        self.models_path = files_path + 'models/' 
        self.move_memory_path = files_path + 'move_memory/' 
        self.kick_memory_path = files_path + 'kick_memory/' 
        self.agents_path = files_path + 'agents/'
        
        # RL params
        self.batch_size = batch_size
        self.epsilon = epsilon
        self.eps_decay = eps_decay
        self.eps_min = eps_min
        self.soft_update_rate = soft_update_rate
        self.gamma = gamma
        
        #agent unused data (it's for c++ part) 
        self.max_mem = max_mem
        
        #models
        self.move_actor  = ActorNetwork()
        self.kick_actor  = ActorNetwork()
        self.critic = CriticNetwork()
        
        self.target_move_actor  = ActorNetwork()
        self.target_kick_actor  = ActorNetwork()
        self.target_critic = CriticNetwork()
        
        #compile models
        self.move_actor.compile  ( optimizer = Adam( learning_rate = learning_rate_alpha ))
        self.kick_actor.compile  ( optimizer = Adam( learning_rate = learning_rate_alpha ))
        self.critic.compile ( optimizer = Adam( learning_rate = learning_rate_beta  ))
        
        self.target_move_actor.compile  ( optimizer = Adam( learning_rate = learning_rate_alpha ))
        self.target_kick_actor.compile  ( optimizer = Adam( learning_rate = learning_rate_alpha ))
        self.target_critic.compile ( optimizer=Adam( learning_rate = learning_rate_beta  ))
        
    def load_agent(self):
        #read the data file
        agent_data = pd.read_csv(self.agents_path + self.name + '_data.csv')
        
        #executing new value on agent
        self.epsilon          = agent_data.epsilon[0]
        self.eps_decay        = agent_data.eps_decay[0]
        self.eps_min          = agent_data.eps_min[0]
        self.max_mem          = agent_data.max_mem[0]
        self.batch_size       = agent_data.batch_size[0]
        self.soft_update_rate = agent_data.soft_update_rate[0]
        self.gamma            = agent_data.gamma[0]
        
        #compile models with new data
        self.move_actor.compile  ( optimizer = Adam( learning_rate = agent_data.learning_rate_alpha[0] ))
        self.kick_actor.compile  ( optimizer = Adam( learning_rate = agent_data.learning_rate_alpha[0] ))
        self.critic.compile ( optimizer = Adam( learning_rate = agent_data.learning_rate_beta[0]  ))
        
        self.target_move_actor.compile  ( optimizer = Adam( learning_rate = agent_data.learning_rate_alpha[0] ))
        self.target_kick_actor.compile  ( optimizer = Adam( learning_rate = agent_data.learning_rate_alpha[0] ))
        self.target_critic.compile ( optimizer=Adam( learning_rate = agent_data.learning_rate_beta[0]  ))
        
    def load_model(self):
        print(Fore.BLUE+'... loading (' + self.name +') models ...'+Fore.WHITE)
        
        self.move_actor.load_weights  ( self.models_path + self.name + '_move_actor.h5'  )
        self.kick_actor.load_weights  ( self.models_path + self.name + '_kick_actor.h5'  )
        self.critic.load_weights ( self.models_path + self.name + '_critic.h5' )
        
        self.move_target_actor.load_weights  ( self.models_path + self.name + '_move_target_actor.h5'  )
        self.kick_target_actor.load_weights  ( self.models_path + self.name + '_kick_target_actor.h5'  )
        self.target_critic.load_weights ( self.models_path + self.name + '_target_critic.h5' )
        
    def read_move_memory_file(self):
        
        #read csv_files                              
        states      = pd.read_csv(self.move_memory_path + "states/"      + self.name + "_states.csv"      ,header = None)
        mem_size = states.shape[1]
        states.to_numpy(dtype=np.float64)

        actions     = pd.read_csv(self.move_memory_path + "actions/"     + self.name + "_actions.csv"     ,header = None).to_numpy(dtype=np.float64)
        rewards     = pd.read_csv(self.move_memory_path + "rewards/"     + self.name + "_rewards.csv"     ,header = None).to_numpy(dtype=np.float64)
        next_states = pd.read_csv(self.move_memory_path + "next_states/" + self.name + "_next_states.csv" ,header = None).to_numpy(dtype=np.float64)
        dones        = pd.read_csv(self.move_memory_path + "done/"        + self.name + "_done_memory.csv" ,header = None).to_numpy(dtype=np.float64)

        states = tf.convert_to_tensor(states, dtype=tf.float32)
        actions = tf.convert_to_tensor(actions, dtype=tf.float32)
        next_states = tf.convert_to_tensor(next_states, dtype=tf.float32)
        rewards = tf.convert_to_tensor(rewards, dtype=tf.float32)
        dones = tf.convert_to_tensor(dones, dtype=tf.float32)

        states      /= 100 
        actions     /= 100
        next_states /= 100
        rewards     /= 100

        state      = []
        action     = []
        next_state = []
        reward     = []
        done       = []

        for a in range(0, self.batch_size):
            index = random.randint(0, mem_size - 1)
            state.append(states[index])
            action.append(actions[index])
            next_state.append(next_states[index])
            reward.append(rewards[index])
            done.append(dones[index])

        state = tf.convert_to_tensor(state, dtype=tf.float32)
        action = tf.convert_to_tensor(action, dtype=tf.float32)
        next_state = tf.convert_to_tensor(next_state, dtype=tf.float32)
        reward = tf.convert_to_tensor(reward, dtype=tf.float32)
        done = tf.convert_to_tensor(done, dtype=tf.float32)

        return state, action, reward, next_state, done
    
    def read_kick_memory_file(self):
        
        #read csv_files                              
        states      = pd.read_csv(self.kick_memory_path + "states/"      + self.name + "_states.csv"      ,header = None)
        mem_size = states.shape[1]
        states.to_numpy(dtype=np.float64)

        actions     = pd.read_csv(self.kick_memory_path + "actions/"     + self.name + "_actions.csv"     ,header = None).to_numpy(dtype=np.float64)
        rewards     = pd.read_csv(self.kick_memory_path + "rewards/"     + self.name + "_rewards.csv"     ,header = None).to_numpy(dtype=np.float64)
        next_states = pd.read_csv(self.kick_memory_path + "next_states/" + self.name + "_next_states.csv" ,header = None).to_numpy(dtype=np.float64)
        dones       = pd.read_csv(self.kick_memory_path + "done/"        + self.name + "_done_memory.csv" ,header = None).to_numpy(dtype=np.float64)

        states = tf.convert_to_tensor(states, dtype=tf.float32)
        actions = tf.convert_to_tensor(actions, dtype=tf.float32)
        next_states = tf.convert_to_tensor(next_states, dtype=tf.float32)
        rewards = tf.convert_to_tensor(rewards, dtype=tf.float32)
        dones = tf.convert_to_tensor(dones, dtype=tf.float32)

        states      /= 100 
        actions     /= 100
        next_states /= 100
        rewards     /= 100

        state = []
        action = []
        next_state = []
        reward = []
        done = []

        for a in range(0, self.batch_size):
            index = random.randint(0, mem_size - 1)
            state.append(states[index])
            action.append(actions[index])
            next_state.append(next_states[index])
            reward.append(rewards[index])
            done.append(dones[index])

        state = tf.convert_to_tensor(state, dtype=tf.float32)
        action = tf.convert_to_tensor(action, dtype=tf.float32)
        next_state = tf.convert_to_tensor(next_state, dtype=tf.float32)
        reward = tf.convert_to_tensor(reward, dtype=tf.float32)
        done = tf.convert_to_tensor(done, dtype=tf.float32)

        return state, action, reward, next_state, done

    def save_model(self):
        print(Fore.GREEN+'... saving (' + self.name +') models ...'+Fore.WHITE)
        
        self.move_actor.save_weights  ( self.models_path + self.name + '_move_actor.h5'  )
        self.kick_actor.save_weights  ( self.models_path + self.name + '_kick_actor.h5'  )
        self.critic.save_weights ( self.models_path + self.name + '_critic.h5' )
        
        self.target_move_actor.save_weights  ( self.models_path + self.name + '_move_target_actor.h5'  )
        self.target_kick_actor.save_weights  ( self.models_path + self.name + '_kick_target_actor.h5'  )
        self.target_critic.save_weights ( self.models_path + self.name + '_target_critic.h5' )
        
    def save_agent(self):
        data = pd.read_csv(agents_path + self.name + '_data.csv')
        
        data.name                = self.name
        data.epsilon             = self.epsilon
        data.eps_decay           = self.eps_decay
        data.eps_min             = self.eps_min
        data.max_mem             = self.max_mem
        data.batch_size          = self.batch_size
        data.learning_rate_alpha = self.learning_rate_alpha
        data.learning_rate_beta  = self.learning_rate_beta
        data.soft_update_rate    = self.soft_update_rate
        data.gamma               = self.gamma
        
        data.to_csv(agents_path + self.name + '_data.csv')

    def move_update_network_parameters(self):

        tau = self.soft_update_rate

        weights = []
        targets = self.target_move_actor.weights

        for i, weight in enumerate(self.move_actor.weights):
            weights.append(weight * tau + targets[i] * (1 - tau))
        self.target_move_actor.set_weights(weights)

        weights = []
        targets = self.target_critic.weights
        for i, weight in enumerate(self.critic.weights):
            weights.append(weight * tau + targets[i] * (1 - tau))
        self.target_critic.set_weights(weights)
    
    def kick_update_network_parameters(self, tau=None):
        if tau is None:
            tau = self.soft_update_rate

        weights = []
        targets = self.target_kick_actor.weights
        for i, weight in enumerate(self.kick_actor.weights):
            weights.append(weight * tau + targets[i] * (1 - tau))
        self.target_kick_actor.set_weights(weights)

        weights = []
        targets = self.target_critic.weights
        for i, weight in enumerate(self.critic.weights):
            weights.append(weight * tau + targets[i] * (1 - tau))
        self.target_critic.set_weights(weights)

    def move_learn(self):
        
        # get our data
        states , actions , rewards , states_ , done =             self.read_move_memory_file()

        with tf.GradientTape() as tape:
            target_actions = self.target_move_actor(states_)
            critic_value_ = tf.squeeze(self.target_critic(
                                states_, target_actions), 1)

            critic_value = tf.squeeze(self.critic(states, actions), 1)
            target = rewards + self.gamma * critic_value_*(1-done)
            critic_loss = keras.losses.MSE(target, critic_value)

        critic_network_gradient = tape.gradient(critic_loss,
                                                self.critic.trainable_variables)
        self.critic.optimizer.apply_gradients(zip(
            critic_network_gradient, self.critic.trainable_variables))

        with tf.GradientTape() as tape:
            new_policy_actions = self.move_actor(states)
            actor_loss = -self.critic(states, new_policy_actions)
            actor_loss = tf.math.reduce_mean(actor_loss)

        actor_network_gradient = tape.gradient(actor_loss,
                                               self.move_actor.trainable_variables)
        self.move_actor.optimizer.apply_gradients(zip(
            actor_network_gradient, self.move_actor.trainable_variables))

        self.move_update_network_parameters()

    def kick_learn(self):
        
        # get our data
        states , actions , rewards , states_ , done =             self.read_kick_memory_file()
        
        with tf.GradientTape() as tape:
            target_actions = self.target_kick_actor(states_)
            critic_value_ = tf.squeeze(self.target_critic(
                                states_, target_actions), 1)

            critic_value = tf.squeeze(self.critic(states, actions), 1)
            target = rewards + self.gamma * critic_value_*(1-done)
            critic_loss = keras.losses.MSE(target, critic_value)

        critic_network_gradient = tape.gradient(critic_loss,
                                                self.critic.trainable_variables)
        self.critic.optimizer.apply_gradients(zip(
            critic_network_gradient, self.critic.trainable_variables))

        with tf.GradientTape() as tape:
            new_policy_actions = self.kick_actor(states)
            actor_loss = -self.critic(states, new_policy_actions)
            actor_loss = tf.math.reduce_mean(actor_loss)

        actor_network_gradient = tape.gradient(actor_loss,
                                               self.kick_actor.trainable_variables)
        self.kick_actor.optimizer.apply_gradients(zip(
            actor_network_gradient, self.kick_actor.trainable_variables))

        self.kick_update_network_parameters()

    def learn(self):

        df = pd.read_csv(self.move_memory_path + "states/"      + self.name + "_states.csv"      ,header = None)
        if(df.shape[1] > self.batch_size * 10):
            self.move_learn()

        #df = pd.read_csv(self.kick_memory_path + "states/"      + self.name + "_states.csv"      ,header = None)
        #if(df.shape[1] > self.batch_size * 10):
            #self.kick_learn()
