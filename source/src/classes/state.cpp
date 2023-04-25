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
    Vector2D middle_goal_pos(52.5 , 0);


    const rcsc::WorldModel & wm = agent->world();

    self_pos = wm.self().pos();

    goal_pos = middle_goal_pos - self_pos;
    ball_pos = wm.ball().pos() - self_pos;

    Angle_from_goal = (middle_goal_pos - wm.self().pos()).th().degree();
    Angle_from_ball = wm.self().angleFromBall().degree() ;

    speed = wm.self().speed();
    stamina = wm.self().stamina();
    stamina_capacity = wm.self().staminaCapacity();

    dist_from_ball = wm.self().distFromBall();
    dist_from_goal = wm.self().pos().dist(middle_goal_pos);


    is_tacklable = Bhv_BasicTackle( 0.8, 80.0 ).execute(agent);

    bool kickable =wm.self().isKickable();

    if ( wm.existKickableTeammate()
        && wm.teammatesFromBall().front()->distFromBall() < wm.ball().distFromSelf() )
    {
        kickable = false;

    }

    is_kickable = kickable;

    exist_kickable_teammate = wm.existKickableTeammate();

    int index = 0;

    for(int i = 1; i<=11 ; i++)
    {
        const AbstractPlayerObject * tm_player  = wm.ourPlayer(i);
        const AbstractPlayerObject * opp_player = wm.theirPlayer(i);

        tm_pos[index].assign(-52.5,-34);
        opp_pos[i - 1].assign(-52.5,-34);

        if(tm_player != NULL && tm_player->unum() != wm.self().unum())
        {
               tm_pos[index] = tm_player->pos() - self_pos;
        }

        if(opp_player != NULL)
        {
            opp_pos[i - 1] = opp_player->pos() - self_pos;
        }
        if(index < 9)
        {
            index +=1;
        }


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
    Angle_from_goal /= 180;

    //speed is alredy normalized
    stamina /= 8000;
    stamina_capacity /= 130600;

    dist_from_ball /= 125.09;
    dist_from_goal /= 110.36;

    for(int i = 0; i< 11 ; i++)
    {
        if(i < 10)
        {
          tm_pos[i].x /= 52.5;
          tm_pos[i].y /= 34;
        }
        opp_pos[i].x /= 52.5;
        opp_pos[i].y /= 34;
    }
}
void state::empty_state()
{

    goal_pos.x = 0;
    goal_pos.y = 0;

    ball_pos.x = 0;
    ball_pos.y = 0;

    Angle_from_goal = 0;
    Angle_from_ball = 0 ;

    speed = 0;
    stamina = 0;
    stamina_capacity = 0;

    dist_from_ball = 0;
    dist_from_goal = 0;

    is_tacklable = false;
    is_kickable = false;
    exist_kickable_teammate = false;


    for(int i = 0; i< 11 ; i++)
    {
        if(i < 10)
        {
          tm_pos[i].assign(0,0);
        }
        opp_pos[i].assign(0,0);

    }
}

void state::operator=(state state)
{

    self_pos = state.self_pos;
    goal_pos = state.goal_pos;
    ball_pos = state.ball_pos;

    Angle_from_goal = state.Angle_from_goal;
    Angle_from_ball = state.Angle_from_ball ;

    speed = state.speed;
    stamina = state.stamina;
    stamina_capacity = state.stamina_capacity;

    dist_from_ball = state.dist_from_ball;
    dist_from_goal = state.dist_from_goal;

    is_tacklable = state.is_tacklable;
    is_kickable = state.is_kickable;
    exist_kickable_teammate = state.exist_kickable_teammate;


    for(int i = 0; i< 11 ; i++)
    {
        if(i < 10)
        {
          tm_pos[i] = state.tm_pos[i];
        }
        opp_pos[i] = state.opp_pos[i];

    }
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
    data += std::to_string(Angle_from_goal  * 100) ; data += "," ;
    data += std::to_string(Angle_from_ball  * 100) ; data += "," ;
    data += std::to_string(speed            * 100) ; data += "," ;
    data += std::to_string(stamina          * 100) ; data += "," ;
    data += std::to_string(stamina_capacity * 100) ; data += "," ;
    data += std::to_string(dist_from_ball   * 100) ; data += "," ;
    data += std::to_string(dist_from_goal   * 100) ; data += "," ;
    data += std::to_string(is_tacklable     * 100) ; data += "," ;
    data += std::to_string(is_kickable      * 100) ; data += "," ;
    data += std::to_string(exist_kickable_teammate * 100) ; data += ",";

    for(int i = 0; i< 10 ; i++)
    {
        data += std::to_string(tm_pos[i].x * 100); data += "," ; data += std::to_string(tm_pos[i].y * 100); data += ",";
    }
    for(int i = 0; i< 10 ; i++)
    {
        data += std::to_string(opp_pos[i].x * 100); data += "," ; data += std::to_string(opp_pos[i].y * 100); data += ",";
    }

    data += std::to_string(opp_pos[10].x * 100); data += "," ; data += std::to_string(opp_pos[10].y * 100);

    return data;
}

std::vector<double> state::to_array()
{
    std::vector<double> array
    {
        (double)self_pos.x ,(double)self_pos.y ,
        (double)ball_pos.x ,(double)ball_pos.y ,
        (double)goal_pos.x ,(double)goal_pos.y ,
        (double)Angle_from_ball ,(double)Angle_from_goal ,
        (double)speed,
        (double)stamina ,(double)stamina_capacity ,
        (double)dist_from_ball ,(double)dist_from_goal ,
        (double)is_tacklable ,(double)is_kickable , (double)exist_kickable_teammate,

        (double)tm_pos[0].x  ,(double)tm_pos[0].y ,
        (double)tm_pos[1].x  ,(double)tm_pos[1].y ,
        (double)tm_pos[2].x  ,(double)tm_pos[2].y ,
        (double)tm_pos[3].x  ,(double)tm_pos[3].y ,
        (double)tm_pos[4].x  ,(double)tm_pos[4].y ,
        (double)tm_pos[5].x  ,(double)tm_pos[5].y ,
        (double)tm_pos[6].x  ,(double)tm_pos[6].y ,
        (double)tm_pos[7].x  ,(double)tm_pos[7].y ,
        (double)tm_pos[8].x  ,(double)tm_pos[8].y ,
        (double)tm_pos[9].x  ,(double)tm_pos[9].y ,

        (double)opp_pos[0].x  ,(double)opp_pos[0].y ,
        (double)opp_pos[1].x  ,(double)opp_pos[1].y ,
        (double)opp_pos[2].x  ,(double)opp_pos[2].y ,
        (double)opp_pos[3].x  ,(double)opp_pos[3].y ,
        (double)opp_pos[4].x  ,(double)opp_pos[4].y ,
        (double)opp_pos[5].x  ,(double)opp_pos[5].y ,
        (double)opp_pos[6].x  ,(double)opp_pos[6].y ,
        (double)opp_pos[7].x  ,(double)opp_pos[7].y ,
        (double)opp_pos[8].x  ,(double)opp_pos[8].y ,
        (double)opp_pos[9].x  ,(double)opp_pos[9].y ,
        (double)opp_pos[10].x ,(double)opp_pos[10].y ,

    };

    state_dims = 58;

    return array;
}
