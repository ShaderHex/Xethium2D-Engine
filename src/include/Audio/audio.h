#pragma once

#include "raylib.h"
#include <vector>
#include <algorithm>
#include <iostream>

class AudioManager {
public:
    void Init()
    {
        InitAudioDevice();
        std::cout << "Audio manager initialized!\n";
    }

    void Update()
    {
        for (Music* music : m_playingMusic)
        {
            UpdateMusicStream(*music);
        }
    }

    // Short audio effects that are loaded into memory
    void PlayAudio(Sound& sound)
    {
        PlaySound(sound);
    }

    void StopAudio(Sound& sound)
    {
        StopSound(sound);
    }

    // Long audio that is streamed from disk
    void PlayMusic(Music& music)
    {
        PlayMusicStream(music);

        // Don't add the same music twice
        if (std::find(m_playingMusic.begin(), m_playingMusic.end(), &music)
            == m_playingMusic.end())
        {
            m_playingMusic.push_back(&music);
        }
    }

    void StopMusic(Music& music)
    {
        StopMusicStream(music);

        auto it = std::find(
            m_playingMusic.begin(),
            m_playingMusic.end(),
            &music
        );

        if (it != m_playingMusic.end())
        {
            m_playingMusic.erase(it);
        }
    }

    void PauseMusic(Music& music)
    {
        PauseMusicStream(music);
    }

    void ResumeMusic(Music& music)
    {
        ResumeMusicStream(music);
    }

    void MasterVolume(float volume)
    {
        SetMasterVolume(volume);
    }

    void Shutdown()
    {
        CloseAudioDevice();
        m_playingMusic.clear();
    }

private:
    std::vector<Music*> m_playingMusic;
};