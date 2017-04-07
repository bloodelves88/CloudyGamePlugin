#include "RemoteControllerPCH.h"

#include "RemoteControllerModule.h"

#include <string>
#include <fstream>
#include <sstream>

#define CLOUDYGAME_REMOTE_CONTROLLER_SERVER_DEFAULT_ENDPOINT FIPv4Endpoint(FIPv4Address(0, 0, 0, 0), 55555)
#define BUFSIZE 512

DEFINE_LOG_CATEGORY(RemoteControllerLog)

UGameInstance* GameInstance;
TArray<UWorld*> WorldArray;

HANDLE pipeInstance;
BOOL isConnected;
BOOL fSuccess;
unsigned long cbRead;
int counter;
TCHAR buf[1];
LPTSTR lpvMessage = TEXT("Default message from server.\n\0");
std::ofstream TestFile0;
std::ofstream TestFile1;
std::ofstream TestFile2;
std::ofstream TestFile3;
std::ofstream TestFile4;

void RemoteControllerModule::StartupModule()
{
	UE_LOG(RemoteControllerLog, Warning, TEXT("CloudyGame: RemoteController Module Starting"));

    const FString& SocketName = "RemoteControllerSocket";
    const FString& IPAddress = "0.0.0.0";
    const int32 Port = 55555;

	//Retrieve GEngine->CNumOfPlayers setting from DefaultGame.ini
	FString NumberOfPlayersString;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("NumOfPlayers"),
		NumberOfPlayersString,
		GGameIni
	);

	WorldArray.Init(NULL, FCString::Atoi(*NumberOfPlayersString));
	//TestFileArray.Init(nullptr, FCString::Atoi(*NumberOfPlayersString));

    InitializeRemoteServer(SocketName, IPAddress, Port);

	//LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\playerInputPipe");
	//pipeInstance = CreateNamedPipe(lpszPipename, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_NOWAIT, 
	//							   PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 0, NULL);
	////pipeInstance = CreateFile(TEXT("test.txt"), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	//
	//if (pipeInstance == INVALID_HANDLE_VALUE)
	//{
	//	UE_LOG(RemoteControllerLog, Warning, TEXT("CreateNamedPipe failed with %d"), GetLastError());
	//}
	//isConnected = 0;
	//fSuccess = 0;

	
	if (FCString::Atoi(*NumberOfPlayersString) <= 1)
	{
		std::ostringstream oss;
		oss << "test" << 0 << ".txt";
		TestFile0.open(oss.str(), std::ios::trunc);
	}
	else if (FCString::Atoi(*NumberOfPlayersString) <= 2)
	{
		std::ostringstream oss;
		oss << "test" << 0 << ".txt";
		TestFile0.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 1 << ".txt";
		TestFile1.open(oss.str(), std::ios::trunc);
	}
	else if (FCString::Atoi(*NumberOfPlayersString) <= 3)
	{
		std::ostringstream oss;
		oss << "test" << 0 << ".txt";
		TestFile0.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 1 << ".txt";
		TestFile1.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 2 << ".txt";
		TestFile2.open(oss.str(), std::ios::trunc);
	}
	else if (FCString::Atoi(*NumberOfPlayersString) <= 4)
	{
		std::ostringstream oss;
		oss << "test" << 0 << ".txt";
		TestFile0.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 1 << ".txt";
		TestFile1.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 2 << ".txt";
		TestFile2.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 3 << ".txt";
		TestFile3.open(oss.str(), std::ios::trunc);
	}
	else if (FCString::Atoi(*NumberOfPlayersString) <= 5)
	{
		std::ostringstream oss;
		oss << "test" << 0 << ".txt";
		TestFile0.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 1 << ".txt";
		TestFile1.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 2 << ".txt";
		TestFile2.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 3 << ".txt";
		TestFile3.open(oss.str(), std::ios::trunc);
		oss.str("");
		oss << "test" << 4 << ".txt";
		TestFile4.open(oss.str(), std::ios::trunc);
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

	if (Chunk.ControllerID == 0)
	{
		TestFile0 << 2 << std::endl;
	}
	else if (Chunk.ControllerID == 1)
	{
		TestFile1 << 2 << std::endl;
	}
	else if (Chunk.ControllerID == 2)
	{
		TestFile2 << 2 << std::endl;
	}
	else if (Chunk.ControllerID == 3)
	{
		TestFile3 << 2 << std::endl;
	}
	else if (Chunk.ControllerID == 4)
	{
		TestFile4 << 2 << std::endl;
	}

	// If world has not been loaded yet
	if (WorldArray[Chunk.ControllerID] == NULL)
	{
		WorldArray[Chunk.ControllerID] = GEngine->GameViewportArray[Chunk.ControllerID]->GetGameInstance()->GetWorld();
	}
	APlayerController* controller = UGameplayStatics::GetPlayerController(WorldArray[Chunk.ControllerID], 0);

	EInputEvent ie;
	if (Chunk.InputEvent == 2){ // Pressed
		ie = EInputEvent::IE_Pressed;
	}
	else if (Chunk.InputEvent == 3){ // Released
		ie = EInputEvent::IE_Released;
	}

	FKey key = FInputKeyManager::Get().GetKeyFromCodes(Chunk.KeyCode, Chunk.CharCode);
    if (controller != nullptr)
    {
        controller->InputKey(key, ie, 1, false);
    }
}


