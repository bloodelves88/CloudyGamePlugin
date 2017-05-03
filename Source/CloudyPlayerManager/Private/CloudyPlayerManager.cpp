//TCP implementation from : https ://wiki.unrealengine.com/TCP_Socket_Listener,_Receive_Binary_Data_From_an_IP/Port_Into_UE4,_(Full_Code_Sample)

#include "CloudyPlayerManagerPrivatePCH.h"
#include "CloudyPlayerManager.h"
#include "../../CloudyStream/Public/CloudyStream.h"
#include "../../CloudyWebConnector/Public/ICloudyWebConnector.h"

#include <fstream>
#include <iostream>
#include <string>

#define LOCTEXT_NAMESPACE "CCloudyPlayerManagerModule"

DEFINE_LOG_CATEGORY(ModuleLog)

void CCloudyPlayerManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	UE_LOG(ModuleLog, Warning, TEXT("CloudyPlayerManager started"));
}

void CCloudyPlayerManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.	
}

bool CCloudyPlayerManagerModule::ExecuteCommand(FString Command, int32 ControllerId)
{
	if (Command == "join")
	{
		UE_LOG(ModuleLog, Warning, TEXT("CloudyPlayerManager: join command received"));

		if (ControllerId == 0)
		{
			std::ofstream TimerFile("TimerLog.txt", std::ios::out | std::ios::trunc);
			TimerFile << "Start (index " << ControllerId << ") = " << GetTickCount() << std::endl;
		}
		else
		{
			std::ofstream TimerFile("TimerLog.txt", std::ios::out | std::ios::app);
			TimerFile << "Start (index " << ControllerId << ") = " << GetTickCount() << std::endl;
		}

		AddPlayer(ControllerId);
		return true; 
	}
	else if (Command == "quit")
	{
		UE_LOG(ModuleLog, Warning, TEXT("CloudyPlayerManager: quit command received"));
		return true;// RemovePlayer(ControllerId);
	}
	else
	{
		return false;
	}
}

bool CCloudyPlayerManagerModule::AddPlayer(int32 ControllerId)
{
	GEngine->CNumberOfPlayers += 1;

	return true;
}


bool CCloudyPlayerManagerModule::RemovePlayer(int32 ControllerId)
{
	UGameInstance* GameInstance = GEngine->GameViewportArray[ControllerId]->GetGameInstance();
	ULocalPlayer* const ExistingPlayer = GameInstance->FindLocalPlayerFromControllerId(ControllerId);
	bool Success = false;

	if (ExistingPlayer != NULL)
	{
		UE_LOG(ModuleLog, Warning, TEXT("Controller Id: %d"), ControllerId);

		// destroy the quitting player's pawn
		APlayerController* Controller = ExistingPlayer->PlayerController;
        Controller->FlushPressedKeys(); // Resetting them will prevent them from "sticking"
		Controller->GetPawn()->Destroy();

		return GameInstance->RemoveLocalPlayer(ExistingPlayer);
	}

	return false;
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(CCloudyPlayerManagerModule, CloudyPlayerManager)
