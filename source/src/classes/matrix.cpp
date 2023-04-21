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
#include<fstream>
#include <sstream> 

#include "matrix.h"

matrix::matrix()
{
    _cols = 0;
    _rows = 0;
    values.clear();
}

matrix::matrix(uint32_t cols, uint32_t rows)
{
    _cols = cols ;
    _rows  = rows  ;
    values.resize(cols * rows);
}

double& matrix::at(uint32_t col, uint32_t row)
{
    return values[row * _cols + col];
}

matrix matrix::negetive()
{
    matrix output(_cols, _rows);
    
    for (uint32_t y = 0; y < output._rows; y++)
    {
        for (uint32_t x = 0; x < output._cols; x++)
        {
            output.at(x, y) = -at(x, y);
        }
    }
    
    return output;
}

matrix matrix::add(matrix& target)
{
    assert(_rows == target._rows && _cols == target._cols);
    
    matrix output(_cols, _rows);
    
    for (uint32_t y = 0; y < output._rows; y++)
    {
        for (uint32_t x = 0; x < output._cols; x++)
        {
            output.at(x, y) = (double)at(x,y) + (double)target.at(x, y);
        }
    }
    
    return output;
}

matrix matrix::multiply(matrix& target)
{
    assert(_cols == target._rows);
    matrix output(target._cols, _rows);
    for (uint32_t y = 0; y < output._rows; y++)
    {
        for (uint32_t x = 0; x < output._cols; x++)
        {
            double result = 0;
            for (uint32_t k = 0; k < _cols; k++)
            {
                result += (double)at(k, y) * (double)target.at(x, k);
            }
            output.at(x, y) = result;
        }
    }
    return output;
}

matrix matrix::multiply_elements(matrix& target)
{
    assert(_rows == target._rows && _cols == target._cols);
    
    matrix output(_cols, _rows);
    
    for (uint32_t y = 0; y < output._rows; y++)
    {
        for (uint32_t x = 0; x < output._cols; x++)
        {
            output.at(x, y) = at(x,y) * target.at(x, y);
        }
    }
    
    return output;
}


matrix matrix::multiply_scaler(double s)
{
    matrix output(_cols, _rows);
    
    for (uint32_t y = 0; y < output._rows; y++)
    {
        for (uint32_t x = 0; x < output._cols; x++)
        {
            output.at(x, y) = at(x, y) * s;
        }
    }
    
    return output;
}

matrix matrix::fill()
{
    matrix output(_cols, _rows);
    
    for (uint32_t y = 0; y < output._rows; y++)
    {
        for (uint32_t x = 0; x < output._cols; x++)
        {
            output.at(x, y) = (double)rand() / RAND_MAX;
        }
    }
    
    return output;
}

matrix matrix::fill(double a)
{
    matrix output(_cols, _rows);
    
    for (uint32_t y = 0; y < output._rows; y++)
    {
        for (uint32_t x = 0; x < output._cols; x++)
        {
            output.at(x, y) =  a ;
        }
    }
    
    return output;
}

matrix matrix::read_from_file(std::string path)
{
    matrix output(_cols,_rows);
    output = output.fill(0);
    
    std::ifstream file;
    file.open(path); // open our file
    
    if(file.fail())
    { return output;}
    
    std::string line , colname;
    
    for(int row = 0 ; row < _rows ; row++)
    {
        std::getline(file,line);
        
        std::stringstream ss(line);
        
        for(int  col = 0 ; col < _cols ; col++)
        {
            
            std::getline(ss, colname, ',');
            output.at(col,row) = std::stod(colname);
        }
    }
    
    return output;
}

matrix matrix::transpose()
{
    matrix output(_rows, _cols);
    for (uint32_t y = 0; y < _rows; y++)
    {
        for (uint32_t x = 0; x < _cols; x++)
        {
            output.at(y, x) = at(x, y);
        }
    }
    return output;
}


