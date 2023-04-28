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

#ifndef AI_AGENT_H
#define AI_AGENT_H
 
#include "replay_bufer.h"
#include "reward_calculator.h"

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_agent.h>

#include <string>

#include "galaxy_ann.h"

//#include "onnxruntime/include/onnxruntime_cxx_api.h"


/**
 * coded by Amir mahdy yari
 * email : yarya524@gmail.com
 * telegram : @a_m_yari
 */

class ai_agent
{
private:
    
    // agent name (to open ann files)
    std::string name = "galaxy_agent";
    std::string path = "team_data/";
    
    // epsilon params
    double epsilon = 0.5;
    double eps_decay = 0.98;
    double eps_min = 0.005;
    int    epsilon_decay_delay = 10000;
    int    actions_taken       = 0;
    
    // memory paraams
    int max_mem = 3001;
    int batch_size = 64;

    // network learning paraams
    double learning_rate_alpha = 0.001;
    double learning_rate_beta  = 0.002;
    
    // learning paraams
    double gamma = 0.95;
    double soft_update_rate = 0.005;
    
    // analysis params

    
    bool use_model = true;
    
    
    
    replay_bufer agent_memory; // agent replay  bufer
    

    
    galaxy_ann model;
    
public:
    
    bool learn_mode = true;  // true for training and false for matchs
    
    bool is_loaded = false;
    bool is_saved  = false;

    double rewards_mean = 0;
    
    reward_calculator reward; //calculator state reward 
    
    void get_params(std::string Name , rcsc::PlayerAgent * agent, std::string Path = "team_data/",
                    double Starting_epsilon = 0.5 ,  double Eps_decay = 0.98 , double Eps_min = 0.005, 
                    int Epsilon_decay_delay = 10000, int Actions_taken = 0,
                    int Max_mem = 3001 ,int Batch_size = 64 ,
                    double Learning_rate_alpha =  0.001, double Learning_rate_beta = 0.002 ,
                    double Soft_update_rate = 0.005 ,double Gamma =0.95 , bool Learn_mode = true , bool use_model = true); // get custom params from user
    
    void save_experience(state State, action Action ); // add experience to the agent replay_bufer
    
    void save_next_state(rcsc::PlayerAgent * agent,state Next_state , bool Done); 
    
    action take_action(state State); // tack action from ann or generate random action 
    
    void save_memory_file(); // save the game experience (agent replay bufer) in .csv file
    
    void load_model(rcsc::PlayerAgent * agent);
    
    std::string to_string(); // write agent params in .csv file
    
    bool load_agent(rcsc::PlayerAgent * agent, std::string Path, std::string Name, bool Learn_mode); // read agent params from .csv file
    
    void save_agent(); // write agent params in .csv file
    
    int get_mem_counter();
    
    void add_goal_reward(int time);
    
    void save_rewards_mean();
    
    double new_rewards_mean(double new_reward , int number_of_rewards);
    
    void change_save_mode();

    double get_random_num(int min_num , int max_num , bool integer_output);

    void clear_memory();


    
};

#endif // AI_AGENT_H
