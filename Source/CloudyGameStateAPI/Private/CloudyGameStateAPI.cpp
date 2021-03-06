#include "CloudyGameStateAPIPrivatePCH.h"
 
#include "CloudyGameStateAPI.h"

#include <string>
#include <fstream>
#include <ctime>
#include <vector>

DEFINE_LOG_CATEGORY(CloudyGameStateAPILog);

/* Currently 6 states. The indexes are used for these:
 *	   Index 0: Active (High activity actions, e.g. shooting)
 *     Index 1: Movement
 *     Index 2: Looking
 *     Index 3: Movie
 *     Index 4: Menu
 *     Index 5: Idle
 * The weights of the state are stored into the index when the 
 * state is entered. When the state is exited, 0 is stored into 
 * the index.
 *
 * IMPORTANT: The last index should always be the IDLE state.
 */

#define NUM_PLAYERS 8
#define NUM_STATES 6

#define INDEX_ACTIVE 0
#define INDEX_MOVEMENT 1
#define INDEX_LOOKING 2
#define INDEX_MOVIE 3
#define INDEX_MENU 4
#define INDEX_IDLE 5

// SP Edit: Changed weights
#define WEIGHT_ACTIVE 3
#define WEIGHT_MOVEMENT 5
#define WEIGHT_LOOKING 2
#define WEIGHT_MOVIE 2
#define WEIGHT_MENU 1
#define WEIGHT_IDLE 1
#define WEIGHT_ZERO 0

#define STATE_CHECK_INTERVAL 1.0

int GameStateTracker[NUM_PLAYERS][NUM_STATES];

// Active state
time_t TimeSinceLastActive[NUM_PLAYERS];
bool IsPressAndHoldToShoot = true;

// Movement state
time_t TimeSinceLastMovement[NUM_PLAYERS];

// Looking state
time_t TimeSinceLastLooking[NUM_PLAYERS];

// Number of players
int CNumOfPlayersOldAPI = 0;

// Writing states to file
std::vector<std::ofstream> GameStateFileArray;


// Automatically starts when UE4 is started.
// Populates the Token variable with the robot user's token.
void CloudyGameStateAPIImpl::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("CloudyGameStateAPI started"));

	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &CloudyGameStateAPIImpl::Cloudy_StateCheck), STATE_CHECK_INTERVAL);

	//Retrieve GEngine->CNumOfPlayersRC setting from DefaultGame.ini
	FString NumberOfPlayersString;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("NumOfPlayers"),
		NumberOfPlayersString,
		GGameIni
	);

	CNumOfPlayersOldAPI = FCString::Atoi(*NumberOfPlayersString);

	for (int i = 0; i < CNumOfPlayersOldAPI; i++)
	{
		std::string fileName = "test" + std::to_string(i) + ".txt";
		GameStateFileArray.emplace_back(std::ofstream{ fileName });
	}
}

// Automatically starts when UE4 is closed
void CloudyGameStateAPIImpl::ShutdownModule()
{
    UE_LOG(CloudyGameStateAPILog, Warning, TEXT("CloudyGameStateAPI stopped"));
}

void CloudyGameStateAPIImpl::IncreaseNumberOfPlayers()
{
	CNumOfPlayersOldAPI++;

	for (int i = CNumOfPlayersOldAPI-1; i < CNumOfPlayersOldAPI; i++)
	{
		std::string fileName = "test" + std::to_string(i) + ".txt";
		GameStateFileArray.emplace_back(std::ofstream{ fileName });
	}

	if (CNumOfPlayersOldAPI > NUM_PLAYERS)
	{
		UE_LOG(CloudyGameStateAPILog, Error, TEXT("CloudyGameStateAPI: Number of players exceeded max"));
	}
}

void CloudyGameStateAPIImpl::DecreaseNumberOfPlayers()
{
	CNumOfPlayersOldAPI--;

	check(CNumOfPlayersOldAPI >= 0);
}

