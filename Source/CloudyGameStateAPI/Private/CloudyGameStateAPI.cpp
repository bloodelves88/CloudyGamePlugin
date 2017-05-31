#include "CloudyGameStateAPIPrivatePCH.h"
 
#include "CloudyGameStateAPI.h"


DEFINE_LOG_CATEGORY(CloudyGameStateAPILog);

/* Currently 6 states. The indexes are used for these:
 *	   Index 0: Shooting
 *     Index 1: Movement
 *     Index 2: Looking
 *     Index 3: Movie
 *     Index 4: Idle
 *     Index 5: Menu
 */
TArray<int> GameStateTracker;

// Automatically starts when UE4 is started.
// Populates the Token variable with the robot user's token.
void CloudyGameStateAPIImpl::StartupModule()
{
	GameStateTracker.Init(false, 6);
	UE_LOG(CloudyGameStateAPILog, Warning, TEXT("CloudyGameStateAPI started"));
}

// Automatically starts when UE4 is closed
void CloudyGameStateAPIImpl::ShutdownModule()
{
    UE_LOG(CloudyGameStateAPILog, Warning, TEXT("CloudyGameStateAPI stopped"));
}

void CloudyGameStateAPIImpl::Cloudy_ShootingStart()
{
	GameStateTracker[0] = 3;
}

void CloudyGameStateAPIImpl::Cloudy_ShootingEnd()
{
	GameStateTracker[0] = 0;
}

void CloudyGameStateAPIImpl::Cloudy_MovementStart()
{
	GameStateTracker[1] = 2;
}

void CloudyGameStateAPIImpl::Cloudy_MovementEnd()
{
	GameStateTracker[1] = 0;
}

void CloudyGameStateAPIImpl::Cloudy_LookingStart()
{
	GameStateTracker[2] = 2;
}

void CloudyGameStateAPIImpl::Cloudy_LookingEnd()
{
	GameStateTracker[2] = 0;
}

void CloudyGameStateAPIImpl::Cloudy_MovieStart()
{
	GameStateTracker[3] = 2;
}

void CloudyGameStateAPIImpl::Cloudy_MovieEnd()
{
	GameStateTracker[3] = 0;
}

void CloudyGameStateAPIImpl::Cloudy_IdleStart()
{
	GameStateTracker[4] = 1;
}

void CloudyGameStateAPIImpl::Cloudy_IdleEnd()
{
	GameStateTracker[4] = 0;
}

void CloudyGameStateAPIImpl::Cloudy_MenuStart()
{
	GameStateTracker[5] = 1;
}

void CloudyGameStateAPIImpl::Cloudy_MenuEnd()
{
	GameStateTracker[5] = 0;
}

int CloudyGameStateAPIImpl::Cloudy_GetWeight()
{
	int weight = 0;
	for (int i = 0; i < GameStateTracker.Num(); ++i)
	{
		if (GameStateTracker[i] > weight)
		{
			weight = GameStateTracker[i];
		}
	}
	return weight;
}
 
IMPLEMENT_MODULE(CloudyGameStateAPIImpl, CloudyGameStateAPI)