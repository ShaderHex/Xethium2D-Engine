#pragma once
#include "raylib.h"

#include "../Scene/Scene.h"
#include "../../vendor/nlohmann/json/json.hpp"
#include "../ECS/entityManager.h"
#include "../ECS/componentStorage.h"

#include <fstream>

using json = nlohmann::ordered_json;

class SceneSerializer {
public:
    void SerializeScene(Scene& scene, const std::string& filePath) {
        std::ofstream file(filePath);

        if (!file.is_open()) {
            std::cout << "Failed to open file and save\n";
            return;
        }

        json sceneData = json::object();
        sceneData["magic"] = "XETHIUM_SCENE";
        sceneData["entities"] = json::array();

        TransformComponent entity_transform;
        RenderableComponent entity_renderable;
        SpriteComponent entity_sprite;
        CameraComponent entity_camera;
        CollisionComponent entity_collision;
        TagComponent entity_tag;
        AnimatedSpriteComponent entity_animation;
        MaterialComponent entity_material;
        
        EntityManager& ecs = scene.m_ecsManager;
        
        for (Entity e : ecs.GetAllEntity()) {
            json entity = json::object();
            
            entity["id"] = e;
            
            if (ecs.HasComponent<TagComponent>(e)) {
                entity_tag = ecs.GetComponent<TagComponent>(e);
                entity["Component"]["Tag"] = entity_tag.tag;
            }

            if (ecs.HasComponent<TransformComponent>(e)) {
                entity_transform = ecs.GetComponent<TransformComponent>(e);
                entity["Component"]["Transform"] = {
                    {"position", {entity_transform.position.x, entity_transform.position.y}},
                    {"rotation", entity_transform.rotation_degree},
                    {"scale", {entity_transform.scale.x, entity_transform.scale.y}}
                };
            }

            if (ecs.HasComponent<RenderableComponent>(e)) {
                entity_renderable = ecs.GetComponent<RenderableComponent>(e);
                if (entity_renderable.is_visible) {
                    entity["Component"]["Renderable"] = true;
                } else {
                    entity["Component"]["Renderable"] = false;
                }
            }

            if (ecs.HasComponent<SpriteComponent>(e)) {
                entity_sprite = ecs.GetComponent<SpriteComponent>(e);
                entity["Component"]["Sprite"] = {{"texture_path", entity_sprite.texturePath}};
                entity["Component"]["Sprite"]["Tint"] = {
                    entity_sprite.tint.r, entity_sprite.tint.g, entity_sprite.tint.b, entity_sprite.tint.a
                };
            }

            if (ecs.HasComponent<CameraComponent>(e)) {
                entity_camera = ecs.GetComponent<CameraComponent>(e);
                entity["Component"]["Camera"]["Zoom"] = entity_camera.zoom;
                entity["Component"]["Camera"]["RotationDegree"] = entity_camera.rotation_degree;
                entity["Component"]["Camera"]["offset"] = {entity_camera.offset.x, entity_camera.offset.y};
            }

            if (ecs.HasComponent<CollisionComponent>(e)) {
                entity_collision = ecs.GetComponent<CollisionComponent>(e);
                entity["Component"]["Collision"]["Size"] = {
                    entity_collision.size.x, entity_collision.size.y
                };
                entity["Component"]["Collision"]["DebugColor"] = {
                    entity_collision.DebugColor.r, entity_collision.DebugColor.g, entity_collision.DebugColor.b, entity_collision.DebugColor.a
                };
            }

            if (ecs.HasComponent<AnimatedSpriteComponent>(e)) {
                entity_animation = ecs.GetComponent<AnimatedSpriteComponent>(e);
                entity["Component"]["AnimatedSprite"]["CurrentFrame"] = entity_animation.current_frame;
                entity["Component"]["AnimatedSprite"]["TotalFrames"] =  entity_animation.elapsed_time;

                entity["Component"]["AnimatedSprite"]["FrameWidth"] = entity_animation.frame_width;
                entity["Component"]["AnimatedSprite"]["FrameHeight"] = entity_animation.frame_height;

                entity["Component"]["AnimatedSprite"]["StartX"] = entity_animation.start_x;
                entity["Component"]["AnimatedSprite"]["StartY"] = entity_animation.start_y;

                entity["Component"]["AnimatedSprite"]["FrameDuration"] = entity_animation.frame_duration;
                entity["Component"]["AnimatedSprite"]["ElapsedTime"] = entity_animation.elapsed_time;

                entity["Component"]["AnimatedSprite"]["Loop"] = entity_animation.loop;
                entity["Component"]["AnimatedSprite"]["Playing"] = entity_animation.playing;
            }

            if (ecs.HasComponent<MaterialComponent>(e)) {
                entity_material = ecs.GetComponent<MaterialComponent>(e);
                entity["Component"]["Material"] = { 
                    entity_material.color.r, entity_material.color.g, entity_material.color.b, entity_material.color.a
                };
            }

            sceneData["entities"].push_back(entity);
        }


        file << sceneData.dump(4);
    }

