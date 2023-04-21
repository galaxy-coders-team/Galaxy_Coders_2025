/*
 * Copyright 2022 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef REWARD_CALCULATOR_H
#define REWARD_CALCULATOR_H

#include"state.h"
#include"action.h"

/**
 * @todo
 * coded by Amir mahdy yari and Shayan Yari and Pouyan Sheikhzadeh
 * email : yarya524@gmail.com
 * telegram : @a_m_yari
 */
class reward_calculator
{   
    bool isGoaler = false; // goaler reward method is different so we use this parametr to see wich method we have to use
    
    double kick_reward = 10;
    
    double ball_distance_change_factor = 1;
    double goal_distance_change_factor = 1;
    
    
    double ball_distance_change(state State,state New_state);
    double goal_distance_change(state State,state New_state);
    double is_kick(rcsc::PlayerAgent * agent,action action);
    double distanc_from_playgrand_edge(state New_state);
  
public:
    double goal_score = 100;
     
    void get_params(bool IsGoaler);  // set the reward_calculator prams
    double reward(rcsc::PlayerAgent * agent , state State,action Actio,state New_state); // calculat reward white agent_state and new_state
    
};

#endif // REWARD_CALCULATOR_H
