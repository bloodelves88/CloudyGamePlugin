// Some copyright should be here...
/*=============================================================================
	CloudPanelPlugin.cpp: Implementation of CloudyPanel TCP Plugin
=============================================================================*/
//TCP implementation from : https ://wiki.unrealengine.com/TCP_Socket_Listener,_Receive_Binary_Data_From_an_IP/Port_Into_UE4,_(Full_Code_Sample)

#include "CloudyPanelPluginPrivatePCH.h"
#include "CloudyPanelPlugin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

#define LOCTEXT_NAMESPACE "CCloudyPanelPluginModule"

DEFINE_LOG_CATEGORY(ModuleLog)

#define SERVER_NAME "Listener"
#define SERVER_ENDPOINT FIPv4Endpoint(FIPv4Address(127, 0, 0, 1), 55556)
#define CONNECTION_THREAD_TIME 1 // in seconds
#define BUFFER_SIZE 1024
#define SUCCESS_MSG "Success"
#define FAILURE_MSG "Failure"


void CCloudyPanelPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Set up to receive commands from CloudyWeb/CloudyPanel
	// Start timer function to check on client connection
	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &CCloudyPanelPluginModule::Tick), CONNECTION_THREAD_TIME);

	// start the server (listener)

	//Create Socket
	FIPv4Endpoint Endpoint(SERVER_ENDPOINT);
	ListenSocket = FTcpSocketBuilder(SERVER_NAME).AsReusable().BoundToEndpoint(Endpoint).Listening(8);

	//Set Buffer Size
	int32 NewSize = 0;
	ListenSocket->SetReceiveBufferSize(BUFFER_SIZE, NewSize);

	TcpListener = new FTcpListener(*ListenSocket, CONNECTION_THREAD_TIME);
	TcpListener->OnConnectionAccepted().BindRaw(this, &CCloudyPanelPluginModule::InputHandler);

	// initialise class variables
	InputStr = "";
	HasInputStrChanged = false;
	
}


void CCloudyPanelPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.	
	delete TcpListener;
	ListenSocket -> Close();

}


bool CCloudyPanelPluginModule::Tick(float DeltaTime)
{
	bool Success = false;

	if (HasInputStrChanged) {

		if (GEngine->GameViewport != nullptr && GIsRunning && IsInGameThread())
		{

			// Split input into Command and ControllerId
			FString CommandStr = InputStr.Mid(0, 4);
			FString ControllerIdStr = InputStr.Mid(4, 4);
			int32 Command = FCString::Atoi(*CommandStr);
			int32 ControllerId = FCString::Atoi(*ControllerIdStr);
			
			UE_LOG(ModuleLog, Warning, TEXT("Command: %d ControllerId: %d"), Command, ControllerId);
			
			Success = ExecuteCommand(Command, ControllerId);

			InputStr = "";
			HasInputStrChanged = false;
		}

		// Send response to client
		if (Success) 
		{
			SendToClient(TCPConnection, SUCCESS_MSG);
		}
		else 
		{
			SendToClient(TCPConnection, FAILURE_MSG);
		}

	}

	return true;
}


bool CCloudyPanelPluginModule::SendToClient(FSocket* Socket, FString Msg)
{
	TCHAR *serialisedChar = Msg.GetCharArray().GetData();
	int32 size = FCString::Strlen(serialisedChar);
	int32 sent = 0;
	return Socket->Send((uint8*)TCHAR_TO_UTF8(serialisedChar), size, sent);

	return true;
}


//Rama's String From Binary Array
//This function requires #include <string>
FString CCloudyPanelPluginModule::StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	//Create a string from a byte array!
	std::string cstr(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(cstr.c_str());
}


bool CCloudyPanelPluginModule::InputHandler(FSocket* ConnectionSocket, const FIPv4Endpoint& Endpoint)
{

	TArray<uint8> ReceivedData;
	uint32 Size;

	// wait for data to arrive
	while (!(ConnectionSocket->HasPendingData(Size))) {
		Sleep(1000);
	}

	// handle data - change current command
	ReceivedData.Init(FMath::Min(Size, 65507u));

	int32 Read = 0;
	ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
	FString ReceivedString = StringFromBinaryArray(ReceivedData);
	// UE_LOG(ModuleLog, Warning, TEXT("Data: %s"), *ReceivedString);

	InputStr = ReceivedString;
	HasInputStrChanged = true;

	TCPConnection = ConnectionSocket;

	return true;

}


bool CCloudyPanelPluginModule::ExecuteCommand(int32 Command, int32 ControllerId)
{
	bool Success = false;
	if (GEngine)
	{
		UGameInstance* GameInstance = GEngine->GameViewport->GetGameInstance();

		if (Command == JOIN_GAME)
		{
			FString Error;
			GameInstance->CreateLocalPlayer(ControllerId, Error, true);

			if (Error.Len() == 0) 
			{
				Success = true;
			}
			else 
			{
				Success = false;
			}
		}
		else if (Command == QUIT_GAME)
		{
			ULocalPlayer* const ExistingPlayer = GameInstance->FindLocalPlayerFromControllerId(ControllerId);
			if (ExistingPlayer != NULL)
			{
				GameInstance->RemoveLocalPlayer(ExistingPlayer);
				Success = true;
			}
			else 
			{
				Success = false;
			}
		}
	}

	return Success;
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(CCloudyPanelPluginModule, CloudyPanelPlugin)