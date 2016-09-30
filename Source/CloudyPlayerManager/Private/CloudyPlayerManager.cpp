//TCP implementation from : https ://wiki.unrealengine.com/TCP_Socket_Listener,_Receive_Binary_Data_From_an_IP/Port_Into_UE4,_(Full_Code_Sample)

#include "CloudyPlayerManagerPrivatePCH.h"
#include "CloudyPlayerManager.h"
#include "../../CloudyStream/Public/CloudyStream.h"
#include "../../CloudyWebConnector/Public/ICloudyWebConnector.h"


#include <stdio.h>
#include <stdlib.h>
#include <string>

#define LOCTEXT_NAMESPACE "CCloudyPlayerManagerModule"

DEFINE_LOG_CATEGORY(ModuleLog)


#define DELETE_URL "/game-session/"
#define DELETE_REQUEST "DELETE"
#define MAX_PLAYERS 6


void CCloudyPlayerManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	UE_LOG(ModuleLog, Warning, TEXT("CloudyPlayerManager started"));

	// initialise game session id mapping
	int GameSessionIdMapping[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		GameSessionIdMapping[i] = -1;
	}
}

void CCloudyPlayerManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.	
	

}


bool CCloudyPlayerManagerModule::ExecuteCommand(FString Command,
	int32 ControllerId, int32 StreamingPort, FString StreamingIP, int32 GameSessionId)
{
	if (Command == "join")
	{
		return AddPlayer(ControllerId, StreamingPort, StreamingIP, GameSessionId);
	}
	else if (Command == "quit")
	{
		return RemovePlayer(ControllerId, GameSessionId);
	}
	else
	{
		return false;
	}
}


bool CCloudyPlayerManagerModule::AddPlayer(int32 ControllerId, int32 StreamingPort,
	FString StreamingIP, int32 GameSessionId)
{
	UGameInstance* GameInstance = GEngine->GameViewport->GetGameInstance();
	FString Error;
	GameInstance->CreateLocalPlayer(ControllerId, Error, true);

	if (Error.Len() == 0) // success. no error message
	{
		CloudyStreamImpl::Get().StartPlayerStream(ControllerId, StreamingPort, StreamingIP);
		GameSessionIdMapping[ControllerId] = GameSessionId;
		return true;
	}

	return false;

}


bool CCloudyPlayerManagerModule::RemovePlayer(int32 ControllerId, int32 GameSessionId)
{
	UGameInstance* GameInstance = GEngine->GameViewport->GetGameInstance();
	ULocalPlayer* const ExistingPlayer = GameInstance->FindLocalPlayerFromControllerId(ControllerId);
	bool Success = false;

	if (ExistingPlayer != NULL)
	{
		UE_LOG(ModuleLog, Warning, TEXT("Controller Id: %d"), ControllerId);

		// destroy the quitting player's pawn
		APlayerController* Controller = ExistingPlayer->PlayerController;
		Controller->GetPawn()->Destroy();

		// delete appropriate game session
		int32 GameSessionId = GameSessionIdMapping[ControllerId];
		FString GameSessionString = DELETE_URL + FString::FromInt(GameSessionId) + "/";
		UE_LOG(ModuleLog, Warning, TEXT("Game Session string: %s"), *GameSessionString);
		Success = ICloudyWebConnector::Get().MakeRequest(GameSessionString, DELETE_REQUEST);

		// check for successful removal from server before removing
		if (Success)
		{
			CloudyStreamImpl::Get().StopPlayerStream(ControllerId);
			GameSessionIdMapping[ControllerId] = -1;
			return GameInstance->RemoveLocalPlayer(ExistingPlayer);
		}
	}

	return false;
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(CCloudyPlayerManagerModule, CloudyPlayerManager)
