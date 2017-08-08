#include "RemoteControllerPCH.h"

#include "RemoteControllerModule.h"

#include <string>
#include <fstream>
#include <ctime>
#include <vector>

#include "../../CloudyGameStateAPI/Public/ICloudyGameStateAPI.h"

#define CLOUDYGAME_REMOTE_CONTROLLER_SERVER_DEFAULT_ENDPOINT FIPv4Endpoint(FIPv4Address(0, 0, 0, 0), 55555)
#define BUFSIZE 512

DEFINE_LOG_CATEGORY(RemoteControllerLog)

TArray<UWorld*> WorldArray;

int CNumOfPlayersOldRC = 0;
float mouseX = 0.0f, mouseY = 0.0f;


void RemoteControllerModule::StartupModule()
{
	UE_LOG(RemoteControllerLog, Warning, TEXT("CloudyGame: RemoteController Module Starting"));

	const FString& SocketName = "RemoteControllerSocket";
	const FString& IPAddress = "0.0.0.0";
	const int32 Port = 55555;

	//Retrieve GEngine->CNumOfPlayersRC setting from DefaultGame.ini
	FString NumberOfPlayersString;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("NumOfPlayers"),
		NumberOfPlayersString,
		GGameIni
	);

	WorldArray.Init(NULL, FCString::Atoi(*NumberOfPlayersString));

	CNumOfPlayersOldRC = FCString::Atoi(*NumberOfPlayersString);
	
	InitializeRemoteServer(SocketName, IPAddress, Port);


}

void RemoteControllerModule::ShutdownModule()
{
	UE_LOG(RemoteControllerLog, Warning, TEXT("CloudyGame: RemoteController Module Shutting Down"));

    delete UDPInputReceiver;
    UDPInputReceiver = nullptr;

    if (ServerListenSocket)
    {
        ServerListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ServerListenSocket);
    }
}

void RemoteControllerModule::IncreaseArraySize() 
{
	if (GEngine->CNumberOfPlayers != CNumOfPlayersOldRC)
	{
		if (GEngine->CNumberOfPlayers > WorldArray.Num())
		{
			for (int i = CNumOfPlayersOldRC; i < GEngine->CNumberOfPlayers; i++)
			{
				UE_LOG(RemoteControllerLog, Warning, TEXT("Increasing WorldArray to include index %d"), i);
				WorldArray.Add(NULL);
			}
		}
	}

	CNumOfPlayersOldRC = GEngine->CNumberOfPlayers;

}

void RemoteControllerModule::DecreaseArraySize()
{
	UE_LOG(RemoteControllerLog, Warning, TEXT("DecreaseArraySize. CNumberOfPlayers = %d, CNumOfPlayersOldRC = %d"), GEngine->CNumberOfPlayers, CNumOfPlayersOldRC);
	if (GEngine->CNumberOfPlayers != CNumOfPlayersOldRC)
	{
		// Used to have 2 players (player 0 and player 1)
		// I remove player 1
		// 1 < 2, so branch succeeds
		if (GEngine->CNumberOfPlayers < WorldArray.Num())
		{
			// Old value should be 2
			// for i = 1; i < 2
			for (int i = GEngine->CNumberOfPlayers; i < CNumOfPlayersOldRC; i++) // needs to be verified if correct
			{
				UE_LOG(RemoteControllerLog, Warning, TEXT("NULLing WorldArray[%d]"), i);
				WorldArray[i] = NULL;
			}
			CNumOfPlayersOldRC = GEngine->CNumberOfPlayers;
		}
	}
}

void RemoteControllerModule::InitializeRemoteServer(const FString& SocketName, const FString& IPAddress, const int32 Port)
{
    FIPv4Address ParsedIP;
    FIPv4Address::Parse(IPAddress, ParsedIP);

    UE_LOG(RemoteControllerLog, Warning, TEXT("CloudyGame: RemoteController Server Starting"));

    // Create Socket
    FIPv4Endpoint Endpoint(ParsedIP, Port);
    FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
    ServerListenSocket = FUdpSocketBuilder(SocketName).AsNonBlocking().AsReusable().BoundToEndpoint(Endpoint);
    UDPInputReceiver = new FUdpSocketReceiver(ServerListenSocket, ThreadWaitTime, TEXT("Udp Input Receiver"));
    UDPInputReceiver->OnDataReceived().BindRaw(this, &RemoteControllerModule::HandleInputReceived);
    UDPInputReceiver->Start(); // New in UE4 4.13
    
    UE_LOG(RemoteControllerLog, Warning, TEXT("CloudyGame: RemoteController Server Started Successfully"));
}

