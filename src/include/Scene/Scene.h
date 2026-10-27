#pragma once
#include "../ECS/entityManager.h"
#include "../Input/input.h"
#include "../Camera/camera.h"
#include "../Audio/audio.h"
#include "../AssetManager/assetManager.h"
#include "../UpdateParams.h"

class Scene {
public:
    Scene()
        : m_cameraManager(m_ecsManager)
    {
    }

    virtual ~Scene() = default;
    virtual void OnEnter() {}
    virtual void OnExit() {}

    virtual void Update(UpdateParams& params) {}
    virtual void Render() {}

    EntityManager m_ecsManager;
    CameraManager m_cameraManager;
    AssetManager m_assetManager;

};