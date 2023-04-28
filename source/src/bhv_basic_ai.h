/*
 * Copyright 2023 <copyright holder> <email>
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

#ifndef BHV_BASIC_AI_H
#define BHV_BASIC_AI_H


#include <rcsc/player/player_agent.h>
#include <rcsc/geom/vector_2d.h>

#include "classes/ai_agent.h"
#include "classes/state.h"
#include "classes/action.h"
#include "classes/galaxy_ann.h"

/**
 * @todo write docs
 */
class bhv_basic_ai : rcsc::SoccerBehavior
{
private:
    
    std::string path_l = "team_data_l/";
    std::string path_r = "team_data_r/";
    
    
    // 6 = Madyar
    // playrs data / unum          =    1         2         3        4          5          6         7        8           9         10        11
    std::string           name[11] = {"Shayan", "Poyan" ,"Diaco", "Hesam" , "Atredin" , "Madyar" ,"Radmehr", "Kian"  ,  "Amir" , "Piroz" , "Iran" };
    double             epsilon[11] = {  0.5   ,   0.5   ,  0.5  ,   0.5   ,    0.5    ,   0.5    ,   0.5   ,   0.5   ,   0.5   ,   0.5   ,   0.5  };
    double           eps_decay[11] = {  0.98  ,   0.98  ,  0.98 ,   0.98  ,    0.98   ,   0.98   ,   0.98  ,   0.98  ,   0.98  ,   0.98  ,   0.98 };
    double             eps_min[11] = { 0.005  ,  0.005  , 0.005 ,  0.005  ,   0.005   ,  0.005   ,  0.005  ,  0.005  ,  0.005  ,  0.005  ,  0.005 };
    int    epsilon_decay_delay[11] = { 1000   ,  10000  , 10000 ,  10000  ,   10000   ,  10000   ,  10000  ,  10000  ,  10000  ,  10000  , 10000  };
    int          actions_taken[11] = {   0    ,    0    ,   0   ,    0    ,     0     ,    0     ,    0    ,    0    ,    0    ,    0    ,   0    };
    int                max_mem[11] = {  3000  ,  6000   , 6000  ,  6000   ,   6000    ,  6000    ,  6000   ,  6000   ,  6000   ,  6000   ,  6000  };
    int             batch_size[11] = {  64    ,  128    , 128   ,  128    ,   64      ,  128     ,  128    ,  128    ,  128    ,  128    ,  128   };
    double learning_rate_alpha[11] = { 0.001  ,  0.001  , 0.001 ,  0.001  ,   0.002   ,  0.001   ,  0.001  ,  0.001  ,  0.001  ,  0.001  ,  0.001 };
    double  learning_rate_beta[11] = { 0.002  ,  0.002  , 0.002 ,  0.002  ,   0.005   ,  0.002   ,  0.002  ,  0.002  ,  0.002  ,  0.002  ,  0.002 };
    double               gamma[11] = { 0.98   ,  0.98   , 0.98  ,  0.98   ,   0.98    ,  0.98    ,  0.98   ,  0.98   ,  0.98   ,  0.98   ,  0.98  };
    double    soft_update_rate[11] = { 0.005  ,  0.005  , 0.005 ,  0.005  ,   0.005   ,  0.005   ,  0.005  ,  0.005  ,  0.005  ,  0.005  ,  0.005 };
    
public:
    
    inline static ai_agent agents[12];
    inline static int unum ;
    
    //typedef
    
    bhv_basic_ai(){}
    
   inline static int last_kick_time = -1;
   inline static int last_kicker_unum = -1;
    
      
    bool execute(rcsc::PlayerAgent * agent);
    rcsc::Vector2D goalie_execute(rcsc::PlayerAgent * agent);
    double penalty_execute(rcsc::PlayerAgent * agent);
    
    bool do_action(rcsc::PlayerAgent* agent , action action,int Last_kick_time);
    
    void load_agent(rcsc::PlayerAgent * agent , 
                                     bool Learn_mode = true , bool read_from_file = true);
    
    void save_agent(rcsc::PlayerAgent * agent );

    ai_agent agent_data(rcsc::PlayerAgent * agent, std::string Path ,bool Learn_mode, bool Read_from_file);
    
    void goal(rcsc::PlayerAgent* agent );
    
    
};

#endif // BHV_BASIC_AI_H
