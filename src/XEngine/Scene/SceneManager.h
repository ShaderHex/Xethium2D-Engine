#pragma once
#include "Scene.h"
#include "../Input/input.h"
#include "../Camera/Camera.h"
#include "../Audio/audio.h"
#include "../UpdateParams.h"

class SceneManager {
public:
    void SetScene(Scene* scene) {
        if (m_currentActiveScene) 
            m_currentActiveScene->OnExit();

        m_currentActiveScene = scene;

        if (m_currentActiveScene) 
            m_currentActiveScene->OnEnter();
    }

    void Update(UpdateParams& params) {
        if (m_currentActiveScene)
            m_currentActiveScene->Update(params);
    }

    void Render() {
        if (m_currentActiveScene)
            m_currentActiveScene->Render();
    }

    // CameraManager& GetCameraManager() {
    //     return m_cameraManager;
    // }

    Scene& GetActiveScene() { return *m_currentActiveScene; }
private:
    Scene* m_currentActiveScene = nullptr;
};