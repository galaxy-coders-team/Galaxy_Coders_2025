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

#include "state.h"

#include <string.h>
#include <vector>
#include "../bhv_basic_tackle.h"

void state::get_state(rcsc::PlayerAgent* agent)
{

    goal_r = Vector2D(-52.5 , 20) ;
    goal_c = Vector2D(-52.5 ,  0) ;
    goal_l = Vector2D(-52.5 ,-20) ;


    const rcsc::WorldModel & wm = agent->world();

    self_pos = wm.self().pos();

    ball_pos = wm.ball().pos() - self_pos;

    Angle_from_ball = wm.self().angleFromBall().degree() ;
    Angle_from_goal_r = (goal_r - self_pos).th().degree();
    Angle_from_goal_c = (goal_c - self_pos).th().degree();
    Angle_from_goal_l = (goal_l - self_pos).th().degree();

    //double dist_from_goal_r;
    //double dist_from_goal_c;
    //double dist_from_goal_l;


    dist_from_ball = wm.self().distFromBall();


    const PlayerObject * opp_player = wm.getOpponentGoalie();
    if(opp_player != NULL)
    {
        //opp_pos = opp_player->pos() - self_pos;
    }

    normalize_state();
}

void state::normalize_state()
{
    self_pos.x /= 52.5;
    self_pos.y /= 34;

    goal_pos.x /= 52.5;
    goal_pos.y /= 34;

    ball_pos.x /= 52.5;
    ball_pos.y /= 34;

    Angle_from_ball /= 180;

    dist_from_ball /= 125.09;

    //opp_pos.x /= 52.5;
    //opp_pos.y /= 34;
}
void state::empty_state()
{

    goal_pos.x = 0;
    goal_pos.y = 0;

    ball_pos.x = 0;
    ball_pos.y = 0;

    Angle_from_ball = 0 ;


    dist_from_ball = 0;


    //opp_pos.assign(0,0);

}

void state::operator=(state state)
{

    self_pos = state.self_pos;
    goal_pos = state.goal_pos;
    ball_pos = state.ball_pos;

    Angle_from_ball = state.Angle_from_ball ;


    dist_from_ball = state.dist_from_ball;


    //opp_pos = state.opp_pos;
}

std::string state::to_string()
{

    std::string data = "";

    data += std::to_string(self_pos.x       * 100) ; data += "," ;
    data += std::to_string(self_pos.y       * 100) ; data += "," ;
    data += std::to_string(ball_pos.x       * 100) ; data += "," ;
    data += std::to_string(ball_pos.y       * 100) ; data += "," ;
    data += std::to_string(goal_pos.x       * 100) ; data += "," ;
    data += std::to_string(goal_pos.y       * 100) ; data += "," ;
    data += std::to_string(Angle_from_ball  * 100) ; data += "," ;
    data += std::to_string(dist_from_ball   * 100) ; data += "," ;
    data += std::to_string(opp_pos.x * 100); data += "," ;
    data += std::to_string(opp_pos.y * 100); data += ",";

    return data;
}

std::vector<double> state::to_array()
{
    std::vector<double> array
    {
        (double)ball_pos.x     , (double)ball_pos.y ,
        (double)goal_pos.x     , (double)goal_pos.y ,
        (double)Angle_from_ball,
        (double)dist_from_ball
    };

    state_dims = 17;

    return array;
}
