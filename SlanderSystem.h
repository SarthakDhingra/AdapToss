#pragma once

#include <queue>
#include <string>
#include "sc2api/sc2_api.h"

using namespace sc2;

//set up slander system strucutre
class SlanderSystem {
public:
	void Init(const ObservationInterface* obs, ActionInterface* act);
	void SlanderStep();
	void addSlander(std::string msg);
private:
	std::queue<std::string> slander;
    std::chrono::time_point<std::chrono::system_clock> time;
    const int delay = 15;
    const ObservationInterface* observation = nullptr;
	ActionInterface* actions = nullptr;
};
