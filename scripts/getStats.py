#! /usr/bin/env python3

# NOTES - How to run the script
# script is intended to be ran on windows from the scripts directory
# cd scripts
# chmod +x getStats.py;
# ./getStats.py

import subprocess
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
import numpy as np

# all possible settings
MAPS = ["CactusValleyLE.SC2Map", "BelShirVestigeLE.SC2Map", "ProximaStationLE.SC2Map"]
RACES = ["zerg" , "protoss", "terran"]
DIFFICULTIES = ["Hard", "HardVeryHard", "VeryHard"] #["VeryEasy", "Easy", "Medium", "MediumHard", "Hard", "HardVeryHard", "VeryHard"] # "CheatVision", "CheatMoney", "CheatInsane"]

# global results hash
global_results = {"Win":0, "Loss":0, "Tie":0, "Undecided":0}

# results broken down by race
race_results = {
    "zerg":{"Win":0, "Loss":0, "Tie":0, "Undecided":0},
    "protoss":{"Win":0, "Loss":0, "Tie":0, "Undecided":0},
    "terran":{"Win":0, "Loss":0, "Tie":0, "Undecided":0}
}

# results broken down by map
map_results = {
    "CactusValleyLE.SC2Map":{"Win":0, "Loss":0, "Tie":0, "Undecided":0},
    "BelShirVestigeLE.SC2Map":{"Win":0, "Loss":0, "Tie":0, "Undecided":0},
    "ProximaStationLE.SC2Map":{"Win":0, "Loss":0, "Tie":0, "Undecided":0}
}

# results broken down by difficulty
difficulty_results = {}

# array to keep track of problematic runs
bad_results = []

# if you want to just run one specific configuration can run this function
def testRun():
    result = subprocess.run(["./../build/bin/BasicSc2Bot.exe", "-c", "-a", "protoss", "-d", "Hard", "-m", "CactusValleyLE.SC2Map"], stdout=subprocess.PIPE)
    parseOutput(str(result.stdout))

# driver to run all simulation
def runSimulations():

    # run all combinations 5 times (9*45) 
    for i in range(5):
        for map in MAPS:
            for race in RACES:
                for difficulty in DIFFICULTIES:
                    # print for ourselves
                    print(f"currently processing enemy: iteration {i} {difficulty} {race} on {map}")
                    # run simulation
                    output = subprocess.run(["./../build/bin/BasicSc2Bot.exe", "-c", "-a", race, "-d", difficulty, "-m", map], stdout=subprocess.PIPE)
                    # get stdout and process it
                    result = parseOutput(output)

                    if result is not None:
                        # update result hashes
                        global_results[result] += 1
                        race_results[race][result] += 1
                        map_results[map][result] += 1
                        difficulty_results[difficulty][result] += 1
                    else:
                        print(f"RESULT WAS NONE ({difficulty} {race} on {map})")
                        bad_results.append(f"RESULT WAS NONE ({difficulty} {race} on {map})")
    
    # generate graphs and tables
    getGraphs(global_results, race_results, map_results, difficulty_results, bad_results)

def getSpecificResults():

    # array of specific runs you want to run
    runs = [
        ["VeryHard", "protoss", "BelShirVestigeLE.SC2Map"],
        ["VeryHard",  "terran", "BelShirVestigeLE.SC2Map"],
        ['VeryHard', "zerg", "ProximaStationLE.SC2Map"],
        ["HardVeryHard", "terran", "ProximaStationLE.SC2Map"]
    ]

    # run desired simulations only
    for difficulty, race, map in runs:

        print(f"currently processing enemy: {difficulty} {race} on {map}")
        # run simulation
        output = subprocess.run(["./../build/bin/BasicSc2Bot.exe", "-c", "-a", race, "-d", difficulty, "-m", map], stdout=subprocess.PIPE)
        # get stdout and process it
        result = parseOutput(output)

        if result is not None:
            # update result hashes
            global_results[result] += 1
            race_results[race][result] += 1
            map_results[map][result] += 1
            difficulty_results[difficulty][result] += 1
        else:
            print(f"RESULT WAS NONE ({difficulty} {race} on {map})")
            print()
            print('stdout:')
            print(output)
            print()
            bad_results.append(f"RESULT WAS NONE ({difficulty} {race} on {map})")

    # generate graphs and tables
    getGraphs(global_results, race_results, map_results, difficulty_results, bad_results)

