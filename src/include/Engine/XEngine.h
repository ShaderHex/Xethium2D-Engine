
#pragma once

#include "raylib.h"
#include "raymath.h"

#include "../Scene/SceneManager.h"
#include "../Input/input.h"
#include "../Audio/audio.h"
#include "../CollisionSystem/collisionSystem.h"
#include "../AnimationSystem/animationSystem.h"

#include "rlImGui.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>

enum class ScalingMode {
    Stretch,
    Fit,
    Integer
};

struct windowSpec {
    int width = 1920;
    int height = 1080;
    const char* title = "No title";

    int renderWidth = 320;
    int renderHeight = 180;

    bool vsync = true;
    int fpsLimit = 60;

    bool pixelated = true;
    ScalingMode scalingMode = ScalingMode::Fit;
};

class IXE {
public:
    void Init(windowSpec& winSpec) {
        if (winSpec.width <= 0 || winSpec.height <= 0 ||
            winSpec.renderWidth <= 0 || winSpec.renderHeight <= 0) {
            TraceLog(LOG_ERROR, "Invalid window or render resolution.");
            return;
        }

        if (winSpec.vsync)
            SetConfigFlags(FLAG_VSYNC_HINT);

        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(winSpec.width, winSpec.height, winSpec.title);

        TraceLog(LOG_INFO, "Monitor: %d x %d",
            GetMonitorWidth(0), GetMonitorHeight(0));

        TraceLog(LOG_INFO, "Window: %d x %d",
            GetScreenWidth(), GetScreenHeight());

        SetWindowFocused();

        if (winSpec.fpsLimit > 0)
            SetTargetFPS(winSpec.fpsLimit);

        m_scalingMode = winSpec.scalingMode;

        m_renderTexture = LoadRenderTexture(
            winSpec.renderWidth,
            winSpec.renderHeight
        );

        if (m_renderTexture.id == 0) {
            TraceLog(LOG_ERROR, "Failed to create render texture.");
            CloseWindow();
            return;
        }

        SetTextureFilter(
            m_renderTexture.texture,
            winSpec.pixelated
                ? TEXTURE_FILTER_POINT
                : TEXTURE_FILTER_BILINEAR
        );

        m_audioManager.Init();

        rlImGuiSetup(true);

        m_initialized = true;
    }

    void Run() {
        if (!m_initialized) {
            TraceLog(LOG_ERROR, "Xethium Engine was not initialized successfully.");
            return;
        }

        while (!WindowShouldClose()) {
            float dt = GetFrameTime();

            m_audioManager.Update();
            m_inputManager.Update();

            auto& scene = m_sceneManager.GetActiveScene();
            auto& ecs = scene.m_ecsManager;

            m_animationSystem.Update(ecs, dt);

            UpdateParams updateParams = {
                dt,
                m_inputManager,
                m_audioManager,
                m_collisionSystem,
                ecs
            };

            m_sceneManager.Update(updateParams);
            m_collisionSystem.Update(ecs);

            if (scene.m_cameraManager.HasActiveCamera())
                scene.m_cameraManager.Update();

            BeginTextureMode(m_renderTexture);
            ClearBackground(RAYWHITE);

            if (scene.m_cameraManager.HasActiveCamera()) {
                BeginMode2D(
                    scene.m_cameraManager.GetActiveCamera()
                );

                Render();

                EndMode2D();
            } else {
                DrawRectangle(
                    0,
                    0,
                    m_renderTexture.texture.width,
                    m_renderTexture.texture.height,
                    GRAY
                );
            }

            m_sceneManager.Render();

            EndTextureMode();

            BeginDrawing();
            ClearBackground(BLACK);

            DrawScaledRenderTexture();

            rlImGuiBegin();
            DebugMenu();
            rlImGuiEnd();

            EndDrawing();
        }

        DestroyGame();
    }

    virtual void DebugMenu() {}

    SceneManager& GetSceneManager() {
        return m_sceneManager;
    }

