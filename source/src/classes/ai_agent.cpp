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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ai_agent.h"

#include <cstdlib>
#include <time.h>
#include <algorithm>

#include"replay_bufer.h"
#include"state.h"

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_agent.h>

#include<fstream>
#include<random>

//#include "onnxruntime/include/onnxruntime_cxx_api.h"
#include <assert.h>
#include <array>
#include <iostream>


#include "classes/galaxy_ann.h"


void ai_agent::get_params(std::string Name, rcsc::PlayerAgent* agent, std::string Path,
                          double Starting_epsilon, double Eps_decay, double Eps_min,
                          int Epsilon_decay_delay,int Actions_taken ,
                          int Max_mem, int Batch_size, 
                          double Learning_rate_alpha , double Learning_rate_beta ,
                          double Soft_update_rate , double Gamma, bool Learn_mode , bool Use_model)
{
   
    name = Name ; //agent name (to open ann files)
    path = Path; // agent main file adress
    
    //agent epsilon params
    epsilon = Starting_epsilon; 
    eps_decay = Eps_decay; 
    eps_min = Eps_min; 
    epsilon_decay_delay = Epsilon_decay_delay;
    actions_taken = Actions_taken;
    
    
    // memory paraams
    max_mem = Max_mem;    

    batch_size = Batch_size; 
    
    // network learning paraams
    learning_rate_alpha = Learning_rate_alpha; 
    learning_rate_beta = Learning_rate_beta; 
    
    // learning paraams
    soft_update_rate = Soft_update_rate; 
    gamma = Gamma; 
    learn_mode = Learn_mode; // true for training and false for matchs 
    
    // analysis params
    rewards_mean = 0;
    
    agent_memory.get_params(max_mem); // build agent replay bufer to save the game experience
    
    const rcsc::WorldModel & wm = agent->world(); // build world mode
    
    bool isGoaler = wm.self().unum() == wm.ourGoalieUnum() ? true : false ; // get our agent type to set the reward method
    reward.get_params(isGoaler); // set the reward params
    
    use_model = Use_model;
    
    if(use_model)
    {
        load_model(agent);
    }
    
    is_loaded = true;
    is_saved  = false;
    
    
}

void ai_agent::save_experience(state State, action Action)
{
    if(learn_mode == false) { return; }
    agent_memory.add_experience(State,Action);
}

void ai_agent::save_next_state(rcsc::PlayerAgent * agent , state Next_state, bool Done)
{
    if(learn_mode == false) { return; }
    
    state State = agent_memory.get_last_state();
    action Action = agent_memory.get_last_action();
    
    double Reward = reward.reward(agent,State,Action,Next_state);
    
    rewards_mean = new_rewards_mean(Reward , agent_memory.get_counter());
    
    agent_memory.add_next_state(Reward,Next_state,Done);
}

action ai_agent::take_action(state State)
{
    action action;
    
    double random_num = 0;
    
    if(learn_mode == true && use_model == true)
    {
        random_num = get_random_num(0,1,false); // generate random number between 0 and 1
    }
    
     int action_type = 1 ? 0 : 1;
     double direction = 0;
     double power = 0;
     
    if(random_num < epsilon) // creat random action
    {
       direction = get_random_num(-1 , 1 , false);
       power = get_random_num( 0 , 1 , false);
       
       action.get_action(direction,power,action_type);
    }
    else //take action using ann
    {
       std::vector<double> model_input = State.to_array();
    
        std::vector<double> output = model.predict(model_input);
    
        direction = output[0] ;
        power = output[1] ;
    }

    action.get_action(direction,power,action_type);
    
    if(learn_mode == true)
    {
        actions_taken += 1;
    
        if(actions_taken >= epsilon_decay_delay )
        {
            actions_taken = 0;
            epsilon *= eps_decay;
        
            if(epsilon < eps_min)
            {
                epsilon = eps_min;
            }
        }
    }
    
    return action;
}

void ai_agent::save_memory_file()
{
     if(learn_mode == false)
    { return; }
    
    std::vector<memory> data = agent_memory.memory_batch(batch_size);
    
    if(data.size() != batch_size)
    { return; }
    
    std::ofstream state_file( path + "memory/" + "states/" + name + "_states.csv"); //creat file
    std::ofstream action_file( path + "memory/"+ "actions/" + name + "_actions.csv"); //creat file
    std::ofstream reward_file( path + "memory/" + "rewards/" + name + "_rewards.csv"); //creat file
    std::ofstream next_state_file( path + "memory/" + "next_states/" + name + "_next_states.csv"); //creat file
    std::ofstream done_file( path + "memory/" + "done/" + name + "_done_memory.csv"); //creat file
    
    state_file.clear();
    action_file.clear();
    reward_file.clear();
    next_state_file.clear();
    done_file.clear();
    
    if(state_file.is_open() && action_file.is_open() 
          && reward_file.is_open() && next_state_file.is_open() && done_file.is_open())
    {
        state_file <<" 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,56,57\n" ;
        reward_file << "Rewards \n";
        action_file << "direction,power \n";
        next_state_file <<" 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,56,57\n" ;
        done_file<<"done \n";

        for(int i = 0 ; i < data.size() ; i++)
        {
            
            state_file<<data[i].mem_state.to_string() << "\n";
            
            action_file << data[i].mem_action.to_string() << "\n";
            
            reward_file << std::to_string(data[i].mem_reward * 100) << "\n";
            
            next_state_file<<data[i].mem_next_state.to_string() << "\n";
            
            done_file << data[i].mem_done << "\n";
            
        }
    
    
        if(!state_file.fail() && !action_file.fail() 
          && !reward_file.fail() && !next_state_file.fail() && !done_file.fail())
        {
            std::cout << "\n" << name << " Memory files saved succesfully in " << path << "\n"; 
        }
        state_file.close();
        action_file.close();
        reward_file.close();
        next_state_file.close();
        done_file.close();
    }
    
}

