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

#include "memory.h"
#include "state.h"
#include "action.h"


void memory::get_memory(state State , action Action, double Reward, state Next_state , bool Done)
{

    mem_state = State;
    mem_action = Action;
    mem_reward = Reward;
    mem_next_state = Next_state;
    mem_done = Done;
}

void memory::operator=(memory mem)
{
    mem_state = mem.mem_state;
    mem_action = mem.mem_action;
    mem_reward = mem.mem_reward;
    mem_next_state = mem.mem_next_state;
    mem_done = mem.mem_done;
}

