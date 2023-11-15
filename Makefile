CXX := g++
CXXFLAGS := -I/opt/homebrew/include # macos: other macos-option is /usr/local/include
SDL2_FLAGS := $(shell sdl2-config --cflags --libs) # macos: install with `brew install SDL2`
GLEW_FLAGS := -lGLEW # macos: install with `brew install glew`
GL_FLAGS := -framework OpenGL # macos
TARGET := build/main
SRC := src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(SDL2_FLAGS) $(GLEW_FLAGS) $(GL_FLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: clean $(TARGET)
	./$(TARGET)
