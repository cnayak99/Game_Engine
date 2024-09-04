# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Iinclude -I/usr/include/SDL2 -MMD -MP

# Library paths for SDL2
LIBS = -lSDL2

# Source files
SRC = $(wildcard src/*.cpp)

# Header files
HEADERS = $(wildcard include/*.h)

# Object files
OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)

# Output executable
TARGET = game_engine_app

# Default target
all: $(TARGET)

# Rule to create the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Rule to create object files
%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Include dependency files
-include $(DEP)

# Clean up build files
clean:
	rm -f $(OBJ) $(DEP) $(TARGET)
