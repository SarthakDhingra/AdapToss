#! /usr/bin/env python3

import math

def standard_deviation():

    # get all win loss values
    values = []

    # 57 wins
    for i in range(57):
        values.append(1)
    
    # 6 losses
    for i in range(6):
        values.append(0)
    
    # numerator
    total = 0

    # mean
    mean = 57/63

    # sum up numerator of standard deviation equation
    for val in values:
        total += (val-mean)**2

    # get standard deviation
    return math.sqrt(total/63)


if __name__ == '__main__':
    print(standard_deviation())
