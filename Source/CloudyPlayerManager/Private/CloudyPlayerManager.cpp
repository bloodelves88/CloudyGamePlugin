//TCP implementation from : https ://wiki.unrealengine.com/TCP_Socket_Listener,_Receive_Binary_Data_From_an_IP/Port_Into_UE4,_(Full_Code_Sample)


#include "CloudyPlayerManagerPrivatePCH.h"
#include "CloudyPlayerManager.h"
#include "../../CloudyRemoteController/Public/IRemoteControllerModule.h"
#include "../../CloudyWebConnector/Public/ICloudyWebConnector.h"
#include "../../CloudyGameStateAPI/Public/ICloudyGameStateAPI.h"

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

		AddPlayer(ControllerId);
		return true; 
	}
	else if (Command == "quit")
	{
		UE_LOG(ModuleLog, Warning, TEXT("CloudyPlayerManager: quit command received"));
		RemovePlayer(ControllerId);
		return true;
	}
	else
	{
		return false;
	}
}

bool CCloudyPlayerManagerModule::AddPlayer(int32 ControllerId)
{
	GEngine->CNumberOfPlayers += 1;
	UE_LOG(ModuleLog, Warning, TEXT("CloudyPlayerManager: Increase CNumberOfPlayers by 1. Value is now %d"), GEngine->CNumberOfPlayers);
	IRemoteControllerModule::Get().IncreaseArraySize();
	ICloudyGameStateAPI::Get().IncreaseNumberOfPlayers();

	return true;
}


bool CCloudyPlayerManagerModule::RemovePlayer(int32 ControllerId)
{
	GEngine->CNumberOfPlayers -= 1;
	IRemoteControllerModule::Get().DecreaseArraySize();
	ICloudyGameStateAPI::Get().DecreaseNumberOfPlayers();

	// Call some engine function to close window.
	FSlateApplication::Get().CloudyOnWindowClose(GEngine->GameViewportArray[ControllerId]->GetWindow());

	return false;
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(CCloudyPlayerManagerModule, CloudyPlayerManager)