void RemoteControllerModule::ProcessKeyboardInput(const FArrayReaderPtr& Data)
{    
    FUdpRemoteControllerSegment::FKeyboardInputChunk Chunk;
	*Data << Chunk;

	if (Chunk.ControllerID < GEngine->GameViewportArray.Num() && Chunk.ControllerID < WorldArray.Num())
	{
		WorldArray[Chunk.ControllerID] = GEngine->GameViewportArray[Chunk.ControllerID]->GetGameInstance()->GetWorld();

		if (WorldArray[Chunk.ControllerID] != NULL)
		{
			APlayerController* controller = UGameplayStatics::GetPlayerController(WorldArray[Chunk.ControllerID], 0);
			if (controller != nullptr)
			{
				EInputEvent ie;
				if (Chunk.InputEvent == 2) { // Pressed
					ie = EInputEvent::IE_Pressed;
				}
				else if (Chunk.InputEvent == 3) { // Released
					ie = EInputEvent::IE_Released;
				}

				FKey key;
				if (Chunk.KeyCode == 999)
				{
					key = EKeys::MouseScrollUp;
				}
				else if (Chunk.KeyCode == 998)
				{
					key = EKeys::MouseScrollDown;
				}
				else
				{
					key = FInputKeyManager::Get().GetKeyFromCodes(Chunk.KeyCode, Chunk.CharCode);
				}

				if (Chunk.KeyCode == 1) { // send left mouse clicks by Slate

					AsyncTask(ENamedThreads::GameThread, [Chunk]()
					{
						AGameModeBase* GameMode = WorldArray[Chunk.ControllerID]->GetAuthGameMode();
						if (GameMode != NULL)
						{
							if (!FSlateApplication::Get().GetActiveTopLevelWindow().IsValid()) 
							{
								TArray<TSharedRef<SWindow>> wins;
								FSlateApplication::Get().GetAllVisibleWindowsOrdered(wins);
								wins[0]->BringToFront(true);
								FSlateApplication::Get().OnMouseDown(wins[0]->GetNativeWindow(), EMouseButtons::Left);
								FSlateApplication::Get().OnMouseUp(EMouseButtons::Left);
							}
							else
							{
								TSharedPtr<FGenericWindow> win = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow();
								FSlateApplication::Get().OnMouseDown(win, EMouseButtons::Left);
								FSlateApplication::Get().OnMouseUp(EMouseButtons::Left);
							}
						}
					});
					
				}
				else {
					controller->InputKey(key, ie, 1, false);
				}
				
			}
		}
	}
}


void RemoteControllerModule::ProcessMouseInput(const FArrayReaderPtr& Data)
{
    FUdpRemoteControllerSegment::FMouseInputChunk Chunk;
	*Data << Chunk;

	if (Chunk.ControllerID < GEngine->GameViewportArray.Num() && Chunk.ControllerID < WorldArray.Num())
	{
		WorldArray[Chunk.ControllerID] = GEngine->GameViewportArray[Chunk.ControllerID]->GetGameInstance()->GetWorld();

		if (WorldArray[Chunk.ControllerID] != NULL)
		{
			APlayerController* controller = UGameplayStatics::GetPlayerController(WorldArray[Chunk.ControllerID], 0);

			// InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
			if (controller != nullptr)
			{
				if (Chunk.XAxis != NULL)
				{
					controller->InputAxis(EKeys::MouseX, Chunk.XAxis-mouseX, WorldArray[Chunk.ControllerID]->GetDeltaSeconds(), 1, false);
					mouseX = Chunk.XAxis;
				}
				if (Chunk.YAxis != NULL)
				{
					controller->InputAxis(EKeys::MouseY, -(Chunk.YAxis-mouseY), WorldArray[Chunk.ControllerID]->GetDeltaSeconds(), 1, false);
					mouseY = Chunk.YAxis;
				}

				AsyncTask(ENamedThreads::GameThread, [Chunk]()
				{
					// update mouse position

					AGameModeBase* GameMode = WorldArray[Chunk.ControllerID]->GetAuthGameMode();
					if (GameMode != NULL)
					{
						APlayerController* ctr = UGameplayStatics::GetPlayerController(WorldArray[Chunk.ControllerID], 0);
						UGameplayStatics::GetPlayerController(WorldArray[Chunk.ControllerID], 0);
						ctr->SetMouseLocation(mouseX, mouseY);
					}
				});
			}
		}
	}
}

void RemoteControllerModule::HandleInputReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Sender)
{
	FUdpRemoteControllerSegment::FHeaderChunk Chunk;
	*Data << Chunk;

    switch (Chunk.SegmentType)
    {
		case EUdpRemoteControllerSegment::KeyboardInput:
    		ProcessKeyboardInput(Data);
    		break;
		case EUdpRemoteControllerSegment::MouseInput:
    		ProcessMouseInput(Data);
       		break;
    }
}

IMPLEMENT_MODULE(RemoteControllerModule, CloudyRemoteController)