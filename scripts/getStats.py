#! /usr/bin/env python3

# script is intended to be ran on windows from the scripts directory
# chmod +x getStats.py; ./getStats.py
import subprocess
import matplotlib.pyplot as plt

# all possible settings
MAPS = ["CactusValleyLE.SC2Map", "BelShirVestigeLE.SC2Map", "ProximaStationLE.SC2Map"]
RACES = ["zerg" , "protoss", "terran"]
DIFFICULTIES = ["VeryEasy", "Easy", "Medium", "MediumHard", "Hard", "HardVeryHard", "VeryHard", "CheatVision", "CheatMoney", "CheatInsane"]

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
    for difficulty in DIFFICULTIES:
        for race in RACES:
            for map in MAPS:
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
    # matplotlib tings
    print(glob)
    print()
    print(race)
    print()
    print(map)
    print()
    print(difficulty)
    
if __name__ == "__main__":
    runSimulations()
    #testRun()


# TODO
# - run all combinations (all difficulties, all maps, all enemy races)
# have log which indicates who won
# parse output to determine who won
# determine what the stats will be tweaking
# have CLI arguments for these things