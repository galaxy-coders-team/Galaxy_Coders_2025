#Socer Simulation Actor Class 
#{ 
#   programmed BY Amir mahdy yari
#   Email : yarya524@gmail.com
#   Telegram : @a_m_yari
#}

import tensorflow as tf
import tensorflow.keras as keras
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Dense

import numpy as np

class ActorNetwork(keras.Model):
    def __init__(self):
        super(ActorNetwork,self).__init__()
        
        #base model 
        self.base_1 = Dense(300, activation="relu")
        self.base_2 = Dense(400, activation="relu")
        self.base_3 = Dense(300, activation="relu")
        self.base_4 = Dense(200, activation="relu")

        # output
        self.direction   = Dense(1, activation="tanh")     # otp=[-1,1]
        self.power       = Dense(1, activation="sigmoid")  # otp=[0,1]
        
    def call(self, inputs):
        
        # pass the input from base model
        x = self.base_1(inputs)
        x = self.base_2(x)
        x = self.base_3(x)
        x = self.base_4(x)

        # get outputs white base value
        direction = self.direction(x)
        power = self.power(x)

        output = tf.concat([direction,power], axis=1)

        return output # action_type , direction , power