void RemoteControllerModule::ProcessMouseInput(const FArrayReaderPtr& Data)
{
    FUdpRemoteControllerSegment::FMouseInputChunk Chunk;
	*Data << Chunk;

	if (Chunk.ControllerID == 0)
	{
		TestFile0 << 1 << std::endl;
	}
	else if (Chunk.ControllerID == 1)
	{
		TestFile1 << 1 << std::endl;
	}
	else if (Chunk.ControllerID == 2)
	{
		TestFile2 << 1 << std::endl;
	}
	else if (Chunk.ControllerID == 3)
	{
		TestFile3 << 1 << std::endl;
	}
	else if (Chunk.ControllerID == 4)
	{
		TestFile4 << 1 << std::endl;
	}
	
	// If world has not been loaded yet
	if (WorldArray[Chunk.ControllerID] == NULL)
	{
		WorldArray[Chunk.ControllerID] = GEngine->GameViewportArray[Chunk.ControllerID]->GetGameInstance()->GetWorld();
	}
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

void RemoteControllerModule::HandleInputReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Sender)
{
	FUdpRemoteControllerSegment::FHeaderChunk Chunk;
	*Data << Chunk;
	
	// This can be blocking until a client connects or non-blocking 
	// However the window will not appear if this blocks.
	// If a window does not appear, NvIFREncoder will not run.
	//if (isConnected == 0) // false
	//{
	//	isConnected = ConnectNamedPipe(pipeInstance, NULL);
	//	if (isConnected != 0) // true
	//	{
	//		UE_LOG(RemoteControllerLog, Warning, TEXT("Client connected to the pipe!"));
	//	} 
	//	else // false
	//	{
	//		UE_LOG(RemoteControllerLog, Warning, TEXT("ConnectNamedPipe failed with %d"), GetLastError());
	//	}
	//}

	//std::string end = "\n\0";
	
	//end = std::to_string(counter) + end;

	//fSuccess = WriteFile(pipeInstance,
	//					 //lpvMessage,
	//					 //(lstrlen(lpvMessage)) *sizeof(TCHAR),
	//					 buf,
	//					 //end.c_str(),
	//					 sizeof(buf),
	//	                 &cbRead,
	//	                 NULL);
	
	//UE_LOG(RemoteControllerLog, Warning, TEXT("lpvMessage length = %d"), (lstrlen(lpvMessage) + 1) * sizeof(TCHAR));
	
	

	// Keyboard or mouse: Chunk.SegmentType
	// ============
	// FUdpRemoteControllerSegment::FKeyboardInputChunk Chunk;
	// *Data << Chunk;
	// Keyboard: uint8 ControllerID;
	//           int16 KeyCode;
	//           int16 CharCode;
	//           uint8 InputEvent;
	// ============
	// FUdpRemoteControllerSegment::FMouseInputChunk Chunk;
	// *Data << Chunk;
	// Mouse: uint8 ControllerID;
	//        int16 XAxis;
	//        int16 YAxis;

	// A pipe can be created ONCE during startup, and all this info 
	// can be constantly piped to NvIFREncoder.cpp to read
	
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