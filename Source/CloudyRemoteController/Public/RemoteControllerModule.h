#pragma once

#include "Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(RemoteControllerLog, Log, All);


class RemoteControllerModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	//static inline RemoteControllerModule& Get()
	//{
	//	return FModuleManager::LoadModuleChecked< RemoteControllerModule >("CloudyRemoteController");
	//}
	//
	//static inline bool IsAvailable()
	//{
	//	return FModuleManager::Get().IsModuleLoaded("CloudyRemoteController");
	//}

	bool IncreaseArraySize(float deltaTime);

protected:
	void InitializeRemoteServer(const FString& SocketName, const FString& IPAddress, const int32 Port);

private:
	FSocket* ServerListenSocket;
    FUdpSocketReceiver* UDPInputReceiver;

    void HandleInputReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Sender);
    void ProcessMouseInput(const FArrayReaderPtr& Data);
    void ProcessKeyboardInput(const FArrayReaderPtr& Data);
};