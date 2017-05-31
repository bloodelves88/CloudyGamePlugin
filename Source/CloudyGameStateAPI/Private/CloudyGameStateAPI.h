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

	void Cloudy_ShootingStart();
	void Cloudy_ShootingEnd();
	
	void Cloudy_MovementStart();
	void Cloudy_MovementEnd();
	
	void Cloudy_LookingStart();
	void Cloudy_LookingEnd();
	
	void Cloudy_MovieStart();
	void Cloudy_MovieEnd();
	
	void Cloudy_IdleStart();
	void Cloudy_IdleEnd();
	
	void Cloudy_MenuStart();
	void Cloudy_MenuEnd();

	int Cloudy_GetWeight();
};
