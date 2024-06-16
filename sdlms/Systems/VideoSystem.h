#pragma once
#include "Systems/System.h"
#include "Components/Video.h"

class VideoSystem : public System
{
public:
    void run(World &world) override;

private:
    void update_video(Video *video); // 视频
};
