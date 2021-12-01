#include "SlanderSystem.h"
#include <chrono>



//initialize slander system by setting up variables and slader
void SlanderSystem::Init(const ObservationInterface* obs, ActionInterface* act){
    observation = obs;
	actions = act;
    auto enemy_race = observation->GetGameInfo().player_info[1].race_requested;

    slander.push("Maybe you should just surrender now?");

    //race specific slander
    if (enemy_race == Race::Terran){
        slander.push("Pesky humans, you couldn't even beat my cousin ET");;
    }
    else if (enemy_race == Race::Protoss){
        slander.push("You dare use my own spells against me?");;
    }
    else{
        slander.push("Who doesn't hate bugs? At least they're fun to squish...");
    }
    time = std::chrono::system_clock::now();
}

void SlanderSystem::addSlander(std::string msg){
    slander.push(msg);
}

void SlanderSystem::SlanderStep(){
    if (!slander.empty() 
    && (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - time).count()) > delay){
        time = std::chrono::system_clock::now();
        auto str = slander.front();
        actions->SendChat(slander.front());
        slander.pop();
        slander.push(str);
    }
}