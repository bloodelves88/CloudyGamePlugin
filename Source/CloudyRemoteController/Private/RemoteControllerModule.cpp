#include "RemoteControllerPCH.h"

#include "RemoteControllerModule.h"

#include <string>
#include <fstream>
#include <ctime>
#include <vector>

#define CLOUDYGAME_REMOTE_CONTROLLER_SERVER_DEFAULT_ENDPOINT FIPv4Endpoint(FIPv4Address(0, 0, 0, 0), 55555)
#define BUFSIZE 512

DEFINE_LOG_CATEGORY(RemoteControllerLog)

TArray<UWorld*> WorldArray;

std::vector<std::ofstream> PlayerInputFileArray;
time_t timeLastWrittenMouseMovement = std::time(0);
time_t timeLastWrittenKeyboardInput = std::time(0);

int CNumOfPlayersOldRC = 0;

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
	
	for (int i = 0; i < FCString::Atoi(*NumberOfPlayersString); i++)
	{
		std::string fileName = "test" + std::to_string(i) + ".txt";
		PlayerInputFileArray.emplace_back(std::ofstream{ fileName });
	}
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
				std::string fileName = "test" + std::to_string(i) + ".txt";
				PlayerInputFileArray.emplace_back(std::ofstream{ fileName });
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

	// Mouse left button down keycode and charcode = 1
	// Mouse right button down keycode and charcode = 2
	// Mouse button up keycode and charcode = 0

	// Mouse buttons
	if (Chunk.KeyCode == 1 || Chunk.KeyCode == 2 || Chunk.KeyCode == 0)
	{
		if (Chunk.ControllerID < PlayerInputFileArray.size())
		{
			PlayerInputFileArray[Chunk.ControllerID] << 3 << std::endl;
		}
	}
	// Keyboard buttons
	else if (std::time(0) - timeLastWrittenKeyboardInput >= 1)
	{
		if (Chunk.ControllerID < PlayerInputFileArray.size())
		{
			timeLastWrittenKeyboardInput = std::time(0);
			PlayerInputFileArray[Chunk.ControllerID] << 2 << std::endl;
		}
	}
	if (Chunk.ControllerID < GEngine->GameViewportArray.Num() && Chunk.ControllerID < WorldArray.Num())
	{
		// If world has not been loaded yet
		if (WorldArray[Chunk.ControllerID] == NULL)
		{
			WorldArray[Chunk.ControllerID] = GEngine->GameViewportArray[Chunk.ControllerID]->GetGameInstance()->GetWorld();
		}
		else
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

				FKey key = FInputKeyManager::Get().GetKeyFromCodes(Chunk.KeyCode, Chunk.CharCode);
			
				controller->InputKey(key, ie, 1, false);
			}
		}
	}
}


void RemoteControllerModule::ProcessMouseInput(const FArrayReaderPtr& Data)
{
    FUdpRemoteControllerSegment::FMouseInputChunk Chunk;
	*Data << Chunk;

	if (std::time(0) - timeLastWrittenMouseMovement >= 1)
	{
		if (Chunk.ControllerID < PlayerInputFileArray.size())
		{
			timeLastWrittenMouseMovement = std::time(0);
			PlayerInputFileArray[Chunk.ControllerID] << 2 << std::endl;
		}
	}

	if (Chunk.ControllerID < GEngine->GameViewportArray.Num() && Chunk.ControllerID < WorldArray.Num())
	{
		// If world has not been loaded yet
		if (WorldArray[Chunk.ControllerID] == NULL)
		{
			int id = Chunk.ControllerID;

			WorldArray[Chunk.ControllerID] = GEngine->GameViewportArray[Chunk.ControllerID]->GetGameInstance()->GetWorld();
		}
		else
		{
			APlayerController* controller = UGameplayStatics::GetPlayerController(WorldArray[Chunk.ControllerID], 0);

			// InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
			if (controller != nullptr)
			{
				if (Chunk.XAxis != NULL)
				{
					controller->InputAxis(EKeys::MouseX, Chunk.XAxis, WorldArray[Chunk.ControllerID]->GetDeltaSeconds(), 1, false);
				}
				if (Chunk.YAxis != NULL)
				{
					controller->InputAxis(EKeys::MouseY, -Chunk.YAxis, WorldArray[Chunk.ControllerID]->GetDeltaSeconds(), 1, false);
				}
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