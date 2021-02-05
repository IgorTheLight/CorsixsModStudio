# Corsix's Mod Studio 0.5.5
The goal of this project is to make a more simple-to-build version of a Corsix's Mod Studio 0.5.5
Original sources were downloaded from here: https://modstudio.corsix.org/downloads-5.html

# What was done:
* Downloaded Lua 5.0.2, Lua 5.1.2, wxWidgets 2.8.0, wxProGrid 1.2.6
* Lua 5.0.2 and wxProGrid were modified as author recommended in his "COMPILE.txt" file
* For building Lua 5.0.2 was added a custom Visual Studio project from ... and upgraded to VS2005 version
* For building Lua 5.1.2 was added a custom build script from ... and upgraded to VS2005 version
* Was created a Visual Studio 2005 solution with projects for Rainman library, zlib library and Corsix's Mod Studio

# What is working:
* zlib                - WORKING
* Lua 5.0.2           - WORKING
* Lua 5.1.2           - WORKING
* wxWidgets 2.8.0     - WORKING (just 4 versions that are needed for Corsix's Mod Studio)
* wxProGrid 1.2.6     - WORKING (just 4 versions that are needed for Corsix's Mod Studio)
* wxStc               - FAILING
* Rainman             - FAILING
* Corsix's Mod Studio - FAILING

# Licence
I wasn't able to find yet any information about licencing

# You help would be very appreciated!