    Input m_inputManager;
    AudioManager m_audioManager;
    CollisionSystem m_collisionSystem;
    AnimationSystem m_animationSystem;

private:
    void DrawScaledRenderTexture() {
        float rw = (float)m_renderTexture.texture.width;
        float rh = (float)m_renderTexture.texture.height;

        float ww = (float)GetScreenWidth();
        float wh = (float)GetScreenHeight();

        float sx = ww / rw;
        float sy = wh / rh;

        Rectangle source = {
            0.0f,
            0.0f,
            rw,
            -rh
        };

        Rectangle dest = {};

        switch (m_scalingMode) {
            case ScalingMode::Stretch: {
                dest = {
                    0.0f,
                    0.0f,
                    ww,
                    wh
                };
                break;
            }

            case ScalingMode::Fit: {
                float scale = std::min(sx, sy);

                float dw = rw * scale;
                float dh = rh * scale;

                dest = {
                    (ww - dw) * 0.5f,
                    (wh - dh) * 0.5f,
                    dw,
                    dh
                };
                break;
            }

            case ScalingMode::Integer: {
                float scale = std::min(sx, sy);

                if (scale >= 1.0f)
                    scale = std::floor(scale);

                float dw = rw * scale;
                float dh = rh * scale;

                dest = {
                    (ww - dw) * 0.5f,
                    (wh - dh) * 0.5f,
                    dw,
                    dh
                };
                break;
            }
        }

        DrawTexturePro(
            m_renderTexture.texture,
            source,
            dest,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );
    }

    void Render() {
        EntityManager& ecs =
            m_sceneManager.GetActiveScene().m_ecsManager;

        for (Entity e : ecs.GetAllEntity()) {
            if (!ecs.HasComponent<TransformComponent>(e))
                continue;

            if (!ecs.HasComponent<RenderableComponent>(e))
                continue;

            const auto& render_e =
                ecs.GetComponent<RenderableComponent>(e);

            if (!render_e.is_visible)
                continue;

            const auto& transform_e =
                ecs.GetComponent<TransformComponent>(e);

            if (ecs.HasComponent<SpriteComponent>(e)) {
                const auto& sprite_e =
                    ecs.GetComponent<SpriteComponent>(e);

                Rectangle sourceRec = {
                    0.0f,
                    0.0f,
                    (float)sprite_e.texture.width,
                    (float)sprite_e.texture.height
                };

                if (ecs.HasComponent<AnimatedSpriteComponent>(e)) {
                    const auto& anim =
                        ecs.GetComponent<AnimatedSpriteComponent>(e);

                    sourceRec = {
                        (float)(
                            anim.start_x +
                            anim.current_frame * anim.frame_width
                        ),
                        (float)anim.start_y,
                        (float)anim.frame_width,
                        (float)anim.frame_height
                    };
                }

                Rectangle destRec = {
                    transform_e.position.x,
                    transform_e.position.y,
                    transform_e.scale.x,
                    transform_e.scale.y
                };

                DrawTexturePro(
                    sprite_e.texture,
                    sourceRec,
                    destRec,
                    { 0.0f, 0.0f },
                    transform_e.rotation_degree,
                    sprite_e.tint
                );
            } else {
                if (!ecs.HasComponent<MaterialComponent>(e)) 
                    continue;

                const auto& material_e = ecs.GetComponent<MaterialComponent>(e);
                DrawRectangle(
                    (int)transform_e.position.x,
                    (int)transform_e.position.y,
                    (int)transform_e.scale.x,
                    (int)transform_e.scale.y,
                    material_e.color
                );
            }
        }
    }

    void DestroyGame() {
        if (!m_initialized)
            return;

        m_sceneManager.GetActiveScene().OnExit();
        m_sceneManager.GetActiveScene().m_assetManager.Clean();

        rlImGuiShutdown();
        UnloadRenderTexture(m_renderTexture);
        CloseWindow();

        m_initialized = false;
    }

    SceneManager m_sceneManager;
    RenderTexture2D m_renderTexture = { 0 };

    ScalingMode m_scalingMode = ScalingMode::Fit;
    bool m_initialized = false;
};
