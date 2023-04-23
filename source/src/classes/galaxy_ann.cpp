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

#include "galaxy_ann.h"
#include <vector>
#include <iostream>
#include <boost/mpl/max_element.hpp>

void galaxy_ann::get_param(std::vector<uint32_t> topology, std::string main_path)
{
    _topology = topology;
    
    uint32_t i;
    
    _biasMatrices.clear();
    _biasMatrices.clear();
    
    for(i = 0; i < topology.size() - 1; i++)
    {
        matrix weightMatrix(topology[i + 1],topology[i] );
        
        weightMatrix = weightMatrix.read_from_file(main_path +  std::to_string(i) + "_weights.csv");
        _weightMatrices.push_back(weightMatrix);
                    
        matrix biasMatrix(1 ,topology[i + 1]  );
        
        biasMatrix = biasMatrix.read_from_file(main_path + std::to_string(i) + "_bias.csv").transpose();
        _biasMatrices.push_back(biasMatrix);
   
    }
    
    i = topology.size() - 1;
    
    //direction 
    matrix direction_weigh(1 , topology[i]);
    matrix direction_bias(1 ,1);
    
    direction_weigh = direction_weigh.read_from_file(main_path +  std::to_string(i) + "_weights.csv");
    direction_bias = direction_bias.read_from_file(main_path + std::to_string(i) + "_bias.csv").transpose();
    
    _weightMatrices.push_back(direction_weigh);
    _biasMatrices.push_back(direction_bias);
    
    // power
    matrix power_weigh(1 , topology[i +1]);
    matrix power_bias(1 ,1);
    
    power_weigh = power_weigh.read_from_file(main_path +  std::to_string(i + 1) + "_weights.csv");
    power_bias = power_bias.read_from_file(main_path + std::to_string(i + 1) + "_bias.csv").transpose();
    
    _weightMatrices.push_back(power_weigh);
    _biasMatrices.push_back(power_bias);
    
    //std::cout.precision(8);
    //std::cout<<std::fixed<<"fff:"<< _weightMatrices[i-2].at(0,0) <<"\n";
}


matrix galaxy_ann::base(matrix& input)
{
    matrix values = input;
    
    //forwording inputs to next layers
    for(uint32_t i = 0; i < _weightMatrices.size() - 3; i++)
    {
        values = values.multiply(_weightMatrices[i]);
        values = values.add(_biasMatrices[i]);
        
        values = relu(values);
    }
    return values; 
}

std::vector<double> galaxy_ann::action_output(matrix& base)
{
    std::vector<double> values;
    
    int index = _weightMatrices.size() - 1;
    matrix x;
    
    x = base.multiply( _weightMatrices[index] );
    x = x.add(_biasMatrices[index]);
    
    values.push_back( tanh(x.at(0,0)) );
    
    x = base.multiply( _weightMatrices[index + 1] );
    x = x.add(_biasMatrices[index + 1]);
    
    values.push_back(sigmoid(x.at(0,0)));
    
    return values; 
}


std::vector<double> galaxy_ann::predict(std::vector<double> input)
{
       if(input.size() != _topology[0])
    {  }
    
    // creating input matrix 
    matrix x(input.size() , 1);
    
    
    for(uint32_t i = 0; i < input.size(); i++)
        x.values[i] = input[i];
    
    x = base(x);
    
    std::vector<double> output = action_output(x); 
    
    return output;
}

double galaxy_ann::sigmoid(double input)
{
    double output;
    
    output = (double)1.0 / (double)(1 + (double)exp(-input));
        
    return output;
}

double galaxy_ann::tanh(double input)
{
    double output ;
     
    output = std::tanh(input);
    return output;
}

matrix galaxy_ann::relu(matrix& input)
{
    matrix output(input._cols, input._rows);
    for (uint32_t y = 0; y < output._rows; y++)
    {
        for (uint32_t x = 0; x < output._cols; x++)
        {
            double value = input.at(x,y);
            output.at(x, y) = std::max<double>(0,value);
        }
    }
    return output;
}



int galaxy_ann::argmax_softmax(matrix& input)
{
    int output = 0;
    
    double max = input.values[0] , val = 0;
    
    for(int i = 1 ; i < 4 ; i++)
    {
        val = input.values[i];
        
        if (val > max)
        {
            output = i;
            max = val;
         }
    }
    
    return output;
}







