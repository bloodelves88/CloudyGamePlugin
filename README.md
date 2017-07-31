# CloudyGamePlugin
## Description

Unreal Engine 4 Plugin for CloudyGame.
This plugin contains 4 modules:

* CloudyWebConnector
* CloudyRemoteController
* CloudyGameStateAPI
* CloudyPlayerManager

## Plugin Setup

We assume that you are using our Unreal Engine fork of the source code, and have successfully built and compiled the engine. If not, please refer to the Unreal Engine fork [here](https://github.com/bloodelves88/UnrealEngine).

0. You should have one Unreal Engine game project created. To do this, open the Unreal Engine executable which can be found at `UnrealEngine\Engine\Binaries\Win64`. Launch `UE4Editor.exe`. 
  - If this is your first run, you should see a window to create a game. Ensure that the circled parts are selected correctly.
    - We recommended using the "First Person" template, but you are free to use others (except "Basic Code").
    - If your computer has a weak GPU, you can reduce the graphical quality of the game (3rd circle from the top, middle square button).
    - Please remember the location and name of the project.
    - ![Create Game](http://i.imgur.com/kj8HO4K.png)
  - If you do not see the above window, then the engine has launched with a previously made game.
    - Click on "File", then click on "New Project". You can now continue with the steps above.

1. In your game folder, create a folder named "Plugins" if it doesn't exist. Put CloudyGamePlugin in your Plugins folder. Build and run your game (details in the next step). The plugin should show up in Menu > Edit > Plugins.

  Your game directory structure should look similar to this (assuming the name of your game is MyProject):
  ```bash
  MyProject
  ├───Binaries
  ├───Build
  ├───Config
  ├───Content
  ├───DerivedDataCache
  ├───Intermediate
  ├───Plugins
  │   └───CloudyGamePlugin
  │       ├───OtherFiles
  │       ├───Resources
  │       └───Source
  │           ├───CloudyPlayerManager
  │           ├───CloudyRemoteController
  │           ├───CloudyGameStateAPI
  │           └───CloudyWebConnector
  ├───Saved
  └───Source
  ```
  Some folders may be missing if you have not compiled your project before.

2. Compile Unreal Engine with the plugins. Go to your game project folder, and right click the `.uproject` file. Click "Generate Visual Studio project files". 

  ![Right click .uproject](http://i.imgur.com/ou3xukU.png)
  
  Once done, open the `.sln` file. Then, in the solution explorer, you should see 3 folders: Engine, Games, Programs. Expand the "Games" folder, right-click your game project, and click "Build". 
  ![Build](http://i.imgur.com/6yGUQud.png)

3. To run the game, double click on the `.uproject` file mentioned in the previous step.

4. If you are installing this plugin as a user, then you are good to go. The information after this step is for game developers. 

# CloudyWebConnector
## Description

Module that listens for JSON packets from the [thin client](https://github.com/bloodelves88/CloudyGameThinClient).

This plugin currently supports join game and quit game. To test, ensure that the game is up and running, and send the following sample JSON packets via TCP to <your public IP>:55556 :

To join game:
```json
{
  "controller": "0",
  "command": "join"
}
```

To quit game:
```json
{
  "controller": "0",
  "command": "quit"
}
```

# CloudyPlayerManager
## Description

Module that kicks off the necessary join and quit functions when the CloudyWebConnector obtains information from the thin client.

## Usage

`OtherFiles/sendTCP.py` has been included to assist testing.

# CloudyRemoteController
## Description
This module will start a server which listens to key and mouse input from the Thin Client.

The key input will be passed to the correct player controller, controlling the player's movement.

# CloudyGameStateAPI
## Description

This module provides the game developer to set the state of the game. There are currently six states that can be used:
- Active (use this for high activity gameplay, such as shooting or combat)
- Moving
- Looking
- Movie (cutscenes)
- Menu
- Idle (not actually used)

## Usage

You can add more states easily. Simply look at `ICloudyGameStateAPI.h`, `CloudyGameStateAPI.h`, and `CloudyGameStateAPI.cpp`, and duplicate the functions while making the necessary modifications.

To use the API in your game, add the following into the game project's Build.cs file:
```
PrivateDependencyModuleNames.AddRange(new string[] { "CloudyGameStateAPI" });
```
In the game's code, include the following header file:
```
#include "ICloudyGameStateAPI.h"
```
Then, to call the function, do this (change the function names as needed):
```
ICloudyGameStateAPI::Get().Cloudy_MovementStart(GetWorld());
```
Do note that all the functions require a call to `GetWorld()`. It should be accessible from any game code.

For `Cloudy_ActiveStart()`, it takes in an additional boolean variable, `bool HasRelease`. This tells the API whether your game has an explicit release key for the action. For example, some shooter games allow you to press and hold the left mouse button to shoot, and releasing it stops. In this case, `HasRelease` should be set to true. If releasing the left mouse button does nothing in your game, `HasRelease` should be set to false.
