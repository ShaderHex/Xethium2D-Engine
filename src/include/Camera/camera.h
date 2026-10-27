#pragma once
#include "raylib.h"

class CameraManager {
public:
    CameraManager(EntityManager& ecs)
        : m_ecsManager(ecs)
    {
    }

    bool HasActiveCamera() {
        return m_hasActiveCamera;
    }
    
    void SetActiveCamera(Entity e) {
        EntityManager& ecs = m_ecsManager;
        if (ecs.HasComponent<CameraComponent>(e)) {
            transform_camera = ecs.GetComponent<TransformComponent>(e);
            property_camera = ecs.GetComponent<CameraComponent>(e);

            m_activeCamera.target = transform_camera.position;
            m_activeCamera.rotation = transform_camera.rotation_degree;
            m_activeCamera.offset = property_camera.offset;
            m_activeCamera.zoom = property_camera.zoom;

            m_hasActiveCamera = true;
            entity = e;
        }
    }

    void Update() {
        transform_camera = m_ecsManager.GetComponent<TransformComponent>(entity);
        property_camera = m_ecsManager.GetComponent<CameraComponent>(entity);

        m_activeCamera.target = transform_camera.position;
        m_activeCamera.rotation = transform_camera.rotation_degree;
        m_activeCamera.offset = property_camera.offset;
        m_activeCamera.zoom = property_camera.zoom;
    }

    Camera2D& GetActiveCamera() { return m_activeCamera; }

private:
    Entity entity; 
    EntityManager& m_ecsManager;
    Camera2D m_activeCamera;
    TransformComponent transform_camera;
    CameraComponent property_camera;
    bool m_hasActiveCamera = false;
};