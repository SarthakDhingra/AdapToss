#! /usr/bin/env python3
# script is intended to be ran on windows from the scripts directory
import subprocess

def runGame():
    result = subprocess.run(["./../build/bin/BasicSc2Bot.exe", "-c", "-a", "zerg", "-d", "Hard", "-m", "CactusValleyLE.SC2Map"], stdout=subprocess.PIPE)
    print(result)

if __name__ == "__main__":
    runGame()