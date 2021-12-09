#! /usr/bin/env python3

import math

def standard_deviation():

    # get all win loss values
    values = []

    # 57 wins
    for i in range(5):
        values.append(1)
    
    # 6 losses
    for i in range(4):
        values.append(0)
    
    # numerator
    total = 0

    # mean
    mean = 5/9

    # sum up numerator of standard deviation equation
    for val in values:
        total += (val-mean)**2

    # get standard deviation
    return math.sqrt(total/8)


if __name__ == '__main__':
    print(standard_deviation())
