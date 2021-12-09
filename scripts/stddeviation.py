#! /usr/bin/env python3

import math

def standard_deviation():

    # get all win loss values
    values = []

    # number of wins
    for i in range(63):
        values.append(1)
    
    # number of losses
    for i in range(81):
        values.append(0)  
    
    # numerator
    total = 0

    # mean
    mean = 63/144

    # sum up numerator of standard deviation equation
    for val in values:
        total += (val-mean)**2

    # get standard deviation
    return math.sqrt(total/144)

if __name__ == '__main__':
    print(standard_deviation())
