RAYLIB ?= ./external/raylib-5.5/src/

all:
    gcc src/main.cpp -I $(RAYLIB) -L $(RAYLIB) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11