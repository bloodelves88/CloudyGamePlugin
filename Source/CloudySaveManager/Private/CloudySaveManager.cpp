#include "CloudySaveManagerPrivatePCH.h"
 
#include "CloudySaveManager.h"
#include "PlatformFeatures.h"
#include "GameFramework/SaveGame.h"
#include "HttpRequestAdapter.h"
#include "HttpModule.h"
#include "IHttpResponse.h"
#include "Base64.h"
#include "AllowWindowsPlatformTypes.h"
#include "ThirdParty/libcurl/include/Windows/curl/curl.h"
#include "HideWindowsPlatformTypes.h"
#include "string"

static const int UE4_SAVEGAME_FILE_TYPE_TAG = 0x53415647;		// "sAvG"
static const int UE4_SAVEGAME_FILE_VERSION = 1;
static const FString BaseUrl = "http://127.0.0.1:8000";
static const FString AuthUrl = "/api-token-auth/";
static const FString SaveDataUrl = "/save-data/";
static FString Token;

// Automatically starts and shuts down when UE4 is started/closed
void CloudySaveManagerImpl::StartupModule()
{
    UE_LOG(LogTemp, Warning, TEXT("CloudySaveManager started"));
    AttemptAuthentication(TEXT("joel"), TEXT("1234")); // Token variable will be populated with robot's token
}
 
void CloudySaveManagerImpl::ShutdownModule()
{
    UE_LOG(LogTemp, Warning, TEXT("CloudySaveManager stopped"));
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
 
bool CloudySaveManagerImpl::Cloudy_SaveGameToSlot(USaveGame* SaveGameObject, const FString& SlotName, 
                                                  const int32 UserIndex, const int32 PCID, bool IsAutosaved)//APlayerController const* PC)
{
    FString theName = SlotName;

    bool bSuccess = false;

    ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
    // If we have a system and an object to save and a save name...
    if (SaveSystem && (SaveGameObject != NULL) && (SlotName.Len() > 0))
    {
        TArray<uint8> ObjectBytes;
        FMemoryWriter MemoryWriter(ObjectBytes, true);

        // write file type tag. identifies this file type and indicates it's using proper versioning
        // since older UE4 versions did not version this data.
        int32 FileTypeTag = UE4_SAVEGAME_FILE_TYPE_TAG;
        MemoryWriter << FileTypeTag;

        // Write version for this file format
        int32 SavegameFileVersion = UE4_SAVEGAME_FILE_VERSION;
        MemoryWriter << SavegameFileVersion;

        // Write out engine and UE4 version information
        MemoryWriter << GPackageFileUE4Version;
        FEngineVersion SavedEngineVersion = GEngineVersion;
        MemoryWriter << SavedEngineVersion;

        // Write the class name so we know what class to load to
        FString SaveGameClassName = SaveGameObject->GetClass()->GetName();
        MemoryWriter << SaveGameClassName;
        
        // Then save the object state, replacing object refs and names with strings
        FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
        SaveGameObject->Serialize(Ar);

        // Stuff that data into the save system with the desired file name
        SaveSystem->SaveGame(false, *SlotName, UserIndex, ObjectBytes);

        bSuccess = UploadFile(SlotName, PCID, IsAutosaved);
    }

    return bSuccess;
}

bool CloudySaveManagerImpl::AttemptAuthentication(FString Username, FString Password)
{
    bool RequestSuccess = false;

    FString Url = BaseUrl + AuthUrl; // "http://127.0.0.1:8000/api-token-auth/";
    FString ContentString;

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("username"), Username);
    JsonObject->SetStringField(TEXT("password"), Password);

    TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&ContentString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

    TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetURL(Url);
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetContentAsString(ContentString);
    HttpRequest->OnProcessRequestComplete().BindRaw(this, &CloudySaveManagerImpl::OnAuthResponseComplete);
    RequestSuccess = HttpRequest->ProcessRequest();

    UE_LOG(LogTemp, Warning, TEXT("URL = %s"), *Url);
    UE_LOG(LogTemp, Warning, TEXT("ContentString = %s"), *ContentString);

    return RequestSuccess;
}