# function to get match result from stdout
def parseOutput(stdout):

    information = str(stdout).split("\\r\\n")
    result = None
    for info in information:
        if "Player 1 result:" in info:
            index = info.find(":")
            result = info[index+2:]

    print(f"Result = {result}")
    return result

# generate graphs
def getGraphs(glob, race, map, difficulty, bad_results):

    # print to stdout
    print('global results')
    print(glob)
    print('race results')
    print(race)
    print('map results')
    print(map)
    print('difficulty results')
    print(difficulty)
    print('bad results')
    print(bad_results)

    # write to file
    file = open("output/results.txt", "w") 
    file.write(f'global = {str(glob)}') 
    file.write('\n')
    file.write('\n')
    file.write(f'race = {str(race)}')  
    file.write('\n')
    file.write('\n')
    file.write(f'map = {str(map)}')  
    file.write('\n')
    file.write('\n')
    file.write(f'difficulty = {str(difficulty)}')  
    file.write('\n')
    file.write('\n')
    file.write(f'bad_results = {str(bad_results)}')  
    file.write('\n')
    file.write('\n')
    file.close() 

    #commented out for group testing, will uncomment later
    # prepare data
    for key in map:
        map[key.replace('.SC2Map', '')] = map.pop(key)

    categorical_results = {"Race": race, "Map":map, "Difficulty": difficulty}

    for name, results in categorical_results.items():
    
        categories = list(results.keys())
        wins = []
        losses = []
        for category in categories:
            wins.append(results[category]['Win'])
            losses.append(results[category]['Loss'])
        
        # the label locations
        x = np.arange(len(categories))  
        
        # the width of the bars
        width = 0.35  

        # clear plot
        plt.clf()
        fig, ax = plt.subplots()
        rects1 = ax.bar(x - width/2, wins, width, label='Wins')
        rects2 = ax.bar(x + width/2, losses, width, label='Losses')

        # Add some text for labels, title and custom x-axis tick labels, etc.
        ax.set_ylabel('Quantity')
        ax.set_xlabel(name)
        ax.legend(loc='upper right')
        ax.set_xticks(x, categories)

        ax.bar_label(rects1, padding=1)
        ax.bar_label(rects2, padding=1)
        ax.yaxis.set_major_locator(MaxNLocator(integer=True))

        fig.tight_layout()
        
        if name == "Difficulty":
            plt.xticks(rotation=45)

        plt.savefig(f'output/{name}.png', bbox_inches='tight')

def generateGraphs():
    #current results
    glob = {'Win': 292, 'Loss': 139, 'Tie': 1, 'Undecided': 0}

    race = {'zerg': {'Win': 87, 'Loss': 57, 'Tie': 0, 'Undecided': 0}, 'protoss': {'Win': 98, 'Loss': 46, 'Tie': 0, 'Undecided': 0}, 'terran': {'Win': 107, 'Loss': 36, 'Tie': 1, 'Undecided': 0}}

    map = {'CactusValleyLE.SC2Map': {'Win': 93, 'Loss': 51, 'Tie': 0, 'Undecided': 0}, 'BelShirVestigeLE.SC2Map': {'Win': 115, 'Loss': 28, 'Tie': 1, 'Undecided': 0}, 'ProximaStationLE.SC2Map': {'Win': 84, 'Loss': 60, 'Tie': 0, 'Undecided': 0}} 

    difficulty = {'Hard': {'Win': 131, 'Loss': 13, 'Tie': 0, 'Undecided': 0}, 'HardVeryHard': {'Win': 98, 'Loss': 45, 'Tie': 1, 'Undecided': 0}, 'VeryHard': {'Win': 63, 'Loss': 81, 'Tie': 0, 'Undecided': 0}}

    getGraphs(glob, race, map, difficulty, [])
    
if __name__ == "__main__":

    for difficulty in DIFFICULTIES:
        difficulty_results[difficulty] = {"Win":0, "Loss":0, "Tie":0, "Undecided":0}

    # runSimulations()
    # testRun()
    # generateGraphs()
    # getSpecificResults()