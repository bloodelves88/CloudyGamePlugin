#include "CloudyWebConnectorPrivatePCH.h"
 
#include "CloudyWebConnector.h"

#include "PlatformFeatures.h"
#include "HttpRequestAdapter.h"
#include "HttpModule.h"
#include "IHttpResponse.h"
#include "string"
#include "../../CloudyPlayerManager/Public/CloudyPlayerManager.h"

DEFINE_LOG_CATEGORY(CloudyWebConnectorLog);

#define SERVER_NAME "Listener"
#define SERVER_ENDPOINT FIPv4Endpoint(FIPv4Address(0, 0, 0, 0), 55556)
#define CONNECTION_THREAD_TIME 0.01 // in seconds
#define BUFFER_SIZE 1024

// Automatically starts when UE4 is started.
// Populates the Token variable with the robot user's token.
void CloudyWebConnectorImpl::StartupModule()
{
	UE_LOG(CloudyWebConnectorLog, Warning, TEXT("CloudyWebConnector started"));

	//Rama's CreateTCPConnectionListener
	FIPv4Endpoint Endpoint(FIPv4Address(0, 0, 0, 0), 55556);
	ListenerSocket = FTcpSocketBuilder("CloudyWebConnectorSocket").AsReusable().BoundToEndpoint(Endpoint).Listening(8);

	//Set Buffer Size
	int32 NewSize = 0;
	ListenerSocket->SetReceiveBufferSize(2 * 1024 * 1024, NewSize);

	//Not created?
	if (!ListenerSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("CloudyWebConnector: Failed to create TCP Socket Listener."));
		return;
	}
	FTimerHandle Test;
	//Start the Listener!
	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &CloudyWebConnectorImpl::TCPConnectionListener), CONNECTION_THREAD_TIME);

	UE_LOG(LogTemp, Warning, TEXT("CloudyWebConnector: TCP Socket Listener Created"));
}

// Automatically starts when UE4 is closed
void CloudyWebConnectorImpl::ShutdownModule()
{
    UE_LOG(CloudyWebConnectorLog, Warning, TEXT("CloudyWebConnector stopped"));
//    delete TcpListener;
//   ListenSocket->Close();
}

//Rama's TCP Connection Listener
bool CloudyWebConnectorImpl::TCPConnectionListener(float dummy)
{
	//~~~~~~~~~~~~~
	if (!ListenerSocket) 
		return false;
	//~~~~~~~~~~~~~

	//Remote address
	TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool Pending;

	// handle incoming connections
	if (ListenerSocket->HasPendingConnection(Pending) && Pending)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//Already have a Connection? destroy previous
		if (ConnectionSocket)
		{
			ConnectionSocket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//New Connection receive!
		ConnectionSocket = ListenerSocket->Accept(*RemoteAddress, TEXT("RamaTCP Received Socket Connection"));

		if (ConnectionSocket != NULL)
		{
			//Global cache of current Remote Address
			RemoteAddressForConnection = FIPv4Endpoint(RemoteAddress);

			//UE_LOG "Accepted Connection! WOOOHOOOO!!!";

			//can thread this too
			//GetWorldTimerManager().SetTimer(this, &CloudyWebConnectorImpl::TCPSocketListener, 0.01, true);
			FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &CloudyWebConnectorImpl::TCPSocketListener), CONNECTION_THREAD_TIME);
		}
	}
	return true;
}

//Rama's String From Binary Array
FString CloudyWebConnectorImpl::StringFromBinaryArray(TArray<uint8> BinaryArray)
{
	BinaryArray.Add(0); // Add 0 termination. Even if the string is already 0-terminated, it doesn't change the results.
						// Create a string from a byte array. The string is expected to be 0 terminated (i.e. a byte set to 0).
						// Use UTF8_TO_TCHAR if needed.
						// If you happen to know the data is UTF-16 (USC2) formatted, you do not need any conversion to begin with.
						// Otherwise you might have to write your own conversion algorithm to convert between multilingual UTF-16 planes.
	return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(BinaryArray.GetData())));
}

