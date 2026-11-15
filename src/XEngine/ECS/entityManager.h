
#pragma once

#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <any>
#include <algorithm>
#include <stdexcept>

using Entity = uint32_t;

class EntityManager {
private:
    using ComponentMap = std::unordered_map<Entity, std::any>;

public:

    Entity CreateEntity() {
        if (!m_freeEntities.empty()) {
            Entity id = m_freeEntities.back();
            m_freeEntities.pop_back();

            m_allEntities.push_back(id);
            return id;
        }

        Entity id = m_nextEntity++;
        m_allEntities.push_back(id);

        return id;
    }

    void DestroyEntity(Entity e) {
        auto entityIt = std::find(m_allEntities.begin(), m_allEntities.end(), e);

        if (entityIt == m_allEntities.end())
            return;

        for (auto& [typeIdx, storage] : m_components) {
            auto* map = std::any_cast<ComponentMap>(&storage);

            if (map)
                map->erase(e);
        }

        m_allEntities.erase(entityIt);

        m_freeEntities.push_back(e);

        std::cout << "Destroyed Entity ID: " << e << "\n";
    }

    template <typename T>
    void AddComponent(Entity e, const T& component) {
        if (!IsEntityValid(e))
            throw std::out_of_range("Invalid entity");

        auto& storage = getStorage<T>();

        storage.insert_or_assign(e, component);

        std::cout << "Added component to Entity ID: " << e << "\n";
    }

    template <typename T>
    T& GetComponent(Entity e) {
        auto typeIt = m_components.find(typeid(T));

        if (typeIt == m_components.end())
            throw std::out_of_range("Component storage does not exist");

        auto& storage = std::any_cast<ComponentMap&>(typeIt->second);

        auto componentIt = storage.find(e);

        if (componentIt == storage.end())
            throw std::out_of_range("Entity does not have this component");

        return std::any_cast<T&>(componentIt->second);
    }

    template <typename T>
    const T& GetComponent(Entity e) const {
        auto typeIt = m_components.find(typeid(T));

        if (typeIt == m_components.end())
            throw std::out_of_range("Component storage does not exist");

        const auto& storage = std::any_cast<const ComponentMap&>(typeIt->second);

        auto componentIt = storage.find(e);

        if (componentIt == storage.end())
            throw std::out_of_range("Entity does not have this component");

        return std::any_cast<const T&>(componentIt->second);
    }

    template <typename T>
    bool HasComponent(Entity e) const {
        auto it = m_components.find(typeid(T));

        if (it == m_components.end())
            return false;

        const auto& storage = std::any_cast<const ComponentMap&>(it->second);

        return storage.find(e) != storage.end();
    }

    template <typename T>
    void RemoveComponent(Entity e) {
        auto it = m_components.find(typeid(T));

        if (it == m_components.end())
            return;

        auto& storage = std::any_cast<ComponentMap&>(it->second);

        storage.erase(e);

        std::cout << "Removed component from Entity ID: "<< e << "\n";
    }

    bool IsEntityValid(Entity e) const {
        return std::find(m_allEntities.begin(), m_allEntities.end(), e) != m_allEntities.end();
    }

    std::vector<Entity>& GetAllEntity() {
        return m_allEntities;
    }

    const std::vector<Entity>& GetAllEntity() const {
        return m_allEntities;
    }

    Entity GetEntityCount() const {
        return static_cast<Entity>(m_allEntities.size());
    }

private:

    template <typename T>
    ComponentMap& getStorage() {
        auto type = std::type_index(typeid(T));

        auto it = m_components.find(type);

        if (it == m_components.end()) {
            it = m_components.emplace(type, ComponentMap{}).first;
        }

        return std::any_cast<ComponentMap&>(it->second);
    }

    std::unordered_map<std::type_index, std::any> m_components;

    std::vector<Entity> m_freeEntities;
    std::vector<Entity> m_allEntities;

    Entity m_nextEntity = 0;
};
