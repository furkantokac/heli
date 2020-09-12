# Heli - Simple FreeGLUT Project

This project is done for fun in pure FreeGLUT, no external resources are used. src/main.cpp is the only source code file of the heli project. It is used to test my [kit-opengl project](https://github.com/furkantokac/kit-opengl) so I didn't spend so much time on the code so it kind of sucks, but may be useful to someone.

![Screenshot 1](https://github.com/furkantokac/heli/blob/master/res/ss1.png)


# How To Play

* Use arrow keys to move
* When you make score, helis go faster
* If you loose 3 times, game over.


# Compile From Source

1. Install dependencies : `sudo apt-get install build-essential git cmake xorg-dev libxmu-dev libxi-dev libgl-dev libglu1-mesa-dev`
1. Clone the Heli project : `git clone git@github.com:furkantokac/heli.git`
1. Option 1 : Just compile and run
    1. Go to heli folder: `cd heli`
    1. cmake CmakeList.txt
    1. Click the compiled "Heli" file
1. Option 2 : Run on Qt Creator
    1. Go to Qt Creator
    1. Open Project
    1. Choose the CMakeLists.txt under heli directory
    1. "Run" the project to test
1. Enjoy..
