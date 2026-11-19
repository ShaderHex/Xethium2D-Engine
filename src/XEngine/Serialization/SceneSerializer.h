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
                entity["Component"]["Tag"] = {entity_tag.tag};
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
                entity["Component"]["Camera"]["Zoom"] = {entity_camera.zoom};
                entity["Component"]["Camera"]["RotationDegree"] = {entity_camera.rotation_degree};
                entity["Component"]["Camera"]["offset"] = {{entity_camera.offset.x, entity_camera.offset.y}};
            }

            if (ecs.HasComponent<CollisionComponent>(e)) {
                entity_collision = ecs.GetComponent<CollisionComponent>(e);
                entity["Component"]["Collision"]["Size"] = {
                    {entity_collision.size.x, entity_collision.size.y}
                };
                entity["Component"]["Collision"]["DebugColor"] = {
                    {entity_collision.DebugColor.r, entity_collision.DebugColor.g, entity_collision.DebugColor.b, entity_collision.DebugColor.a}
                };
            }

            // TODO: change the array to integer/bool
            if (ecs.HasComponent<AnimatedSpriteComponent>(e)) {
                entity_animation = ecs.GetComponent<AnimatedSpriteComponent>(e);
                entity["Component"]["AnimatedSprite"]["CurrentFrame"] = { entity_animation.current_frame };
                entity["Component"]["AnimatedSprite"]["TotalFrames"] = { entity_animation.elapsed_time };

                entity["Component"]["AnimatedSprite"]["FrameWidth"] = { entity_animation.frame_width };
                entity["Component"]["AnimatedSprite"]["FrameHeight"] = { entity_animation.frame_height };

                entity["Component"]["AnimatedSprite"]["StartX"] = { entity_animation.start_x };
                entity["Component"]["AnimatedSprite"]["StartY"] = { entity_animation.start_y };

                entity["Component"]["AnimatedSprite"]["FrameDuration"] = { entity_animation.frame_duration };
                entity["Component"]["AnimatedSprite"]["ElapsedTime"] = { entity_animation.elapsed_time };

                entity["Component"]["AnimatedSprite"]["Loop"] = { entity_animation.loop };
                entity["Component"]["AnimatedSprite"]["Playing"] = { entity_animation.playing };
            }

            if (ecs.HasComponent<MaterialComponent>(e)) {
                entity_material = ecs.GetComponent<MaterialComponent>(e);
                entity["Component"]["Material"] = { 
                    {entity_material.color.r, entity_material.color.g, entity_material.color.b, entity_material.color.a}
                };
            }

            sceneData["entities"].push_back(entity);
        }


        file << sceneData.dump(4);
    }
};