bool CloudySaveManagerImpl::UploadFile(FString Filename, int32 PlayerControllerId, bool IsAutosaved)
{
    bool RequestSuccess = false;

    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    readBuffer.clear();
    std::string isAutosavedString;
    
    FString Url = BaseUrl + SaveDataUrl;
    std::string UrlCString(TCHAR_TO_UTF8(*Url));
    
    // Filepath of .sav file
    FString Filepath = FPaths::GameDir();
    Filepath += "Saved/SaveGames/" + Filename + ".sav";
    std::string filePath(TCHAR_TO_UTF8(*Filepath));
    
    // Get game name
    FString GameName = FApp::GetGameName();
    std::string gameName(TCHAR_TO_UTF8(*GameName));
    
    // Convert PlayerControllerId
    FString playerControllerIdFString = FString::FromInt(PlayerControllerId);
    std::string playerControllerId(TCHAR_TO_UTF8(*playerControllerIdFString));
    
    // Convert IsAutosaved to string
    if (IsAutosaved)
    {
        isAutosavedString = "true";
    }
    else
    {
        isAutosavedString = "false";
    }
    
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;
    struct curl_slist *headerlist = NULL;
    //static const char buf[] = "Expect:";
    FString AuthHeader = "Authorization: Token " + Token;
    std::string AuthHeaderCString(TCHAR_TO_UTF8(*AuthHeader));
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    /* Fill in the file upload field */
    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "saved_file",
        CURLFORM_FILE, filePath.c_str(),
        CURLFORM_END);
    
    /* Fill in the player controller ID field */
    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "controller",
        CURLFORM_COPYCONTENTS, playerControllerId.c_str(),
        CURLFORM_END);
    
    /* Fill in the game name field */
    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "game_name",
        CURLFORM_COPYCONTENTS, gameName.c_str(),
        CURLFORM_END);
    
    curl = curl_easy_init();
    /* initialize custom header list (stating that Expect: 100-continue is not
    wanted */
    headerlist = curl_slist_append(headerlist, AuthHeaderCString.c_str());
    if (curl) {
        /* what URL that receives this POST */
        curl_easy_setopt(curl, CURLOPT_URL, UrlCString.c_str());
        //curl_easy_setopt(curl, CURLOPT_URL, "http://posttestserver.com/post.php?dir=bloodelves88");

        /* only disable 100-continue header if explicitly requested */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        /* What form to send */
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    
        /* Set up string to write response into */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
    
        /* always cleanup */
        curl_easy_cleanup(curl);
    
        /* then cleanup the formpost chain */
        curl_formfree(formpost);
        /* free slist */
        curl_slist_free_all(headerlist);
    
        UE_LOG(LogTemp, Warning, TEXT("Response data: %s"), UTF8_TO_TCHAR(readBuffer.c_str()));
    }

    return RequestSuccess;
}

void CloudySaveManagerImpl::OnAuthResponseComplete(FHttpRequestPtr Request, 
                                                   FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        FString MessageBody;

        UE_LOG(LogTemp, Warning, TEXT("Response Code = %d"), Response->GetResponseCode());

        if (!Response.IsValid())
        {
            MessageBody = "{\"success\":\"Error: Unable to process HTTP Request!\"}";
            GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, TEXT("Request failed!"));

            UE_LOG(LogTemp, Warning, TEXT("Request failed!"));
        }
        else if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
        {
            TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
            TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
            FJsonSerializer::Deserialize(JsonReader, JsonObject);

            Token = JsonObject->GetStringField("token");

            UE_LOG(LogTemp, Warning, TEXT("Token = %s"), *Token);
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, TEXT("Request error!"));
            MessageBody = FString::Printf(TEXT("{\"success\":\"HTTP Error: %d\"}"), Response->GetResponseCode());
        }
    }
    else 
    {
        UE_LOG(LogTemp, Warning, TEXT("Request failed! Is the server up?"));
    }
}

USaveGame* CloudySaveManagerImpl::Cloudy_LoadGameFromSlot(const FString& SlotName, 
                                                          const int32 UserIndex, bool IsAutosaved)
{
    // Load from CloudyWeb
    // ...
    
    USaveGame* OutSaveGameObject = NULL;
    
    ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
    // If we have a save system and a valid name..
    if (SaveSystem && (SlotName.Len() > 0))
    {
        // Load raw data from slot
        TArray<uint8> ObjectBytes;
        bool bSuccess = SaveSystem->LoadGame(false, *SlotName, UserIndex, ObjectBytes);
        if (bSuccess)
        {
            FMemoryReader MemoryReader(ObjectBytes, true);

            int32 FileTypeTag;
            MemoryReader << FileTypeTag;

            int32 SavegameFileVersion;
            if (FileTypeTag != UE4_SAVEGAME_FILE_TYPE_TAG)
            {
                // this is an old saved game, back up the file pointer to the beginning and assume version 1
                MemoryReader.Seek(0);
                SavegameFileVersion = 1;

                // Note for 4.8 and beyond: if you get a crash loading a pre-4.8 version of your savegame file and 
                // you don't want to delete it, try uncommenting these lines and changing them to use the version 
                // information from your previous build. Then load and resave your savegame file.
                //MemoryReader.SetUE4Ver(MyPreviousUE4Version);				// @see GPackageFileUE4Version
                //MemoryReader.SetEngineVer(MyPreviousEngineVersion);		// @see FEngineVersion::Current()
            }
            else
            {
                // Read version for this file format
                MemoryReader << SavegameFileVersion;

                // Read engine and UE4 version information
                int32 SavedUE4Version;
                MemoryReader << SavedUE4Version;

                FEngineVersion SavedEngineVersion;
                MemoryReader << SavedEngineVersion;

                MemoryReader.SetUE4Ver(SavedUE4Version);
                MemoryReader.SetEngineVer(SavedEngineVersion);
            }

            // Get the class name
            FString SaveGameClassName;
            MemoryReader << SaveGameClassName;

            // Try and find it, and failing that, load it
            UClass* SaveGameClass = FindObject<UClass>(ANY_PACKAGE, *SaveGameClassName);
            if (SaveGameClass == NULL)
            {
                SaveGameClass = LoadObject<UClass>(NULL, *SaveGameClassName);
            }

            // If we have a class, try and load it.
            if (SaveGameClass != NULL)
            {
                OutSaveGameObject = NewObject<USaveGame>(GetTransientPackage(), SaveGameClass);

                FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
                OutSaveGameObject->Serialize(Ar);
            }
        }
    }
    
    return OutSaveGameObject;
}

 
IMPLEMENT_MODULE(CloudySaveManagerImpl, CloudySaveManager)