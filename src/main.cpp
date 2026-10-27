#include <iostream>

#include "raylib.h"
#include "include/ECS/entityManager.h"
#include "include/ECS/componentStorage.h"
#include "include/Engine/XEngine.h"
#include "include/Scene/Scene.h"
#include "include/Input/keycode.h"
#include "include/Audio/audio.h"
#include "include/CollisionSystem/collisionSystem.h"

Sound coin_click;
Music country;
int highFPS = 0;

class Game : public IXE {
    void DebugMenu() override {
        ImGui::Begin("Debug Menu");
            ImGui::Text("Loaded from memory: %d", this->GetSceneManager().GetActiveScene().m_assetManager.GetLoadedCount());
            ImGui::Text("Loaded from Cache: %d", this->GetSceneManager().GetActiveScene().m_assetManager.GetCachedCount());
        ImGui::End();
    }
};

class TestScene : public Scene {
public:
    void OnEnter() override {
        entity = m_ecsManager.CreateEntity();
        
        transform_entity.position = { 0, 0 };
        transform_entity.scale = { 100, 100 };
        m_ecsManager.AddComponent<TransformComponent>(entity, transform_entity);

        RenderableComponent render_entity;
        m_ecsManager.AddComponent<RenderableComponent>(entity, render_entity);

        // Non-Animated sprite

        SpriteComponent sprite_entity;
        sprite_entity.texture = m_assetManager.LoadTexture("scarfy.png");
        m_ecsManager.AddComponent<SpriteComponent>(entity, sprite_entity);
        
        // Animated sprite

        AnimatedSpriteComponent anim_entity;
        anim_entity.total_frames = 6;
        anim_entity.frame_duration = 0.1f;
        anim_entity.start_x = 0;
        anim_entity.start_y = 0;
        anim_entity.frame_width = 128;
        anim_entity.frame_height = 128;
        anim_entity.loop = true;
        anim_entity.playing = true;
        
        m_ecsManager.AddComponent<AnimatedSpriteComponent>(entity, anim_entity);

        camera = m_ecsManager.CreateEntity();

        CameraComponent cameraComponent;
        cameraComponent.zoom = 1.0f;
        cameraComponent.offset = { 400, 200 };

        TransformComponent transform_camera;
        transform_camera.position = { 0, 0 };

        m_ecsManager.AddComponent<CameraComponent>(camera, cameraComponent);
        m_ecsManager.AddComponent<TransformComponent>(camera, transform_camera);
        m_ecsManager.AddComponent<CollisionComponent>(entity, CollisionComponent{ { 100, 100 }, RED });

        
        m_cameraManager.SetActiveCamera(camera);
        
        Entity entity_2 = m_ecsManager.CreateEntity();
        
        TransformComponent transform_entity_2;
        transform_entity_2.position = { 200, 0 };
        transform_entity_2.scale = { 100, 100 };
        RenderableComponent render_entity_2;
        m_ecsManager.AddComponent<TransformComponent>(entity_2, transform_entity_2);
        m_ecsManager.AddComponent<RenderableComponent>(entity_2, render_entity_2);
        CollisionComponent collision_entity_2;
        collision_entity_2.size = { 100, 100 };
        m_ecsManager.AddComponent<CollisionComponent>(entity_2, collision_entity_2);
        TagComponent tag_entity_2;
        tag_entity_2.tag = "Fire";
        m_ecsManager.AddComponent<TagComponent>(entity_2, tag_entity_2);
        
        MaterialComponent material_entity_2;
        material_entity_2.color = RED;
        m_ecsManager.AddComponent<MaterialComponent>(entity_2, material_entity_2);

        Entity entity_3 = m_ecsManager.CreateEntity();

        TransformComponent transform_entity_3;
        transform_entity_3.position = { -200, 0 };
        transform_entity_3.scale = { 100, 100 };
        RenderableComponent render_entity_3;
        m_ecsManager.AddComponent<TransformComponent>(entity_3, transform_entity_3);
        m_ecsManager.AddComponent<RenderableComponent>(entity_3, render_entity_3);
        CollisionComponent collision_entity_3;
        collision_entity_3.size = { 100, 100 };
        m_ecsManager.AddComponent<CollisionComponent>(entity_3, collision_entity_3);
        TagComponent tag_entity_3;
        tag_entity_3.tag = "Water";
        m_ecsManager.AddComponent<TagComponent>(entity_3, tag_entity_3);

        MaterialComponent material_entity_3;
        material_entity_3.color = BLUE;
        m_ecsManager.AddComponent<MaterialComponent>(entity_3, material_entity_3);
    }

    void OnExit() override {
        std::cout << "Scene exited TestScene\n";
    }
    
    void Update(UpdateParams& params) override {
        if (params.inputManager.IsActionHeld("right")) {
            TransformComponent& transform_e = m_ecsManager.GetComponent<TransformComponent>(entity);
            
            transform_e.position.x += 20.5 * params.dt;
        }

        if (params.inputManager.IsActionHeld("left")) {
            TransformComponent& transform_e = m_ecsManager.GetComponent<TransformComponent>(entity);
            
            transform_e.position.x -= 20.5 * params.dt;
        }

        // if (m_ecsManager.HasComponent<TransformComponent>(camera))
        TransformComponent& transform_camera = m_ecsManager.GetComponent<TransformComponent>(camera);
        // transform_camera.position.x += 15*params.dt;

        if (params.collisionSystem.IsColliding(entity)) {
            // std::cout <<"COLLIDING!!!!!\n";
            for (std::string tag : params.collisionSystem.GetCollisionTags(params.entityManager, entity)) {
                std::cout << "Collided with entity: " << tag << '\n';
            }
        }

    }

    void Render() override {
        
        int fps = GetFPS();
        int currentFPS = fps;
        if (currentFPS > highFPS) {
            highFPS = currentFPS;
        }
        // DrawText("Hello Xethium from TestScene", 100, 160, 30, BLACK);
        DrawText(TextFormat("%d FPS", fps), 10, 10, 45, BLACK);
        DrawText(TextFormat("%d High FPS", highFPS), 10, 60, 45, BLACK);
    }

private:
    Entity entity;
    Entity camera;
    TransformComponent transform_entity;
};

int main(void)
{
    windowSpec winSpec;
    winSpec.width = 2650;
    winSpec.height = 1200;
    winSpec.title = "Xethium Test Game";
    winSpec.fpsLimit = 0;
    winSpec.renderWidth = 2650;
    winSpec.renderHeight = 1200;
    winSpec.vsync = false;
    winSpec.pixelated = true;
    winSpec.scalingMode = ScalingMode::Fit; // Stretch, Fit, Integer available

    Game engine;
    engine.Init(winSpec);
    TestScene testScene;
    engine.GetSceneManager().SetScene(&testScene);

    engine.m_audioManager.MasterVolume(0.0f);
    coin_click = engine.GetSceneManager().GetActiveScene().m_assetManager.LoadSound("coin.wav");
    country = engine.GetSceneManager().GetActiveScene().m_assetManager.LoadMusicStream("country.mp3");
    engine.m_audioManager.PlayMusic(country);

    
    engine.m_inputManager.CreateAction("right", X_KEY_D);
    engine.m_inputManager.CreateAction("left", X_KEY_A);

    engine.Run();

    return 0;
}