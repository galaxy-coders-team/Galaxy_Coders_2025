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

#ifndef GALAXY_ANN_H
#define GALAXY_ANN_H

#include <vector>
#include <cstdlib>
#include <cmath>

#include "matrix.h"
#include "galaxy_ann.h"

/**
 * @todo write docs
 */
class galaxy_ann
{
    matrix base(matrix& input);
    std::vector<double> action_output(matrix & base);
    
    bool is_goaler = false;

    //Activation Functions
    double sigmoid(double input);
    double tanh(double input);
    double liner(double input , double min , double max);
    matrix relu(matrix & input);
    int argmax_softmax(matrix & input);
    
public:
    
    std::vector<uint32_t> _topology;
    std::vector<matrix>  _weightMatrices;
    std::vector<matrix>  _biasMatrices;
    
    void get_param(std::vector<uint32_t> topology ,std::string main_path , bool Is_goaler);
    
    void read_weight(std::string path);
    
    std::vector<double> predict(std::vector<double> input);
    
};


#endif // GALAXY_ANN_H
