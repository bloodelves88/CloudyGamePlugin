#pragma once
#include "Engine.h"
#include "Http.h"
#include "Networking.h"
#include "Sockets.h"

DECLARE_LOG_CATEGORY_EXTERN(CloudyWebConnectorLog, Log, All);
 
class CloudyWebConnectorImpl : public ICloudyWebConnector
{

public:
	void StartupModule();
	void ShutdownModule();
	FSocket* ListenerSocket;
	FSocket* ConnectionSocket;
	FIPv4Endpoint RemoteAddressForConnection;

	bool CheckConnection(float DeltaTime);
	bool ParseInputJSON(FString InputString);

	//Timer functions, could be threads
	bool TCPConnectionListener(float dummy); 	//can thread this eventually
	bool TCPSocketListener(float dummy);		//can thread this eventually

	//Rama's StringFromBinaryArray
	FString StringFromBinaryArray(TArray<uint8> BinaryArray);

private:
	FString Command;
	int ControllerId;
};
