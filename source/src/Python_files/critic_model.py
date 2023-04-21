#Socer Simulation Critic Class 
#{ 
#   programmed BY Amir mahdy yari
#   Email : yarya524@gmail.com
#   Telegram : @a_m_yari
#}

import tensorflow as tf
import tensorflow.keras as keras
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Dense

class CriticNetwork(keras.Model):
    def __init__(self):
        super(CriticNetwork, self).__init__()
        
        #hidden layers
        self.fc1     = Dense(400, activation='relu')
        self.fc2     = Dense(300, activation='relu')
        self.fc3     = Dense(400, activation='relu')
        self.fc4     = Dense(200, activation='relu')
        #output
        self.q_value = Dense(1, activation=None)

    def call(self, state, action):
        inputs = tf.concat([state ,action ] , axis = 1)
        
        action_value = self.fc1(inputs)
        action_value = self.fc2(action_value)
        action_value = self.fc3(action_value)
        action_value = self.fc4(action_value)

        action_value = self.q_value(action_value)

        return action_value