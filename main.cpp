#include <iostream>
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "BasicSc2Bot.h"
#include "LadderInterface.h"

using namespace sc2;

// LadderInterface allows the bot to be tested against the built-in AI or
// played against other bots
int main(int argc, char* argv[]) {
	//RunBot(argc, argv, new BasicSc2Bot(), sc2::Race::Terran);
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);

	BasicSc2Bot bot;
	coordinator.SetParticipants({
		CreateParticipant(Race::Protoss, &bot),
		CreateComputer(Race::Zerg)
		});

	coordinator.LaunchStarcraft();
	coordinator.StartGame();

	while (coordinator.Update()) {
	}

	return 0;
}