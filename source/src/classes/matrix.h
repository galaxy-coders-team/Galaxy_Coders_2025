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

#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <functional>

/**
 * @todo write docs
 */
class matrix
{
public:

    uint32_t _cols , _rows;
    std::vector<double> values;
    
    matrix ( uint32_t cols , uint32_t rows );
    matrix ( );
    
    double& at       ( uint32_t col , uint32_t row );
    matrix negetive ();
    
    matrix add(matrix &target);
    
    matrix multiply          ( matrix &target );
    matrix multiply_elements ( matrix &target );
    matrix multiply_scaler   ( double s );
    
    matrix read_from_file( std::string path );
    
    matrix fill();
    matrix fill(double a);
    
    matrix transpose();

};

#endif // MATRIX_H
