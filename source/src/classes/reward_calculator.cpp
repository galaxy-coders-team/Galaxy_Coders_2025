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
    if(isGoaler == false)
    {
        // reward main method
        Reward += ball_distance_change(State,New_state);
        Reward += goal_distance_change(State,New_state);
        Reward -= distanc_from_playgrand_edge(New_state);
    }
    else
    {
        if
            ((agent->world().gameMode().type() == GameMode::PenaltyMiss_ && agent->world().gameMode().side() != agent->world().ourSide()) )
        {
            Reward = 1;
        }
        else if(
            (agent->world().gameMode().type() == GameMode::PenaltyScore_ &&
                 agent->world().gameMode().side() != agent->world().ourSide(
            )))
            {
                Reward = -1;
            }
    }
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
    double reward = New_state.dist_from_goal_c - State.dist_from_goal_c;
    reward *= goal_distance_change_factor;
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








