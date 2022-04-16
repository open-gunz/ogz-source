# Open GunZ Source
This is the Open GunZ (GunZ The Duel game) source repo. It was forked from the Refined GunZ source (https://github.com/Asunaya/RefinedGunz) and updated by the International GunZ (http://igunz.net) private server developers.

# Building the source for the first time

## Windows:
Requisites:

* GCC x64 Windows: http://tdm-gcc.tdragon.net/download
* CMake 3.7 or later: Download the latest release from https://cmake.org/download/ under "Binary distributions." In the installer, select "Add CMake to the system PATH for all users". [Picture here](https://i.imgur.com/rQHLXX8.png).

* Install Visual Studio 2019 Community edition (https://visualstudio.microsoft.com/downloads/)

  - In the installer, select the `Desktop development with C++`

  - Under Installation details on the right, select the following option: C++ MFC for latest v142 build tools (x86 & x64)

  - Select Individual components on the top bar and search for C++ Windows XP Support for VS 2017 (v141) tools [Depreciated]


* OPENSSL: Download and install in `C:\OpenSSL-Win32` from [here](https://mega.nz/#!jLQWAQBJ!nT3v1FDHO80ikOmzsBZrImUpyh5ozE9mYnxdGXDDKWA)
  - For "copy OpenSSL DLLs to" prompt, choose "The Windows system directory".
  - Next download vs2017 OpenSSL files from [here](https://mega.nz/#!HfwCkIJa!UYvC9Sv2S24PFwHYEzWfrHevISTf1AmD9LuEMI8Yhco)
  - Delete everything in `C:\OpenSSL-Win32` and replace it with the vs2017 files.
* Zlib Download: From [here](https://mega.nz/#!LepTgbTT!AHdYH0Kil1jxaINwhbm5uh7VtjKtcx6vMz6WZVHpCOU) and extract that on your system `C:\Program Files (x86)\zlib`.

To build, run `build-win32-VS2019.bat` in source folder. Wait until it says "Build succeeded" with possibly some warnings in yellow.
The built files can be found in `ogz-source\build\win32` and `ogz-source\install`.

### Optional: Removing warning MSB8051 during build process
Warning MSB8051: Support for targeting Windows XP is deprecated and will not be present
in future releases of Visual Studio.

To disable the warning open  
`C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\VC\VCTargets\Platforms\Win32\PlatformToolsets\v141_xp\Toolset.targets`

Replace:  `<VCMessage Code="MSB8051" Type="Warning" Condition="'$(XPDeprecationWarning)' != 'false'" />`  
With:     `<VCMessage Code="MSB8051" Type="Warning" Condition="'$(XPDeprecationWarning)' == 'false'" />`  

## Linux:
Requisites:

* Install the following packages: 
    - sudo apt install cmake
    - sudo apt install zlib1g-dev
    - sudo apt install build-essential checkinstall zlib1g-dev -y
    - sudo apt-get install -y libsodium-dev
    - apt-get install libssl-dev
    - sudo apt-get install libsqlite3-dev
    - sudo apt-get install libasio-dev
    - sudo apt-get install libcurl4-openssl-dev
    - sudo apt-get install libsystemd-dev

To build, run `build-linux.sh` in source folder.

If you are using Windows Subsystem for Linux, and the source is somwhere in `/mnt/c`, during compilation you might get messages saying that some packages are missing. But, if everything is set correctly it should still compile fine. To avoid these messages all together, compile from somwhere in the Windows Subsystem for Linux for example: `/opt/igunz/source`

# Launching local test client for the first time
Requisites:

* Download the server and client files.
	- [Client files](https://github.com/open-gunz/client)
	- [Server files](https://github.com/open-gunz/server)
* Replace the GunZ.exe and MatchServer.exe in their respective client/server folders, you can either build them yourself as described above or download the executables [here](https://github.com/open-gunz/ogz-source/releases)
* Change IP to `127.0.0.1` in config file --> `C:\Users\[username]\Documents\Open GunZ\config.xml`.
* In the server folder, make sure `server.ini` has these values:
  - `FREELOGINIP`, `KEEPERIP`, `DBAgentIP` set to `127.0.0.1`.
* Launch MatchServer.exe from the server folder.
* Run GunZ.exe from client folder (DO NOT USE Launcher.exe as it will downgrade to what is placed on the website for patching).
* You can also run GunZ.exe on Linux using Wine. Currently everything seems to work, but text rendering is broken and text is unreadable.
* Register a test account within the client and login.

# Contact
Feel free to post any issue in the issues section.  
You can also reach us on our discord via [this](https://discord.gg/CWjzsu5) link.

# Credits
[Crawly](https://github.com/Asunaya) (https://github.com/Asunaya/RefinedGunz)

[SuhBruh](https://github.com/suhbruh) (https://igunz.net)

[Michael Steshenko](https://github.com/Michael-Steshenko) (https://igunz.net)

ThievingSix (https://igunz.net)
