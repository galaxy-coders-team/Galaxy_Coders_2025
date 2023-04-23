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
  
public:
    
    double goal = 5;
    double player_goal = 3;
    double scored_goal = -5;
    double equal_game = -2;
   
    double reward(rcsc::PlayerAgent * agent , state State,action Actio,state New_state); // calculat reward white agent_state and new_state
    
};

#endif // REWARD_CALCULATOR_H
