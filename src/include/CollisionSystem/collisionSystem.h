#pragma once

#include <vector>
#include <unordered_map>

#include "../ECS/entityManager.h"
#include "../ECS/componentStorage.h"

class CollisionSystem {
public:
    void Update(EntityManager& entityManager) {
        m_collisions.clear();

        auto& entities = entityManager.GetAllEntity();

        for (Entity entity : entities) {
            if (!entityManager.HasComponent<CollisionComponent>(entity) ||
                !entityManager.HasComponent<TransformComponent>(entity)) {
                continue;
            }

            auto& collision =
                entityManager.GetComponent<CollisionComponent>(entity);
            auto& transform =
                entityManager.GetComponent<TransformComponent>(entity);

            collision.size = transform.scale;
        }

        for (size_t i = 0; i < entities.size(); ++i) {
            Entity a = entities[i];

            if (!HasValidCollider(entityManager, a))
                continue;

            for (size_t j = i + 1; j < entities.size(); ++j) {
                Entity b = entities[j];

                if (!HasValidCollider(entityManager, b))
                    continue;

                if (IsOverlapping(entityManager, a, b)) {
                    m_collisions[a].push_back(b);
                    m_collisions[b].push_back(a);
                }
            }
        }
    }

    bool IsColliding(Entity entity) const {
        auto it = m_collisions.find(entity);

        return it != m_collisions.end() && !it->second.empty();
    }

    const std::vector<Entity>& GetCollisions(Entity entity) const {
        static const std::vector<Entity> empty;

        auto it = m_collisions.find(entity);
        return it != m_collisions.end() ? it->second : empty;
    }

    std::vector<std::string> GetCollisionTags(EntityManager& entityManager, Entity entity) const {
        std::vector<std::string> tags;

        for (Entity other : GetCollisions(entity)) {
            if (entityManager.HasComponent<TagComponent>(other)) {
                tags.push_back(
                    entityManager.GetComponent<TagComponent>(other).tag
                );
            }
        }

        return tags;
    }

private:
    std::unordered_map<Entity, std::vector<Entity>> m_collisions;

    bool HasValidCollider(EntityManager& entityManager, Entity entity) const {
        return entityManager.HasComponent<CollisionComponent>(entity) &&
               entityManager.HasComponent<TransformComponent>(entity);
    }

    bool IsOverlapping(
        EntityManager& entityManager,
        Entity a,
        Entity b
    ) const {
        const auto& transformA =
            entityManager.GetComponent<TransformComponent>(a);
        const auto& colliderA =
            entityManager.GetComponent<CollisionComponent>(a);

        const auto& transformB =
            entityManager.GetComponent<TransformComponent>(b);
        const auto& colliderB =
            entityManager.GetComponent<CollisionComponent>(b);

        Rectangle rectA = {
            transformA.position.x,
            transformA.position.y,
            colliderA.size.x,
            colliderA.size.y
        };

        Rectangle rectB = {
            transformB.position.x,
            transformB.position.y,
            colliderB.size.x,
            colliderB.size.y
        };

        return CheckCollisionRecs(rectA, rectB);
    }
};