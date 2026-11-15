#pragma once

#include "../ECS/entityManager.h"
#include "../ECS/componentStorage.h"

class AnimationSystem {
public:
    void Update(EntityManager& ecs, float deltaTime) {
        for (Entity e : ecs.GetAllEntity()) {
            if (!ecs.HasComponent<AnimatedSpriteComponent>(e))
                continue;

            auto& anim =
                ecs.GetComponent<AnimatedSpriteComponent>(e);

            if (!anim.playing || anim.total_frames <= 1)
                continue;

            if (anim.frame_duration <= 0.0f)
                continue;

            anim.elapsed_time += deltaTime;

            while (anim.elapsed_time >= anim.frame_duration) {
                anim.elapsed_time -= anim.frame_duration;

                if (anim.current_frame + 1 < anim.total_frames) {
                    anim.current_frame++;
                } else if (anim.loop) {
                    anim.current_frame = 0;
                } else {
                    anim.current_frame = anim.total_frames - 1;
                    anim.playing = false;
                    anim.elapsed_time = 0.0f;
                    break;
                }
            }
        }
    }
};