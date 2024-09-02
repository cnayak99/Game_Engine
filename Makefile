# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall

# Include and library paths for SDL2
INCLUDES = -I/usr/include/SDL2
LIBS = -lSDL2

# Source files
SRC = main.cpp sdl_setup.cpp

# Object files
OBJ = $(SRC:.cpp=.o)

# Output executable
TARGET = game_engine_app

# Default target
all: $(TARGET)

# Rule to create the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Rule to create object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJ) $(TARGET)
