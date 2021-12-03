#! /usr/bin/env python3
# script is intended to be ran on windows from the scripts directory
import subprocess
import matplotlib
import matplotlib.pyplot as plt

MAPS = ["CactusValleyLE.SC2Map", "BelShirVestigeLE.SC2Map", "ProximaStationLE.SC2Map"]
RACES = ["zerg", "protoss", "terran"]
DIFFICULTIES = ["Easy", "Medium", "Hard"]

def testRun():
    result = subprocess.run(["./../build/bin/BasicSc2Bot.exe", "-c", "-a", "zerg", "-d", "Hard", "-m", "BelShirVestigeLE.SC2Map"], stdout=subprocess.PIPE)
    parseOutput(str(result.stdout))

def runSimulations():

    global_results = {"wins":0, "losses":0, "ties":0, "undecided":0}

    race_results = {
        "zerg":{"wins":0, "losses":0, "ties":0, "undecided":0},
        "protoss":{"wins":0, "losses":0, "ties":0, "undecided":0},
        "terran":{"wins":0, "losses":0, "ties":0, "undecided":0}
    }

    map_results = {
        "CactusValleyLE.SC2Map":{"wins":0, "losses":0, "ties":0, "undecided":0},
        "BelShirVestigeLE.SC2Map":{"wins":0, "losses":0, "ties":0, "undecided":0},
        "ProximaStationLE.SC2Map":{"wins":0, "losses":0, "ties":0, "undecided":0}
    }

    difficulty_results = {
        "Easy":{"wins":0, "losses":0, "ties":0, "undecided":0},
        "Medium":{"wins":0, "losses":0, "ties":0, "undecided":0},
        "Hard":{"wins":0, "losses":0, "ties":0, "undecided":0}
    }

    # run all combinations 
    for difficulty in DIFFICULTIES:
        for race in RACES:
            for map in MAPS:
                print(f"currently processing enemny: {difficulty} {race} on {map}")
                result = subprocess.run(["./../build/bin/BasicSc2Bot.exe", "-c", "-a", race, "-d", difficulty, "-m", map], stdout=subprocess.PIPE)
                global_results[result] += 1
                race_results[race][result] += 1
                map_results[map][result] += 1
                difficulty_results[difficulty][result] += 1
    
    getGraphs(global_results, race_results, map_results, difficulty_results)


def parseOutput(stdout):


    #player id: 1\r\nPlayer 1 result: Win\r\nPlayer 2 result: Loss\r\n'

    information = stdout.split("\\r\\n")

    for info in information:
        print('wow')
        print(info)

    return "string"

def getGraphs(glob, race, map, difficulty):
    # matplotlib tings
    print(glob)
    print(race)
    print(map)
    print(difficulty)
    
if __name__ == "__main__":
    testRun()


# TODO
# - run all combinations (all difficulties, all maps, all enemy races)
# have log which indicates who won
# parse output to determine who won
# determine what the stats will be tweaking
# have CLI arguments for these things