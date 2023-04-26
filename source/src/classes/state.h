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

#ifndef STATE_H
#define STATE_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_agent.h>

#include<vector>

using namespace rcsc;

/**
 * @todo write docs
 */
class state
{
public:
    
    uint32_t state_dims = 17;

    Vector2D self_pos;
    Vector2D ball_pos;

    double Angle_from_goal_r;
    double Angle_from_goal_c;
    double Angle_from_goal_l;
    double Angle_from_ball;

    double dist_from_ball;
    double dist_from_goal_r;
    double dist_from_goal_c;
    double dist_from_goal_l;

    
    
    void get_state(rcsc::PlayerAgent * agent);

    void empty_state();
    
    void operator =(state s);
   
    void normalize_state();
   
   std::vector<double> to_array();
   
   std::string to_string();
    
    
};

#endif // STATE_H
