module;

#include "wz/Property.hpp"

export module components:sound;

export struct SoundWarp
{
    std::vector<uint8_t> pcm_data;
    unsigned int offset = 0; // 记录当前播放的位置
    unsigned int delay = 0;  // 播放声音后下次播放的间隔,若为-1,则表示播放1次
    bool circulate = false;       // 当前是否循环

    SoundWarp(wz::Node *node);
    SoundWarp() = default;
    static SoundWarp *load(wz::Node *node);
};

export struct Sound
{
    SoundWarp *sou = nullptr;
    Sound(wz::Node *node);
    Sound() = default;

    static bool init();
    static inline std::array<SoundWarp *, 16> sound_list;
    enum Sound_Type{
        Bgm,
        CharacterAttack,
        CharacterSkill,
        MobHit,
        MobDie
    };
};