//Rama's TCP Socket Listener
bool CloudyWebConnectorImpl::TCPSocketListener(float dummy)
{
	//~~~~~~~~~~~~~
	if (!ConnectionSocket)
		return false;
	//~~~~~~~~~~~~~


	//Binary Array!
	TArray<uint8> ReceivedData;

	uint32 Size;
	while (ConnectionSocket->HasPendingData(Size))
	//while (ConnectionSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(2)))); // this is running constantly.
	{
		ReceivedData.Init(FMath::Min(Size, 65507u), Size);

		int32 Read = 0;
		ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
		UE_LOG(LogTemp, Warning, TEXT("CloudyWebConnector: Data received"));
	}
	if (ReceivedData.Num() <= 0)
	{
		//No Data Received
		return false;
	}
	const FString ReceivedUE4String = StringFromBinaryArray(ReceivedData);
	ParseInputJSON(ReceivedUE4String);
	CCloudyPlayerManagerModule::Get().ExecuteCommand(Command, ControllerId);

	UE_LOG(LogTemp, Warning, TEXT("CloudyWebConnector: Data received = %s"), *ReceivedUE4String);
	//VShow("As String!!!!! ~>", ReceivedUE4String);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("As String Data ~> %s"), *ReceivedUE4String));
	return true;
}


/**
* Parses string and stores as global variables for access by other modules
*
* @param InputStr Input string to parse
* @return Whether parsing was successful or not
*/
bool CloudyWebConnectorImpl::ParseInputJSON(FString InputString)
{
    bool isSuccessful = false;

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    TSharedRef<TJsonReader<TCHAR>>JsonReader = TJsonReaderFactory<TCHAR>::Create(InputString);
    isSuccessful = FJsonSerializer::Deserialize(JsonReader, JsonObject);

    // these 2 fields will be populated for both join/quit commands
    Command = JsonObject->GetStringField("command");
    ControllerId = JsonObject->GetIntegerField("controller");
    
    return isSuccessful;
}


///**
//* Timer to periodically check for join/quit signals from client, and call
//* appropriate input handler.
//*
//* @param DeltaTime Time taken by method
//*/
//bool CloudyWebConnectorImpl::CheckConnection(float DeltaTime)
//{
//    bool Success = false;
//   // if (HasInputStrChanged) 
//    //{
//        if (GEngine->GameViewportArray[0] != nullptr && GIsRunning && IsInGameThread())
//        {
//           // UE_LOG(CloudyWebConnectorLog, Warning, TEXT("Success! input str: %s"), *InputStr);
//            //GetCloudyWebData(InputStr);
//           // UE_LOG(CloudyWebConnectorLog, Warning, TEXT("Success! Controllerid: %d command: %s"), ControllerId, *Command);
//         //   Success = CCloudyPlayerManagerModule::Get().ExecuteCommand(Command, ControllerId);
//        //    InputStr = "";
//        //    HasInputStrChanged = false;
//        }        
//   // }
//
//    return true; // continue timer to check for requests
//}
//
///**
//* Handles input passed by TCP listener
//*
//* @param ConnectionSocket The TCP socket connecting the listener and client
//* @param Endpoint The endpoint of the socket connection
//*/
//bool CloudyWebConnectorImpl::InputHandler(FSocket* ConnectionSocket, const FIPv4Endpoint& Endpoint)
//{
//
//    TArray<uint8> ReceivedData;
//    uint32 Size = 12;
//
//    // wait for data to arrive
//	// Blocks until the specified condition is met.
//	// true if the condition was met, false if the time limit expired or an error occurred.
//    while (!(ConnectionSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(2))));
//
//    // handle data - change global InputStr
//    ReceivedData.SetNumUninitialized(Size);
//
//    int32 Read = 0;
//    ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
//    FString ReceivedString = StringFromBinaryArray(ReceivedData);
//
//	UE_LOG(CloudyWebConnectorLog, Warning, TEXT("CloudyWebConnector: Input received"));
//    InputStr = ReceivedString;
//    HasInputStrChanged = true;
//
//    TCPConnection = ConnectionSocket;
//
//    return true;
//
//}

 
IMPLEMENT_MODULE(CloudyWebConnectorImpl, CloudyWebConnector)