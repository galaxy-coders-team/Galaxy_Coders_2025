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

#ifndef REPLAY_BUFER_H
#define REPLAY_BUFER_H

#include"action.h"
#include"state.h"
#include"memory.h"

#include <vector>

/**
 * @todo
 * coded by Amir mahdy yari
 * email : yarya524@gmail.com
 * telegram : @a_m_yari
 */
class replay_bufer
{
    std::vector<memory> exp_memory; // we will save our game experience in that list
    int mem_counter = 0; // use to get memory index
    int mem_size = 0;  // memory maximom size
    
public:
    
    
    void get_params(int memSize); // get memory size and build memory list
    void rebuild(int memSize); // just like get_params() but we clear the exp_memory list in this one
    void add_experience(state State, action Action);// add memory to the list
    void add_next_state(double,state,bool); // add next_statr to the perevis memory
    
    state get_last_state();
    action get_last_action();
    
    std::vector<memory> get_memory(); // return batch  of random memory from exp_memory list
    int get_counter();
    void add_reward(int Mem_counter ,double Reward);
    
};
#endif // REPLAY_BUFER_H
