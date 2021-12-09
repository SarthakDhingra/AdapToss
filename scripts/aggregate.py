#! /usr/bin/env python3


# all possible settings
MAPS = ["CactusValleyLE.SC2Map", "BelShirVestigeLE.SC2Map", "ProximaStationLE.SC2Map"]
RACES = ["zerg" , "protoss", "terran"]
DIFFICULTIES = ["Hard", "HardVeryHard", "VeryHard"]

def setup():

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
    
    bad_results = []
    
    return global_results, race_results, map_results, difficulty_results, bad_results

def createDicts(file_name):

    global_results, race_results, map_results, difficulty_results, bad_results = setup()

    infile = open(file_name, 'r')
    lines = infile.readlines()

    i = 0

    while i < len(lines):
        if "currently processing enemy" in lines[i]:
            # extract settings
            setting = lines[i].strip()
            index = setting.find(':')
            setting = setting[index:].split()
            difficulty, race, map = setting[1], setting[2], setting[-1]

            # extract result
            result = lines[i+1].strip()
            index = result.find('=')
            result = result[index+2:]

            # update results
            if result != 'None':
                global_results[result] += 1
                race_results[race][result] += 1
                map_results[map][result] += 1
                difficulty_results[difficulty][result] += 1
            else:
                bad_results.append(f"RESULT WAS NONE ({difficulty} {race} on {map})")
        i += 1
    
    # print to stdout
    print('global results')
    print(global_results)
    print('race results')
    print(race_results)
    print('map results')
    print(map_results)
    print('difficulty results')
    print(difficulty_results)
    print('bad results')
    print(bad_results)
 
    

def aggregateDicts():

    glob = {'Win': 91, 'Loss': 40, 'Tie': 0, 'Undecided': 0}

    race = {'zerg': {'Win': 27, 'Loss': 17, 'Tie': 0, 'Undecided': 0}, 'protoss': {'Win': 33, 'Loss': 11, 'Tie': 0, 'Undecided': 0}, 'terran': {'Win': 31, 'Loss': 12, 'Tie': 0, 'Undecided': 0}}

    map = {'CactusValleyLE.SC2Map': {'Win': 27, 'Loss': 18, 'Tie': 0, 'Undecided': 0}, 'BelShirVestigeLE.SC2Map': {'Win': 36, 'Loss': 7, 'Tie': 0, 'Undecided': 0}, 'ProximaStationLE.SC2Map': {'Win': 28, 'Loss': 15, 'Tie': 0, 'Undecided': 0}}

    difficulty = {'Hard': {'Win': 42, 'Loss': 3, 'Tie': 0, 'Undecided': 0}, 'HardVeryHard': {'Win': 28, 'Loss': 16, 'Tie': 0, 'Undecided': 0}, 'VeryHard': {'Win': 21, 'Loss': 21, 'Tie': 0, 'Undecided': 0}}





if __name__ == "__main__":
    createDicts("files/stdout.txt")
