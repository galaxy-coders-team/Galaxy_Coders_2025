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

void reward_calculator::get_params(bool IsGoaler)
{
    isGoaler = IsGoaler;
}

double reward_calculator::reward(rcsc::PlayerAgent * agent , state State , action Action , state New_state)
{
    double Reward = 0;
    
    // reward main method
    Reward += Action.action_type == 0 && State.is_kickable == false ? -200 : 0;
    Reward += Action.action_type == 1 && State.is_tacklable == false ? -200 : 0;

    Reward += ball_distance_change(State,New_state);
    Reward += goal_distance_change(State,New_state);
    Reward += is_kick(agent,Action);
    Reward -= distanc_from_playgrand_edge(New_state);
        
    return Reward;
}

double reward_calculator::ball_distance_change(state State, state New_state)
{
    double reward = New_state.dist_from_ball - State.dist_from_ball;
    reward *= ball_distance_change_factor;
    return reward;
}

double reward_calculator::goal_distance_change(state State, state New_state)
{
    double reward = New_state.dist_from_goal - State.dist_from_goal;
    reward *= goal_distance_change_factor;
    return reward;
}

double reward_calculator::is_kick(rcsc::PlayerAgent * agent,action action)
{
    double reward = 0 ;
    
    if(action.action_type == 0 && bhv_basic_ai().last_kicker_unum == agent->world().self().unum())
    {
        reward = kick_reward;
    }
    
    return reward;
}

double reward_calculator::distanc_from_playgrand_edge(state New_state)
{
    double output = 0;
    
    if(std::abs(New_state.self_pos.x) > 1)
    {
        output += (std::abs(New_state.self_pos.x * 52.5) - 52.5) * 2;
    }
    if(std::abs(New_state.self_pos.y) > 1)
    {
        output += (std::abs(New_state.self_pos.y * 34) - 34) * 2;
    }
    
    return output;
}








