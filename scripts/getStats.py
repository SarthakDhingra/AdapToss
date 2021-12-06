#! /usr/bin/env python3

# script is intended to be ran on windows from the scripts directory
# chmod +x getStats.py; ./getStats.py
import subprocess
import matplotlib.pyplot as plt
import numpy as np

# all possible settings
MAPS = ["CactusValleyLE.SC2Map", "BelShirVestigeLE.SC2Map", "ProximaStationLE.SC2Map"]
RACES = ["zerg" , "protoss", "terran"]
DIFFICULTIES = ["VeryEasy", "Easy", "Medium", "MediumHard", "Hard", "HardVeryHard", "VeryHard"] # "CheatVision", "CheatMoney", "CheatInsane"]

# if you want to just run one specific configuration
def testRun():
    result = subprocess.run(["./../build/bin/BasicSc2Bot.exe", "-c", "-a", "zerg", "-d", "Hard", "-m", "BelShirVestigeLE.SC2Map"], stdout=subprocess.PIPE)
    parseOutput(str(result.stdout))

# driver to run all simulation
def runSimulations():

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
    for difficulty in DIFFICULTIES:
        difficulty_results[difficulty] = {"Win":0, "Loss":0, "Tie":0, "Undecided":0}

    # run all combinations 
    for map in MAPS:
        for race in RACES:
            for difficulty in DIFFICULTIES:
                # print for ourselves
                print(f"currently processing enemy: {difficulty} {race} on {map}")
                # run simulation
                output = subprocess.run(["./../build/bin/BasicSc2Bot.exe", "-c", "-a", race, "-d", difficulty, "-m", map], stdout=subprocess.PIPE)
                # get stdout and process it
                result = parseOutput(output)

                # update result hashes
                global_results[result] += 1
                race_results[race][result] += 1
                map_results[map][result] += 1
                difficulty_results[difficulty][result] += 1

    # generate graphs and tables
    getGraphs(global_results, race_results, map_results, difficulty_results)

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

# TODO: generate graphs and tables from results
def getGraphs(glob, race, map, difficulty):

    for key in map:
        map[key.replace('.SC2Map', '')] = map.pop(key)

    categorical_results = {"Race": race, "Map":map, "Difficulty": difficulty}

    for name, results in categorical_results.items():
    
        # prepare data
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
        ax.set_title(f'Results by {name}')
        ax.set_xticks(x, categories)

        ax.bar_label(rects1, padding=1)
        ax.bar_label(rects2, padding=1)

        fig.tight_layout()
        
        if name == "Difficulty":
            plt.xticks(rotation=45)

        plt.savefig(f'output/{name}.png', bbox_inches='tight')

    
if __name__ == "__main__":
    #runSimulations()
    #testRun()

    glob = {'Win': 47, 'Loss': 43, 'Tie': 0, 'Undecided': 0}

    race = {
        'zerg': {'Win': 14, 'Loss': 16, 'Tie': 0, 'Undecided': 0}, 
        'protoss': {'Win': 17, 'Loss': 13, 'Tie': 0, 'Undecided': 0}, 
        'terran': {'Win': 16, 'Loss': 14, 'Tie': 0, 'Undecided': 0}
    }

    map = {
        'CactusValleyLE.SC2Map': {'Win': 16, 'Loss': 14, 'Tie': 0, 'Undecided': 0}, 
        'BelShirVestigeLE.SC2Map': {'Win': 17, 'Loss': 13, 'Tie': 0, 'Undecided': 0}, 
        'ProximaStationLE.SC2Map': {'Win': 14, 'Loss': 16, 'Tie': 0, 'Undecided': 0}
    }

    difficulty = {
        'VeryEasy': {'Win': 9, 'Loss': 0, 'Tie': 0, 'Undecided': 0}, 
        'Easy': {'Win': 9, 'Loss': 0, 'Tie': 0, 'Undecided': 0}, 
        'Medium': {'Win': 8, 'Loss': 1, 'Tie': 0, 'Undecided': 0}, 
        'MediumHard': {'Win': 8, 'Loss': 1, 'Tie': 0, 'Undecided': 0}, 
        'Hard': {'Win': 5, 'Loss': 4, 'Tie': 0, 'Undecided': 0}, 
        'HardVeryHard': {'Win': 5, 'Loss': 4, 'Tie': 0, 'Undecided': 0}, 
        'VeryHard': {'Win': 3, 'Loss': 6, 'Tie': 0, 'Undecided': 0}
    }

    getGraphs(glob, race, map, difficulty)


# TODO
# - run all combinations (all difficulties, all maps, all enemy races)
# have log which indicates who won
# parse output to determine who won
# determine what the stats will be tweaking
# have CLI arguments for these things