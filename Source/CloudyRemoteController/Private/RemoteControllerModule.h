#pragma once

#include "Engine.h"
#include "Networking.h"

DECLARE_LOG_CATEGORY_EXTERN(RemoteControllerLog, Log, All);

class RemoteControllerModule : public IRemoteControllerModule
{

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void IncreaseArraySize();
	void DecreaseArraySize();

private:
	FSocket* ServerListenSocket;
    FUdpSocketReceiver* UDPInputReceiver;

	void InitializeRemoteServer(const FString& SocketName, const FString& IPAddress, const int32 Port);

    void HandleInputReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Sender);
    void ProcessMouseInput(const FArrayReaderPtr& Data);
    void ProcessKeyboardInput(const FArrayReaderPtr& Data);
};