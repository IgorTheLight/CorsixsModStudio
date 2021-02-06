# Corsix's Mod Studio 0.5.5
The goal of this project is to make a more simple-to-build version of a Corsix's Mod Studio 0.5.5
Original sources were downloaded from here: https://modstudio.corsix.org/downloads-5.html

# What was done:
* Downloaded Lua 5.0.2, Lua 5.1.2, wxWidgets 2.8.0, wxProGrid 1.2.6
* Lua 5.0.2 and wxProGrid were modified as author recommended in his "COMPILE.txt" file
* For building Lua 5.0.2 was added a custom Visual Studio project from ... and upgraded to VS2005 version
* For building Lua 5.1.2 was added a custom build script from ... and upgraded to VS2005 version
* A Visual Studio 2005 solution was created with projects for Rainman library, zlib library and Corsix's Mod Studio

# What is working:
* zlib................- WORKING
* Lua 5.0.2...........- WORKING
* Lua 5.1.2...........- WORKING
* wxWidgets 2.8.0.....- WORKING (just 4 versions that are needed for Corsix's Mod Studio)
* wxProGrid 1.2.6.....- WORKING (just 4 versions that are needed for Corsix's Mod Studio)
* wxStc...............- FAILING
* Rainman.............- FAILING
* Corsix's Mod Studio.- FAILING

# How to build
Many of Corsix's Mod Studio 0.5.5 files are from 2006-2008 year so I'm using Visual Studio 2005 SP1 (with Vista patch) inside a virtual machine for more compatibility. So my setup is: Oracle VirtualBox 6.1.18 + Microsoft Windows 7 x64 SP1 Ultimate + Microsoft Visual Studio 2005 SP1 Professional (with Vista patch). I will test it on less ancient software later but for now I'm trying to get as close to original environment as possible.

So how to build it anyway:
* Clone this repo. If you have git just use: git clone --depth 1 https://github.com/IgorTheLight/CorsixsModStudio.git
* UnRAR wxWidgets-2.8.0.rar in the same folder. 7-Zip or WinRAR will do the job.
* Rename "tdm-gcc-4.9.2.ex_" to "tdm-gcc-4.9.2.exe" and install TDM GCC compiler to default path (!). This is needed for Lua 5.1.2 compilation
* Run "build_lua_5_1_2.cmd" - now you have Lua in lua-5.1.2\bin
* Go to "lua-5.0.2" and run Lua.sln - now you could build it in Visual Studio.
* Go to "wxWidgets-2.8.0\build\msw" and run wx.sln - now you could build wxWidgets using Build -> Batch Build -> Select "Debug", "Release", "Debug Unicode" and "Release Unicode"
* Go to "wxWidgets-2.8.0/contrib/build/propgrid" and build it as you did with wxWidgets
* Go to "wxWidgets/contrib/build/stc" and build it as you did with wxWidgets. But this time it fails :-(
* Go to the "SDMS" folder and run SDMS.sln - now theoretically you would be able to build the rest. But you could only build zlib. Rainman and SDMS are failing to build for now.

# Licence
I wasn't able to find any information about licencing yet

# Your help would be very appreciated!