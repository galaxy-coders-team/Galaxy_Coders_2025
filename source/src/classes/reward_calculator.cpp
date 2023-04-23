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

#include "reward_calculator.h"
#include "state.h"
#include "../bhv_basic_ai.h"


double reward_calculator::reward(rcsc::PlayerAgent * agent , state State , action Action , state New_state)
{
    double Reward = 0;
       
    if(New_state.game_mode == 1)
    {
        Reward = goal;
    }
    else if(New_state.game_mode == 0)
    {
        Reward = equal_game;
    }
    else // game_mode == -1
    {
        Reward = scored_goal;
    }
    
    return Reward;
}









