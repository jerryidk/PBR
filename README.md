
# PBR 

Final project for class cs 6610 (interactive computer graphics) following [pbr guide](https://learnopengl.com/PBR/Theory). I have implemented the microfacet model with direct analytical lighting. In replacement of simple blinn-phong shading model, this project has used much more physical accurate model taking into consideration of energy conservation, material roughness, and microscopic level of surfaces details.

## External Dependencies

1. GLEW
2. GLFW
3. CyCodeBase
4. IMGUI

## Build 

I provide a Makefile. If you are using windows, make sure you have **wsl2** and **mingw64** installed (See below for how to install mingw64). If are on other system, modify the **Makefile** accordingly (see Makefile). Once you have all dependencies, call `make all` and find executable under `./build/App.exe`

### How to install mingw64 for OpenGL application

1. Obtain an installer from [msys2](https://www.msys2.org/).
2. Follow the installer or the website to finish the installation.
3. After installation, ensure you open `mingw64.exe` in folder `msys64/mingw64.exe`  *(Note: msys2 shell is prompted immediately after installation, make sure you DON’T use msys2. Instead, open mingw64.exe in msys64/ folder.)*
4. Put following command into the terminal:
```
$pacman -Syu 
$pacman -Su 
$pacman -S --needed base-devel mingw-w64-x86_64-toolchain 
$pacman -S mingw-w64-x86_64-glew 
$pacman -S mingw-w64-x86_64-glfw
```

### Toubleshoot for Makefile

Check following variables in `Makefile` if you can't build the application
```
GPP = <path to g++>
LINK = <libraries>
EXE = <output executable>
FLAGS = -I./include <add your own flags>
```
An example of the Makefile configuration on `wsl2`with `mingw64`.
```
GPP = /mnt/c/msys64/mingw64/bin/g++.exe
LINK = -lglfw3 -lglew32 -lopengl32 -lgdi32 -limm32
EXE = ./build/App.exe
FLAGS = -g -Wall -I./include
```
Another example for linux system (I did on arch)
```
GPP = g++
LINK = -lglfw -lGLEW -lGL
EXE = ./build/App
FLAGS = -g -Wall -I./include
```
For windows, if you want to build this in powershell, you need to get gnu make and change file paths, and other things accordingly. Recommond you to install `wsl2` (most distro will be fine), so you can launch `bash` in your windows terminal. 
