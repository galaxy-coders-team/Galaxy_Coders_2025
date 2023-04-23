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

#include "bhv_basic_ai.h"

#include "classes/ai_agent.h"
#include "classes/state.h"
#include "classes/action.h"

#include "classes/galaxy_ann.h"
#include "classes/matrix.h"

#include "sample_player.h"
#include "bhv_basic_tackle.h"

#include <stdio.h>
#include <stdlib.h>


bool bhv_basic_ai::execute(rcsc::PlayerAgent* agent )
{
    
    if(Bhv_BasicTackle(80,0.80).execute(agent))
    {
        return true;
    }
    
    const WorldModel & wm = agent->world();

    if(wm.self().unum() != 5)
    {
        const AbstractPlayerObject * opp_player = wm.theirPlayer(5);

        Vector2D d = opp_player->pos();
        std::cout<<"\n"<<d.x<<"\n";
        //return false;
    }
    state state;
    state.get_state(agent);
    
    if(std::abs(state.Angle_from_goal) > 20)
    {
        
    }
    
    
    bool done = wm.gameMode().type() == rcsc::GameMode::PlayOn ? false : true ;
    
    if( agents[wm.self().unum()].get_mem_counter() != 0)
    {
        agents[wm.self().unum()].save_next_state(agent,state,done);
    }
    
    if(done) {return true;}
    
    action action;
    action = agents[wm.self().unum()].take_action(state);
    
    do_action(agent , action ,agents[wm.self().unum()].get_mem_counter());
    
    agents[wm.self().unum()].save_experience(state,action);
    
    if(wm.existKickableOpponent())
    {
        last_kick_time = -1;
        last_kicker_unum = -1;
    }
    
    return true;
}

void bhv_basic_ai::load_agent(rcsc::PlayerAgent* agent , bool Learn_mode , bool Read_from_file)
{
    const WorldModel & wm = agent->world();
    
    if(agents[wm.self().unum()].is_loaded){ return; }
    
    std::string Name = name[wm.self().unum() - 1] , path = "team_data/";

    if(wm.self().unum() ==5)
    {
        SamplePlayer().s.assign(agents[wm.self().unum()].get_random_num(-50,-10,false),agents[wm.self().unum()].get_random_num(-30,30,false));
    }

    if(Learn_mode)
    {
        if(wm.ourSide() == rcsc::SideID::LEFT)
        {
            path = path_l;
        }
        else { path = path_r; }
    }
    unum = load_unum(path);

    if(Read_from_file)
    {
        if(agents[wm.self().unum()].load_agent(agent , path , Name , Learn_mode) == false)
        {
            agents[wm.self().unum()] = agent_data(agent,path,Learn_mode,false);
        }
    }
    else
    {
        agents[wm.self().unum()] = agent_data(agent,path ,Learn_mode, Read_from_file);
    }
    
    agents[wm.self().unum()].learn_mode = Learn_mode;
    
    
    
    std::cout<<"\n agent {" << wm.self().unum() << "} " << Name <<"\n";
    
}


void bhv_basic_ai::save_agent(rcsc::PlayerAgent* agent)
{
    const WorldModel & wm = agent->world();
    
    if(SamplePlayer().learn_mode == false){ return; }
    
    if(agents[wm.self().unum()].is_saved){ return; }

    std::cout<<"***** rewards mean : "<<agents[wm.self().unum()].rewards_mean<<"\n";

    agents[wm.self().unum()].save_agent();
    agents[wm.self().unum()].save_rewards_mean();
    

    
}


bool bhv_basic_ai::do_action(rcsc::PlayerAgent* agent , action action , int Last_kick_time)
{
    
    const WorldModel & wm = agent->world();
    
    double direction = action.direction * 180;
    double power = action.power * 100;
    
    if(action.action_type == 0) // kick
    {
        if(agent->doKick(power,direction))
        {
            last_kicker_unum = wm.self().unum();
            last_kick_time = Last_kick_time;
            return true;
        }
    }
    else //Move
    {
        return agent->doDash(power,direction);
    }
    
    
    return false;
}


void bhv_basic_ai::goal(rcsc::PlayerAgent* agent)
{
    if(last_kicker_unum != -1 && last_kick_time != -1)
    {
        if(agent->world().self().unum() == last_kicker_unum)
        { agents[agent->world().self().unum()].add_goal_reward(last_kick_time);}
    }
}


ai_agent bhv_basic_ai::agent_data(rcsc::PlayerAgent* agent , std::string Path ,bool Learn_mode, bool Read_from_file)
{
    ai_agent ai_agent ;
    
    const WorldModel & wm = agent->world();
    int unum = wm.self().unum() -1;

    ai_agent.get_params( name[unum] , agent, Path ,
                           epsilon[unum], eps_decay[unum], eps_min[unum],
                           epsilon_decay_delay[unum],actions_taken[unum],
                           max_mem[unum], batch_size[unum], 
                           learning_rate_alpha[unum] , learning_rate_beta[unum] ,
                           soft_update_rate[unum] , gamma[unum], Learn_mode , Read_from_file);
    
    return ai_agent;
    
}

int bhv_basic_ai::load_unum(std::string path)
{
    std::ifstream file;
    file.open(path + "random_num.csv"); // open our file

    if(file.fail())
    { return false;}

    std::string dummy; // dummy string to hold our data for exchanging data
    std::getline(file , dummy); // this line is just the header line

    std::getline(file , dummy );
    int n = std::stoi(dummy);
    file.close();
    return n;
}

