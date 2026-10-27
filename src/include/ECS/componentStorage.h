#pragma once
#include "raylib.h"

struct TransformComponent {
    Vector2 position = { 0, 0 };
    Vector2 scale = { 10, 10 };
    float rotation_degree = 0;
};

struct RenderableComponent {
    bool is_visible = true;
};

struct SpriteComponent {
    Texture2D texture;
    Color tint = WHITE;
};

struct CameraComponent {
    float zoom = 1.0f;
    float rotation_degree = 0.0f;;
    Vector2 offset;
};

struct CollisionComponent {
    Vector2 size;
    Color DebugColor;
};

struct TagComponent {
    std::string tag;
};

struct AnimatedSpriteComponent {
    int current_frame = 0;
    int total_frames = 1;

    int frame_width = 32;
    int frame_height = 32;

    int start_x = 0;
    int start_y = 0;

    float frame_duration = 0.1f;
    float elapsed_time = 0.0f;

    bool loop = true;
    bool playing = true;
};

struct MaterialComponent {
    Color color = WHITE;
};