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
    
    UFUNCTION(BlueprintCallable, Category = "CloudyGame")
    virtual void Cloudy_ShootingStart() = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_ShootingEnd() = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MovementStart() = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MovementEnd() = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_LookingStart() = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_LookingEnd() = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MovieStart() = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MovieEnd() = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_IdleStart() = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_IdleEnd() = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MenuStart() = 0;
	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual void Cloudy_MenuEnd() = 0;

	UFUNCTION(BlueprintCallable, Category = "CloudyGame")
	virtual int Cloudy_GetWeight() = 0;
};