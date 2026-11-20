#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "XEngine/ECS/entityManager.h"
#include "XEngine/ECS/componentStorage.h"
#include "XEngine/Engine/XEngine.h"
#include "XEngine/Scene/Scene.h"

class Game : public IXE {
public:
    void DebugMenu() override {
        auto& scene = m_sceneManager.GetActiveScene();
        auto& ecs = scene.m_ecsManager;
        const auto& entities = ecs.GetAllEntity();

        // Do not keep showing an entity that was deleted somewhere else.
        if (m_hasSelection &&
            std::find(entities.begin(), entities.end(), m_selectedEntity) == entities.end()) {
            m_hasSelection = false;
        }

        DrawHierarchy(ecs, entities);

        bool deleteSelectedEntity = false;
        DrawViewport();
        DrawInspector(ecs, scene, deleteSelectedEntity);

        if (deleteSelectedEntity && m_hasSelection) {
            ecs.DestroyEntity(m_selectedEntity);
            m_hasSelection = false;
        }

        DrawStatusBar(entities.size());
    }

private:
    void DrawViewport() {
        const float screenWidth = static_cast<float>(GetScreenWidth());
        const float screenHeight = static_cast<float>(GetScreenHeight());
        const float inspectorWidth = 340.0f;
        const float hierarchyWidth = 245.0f;
        const float sidePadding = 16.0f;
        const float viewportWidth = std::max(
            320.0f,
            screenWidth - hierarchyWidth - inspectorWidth - sidePadding * 3.0f
        );

        ImGui::SetNextWindowPos(ImVec2(hierarchyWidth + sidePadding, 8.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(viewportWidth, screenHeight - 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Viewport");

        const ImVec2 available = ImGui::GetContentRegionAvail();
        const RenderTexture2D& renderTexture = GetRenderTexture();
        const float textureWidth = static_cast<float>(renderTexture.texture.width);
        const float textureHeight = static_cast<float>(renderTexture.texture.height);

        if (available.x > 0.0f && available.y > 0.0f &&
            textureWidth > 0.0f && textureHeight > 0.0f) {

            const float textureAspect = textureWidth / textureHeight;
            const float viewportAspect = available.x / available.y;
            ImVec2 imageSize{};

            if (viewportAspect > textureAspect) {
                imageSize.y = available.y;
                imageSize.x = imageSize.y * textureAspect;
            } else {
                imageSize.x = available.x;
                imageSize.y = imageSize.x / textureAspect;
            }

            const ImVec2 cursor = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(
                cursor.x + (available.x - imageSize.x) * 0.5f,
                cursor.y + (available.y - imageSize.y) * 0.5f
            ));

            // Render textures are vertically flipped when displayed as ImGui images.
            rlImGuiImageSizeVFlip(&renderTexture.texture, {imageSize.x, imageSize.y});
        } else {
            ImGui::TextDisabled("No render texture available.");
        }

        ImGui::End();
    }

    template <typename ECS, typename EntityList>
    void DrawHierarchy(ECS& ecs, const EntityList& entities) {
        const float screenHeight = static_cast<float>(GetScreenHeight());
        ImGui::SetNextWindowPos(ImVec2(8.0f, 8.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(245.0f, screenHeight - 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Hierarchy");

        if (ImGui::Button("+ Create Entity", ImVec2(-1.0f, 0.0f))) {
            const Entity entity = ecs.CreateEntity();
            ecs.template AddComponent<TagComponent>(entity, TagComponent{ "New Entity" });
            ecs.template AddComponent<TransformComponent>(entity, TransformComponent{});
            m_selectedEntity = entity;
            m_hasSelection = true;
        }

        ImGui::Separator();

        if (entities.empty()) {
            ImGui::TextDisabled("No entities in this scene.");
        }

        for (const Entity entity : entities) {
            const std::string entityId = std::to_string(static_cast<long long>(entity));
            std::string label = "Entity " + entityId;

            if (ecs.template HasComponent<TagComponent>(entity)) {
                const auto& tag = ecs.template GetComponent<TagComponent>(entity);
                if (!tag.tag.empty()) {
                    label = tag.tag;
                }
            }

            // Entity ID keeps the ImGui label unique even when names are identical.
            label += "##entity_" + entityId;
            const bool selected = m_hasSelection && entity == m_selectedEntity;

            if (ImGui::Selectable(label.c_str(), selected)) {
                m_selectedEntity = entity;
                m_hasSelection = true;
            }
        }

        ImGui::End();
    }

    bool DrawComponentHeader(const char* label, bool defaultOpen = true) {
        return ImGui::CollapsingHeader(
            label, defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None
        );
    }

    bool DrawRemoveComponentButton(const char* componentId) {
        ImGui::Spacing();
        ImGui::PushID(componentId);
        const bool remove = ImGui::Button("Remove Component");
        ImGui::PopID();
        return remove;
    }

    template <typename ECS, typename SceneType>
    void DrawInspector(ECS& ecs, SceneType& scene, bool& deleteSelectedEntity) {
        const float screenWidth = static_cast<float>(GetScreenWidth());
        const float screenHeight = static_cast<float>(GetScreenHeight());
        ImGui::SetNextWindowPos(ImVec2(screenWidth - 348.0f, 8.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(340.0f, screenHeight - 48.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Inspector");

        if (!m_hasSelection) {
            ImGui::TextDisabled("Select an entity from the Hierarchy.");
            ImGui::End();
            return;
        }

        ImGui::Text("Entity %lld", static_cast<long long>(m_selectedEntity));
        ImGui::Separator();

        if (ecs.template HasComponent<TagComponent>(m_selectedEntity)) {
            if (DrawComponentHeader("Tag")) {
                auto& tag = ecs.template GetComponent<TagComponent>(m_selectedEntity);
                char name[256]{};
                std::snprintf(name, sizeof(name), "%s", tag.tag.c_str());
                if (ImGui::InputText("Name", name, sizeof(name))) {
                    tag.tag = name;
                }
                if (DrawRemoveComponentButton("Tag")) {
                    ecs.template RemoveComponent<TagComponent>(m_selectedEntity);
                }
            }
        }

        if (ecs.template HasComponent<TransformComponent>(m_selectedEntity)) {
            if (DrawComponentHeader("Transform")) {
                auto& transform = ecs.template GetComponent<TransformComponent>(m_selectedEntity);
                ImGui::DragFloat2("Position", &transform.position.x, 0.1f);
                ImGui::DragFloat2("Scale", &transform.scale.x, 0.1f);
                ImGui::DragFloat("Rotation", &transform.rotation_degree, 0.5f);
                if (DrawRemoveComponentButton("Transform")) {
                    ecs.template RemoveComponent<TransformComponent>(m_selectedEntity);
                }
            }
        }

        if (ecs.template HasComponent<SpriteComponent>(m_selectedEntity)) {
            if (DrawComponentHeader("Sprite")) {
                auto& sprite = ecs.template GetComponent<SpriteComponent>(m_selectedEntity);
                char buffer[256];
                strncpy(buffer, sprite.texturePath.c_str(), sizeof(buffer));
                buffer[sizeof(buffer) - 1] = '\0';

                if (ImGui::InputText("Texture path", buffer, sizeof(buffer))) {
                    sprite.texturePath = buffer;
                }
                ImGui::InputText("Texture path", &sprite.texturePath);
                if (ImGui::Button("Load / Replace Texture", ImVec2(-1.0f, 0.0f)) && sprite.texturePath[0] != '\0') {
                    sprite.texture = scene.m_assetManager.LoadTexture(sprite.texturePath.c_str());
                }
                ImGui::TextDisabled("Path is relative to the game's working directory.");
                if (DrawRemoveComponentButton("Sprite")) {
                    ecs.template RemoveComponent<SpriteComponent>(m_selectedEntity);
                }
            }
        }

        if (ecs.template HasComponent<CameraComponent>(m_selectedEntity)) {
            if (DrawComponentHeader("Camera")) {
                auto& camera = ecs.template GetComponent<CameraComponent>(m_selectedEntity);
                ImGui::DragFloat("Zoom", &camera.zoom, 0.01f, 0.01f, 100.0f);
                ImGui::DragFloat("Rotation", &camera.rotation_degree, 0.5f);
                ImGui::DragFloat2("Offset", &camera.offset.x, 0.1f);
                if (DrawRemoveComponentButton("Camera")) {
                    ecs.template RemoveComponent<CameraComponent>(m_selectedEntity);
                }
            }
        }

        if (ecs.template HasComponent<AnimatedSpriteComponent>(m_selectedEntity)) {
            if (DrawComponentHeader("Animated Sprite")) {
                auto& animation = ecs.template GetComponent<AnimatedSpriteComponent>(m_selectedEntity);
                animation.total_frames = std::max(1, animation.total_frames);
                animation.current_frame = std::max(0, std::min(animation.current_frame, animation.total_frames - 1));

                ImGui::DragInt("Total Frames", &animation.total_frames, 1.0f, 1, 100000);
                animation.total_frames = std::max(1, animation.total_frames);
                animation.current_frame = std::max(0, std::min(animation.current_frame, animation.total_frames - 1));
                ImGui::SliderInt("Current Frame", &animation.current_frame, 0, animation.total_frames - 1);
                ImGui::DragInt("Frame Width", &animation.frame_width, 1.0f, 1, 100000);
                ImGui::DragInt("Frame Height", &animation.frame_height, 1.0f, 1, 100000);
                ImGui::DragFloat("Frame Duration", &animation.frame_duration, 0.01f, 0.01f, 10.0f);
                ImGui::Checkbox("Loop", &animation.loop);
                ImGui::Checkbox("Playing", &animation.playing);
                if (DrawRemoveComponentButton("AnimatedSprite")) {
                    ecs.template RemoveComponent<AnimatedSpriteComponent>(m_selectedEntity);
                }
            }
        }

        if (ecs.template HasComponent<CollisionComponent>(m_selectedEntity)) {
            if (DrawComponentHeader("Collision")) {
                auto& collision = ecs.template GetComponent<CollisionComponent>(m_selectedEntity);
                ImGui::DragFloat2("Size", &collision.size.x, 0.1f, 0.0f, 100000.0f);
                if (DrawRemoveComponentButton("Collision")) {
                    ecs.template RemoveComponent<CollisionComponent>(m_selectedEntity);
                }
            }
        }

        if (ecs.template HasComponent<RenderableComponent>(m_selectedEntity)) {
            if (DrawComponentHeader("Renderable")) {
                auto& renderable = ecs.template GetComponent<RenderableComponent>(m_selectedEntity);
                ImGui::Checkbox("Visible", &renderable.is_visible);
                if (DrawRemoveComponentButton("Renderable")) {
                    ecs.template RemoveComponent<RenderableComponent>(m_selectedEntity);
                }
            }
        }

        if (ecs.template HasComponent<MaterialComponent>(m_selectedEntity)) {
            if (DrawComponentHeader("Material", false)) {
                ImGui::TextDisabled("MaterialComponent is attached.");
                ImGui::TextDisabled("Its editable fields are not exposed by the code provided.");
                if (DrawRemoveComponentButton("Material")) {
                    ecs.template RemoveComponent<MaterialComponent>(m_selectedEntity);
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::BeginCombo("Add Component", "Choose component...")) {
            if (!ecs.template HasComponent<TagComponent>(m_selectedEntity) && ImGui::Selectable("Tag")) {
                ecs.template AddComponent<TagComponent>(m_selectedEntity, TagComponent{ "New Entity" });
            }
            if (!ecs.template HasComponent<TransformComponent>(m_selectedEntity) && ImGui::Selectable("Transform")) {
                ecs.template AddComponent<TransformComponent>(m_selectedEntity, TransformComponent{});
            }
            if (!ecs.template HasComponent<SpriteComponent>(m_selectedEntity) && ImGui::Selectable("Sprite")) {
                ecs.template AddComponent<SpriteComponent>(m_selectedEntity, SpriteComponent{});
            }
            if (!ecs.template HasComponent<CameraComponent>(m_selectedEntity) && ImGui::Selectable("Camera")) {
                ecs.template AddComponent<CameraComponent>(m_selectedEntity, CameraComponent{});
            }
            if (!ecs.template HasComponent<AnimatedSpriteComponent>(m_selectedEntity) && ImGui::Selectable("Animated Sprite")) {
                ecs.template AddComponent<AnimatedSpriteComponent>(m_selectedEntity, AnimatedSpriteComponent{});
            }
            if (!ecs.template HasComponent<CollisionComponent>(m_selectedEntity) && ImGui::Selectable("Collision")) {
                ecs.template AddComponent<CollisionComponent>(m_selectedEntity, CollisionComponent{});
            }
            if (!ecs.template HasComponent<RenderableComponent>(m_selectedEntity) && ImGui::Selectable("Renderable")) {
                ecs.template AddComponent<RenderableComponent>(m_selectedEntity, RenderableComponent{});
            }
            if (!ecs.template HasComponent<MaterialComponent>(m_selectedEntity) && ImGui::Selectable("Material")) {
                ecs.template AddComponent<MaterialComponent>(m_selectedEntity, MaterialComponent{});
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Delete Entity", ImVec2(-1.0f, 0.0f))) {
            ImGui::OpenPopup("Confirm Delete Entity");
        }

        if (ImGui::BeginPopupModal("Confirm Delete Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Delete this entity and all its components?");
            ImGui::Spacing();
            if (ImGui::Button("Delete", ImVec2(110.0f, 0.0f))) {
                deleteSelectedEntity = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(110.0f, 0.0f))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void DrawStatusBar(std::size_t entityCount) {
        const float screenWidth = static_cast<float>(GetScreenWidth());
        const float screenHeight = static_cast<float>(GetScreenHeight());

        ImGui::SetNextWindowPos(ImVec2(0.0f, screenHeight - 26.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(screenWidth, 26.0f), ImGuiCond_Always);
        ImGui::Begin("##EditorStatus", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings);

        ImGui::Text("FPS: %d", GetFPS());
        ImGui::SameLine(105.0f);
        ImGui::Text("Frame: %.2f ms", GetFrameTime() * 1000.0f);
        ImGui::SameLine(245.0f);
        ImGui::Text("Entities: %zu", entityCount);
        ImGui::SameLine(370.0f);
        ImGui::Text("Window: %d x %d", GetScreenWidth(), GetScreenHeight());
        ImGui::SameLine(540.0f);
        ImGui::Text("Render: %d x %d", GetRenderWidth(), GetRenderHeight());

        ImGui::End();
    }

private:
    Entity m_selectedEntity{};
    bool m_hasSelection = false;
    // char m_texturePath[512] = "";
};

class EditorScene : public Scene {
public:
    void OnEnter() override {
        std::cout << "Entered editor scene.\n";

        const Entity camera = m_ecsManager.CreateEntity();
        m_ecsManager.AddComponent<TagComponent>(camera, TagComponent{ "Main Camera" });
        m_ecsManager.AddComponent<CameraComponent>(camera, CameraComponent{});
        m_ecsManager.AddComponent<TransformComponent>(camera, TransformComponent{});
        m_cameraManager.SetActiveCamera(camera);

        // A sample sprite gives the viewport something visible on first launch.
        const Entity sample = m_ecsManager.CreateEntity();

        TransformComponent transform;
        transform.position = { 0.0f, 0.0f };
        transform.scale = { 100.0f, 100.0f };

        m_ecsManager.AddComponent<TagComponent>(sample, TagComponent{ "Sample Sprite" });
        m_ecsManager.AddComponent<TransformComponent>(sample, transform);
        m_ecsManager.AddComponent<RenderableComponent>(sample, RenderableComponent{});

        SpriteComponent sprite;
        sprite.texture = m_assetManager.LoadTexture("image.png");
        m_ecsManager.AddComponent<SpriteComponent>(sample, sprite);
    }

    void Update(UpdateParams& params) override {
        (void)params;
    }

    void OnExit() override {
    }
};

int main() {
    windowSpec winSpec;
    winSpec.width = 1920;
    winSpec.height = 1080;
    winSpec.title = "Xethium Engine Editor";
    winSpec.renderWidth = 1920;
    winSpec.renderHeight = 1080;
    winSpec.fpsLimit = 60;
    winSpec.pixelated = false;
    winSpec.scalingMode = ScalingMode::Fit;
    winSpec.vsync = true;

    Game editor;
    editor.Init(winSpec);

    // EditorScene editorScene;
    Scene testScene;
    editor.m_sceneSerializer.DeserializeScene(testScene, "Test.scene");
    editor.GetSceneManager().SetScene(&testScene);



    editor.Run();
    return 0;
}
