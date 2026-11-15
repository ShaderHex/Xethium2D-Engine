#pragma once

#include "Input/input.h"
#include "Audio/audio.h"
#include "CollisionSystem/collisionSystem.h"
#include "ECS/entityManager.h"

struct UpdateParams {
    float dt;
    Input& inputManager;
    AudioManager& audioManager;
    CollisionSystem& collisionSystem;
    EntityManager& entityManager;
};