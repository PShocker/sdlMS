#include "VideoSystem.h"

void VideoSystem::run(World &world)
{
    if (world.components_exist_of_type<Video>())
    {
        for (auto &[index, vid] : world.get_components<Video>())
        {
            // SDL_CreateThread(UpdateSystem::update_video, "decodeThread", (void *)vid);
            update_video(vid);
        }
    }
}

void VideoSystem::update_video(Video *video)
{
    // 解码并显示视频帧
    AVPacket packet;
    AVFrame *frame = av_frame_alloc();
    while (av_read_frame(video->formatContext, &packet) >= 0)
    {
        if (packet.stream_index == video->videoStreamIndex)
        {
            avcodec_send_packet(video->codecContext, &packet);
            if (avcodec_receive_frame(video->codecContext, frame) == 0)
            {
                SDL_UpdateYUVTexture(video->texture, NULL,
                                     frame->data[0], frame->linesize[0],
                                     frame->data[1], frame->linesize[1],
                                     frame->data[2], frame->linesize[2]);
                av_packet_unref(&packet);
                break;
            }
        }
        av_packet_unref(&packet);
    }
    av_frame_free(&frame);
    return;
}