#! /usr/bin/env python3
# script is intended to be ran on windows from the scripts directory
import subprocess

def runGame():
    result = subprocess.run(["./../build/bin/BasicSc2Bot.exe", "-c", "-a", "zerg", "-d", "Hard", "-m", "CactusValleyLE.SC2Map"], stdout=subprocess.PIPE)
    print(result.stdout)

if __name__ == "__main__":
    runGame()


# TODO
# - run all combinations (all difficulties, all maps, all enemy races)
# have log which indicates who won
# parse output to determine who won
# determine what the stats will be tweaking
# have CLI arguments for these things