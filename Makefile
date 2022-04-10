# Using MinGW, if using other system, modify following
GPP = /mnt/c/msys64/mingw64/bin/g++.exe
LINK = -lglfw3 -lopengl32 -lgdi32 -limm32
EXE = ./build/App.exe
#-----------------------------------------------------
#--------Following should be changed carefully--------

FLAGS = -g -Wall -I./include -Llib
SRC = ./main.cpp
SRC += ./include/GLEW/glew.c
SRC += $(wildcard ./include/IMGUI/imgui*.cpp)
OBJS = $(addprefix ./build/, $(addsuffix .o, $(basename $(notdir $(SRC)))))

#---------------------------Make rules ---------------
all: $(EXE)
	@echo Build complete

$(EXE): $(OBJS)
	$(GPP) -o $@ $^ $(FLAGS) $(LINK)

./build/main.o: main.cpp *.h
	$(GPP) -c -o $@ $< $(FLAGS)

./build/glew.o: ./include/GLEW/glew.c ./include/GLEW/*.h
	$(GPP) -c -o $@ $< $(FLAGS)

./build/%.o: ./include/IMGUI/%.cpp
	$(GPP) -c -o $@ $< $(FLAGS)

debug : 
	@echo $(OBJS)

clean: 
	rm -f $(OBJS) $(EXE)

 
