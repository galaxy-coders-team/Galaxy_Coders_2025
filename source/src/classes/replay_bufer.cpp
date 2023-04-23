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


#include "replay_bufer.h"
#include "state.h"
#include "action.h"
#include "memory.h"

#include <cstdlib>
#include <time.h>
#include <algorithm>
#include <random>

 
void replay_bufer::get_params(int memSize)
{
    mem_counter = 0;
    mem_size = memSize;
    exp_memory.resize(mem_size); //set vector size
    
}

void replay_bufer::add_experience(state State, action Action)
{
    int index = mem_counter % mem_size; // find memory index
    
    exp_memory[index].mem_state = State;
    exp_memory[index].mem_action = Action;
    
    mem_counter += 1; 
}

void replay_bufer::add_next_state(double Reward,state Next_state , bool Done)
{
    if(mem_counter == 0) { return; }
    int index = (mem_counter - 1) % mem_size; // find previce memory index
    
    exp_memory[index].mem_next_state = Next_state;
    exp_memory[index].mem_done = Done;
    exp_memory[index].mem_reward = Reward;
}

void replay_bufer::add_reward(int Mem_counter, double Reward)
{
    int index = Mem_counter % mem_size; // find memory index
    
    exp_memory[index].mem_reward += Reward;
}


std::vector<memory> replay_bufer::get_memory()
{
    std::vector<memory> output;
    
    int mem_fill = std::min(mem_counter,mem_size); // number of filled memory
    
    
    for(int i = 0 ; i < mem_fill ; i++)
    {
        output.push_back(exp_memory[i]); // add memory to list
    }
    
    return output;
}

state replay_bufer::get_last_state()
{
    int index = (mem_counter - 1) % mem_size; // find previce memory index
    
    return exp_memory[index].mem_state;
}

action replay_bufer::get_last_action()
{
    int index = (mem_counter - 1) % mem_size; // find previce memory index
    
    return exp_memory[index].mem_action;
}

int replay_bufer::get_counter()
{
    return mem_counter;
}
