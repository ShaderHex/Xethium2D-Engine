#pragma once
#include "raylib.h"
#include <iostream>
#include <unordered_map>
#include <string>

class AssetManager {
public:
    Texture2D LoadTexture(const char* path) {
        if (!path) {
            TraceLog(LOG_ERROR, "AssetManager::LoadTexture: null path");
            return {};
        }

        auto it = m_textures.find(path);

        if (it != m_textures.end()) {
            m_cachedCount++;
            return it->second;
        }
        
        Image image = ::LoadImage(path);
        Texture2D texture = ::LoadTextureFromImage(image);
        ::UnloadImage(image);

        m_textures[path] = texture;
        
        m_loadedCount++;
        return texture;
    };

    Sound LoadSound(const char* path) {
        if (!path) {
            TraceLog(LOG_ERROR, "AssetManager::LoadSound: null path");
            return {};
        }

        auto it = m_sound.find(path);
        if (it != m_sound.end()) {
            m_cachedCount++;
            return it->second;
        }

        Sound sound;
        sound = ::LoadSound(path);

        m_sound[path] = sound;

        m_loadedCount++;
        return sound;
    }

    Music LoadMusicStream(const char* path) {
        if (!path) {
            TraceLog(LOG_ERROR, "AssetManager::LoadMusicStream: null path");
            return {};
        }
        
        auto it = m_music.find(path);

        if (it != m_music.end()) {
            m_cachedCount++;
            return it->second;
        }

        Music music = ::LoadMusicStream(path);
        m_loadedCount++;

        m_music[path] = music;

        return music;
    }

    void Clean() {
        for (auto& [path, texture] : m_textures) {
            ::UnloadTexture(texture);
        }

        for (auto& [path, sound] : m_sound) {
            ::UnloadSound(sound);
        }

        for (auto& [path, music] : m_music) {
            ::UnloadMusicStream(music);
        }

        m_textures.clear();
        m_sound.clear();
        m_music.clear();
    }

    int GetLoadedCount() { return m_loadedCount; }
    int GetCachedCount() { return m_cachedCount; }

private:
    std::unordered_map<std::string, Texture2D> m_textures;
    std::unordered_map<std::string, Sound> m_sound;
    std::unordered_map<std::string, Music> m_music;

    int m_loadedCount = 0;
    int m_cachedCount = 0;
};