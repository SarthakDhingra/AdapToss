#! /usr/bin/env python3
# all possible settings

MAPS = ["CactusValleyLE.SC2Map", "BelShirVestigeLE.SC2Map", "ProximaStationLE.SC2Map"]
RACES = ["zerg" , "protoss", "terran"]
DIFFICULTIES = ["Hard", "HardVeryHard", "VeryHard"]

# initialize results
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

# want to combine these results with other results
def setup2():

    global_results = {'Win': 91, 'Loss': 43, 'Tie': 1, 'Undecided': 0}

    race_results = {
                        'zerg': {'Win': 28, 'Loss': 17, 'Tie': 0, 'Undecided': 0}, 
                        'protoss': {'Win': 28, 'Loss': 17, 'Tie': 0, 'Undecided': 0}, 
                        'terran': {'Win': 35, 'Loss': 9, 'Tie': 1, 'Undecided': 0}
                    }

    map_results = {
                    'CactusValleyLE.SC2Map': {'Win': 29, 'Loss': 16, 'Tie': 0, 'Undecided': 0}, 
                    'BelShirVestigeLE.SC2Map': {'Win': 36, 'Loss': 8, 'Tie': 1, 'Undecided': 0}, 
                    'ProximaStationLE.SC2Map': {'Win': 26, 'Loss': 19, 'Tie': 0, 'Undecided': 0}
                    }

    difficulty_results = {
                            'Hard': {'Win': 41, 'Loss': 4, 'Tie': 0, 'Undecided': 0}, 
                            'HardVeryHard': {'Win': 32, 'Loss': 12, 'Tie': 1, 'Undecided': 0}, 
                            'VeryHard': {'Win': 18, 'Loss': 27, 'Tie': 0, 'Undecided': 0}
                            }

    bad_results = []
    
    return global_results, race_results, map_results, difficulty_results, bad_results

# create the dictionaries 
def createDicts(file_name):

    # initialize settings
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
            difficulty, race, map = setting[-4], setting[-3], setting[-1]

            # extract result
            result = lines[i+1].strip()
            index = result.find('=')
            result = result[index+2:]

            # update results
            if result != 'None':# and difficulty == 'Hard':
                global_results[result] += 1
                race_results[race][result] += 1
                map_results[map][result] += 1
                difficulty_results[difficulty][result] += 1
            else:
                bad_results.append(f"RESULT WAS NONE ({difficulty} {race} on {map})")
        i += 1

    return global_results, race_results, map_results, difficulty_results, bad_results
 
def aggregateDicts():

    global_results, race_results, map_results, difficulty_results, bad_results = setup2()

    file_names = ["files/evan2.txt", "files/sarthak.txt", "files/evan.txt"]

    for name in file_names:
        # get other dicts
        current_global, current_race, current_map, current_difficulty, current_bad = createDicts(name)
        
        if len(current_bad) != 0:
            print("error 1")
            return 1

        total_sims = 0
        
        # combine global values
        for key in global_results:
            global_results[key] += current_global[key]
            total_sims += current_global[key]

        if total_sims%27 != 0:
            print("error 2")
            print(name)
            print(current_global)
            return 1
        

        # combine race values
        for parent_key in race_results:
            for child_key in race_results[parent_key]:
                race_results[parent_key][child_key] += current_race[parent_key][child_key]
        
        # combine map values
        for parent_key in map_results:
            for child_key in map_results[parent_key]:
                map_results[parent_key][child_key] += current_map[parent_key][child_key]
        
        # combine difficulty keys
        for parent_key in difficulty_results:
            for child_key in difficulty_results[parent_key]:
                difficulty_results[parent_key][child_key] += current_difficulty[parent_key][child_key]
    
    print(global_results)
    print(race_results)
    print(map_results)
    print(difficulty_results)
    return global_results, race_results, map_results, difficulty_results


if __name__ == "__main__":
    aggregateDicts()