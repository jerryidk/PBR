# Using MinGW (windows only)
# If you want to build on windows, install minGW and mesa dependencies
#GPP = /mnt/c/msys64/mingw64/bin/g++.exe
#LINK = -lglfw3 -lglew32 -lopengl32 -lgdi32 -limm32
#EXE = ./build/App.exe
#FLAGS = -g -Wall -I./include

#Linux
GPP = g++
LINK = -lglfw -lGLEW -lGL
EXE = ./build/App
FLAGS = -g -w -I./include

#-----------------------------------------------------
#--------Following should be changed carefully--------
SRC = ./main.cpp
SRC += $(wildcard ./include/IMGUI/imgui*.cpp)
OBJS = $(addprefix ./build/, $(addsuffix .o, $(basename $(notdir $(SRC)))))
#---------------------------Make rules ---------------
all: $(EXE)
	@echo Build complete

$(EXE): $(OBJS)
	$(GPP) -o $@ $^ $(FLAGS) $(LINK)

./build/main.o: main.cpp *.h
	$(GPP) -c -o $@ $< $(FLAGS)

./build/%.o: ./include/IMGUI/%.cpp
	$(GPP) -c -o $@ $< $(FLAGS)

debug : 
	@echo $(OBJS)

clean: 
	rm -f $(OBJS) $(EXE)

 
