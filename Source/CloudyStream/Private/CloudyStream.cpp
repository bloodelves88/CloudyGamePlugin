// Some copyright should be here...

#include "CloudyStreamPrivatePCH.h"

#include <stdio.h>
#include <sstream>


#define LOCTEXT_NAMESPACE "CloudyStream"

#define PIXEL_SIZE 4
#define FPS 30 // frames per second

// Layout of the split screen. Ensure that this is same as the values in GameViewportClient.cpp line 120~
#define MAX_NUM_PLAYERS 6
#define NUM_ROWS 2.0
#define NUM_COLS 3.0


TArray<FILE*> VideoPipeList;

DEFINE_LOG_CATEGORY(CloudyStreamLog)

void CloudyStreamImpl::StartupModule()
{
    ensure(MAX_NUM_PLAYERS == NUM_ROWS * NUM_COLS);

	// timer to capture frames
	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &CloudyStreamImpl::CaptureFrame), 1.0);
	UE_LOG(CloudyStreamLog, Warning, TEXT("Streaming module started"));

}


void CloudyStreamImpl::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

}


// call this for each player join
void CloudyStreamImpl::SetUpPlayer(int ControllerId, int StreamingPort, FString StreamingIP) {

	// encode and write players' frames to http stream
	std::stringstream *StringStream = new std::stringstream();

	std::string StreamingIPString(TCHAR_TO_UTF8(*StreamingIP)); // convert to std::string

    UE_LOG(CloudyStreamLog, Warning, TEXT("Streaming IP: %s, Streaming port: %d"), *StreamingIP, StreamingPort);

    //*StringStream << "ffmpeg -y " << " -f rawvideo -pix_fmt bgra -s " << ColIncInt << "x" << RowIncInt << " -r " << FPS << " -loglevel quiet -i - -listen 1 -c:v libx264 -preset ultrafast -f avi -an -tune zerolatency http://" << StreamingIPString << ":" << StreamingPort;
    //
    //VideoPipeList.Add(_popen(StringStream->str().c_str(), "wb"));

	// add frame buffer for new player
	TArray<FColor> TempFrameBuffer;
	FrameBufferList.Add(TempFrameBuffer);

	PlayerFrameMapping.Add(ControllerId);
	NumberOfPlayers++;
}


void CloudyStreamImpl::SetUpVideoCapture() {

    ScreenList.Empty();

	// init frame dimension variables
	FViewport* ReadingViewport = GEngine->GameViewport->Viewport;
	sizeX = ReadingViewport->GetSizeXY().X;
	sizeY = ReadingViewport->GetSizeXY().Y;

    UE_LOG(CloudyStreamLog, Warning, TEXT("Height: %d Width: %d"), sizeY, sizeX);

    RowIncrement = sizeY / NUM_ROWS;
    ColIncrement = sizeX / NUM_COLS;

    RowIncInt = sizeY / (int)NUM_ROWS;
    ColIncInt = sizeX / (int)NUM_COLS;

    UE_LOG(CloudyStreamLog, Warning, TEXT("RowIncrement: %f ColIncrement: %f"), RowIncrement, ColIncrement);

    for (float i = 0.0f; i < sizeY; i += RowIncrement)
    {
        for (float k = 0.0f; k < sizeX; k += ColIncrement)
        {
            // FIntRect(TopLeftX, TopLeftY, BottomRightX, BottomRightY)
            ScreenList.Add(FIntRect(k, i, k + ColIncInt, i + RowIncInt));
            // UE_LOG(CloudyStreamLog, Warning, TEXT("Iteration: k: %f i: %f"), k, i);
        }
    }

	flags = FReadSurfaceDataFlags(ERangeCompressionMode::RCM_MinMaxNorm, ECubeFace::CubeFace_NegX);

}


bool CloudyStreamImpl::CaptureFrame(float DeltaTime) {

	// engine has been started
	if (!isEngineRunning && GEngine->GameViewport != nullptr && GIsRunning && IsInGameThread()) {
		isEngineRunning = true;
		SetUpVideoCapture();
		UGameInstance* GameInstance = GEngine->GameViewport->GetGameInstance();
		NumberOfPlayers = 0;
		
		ULocalPlayer* const ExistingPlayer = GameInstance->FindLocalPlayerFromControllerId(0);
		APlayerController* Controller = ExistingPlayer->PlayerController;
		Controller->GetPawn()->Destroy();
		GameInstance->DebugRemovePlayer(0); // remove default first player
	}

	// engine has been stopped
	else if (isEngineRunning && !(GEngine->GameViewport != nullptr && GIsRunning && IsInGameThread())) {
		
		isEngineRunning = false;

		for (int i = 0; i < NumberOfPlayers; i++) {
			// flush and close video pipes
			fflush(VideoPipeList[i]);
			fclose(VideoPipeList[i]);
		}
	}

	return true;
}


void CloudyStreamImpl::StartPlayerStream(int32 ControllerId, int32 StreamingPort, FString StreamingIP)
{
	UE_LOG(CloudyStreamLog, Warning, TEXT("Player %d stream started"), ControllerId);
	SetUpPlayer(ControllerId, StreamingPort, StreamingIP);	
}


void CloudyStreamImpl::StopPlayerStream(int32 ControllerId)
{
	UE_LOG(CloudyStreamLog, Warning, TEXT("Player %d stream stopped"), ControllerId);
	int PipeIndex = PlayerFrameMapping.Find(ControllerId);
	fflush(VideoPipeList[PipeIndex]);
	fclose(VideoPipeList[PipeIndex]);
	PlayerFrameMapping.Remove(ControllerId);
	VideoPipeList.RemoveAt(PipeIndex);
	NumberOfPlayers--;
}

void CloudyStreamImpl::AddPipeToList(FILE* TheFile)
{
    VideoPipeList.Add(TheFile);
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(CloudyStreamImpl, CloudyStream)