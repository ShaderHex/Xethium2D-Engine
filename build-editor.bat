g++ src/editor.cpp vendor/rlImGui/rlImGui.cpp vendor/imgui/imgui*.cpp vendor/imgui/misc/cpp/imgui_stdlib.cpp -o build/editor -Ivendor/raylib/src -Ivendor/rlImGui -Ivendor/imgui -Lvendor/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm
cd build
editor.exe