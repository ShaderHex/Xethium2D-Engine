g++ src/main.cpp vendor/rlImGui/rlImGui.cpp vendor/imgui/imgui*.cpp -o build/main -Ivendor/raylib/src -Ivendor/rlImGui -Ivendor/imgui -Lvendor/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm
cd build
main.exe