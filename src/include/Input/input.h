#pragma once
#include <string>
#include <unordered_map>
#include "raylib.h"

class Input {
public:
    std::unordered_map<std::string, int> keymap;

    std::unordered_map<int, bool> currentFrame;
    std::unordered_map<int, bool> lastFrame;
    
    void CreateAction(std::string action, int key) {
        keymap[action] = key;
        std::cout << "Action key " << action << " has been created successfully" << std::endl;
    }

    void Update() {
        lastFrame = currentFrame;

        for (auto& km : keymap) {
            currentFrame[km.second] = IsKeyDown(km.second);
        }
    }

    bool IsActionPressed(std::string action) {
        auto it = keymap.find(action);

        if(it == keymap.end()) {
            std::cout<< "keymap " << action << " is not found";
            return false;
        }

        int key = it->second;

        return currentFrame[key] && !lastFrame[key];
    }

    bool IsActionReleased(std::string action) {
        auto it = keymap.find(action);

        if(it == keymap.end()) {
            std::cout<< "keymap " << action << " is not found";
            return false;
        }

        int key = it->second;

        return !currentFrame[key] && lastFrame[key];
    }

    bool IsActionHeld(std::string action) {
        auto it = keymap.find(action);

        if(it == keymap.end()) {
            std::cout<< "keymap " << action << " is not found";
            return false;
        }

        int key = it->second;
        return currentFrame[key];
    };

    void CleanUp();


};