bool CloudyGameStateAPIImpl::Cloudy_StateCheck(float DeltaTime)
{
	for (int i = 0; i < CNumOfPlayersOldAPI; ++i)
	{
		// Check if movement state has ended
		if (std::time(0) - TimeSinceLastMovement[i] > STATE_CHECK_INTERVAL)
		{
			GameStateTracker[i][INDEX_MOVEMENT] = WEIGHT_ZERO;
		}

		// Check if active state has ended
		if (!IsPressAndHoldToShoot && std::time(0) - TimeSinceLastActive[i] > STATE_CHECK_INTERVAL)
		{
			GameStateTracker[i][INDEX_ACTIVE] = WEIGHT_ZERO;
		}

		// Check if looking state has ended
		if (std::time(0) - TimeSinceLastLooking[i] > STATE_CHECK_INTERVAL)
		{
			GameStateTracker[i][INDEX_LOOKING] = WEIGHT_ZERO;
		}

		// Check for idle state
		for (int k = 0; k < NUM_STATES - 1; ++k)
		{
			if (GameStateTracker[i][k] != 0)
			{
				GameStateTracker[i][NUM_STATES - 1] = WEIGHT_ZERO;
				break;
			}
			if (k == NUM_STATES - 2)
			{
				GameStateTracker[i][INDEX_IDLE] = WEIGHT_IDLE;
			}
		}

		// Check the weight and write to file
		int Weight = Cloudy_GetTotalWeight(i);
		GameStateFileArray[i] << Weight << std::endl;
	}	

	return true;
}

int CloudyGameStateAPIImpl::Cloudy_FindIndex(UWorld* world)
{
	int index = -1;

	for (int i = 0; i < GEngine->GameViewportArray.Num(); i++)
	{
		if (GEngine->GameViewportArray[i] == world->GetGameViewport())
		{
			index = i;
			break;
		}
	}

	return index;
}

void CloudyGameStateAPIImpl::Cloudy_ActiveStart(UWorld* world, bool HasRelease)
{
	int index = Cloudy_FindIndex(world);

	if (!HasRelease)
	{
		TimeSinceLastActive[index] = std::time(0);
		IsPressAndHoldToShoot = HasRelease;
	}

	GameStateTracker[index][INDEX_ACTIVE] = WEIGHT_ACTIVE;
}

void CloudyGameStateAPIImpl::Cloudy_ActiveStop(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_ACTIVE] = WEIGHT_ZERO;
}

void CloudyGameStateAPIImpl::Cloudy_MovementStart(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	TimeSinceLastMovement[index] = std::time(0);
	GameStateTracker[index][INDEX_MOVEMENT] = WEIGHT_MOVEMENT;
}

void CloudyGameStateAPIImpl::Cloudy_MovementStop(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_MOVEMENT] = WEIGHT_ZERO;
}

void CloudyGameStateAPIImpl::Cloudy_LookingStart(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	TimeSinceLastLooking[index] = std::time(0);

	GameStateTracker[index][INDEX_LOOKING] = WEIGHT_LOOKING;
}

void CloudyGameStateAPIImpl::Cloudy_LookingStop(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_LOOKING] = WEIGHT_ZERO;
}

void CloudyGameStateAPIImpl::Cloudy_MovieStart(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_MOVIE] = WEIGHT_MOVIE;
}

void CloudyGameStateAPIImpl::Cloudy_MovieStop(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_MOVIE] = WEIGHT_ZERO;
}

void CloudyGameStateAPIImpl::Cloudy_MenuStart(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_MENU] = WEIGHT_MENU;
}

void CloudyGameStateAPIImpl::Cloudy_MenuStop(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_MENU] = WEIGHT_ZERO;
}

void CloudyGameStateAPIImpl::Cloudy_IdleStart(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_IDLE] = WEIGHT_IDLE;
}

void CloudyGameStateAPIImpl::Cloudy_IdleStop(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_IDLE] = WEIGHT_ZERO;
}

int CloudyGameStateAPIImpl::Cloudy_GetTotalWeight(int playerIndex)
{
	// SP Edit: sum weight
	int weight = 0;

	for (int i = 0; i < NUM_STATES; i++)
	{
		weight += GameStateTracker[playerIndex][i];
	}

	return weight;
}
 
IMPLEMENT_MODULE(CloudyGameStateAPIImpl, CloudyGameStateAPI)