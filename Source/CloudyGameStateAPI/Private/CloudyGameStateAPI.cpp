#include "CloudyGameStateAPIPrivatePCH.h"
 
#include "CloudyGameStateAPI.h"

#include <ctime>

DEFINE_LOG_CATEGORY(CloudyGameStateAPILog);

/* Currently 6 states. The indexes are used for these:
 *	   Index 0: Shooting
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

#define NUM_PLAYERS 4
#define NUM_STATES 6

#define INDEX_SHOOTING 0
#define INDEX_MOVEMENT 1
#define INDEX_LOOKING 2
#define INDEX_MOVIE 3
#define INDEX_MENU 4
#define INDEX_IDLE 5

#define WEIGHT_SHOOTING 3
#define WEIGHT_MOVEMENT 2
#define WEIGHT_LOOKING 2
#define WEIGHT_MOVIE 2
#define WEIGHT_MENU 1
#define WEIGHT_IDLE 1
#define WEIGHT_ZERO 0

#define STATE_CHECK_INTERVAL 1.0

int GameStateTracker[NUM_PLAYERS][NUM_STATES];

// Shooting state
time_t TimeSinceLastShooting[NUM_PLAYERS];
bool IsPressAndHoldToShoot = true;

// Movement state
time_t TimeSinceLastMovement[NUM_PLAYERS];

// Looking state
time_t TimeSinceLastLooking[NUM_PLAYERS];

// Automatically starts when UE4 is started.
// Populates the Token variable with the robot user's token.
void CloudyGameStateAPIImpl::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("CloudyGameStateAPI started"));

	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &CloudyGameStateAPIImpl::Cloudy_StateCheck), STATE_CHECK_INTERVAL);
}

// Automatically starts when UE4 is closed
void CloudyGameStateAPIImpl::ShutdownModule()
{
    UE_LOG(CloudyGameStateAPILog, Warning, TEXT("CloudyGameStateAPI stopped"));
}

bool CloudyGameStateAPIImpl::Cloudy_StateCheck(float DeltaTime)
{
	
	for (int i = 0; i < NUM_PLAYERS; ++i)
	{
		// Check for idle state
		for (int k = 0; k < NUM_STATES - 1; ++k)
		{
			if (GameStateTracker[i][k] != 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Player %d is not idle"), i);
				GameStateTracker[i][NUM_STATES-1] = WEIGHT_ZERO;
				break;
			}
			if (k == NUM_STATES - 2)
			{
				UE_LOG(LogTemp, Warning, TEXT("Player %d is idle"), i);
				GameStateTracker[i][INDEX_IDLE] = WEIGHT_IDLE;
			}
		}

		// Check if movement state has ended
		if (std::time(0) - TimeSinceLastMovement[i] > STATE_CHECK_INTERVAL)
		{
			GameStateTracker[i][INDEX_MOVEMENT] = WEIGHT_ZERO;
		}

		// Check if shooting state has ended
		if (!IsPressAndHoldToShoot && std::time(0) - TimeSinceLastShooting[i] > STATE_CHECK_INTERVAL)
		{
			GameStateTracker[i][INDEX_SHOOTING] = WEIGHT_ZERO;
		}

		// Check if looking state has ended
		if (std::time(0) - TimeSinceLastLooking[i] > STATE_CHECK_INTERVAL)
		{
			GameStateTracker[i][INDEX_LOOKING] = WEIGHT_ZERO;
		}
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

void CloudyGameStateAPIImpl::Cloudy_ShootingStart(UWorld* world, bool HasRelease)
{
	//UE_LOG(LogTemp, Warning, TEXT("Window title = %s"), *world->GetGameViewport()->GetWindow()->GetTitle().ToString());

	int index = Cloudy_FindIndex(world);

	if (!HasRelease)
	{
		TimeSinceLastShooting[index] = std::time(0);
		IsPressAndHoldToShoot = HasRelease;
	}

	GameStateTracker[index][INDEX_SHOOTING] = WEIGHT_SHOOTING;

	UE_LOG(LogTemp, Warning, TEXT("Window title = %d"), index);
	UE_LOG(LogTemp, Warning, TEXT("GameStateTracker = [%d, %d, %d, %d, %d, %d]"), 
		GameStateTracker[index][INDEX_SHOOTING], GameStateTracker[index][INDEX_MOVEMENT], GameStateTracker[index][INDEX_LOOKING],
		GameStateTracker[index][INDEX_MOVIE], GameStateTracker[index][INDEX_MENU], GameStateTracker[index][INDEX_IDLE]);
}

void CloudyGameStateAPIImpl::Cloudy_ShootingStop(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_SHOOTING] = WEIGHT_ZERO;

	UE_LOG(LogTemp, Warning, TEXT("GameStateTracker = [%d, %d, %d, %d, %d, %d]"),
		GameStateTracker[index][INDEX_SHOOTING], GameStateTracker[index][INDEX_MOVEMENT], GameStateTracker[index][INDEX_LOOKING],
		GameStateTracker[index][INDEX_MOVIE], GameStateTracker[index][INDEX_MENU], GameStateTracker[index][INDEX_IDLE]);
}

void CloudyGameStateAPIImpl::Cloudy_MovementStart(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	TimeSinceLastMovement[index] = std::time(0);
	GameStateTracker[index][INDEX_MOVEMENT] = WEIGHT_MOVEMENT;

	UE_LOG(LogTemp, Warning, TEXT("GameStateTracker = [%d, %d, %d, %d, %d, %d]"),
		GameStateTracker[index][INDEX_SHOOTING], GameStateTracker[index][INDEX_MOVEMENT], GameStateTracker[index][INDEX_LOOKING],
		GameStateTracker[index][INDEX_MOVIE], GameStateTracker[index][INDEX_MENU], GameStateTracker[index][INDEX_IDLE]);
}

void CloudyGameStateAPIImpl::Cloudy_MovementStop(UWorld* world)
{
	int index = Cloudy_FindIndex(world);

	GameStateTracker[index][INDEX_MOVEMENT] = WEIGHT_ZERO;

	UE_LOG(LogTemp, Warning, TEXT("GameStateTracker = [%d, %d, %d, %d, %d, %d]"),
		GameStateTracker[index][INDEX_SHOOTING], GameStateTracker[index][INDEX_MOVEMENT], GameStateTracker[index][INDEX_LOOKING],
		GameStateTracker[index][INDEX_MOVIE], GameStateTracker[index][INDEX_MENU], GameStateTracker[index][INDEX_IDLE]);
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

int CloudyGameStateAPIImpl::Cloudy_GetWeight(int playerIndex)
{
	int weight = -1;
	for (int i = 0; i < sizeof(GameStateTracker) / sizeof(GameStateTracker[0]); ++i)
	{
		if (GameStateTracker[playerIndex][i] > weight)
		{
			weight = GameStateTracker[playerIndex][i];
		}
	}
	return weight;
}
 
IMPLEMENT_MODULE(CloudyGameStateAPIImpl, CloudyGameStateAPI)