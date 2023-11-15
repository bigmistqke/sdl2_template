# Define source and destination directories
BUILD_DIR := build
SRC_DIR := src
# Define asset folder
ASSETS_DIR := assets
ASSETS_BUILD_DIR := $(BUILD_DIR)/assets

# Compiler and flags
CXX := g++
CXXFLAGS := -I/opt/homebrew/include -std=c++17 # macOS: other macOS-option is /usr/local/include
SDL2_FLAGS := $(shell sdl2-config --cflags --libs) -lSDL2_ttf -lSDL2_mixer # macOS: install with `brew install SDL2`
GLEW_FLAGS := -lGLEW # macOS: install with `brew install glew`
GL_FLAGS := -framework OpenGL # macOS

# Define the rsync command
RSYNC := rsync -ru

# List of files to copy (you can specify file patterns or individual files)
FILES_TO_COPY := $(wildcard $(ASSETS_DIR)/*)

# Target to copy new or modified files
copy_assets:
	mkdir -p $(ASSETS_BUILD_DIR)
	$(RSYNC) $(FILES_TO_COPY) $(ASSETS_BUILD_DIR)

# Define the default target (all) and its dependencies
all: build

# Pattern rule to build C++ source files
build/%: src/%.cpp
	$(CXX) $(CXXFLAGS) $^ $(SDL2_FLAGS) $(GLEW_FLAGS) $(GL_FLAGS) $(foreach dep,$(deps), $(dep)) -o $@

# Pattern rule to compile object files
build/%.o: src/%.cpp src/%.h
	$(CXX) $(CXXFLAGS) $(SDL2_FLAGS) -c $< -o $@

# Target to build SDLConsole
build/SDLConsole: $(SRC_DIR)/SDLConsole.cpp $(SRC_DIR)/SDLConsole.h
	$(CXX) $(CXXFLAGS) $(SDL2_FLAGS) -c $< -o $(BUILD_DIR)/SDLConsole.o

# Target to build SDLAudioManager
build/SDLAudioManager: $(SRC_DIR)/SDLAudioManager.cpp $(SRC_DIR)/SDLAudioManager.h
	$(CXX) $(CXXFLAGS) $(SDL2_FLAGS) -c $< -o $(BUILD_DIR)/SDLAudioManager.o

# Clean target
clean:
	rm -f $(BUILD_DIR)/*

# Run target
run:
	@if [ -z "$(file)" ]; then \
		echo "Usage: make run file=<source_file> deps=<comma-separated-dependencies>"; \
	else \
		make copy_assets; \
		src=$(SRC_DIR)/$(basename $(notdir $(file))); \
		target=$(BUILD_DIR)/$(basename $(notdir $(file))); \
		dep_list="$(deps)"; \
		IFS=',' read -ra dep_array <<< "$$dep_list"; \
		dep_objects=""; \
		for dep in $${dep_array[@]}; do \
			if [ ! -f $(BUILD_DIR)/$$dep.o ]; then \
				make build/$$dep.o; \
			fi; \
			dep_objects+=" $(BUILD_DIR)/$$dep.o"; \
		done; \
		make $$target deps="$$dep_objects"; \
		$$target; \
	fi

# Ignore "clean" and "run" when running other make targets
%: build/%.cpp
	@:
