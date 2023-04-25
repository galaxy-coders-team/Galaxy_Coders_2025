
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

#include "action.h"
#include <boost/array.hpp>

void action::get_action(double Direction, double Power, int at)
{
    direction = Direction;
    power = Power;
    action_type = at;
}
void action::empty_action()
{
    direction = 0;
    power = 0;
    action_type = 0; 
}

void action::operator=(action action)
{
    action_type = action.action_type;
    direction = action.direction;
    power = action.power;
}

std::string action::to_string()
{
    std::string output = "";
    
    output += std::to_string(action_type * 100); output += ",";
    output += std::to_string(direction   * 100); output += ",";
    output += std::to_string(power       * 100);
    
    
    return output;
}