    void DeserializeScene(Scene& scene, const std::string& path) {
        EntityManager& ecs = scene.m_ecsManager;

        std::ifstream file(path);

        if (!file.is_open()) {
            std::cout << "Failed to load scene: " << path << std::endl;
            return;
        }

        json sceneData;

        try{
            file >> sceneData;
        }
        catch (const json::parse_error& e) {
            std::cout << "Failed to parse scene: " << e.what() << '\n';
                return;
        }

        if(sceneData["magic"] != "XETHIUM_SCENE") {
            std::cout << "Invalid scene file\n";
            return;
        }

        for (auto& entity : sceneData["entities"]) {
            Entity e = ecs.CreateEntity();
        
            if (entity["Component"].contains("Transform")) {
                auto& transform = entity["Component"]["Transform"];

                TransformComponent entity_component;

                entity_component.position.x = transform["position"][0];
                entity_component.position.y = transform["position"][1];

                entity_component.rotation_degree = transform["rotation"];

                entity_component.scale.x = transform["scale"][0];
                entity_component.scale.y = transform["scale"][1];

                ecs.AddComponent<TransformComponent>(e, entity_component);
            }

            if (entity["Component"].contains("Renderable")) {
                auto renderable = entity["Component"]["Renderable"].get<bool>();

                RenderableComponent entity_component;

                entity_component.is_visible = renderable;

                ecs.AddComponent<RenderableComponent>(e, entity_component);
            }

            if (entity["Component"].contains("Sprite")) {
                auto& sprite = entity["Component"]["Sprite"];

                SpriteComponent entity_component;

                entity_component.texturePath = sprite["texture_path"];

                entity_component.tint.r = sprite["Tint"][0];
                entity_component.tint.g = sprite["Tint"][1];
                entity_component.tint.b = sprite["Tint"][2];
                entity_component.tint.a = sprite["Tint"][3];

                ecs.AddComponent<SpriteComponent>(e, entity_component);
            }

            if (entity["Component"].contains("Collision")) {
                auto& collision = entity["Component"]["Collision"];

                CollisionComponent entity_component;

                entity_component.size.x = collision["Size"][0];
                entity_component.size.y = collision["Size"][1];

                entity_component.DebugColor.r = collision["DebugColor"][0];
                entity_component.DebugColor.g = collision["DebugColor"][1];
                entity_component.DebugColor.b = collision["DebugColor"][2];
                entity_component.DebugColor.a = collision["DebugColor"][3];

                ecs.AddComponent<CollisionComponent>(e, entity_component);
            }

            if (entity["Component"].contains("AnimatedSprite")) {
                auto& animatedSprite = entity["Component"]["AnimatedSprite"];

                AnimatedSpriteComponent entity_component;

                entity_component.current_frame = animatedSprite["CurrentFrame"];
                entity_component.total_frames = animatedSprite["TotalFrames"];
                entity_component.frame_width = animatedSprite["FrameWidth"];
                entity_component.frame_height = animatedSprite["FrameHeight"];
                entity_component.start_x = animatedSprite["StartX"];
                entity_component.start_y = animatedSprite["StartY"];
                entity_component.frame_duration = animatedSprite["FrameDuration"];
                entity_component.elapsed_time = animatedSprite["ElapsedTime"];
                entity_component.loop = animatedSprite["Loop"].get<bool>();
                entity_component.playing = animatedSprite["Playing"].get<bool>();

                ecs.AddComponent<AnimatedSpriteComponent>(e, entity_component);
            }

            if (entity["Component"].contains("Tag")) {
                auto& tag = entity["Component"]["Tag"];

                TagComponent entity_component;

                entity_component.tag = tag.get<std::string>();
                
                ecs.AddComponent<TagComponent>(e, entity_component);
            }

            if (entity["Component"].contains("Camera")) {
                auto& camera = entity["Component"]["Camera"];

                CameraComponent entity_component;

                entity_component.zoom = camera["Zoom"];
                entity_component.rotation_degree = camera["RotationDegree"];

                entity_component.offset.x = camera["offset"][0];
                entity_component.offset.y = camera["offset"][1];

                ecs.AddComponent<CameraComponent>(e, entity_component);
            }

            if (entity["Component"].contains("Material")) {
                auto material = entity["Component"]["Material"];

                MaterialComponent entity_component;

                entity_component.color.r = material[0];
                entity_component.color.r = material[1];
                entity_component.color.r = material[2];
                entity_component.color.r = material[3];

                ecs.AddComponent<MaterialComponent>(e, entity_component);
            }
        }
    }
};