std::string ai_agent::to_string()
{
    std::string output = "";
    
    output += std::to_string(epsilon) + ",";
    output += std::to_string(eps_decay) + ",";
    output += std::to_string(eps_min) + ",";
    output += std::to_string(epsilon_decay_delay) + ",";
    output += std::to_string(actions_taken) + ",";
    output += std::to_string(max_mem) + ",";
    output += std::to_string(batch_size) + ",";
    output += std::to_string(learning_rate_alpha) + ",";
    output += std::to_string(learning_rate_beta) + ",";
    output += std::to_string(soft_update_rate) + ",";
    output += std::to_string(gamma) + ",";;
    
    return output;
}

void ai_agent::save_agent()
{
    if(learn_mode == false)
    { return; }
    
    std::ofstream file( path + "agents/" + name + "_data.csv"); //creat file
    if(file.is_open())
    {
       file.clear();

       file << "epsilon,eps_decay,eps_min,epsilon_decay_delay,actions_taken,max_mem,batch_size,learning_rate_alpha,learning_rate_beta,soft_update_rate,gamma, \n"; // csv headers 
       file << to_string();
       // now writting our data to the csv file
       
       if(!file.fail())
       {
           std::cout << name << " saved succesfully in " << path << std::endl; 
       }
       
       file.close();
       
       is_saved  =  true;
    }
}

bool ai_agent::load_agent(rcsc::PlayerAgent * agent , std::string Path, std::string Name , bool Learn_mode)
{
    std::ifstream file;
    file.open(Path + "agents/" + Name + "_data.csv"); // open our file
    
    if(file.fail())
    { return false;}

    path = Path;
    name = Name;
    
    std::string dummy; // dummy string to hold our data for exchanging data
    std::getline(file , dummy); // this line is just the header line
    
    std::getline(file , dummy , ',');
    epsilon = std::stod(dummy);
    
    std::getline(file , dummy , ',');
    eps_decay = std::stod(dummy);
    
    std::getline(file , dummy , ',');
    eps_min = std::stod(dummy);
    
    std::getline(file , dummy , ',');
    epsilon_decay_delay = std::stoi(dummy);
    
    std::getline(file , dummy , ',');
    actions_taken = std::stoi(dummy);
    
    std::getline(file , dummy , ',');
    max_mem = std::stoi(dummy);
    
    std::getline(file , dummy , ',');
    batch_size = std::stoi(dummy);
    
    std::getline(file , dummy , ',');
    learning_rate_alpha = std::stod(dummy);
    
    std::getline(file , dummy , ',');
    learning_rate_beta = std::stod(dummy);
    
    std::getline(file , dummy , ',');
    soft_update_rate = std::stod(dummy);
    
    std::getline(file , dummy , ',');
    gamma = std::stod(dummy);
    
    learn_mode = Learn_mode;
    
    file.close();
    
    agent_memory.get_params(max_mem); // build agent replay bufer to save the game experience
    
    const rcsc::WorldModel & wm = agent->world(); // build world mode
    
    bool isGoaler = wm.self().unum() == wm.ourGoalieUnum() ? true : false ; // get our agent type to set the reward method
    reward.get_params(isGoaler); // set the reward params
    
    load_model(agent);
    
    use_model = true;
    is_loaded = true;
    is_saved  = false;
    
    
    return true;
}


void ai_agent::load_model(rcsc::PlayerAgent * agent)
{
    state s;
    uint32_t input_dims = s.state_dims;
    std::vector<uint32_t> model_shape = { input_dims ,400,300,200};
    bool is_goaler = agent->world().self().unum() == agent->world().ourGoalieUnum();

    model.get_param(model_shape , path + "models_weights/" + name + "_" , is_goaler);
}


int ai_agent::get_mem_counter()
{
    return agent_memory.get_counter();
}

void ai_agent::add_goal_reward(int time)
{
    if(learn_mode == false) { return; }
    agent_memory.add_reward(time , reward.goal_score);
}


double ai_agent::get_random_num(int min_num, int max_num, bool integer_output)
{
    // setup random generate (this code needs random library)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min_num , max_num); 
    
    if(integer_output == true) // return random number as integer 
    { return (int)dis(gen); }
    else  // return random number as double
    { return (double)dis(gen); }
}

void ai_agent::save_rewards_mean()
{
    if(learn_mode == false) {return;}
    
    std::ofstream file(path + "agents_reward_avg/" + name + "_rewards_mean.csv", std::ios_base::app); //creat file
    
    if(file.is_open())
    {
        file<< rewards_mean<< "\n";
        
        if(!file.fail())
        {
            std::cout << "\n" << name << " Rewards mean saved succesfully in " << path << "\n"; 
        }
        
        file.close();
    }
    
    
}

double ai_agent::new_rewards_mean(double new_reward, int number_of_rewards)
{
    double new_mean = 0;
    
    new_mean = rewards_mean + (new_reward - rewards_mean) / number_of_rewards;
    
    return new_mean;
}

void ai_agent::change_save_mode()
{
    if(is_saved == true)
    { is_saved = false; }
}

void ai_agent::clear_memory()
{
    agent_memory.mem
}






