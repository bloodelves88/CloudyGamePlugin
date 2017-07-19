#pragma once
 
#include "ModuleManager.h"
#include "Engine.h"
 
/**
* The public interface to this module.  In most cases, this interface is only public to sibling modules
* within this plugin.
*/
class ICloudyGameStateAPI : public IModuleInterface
{
 
public:
 
    /**
    * Singleton-like access to this module's interface.  This is just for convenience!
    * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
    *
    * @return Returns singleton instance, loading the module on demand if needed
    */
    static inline ICloudyGameStateAPI& Get()
    {
        return FModuleManager::LoadModuleChecked< ICloudyGameStateAPI >("CloudyGameStateAPI");
    }
 
    /**
    * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
    *
    * @return True if the module is loaded and ready to use
    */
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("CloudyGameStateAPI");
    }

	virtual void DecreaseNumberOfPlayers() = 0;
	virtual void IncreaseNumberOfPlayers() = 0;
    
    UFUNCTION(BlueprintCallable, Category = "CloudyGame")
    virtual void Cloudy_ActiveStart(UWorld* world, bool HasRelease) = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_ActiveStop(UWorld* world) = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MovementStart(UWorld* world) = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MovementStop(UWorld* world) = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_LookingStart(UWorld* world) = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_LookingStop(UWorld* world) = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MovieStart(UWorld* world) = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MovieStop(UWorld* world) = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_IdleStart(UWorld* world) = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_IdleStop(UWorld* world) = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MenuStart(UWorld* world) = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MenuStop(UWorld* world) = 0;
};