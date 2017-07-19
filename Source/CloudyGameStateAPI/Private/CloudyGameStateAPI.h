#pragma once
#include "Engine.h"
#include "Http.h"
#include "Networking.h"
#include "Sockets.h"

DECLARE_LOG_CATEGORY_EXTERN(CloudyGameStateAPILog, Log, All);
 
class CloudyGameStateAPIImpl : public ICloudyGameStateAPI
{

public:
    void StartupModule();
    void ShutdownModule();

	void IncreaseNumberOfPlayers();
	void DecreaseNumberOfPlayers();

	void Cloudy_ActiveStart(UWorld* world, bool HasRelease);
	void Cloudy_ActiveStop(UWorld* world);
	
	void Cloudy_MovementStart(UWorld* world);
	void Cloudy_MovementStop(UWorld* world);
	
	void Cloudy_LookingStart(UWorld* world);
	void Cloudy_LookingStop(UWorld* world);
	
	void Cloudy_MovieStart(UWorld* world);
	void Cloudy_MovieStop(UWorld* world);
	
	void Cloudy_IdleStart(UWorld* world);
	void Cloudy_IdleStop(UWorld* world);
	
	void Cloudy_MenuStart(UWorld* world);
	void Cloudy_MenuStop(UWorld* world);

private:
	int Cloudy_FindIndex(UWorld* world);
	bool Cloudy_StateCheck(float DeltaTime);
	int Cloudy_GetLargestWeight(int playerIndex);
};
