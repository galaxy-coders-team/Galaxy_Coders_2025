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

    Vector2D goal_r = Vector2D(-52.5 , 20);
    Vector2D goal_c = Vector2D(-52.5 ,  0);
    Vector2D goal_l = Vector2D(-52.5 ,-20);


    const rcsc::WorldModel & wm = agent->world();

    self_pos = wm.self().pos();

    ball_pos = wm.ball().pos() ;

    Angle_from_ball = wm.self().angleFromBall().degree() ;
    Angle_from_goal_r = (goal_r - self_pos).th().degree();
    Angle_from_goal_c = (goal_c - self_pos).th().degree();
    Angle_from_goal_l = (goal_l - self_pos).th().degree();

    dist_from_goal_r = self_pos.dist(goal_r);
    dist_from_goal_c = self_pos.dist(goal_c);
    dist_from_goal_l = self_pos.dist(goal_l);


    dist_from_ball = wm.self().distFromBall();

    normalize_state();
}

void state::normalize_state()
{
    Angle_from_goal_r /= 180;
    Angle_from_goal_c /= 180;
    Angle_from_goal_l /= 180;
    dist_from_goal_r /=109.76;
    dist_from_goal_c /=109.76;
    dist_from_goal_l /=109.76;

    self_pos.x /= 52.5;
    self_pos.y /= 34;


    ball_pos.x /= 52.5;
    ball_pos.y /= 34;

    Angle_from_ball /= 180;

    dist_from_ball /= 125.09;

}
void state::empty_state()
{
    Angle_from_goal_r  = 0;
    Angle_from_goal_c  = 0;
    Angle_from_goal_l  = 0;
    dist_from_goal_r = 0;
    dist_from_goal_c = 0;
    dist_from_goal_l = 0;


    ball_pos.x = 0;
    ball_pos.y = 0;

    Angle_from_ball = 0 ;


    dist_from_ball = 0;


}

void state::operator=(state state)
{

    self_pos = state.self_pos;
    ball_pos = state.ball_pos;

    Angle_from_ball = state.Angle_from_ball ;


    dist_from_ball = state.dist_from_ball;


}

std::string state::to_string()
{

    std::string data = "";

    data += std::to_string(ball_pos.x       * 100) ; data += "," ;
    data += std::to_string(ball_pos.y       * 100) ; data += "," ;
    data += std::to_string(Angle_from_ball  * 100) ; data += "," ;
    data += std::to_string(dist_from_ball   * 100) ; data += "," ;
    data += std::to_string(dist_from_goal_l      * 100) ; data += "," ;
    data += std::to_string(dist_from_goal_c      * 100) ; data += "," ;
    data += std::to_string(dist_from_goal_r      * 100) ; data += "," ;
    data += std::to_string(Angle_from_goal_r      * 100) ; data += "," ;
    data += std::to_string(Angle_from_goal_c      * 100) ; data += "," ;
    data += std::to_string(Angle_from_goal_l      * 100) ; data += "," ;

    return data;
}

std::vector<double> state::to_array()
{
    std::vector<double> array
    {
        (double)ball_pos.x     , (double)ball_pos.y ,
        (double)Angle_from_ball ,
        (double)dist_from_ball , (double)dist_from_goal_r ,
        (double)dist_from_goal_c , (double)dist_from_goal_l ,
        (double)Angle_from_goal_r , (double)Angle_from_goal_c ,
        (double)Angle_from_goal_l

    };

    state_dims = 17;

    return array;
}

void goalie_state::get_state(rcsc::PlayerAgent* agent)
{
    const WorldModel &wm = agent->world();
    Vector2D middle_goal_pos(52.5 , 0);

    self_pos = wm.self().pos();
    opp_pos = wm.getOpponentNearestToSelf(5,false)->pos();
    ball_pos = wm.ball().pos();

    Angle_from_self = wm.self().angleFromSelf().degree();
    Angle_from_ball = wm.self().angleFromBall().degree();
    
    speed = wm.self().speed();

    opp_dist_from_ball = ball_pos.dist(opp_pos);
    dist_from_ball = ball_pos.dist(self_pos);
    dist_from_center_goal = self_pos.dist(middle_goal_pos);

    normalize_state();
}

void goalie_state::normalize_state()
{
    self_pos.x /= 52.5;
    self_pos.y /= 34;

    opp_pos.x /= 52.5;
    opp_pos.y /= 34;

    ball_pos.x /= 52.5;
    ball_pos.y /= 34;

    Angle_from_ball /= 180;
    Angle_from_self /= 180;

    dist_from_ball /= 125.09;
    opp_dist_from_ball /= 125.09;
    dist_from_center_goal /= 110.36;
}
void goalie_state::empty_state()
{

    self_pos.x = 0;
    self_pos.y = 0;

    ball_pos.x = 0;
    ball_pos.y = 0;

    opp_pos.x = 0;
    opp_pos.y = 0;

    ball_pos.x = 0;
    ball_pos.y = 0;

    Angle_from_ball = 0;
    Angle_from_self = 0;

    dist_from_ball = 0;
    opp_dist_from_ball = 0;
    dist_from_center_goal = 0;
}

void goalie_state::operator =(goalie_state state)
{
    self_pos = state.self_pos;
    ball_pos = state.ball_pos;

    Angle_from_self = state.Angle_from_self;
    Angle_from_ball = state.Angle_from_ball ;

    speed = state.speed;

    opp_dist_from_ball = state.opp_dist_from_ball;
    dist_from_ball = state.dist_from_ball;
    dist_from_center_goal = state.dist_from_center_goal;

}

std::string goalie_state::to_string()
{

    std::string data = "";

    data += std::to_string(self_pos.x            * 100) ; data += "," ;
    data += std::to_string(self_pos.y            * 100) ; data += "," ;
    data += std::to_string(ball_pos.x            * 100) ; data += "," ;
    data += std::to_string(ball_pos.y            * 100) ; data += "," ;
    data += std::to_string(opp_pos.x             * 100) ; data += "," ;
    data += std::to_string(opp_pos.y             * 100) ; data += "," ;
    data += std::to_string(Angle_from_self       * 100) ; data += "," ;
    data += std::to_string(Angle_from_ball       * 100) ; data += "," ;
    data += std::to_string(speed                 * 100) ; data += "," ;
    data += std::to_string(opp_dist_from_ball    * 100) ; data += "," ;
    data += std::to_string(dist_from_ball        * 100) ; data += "," ;
    data += std::to_string(dist_from_center_goal * 100) ; data += "," ;

    return data;
}

std::vector<double> goalie_state::to_array()
{
    std::vector<double> array
    {
        (double)self_pos.x ,(double)self_pos.y ,
        (double)ball_pos.x ,(double)ball_pos.y ,
        (double)opp_pos.x ,(double)opp_pos.y ,
        (double)Angle_from_self ,(double)Angle_from_ball ,
        (double)speed,
        (double)opp_dist_from_ball ,(double)dist_from_ball, (double)dist_from_center_goal

    };

    state_dims = 12;

    return array;
}


