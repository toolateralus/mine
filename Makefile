CXX = clang++
CXXFLAGS = -g -std=c++2b -Ithirdparty/imgui
LDFLAGS = -lGL -lGLEW -lglfw -lassimp -ldraco
SRC = $(wildcard src/*.cpp) $(wildcard thirdparty/imgui/*.cpp) thirdparty/imgui/imgui_impl_glfw.cpp thirdparty/imgui/imgui_impl_opengl3.cpp
OBJ_DIR = obj
OBJ = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))
TARGET = bin/mine

.PHONY: all clean run run_asan

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

run_asan: $(TARGET)
	ASAN_OPTIONS=detect_leaks=1 ./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)