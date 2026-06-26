#include "ffmpeg.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

game_audio_cache ffmpeg::decode_audio_pcm(const std::vector<uint8_t> &audio_data) {
  game_audio_cache gac;
  std::vector<uint8_t> pcm_data;

  if (audio_data.empty()) {
    return gac;
  }

  // 1. 分配输入缓冲区
  unsigned char *buffer = (unsigned char *)av_malloc(audio_data.size());
  if (!buffer)
    return gac;
  memcpy(buffer, audio_data.data(), audio_data.size());

  // 2. 创建内存IO上下文
  AVIOContext *avio_ctx = avio_alloc_context(
      buffer, audio_data.size(), 0, nullptr, nullptr, nullptr, nullptr);
  if (!avio_ctx) {
    av_free(buffer);
    return gac;
  }

  // 3. 打开格式上下文（自动检测格式）
  AVFormatContext *fmt_ctx = avformat_alloc_context();
  if (!fmt_ctx) {
    avio_context_free(&avio_ctx);
    av_free(buffer);
    return gac;
  }
  fmt_ctx->pb = avio_ctx;

  // 关键：不指定输入格式，让 FFmpeg 自动探测
  if (avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr) < 0) {
    avformat_free_context(fmt_ctx);
    avio_context_free(&avio_ctx);
    av_free(buffer);
    return gac;
  }

  // 获取流信息
  if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
    avformat_close_input(&fmt_ctx);
    avio_context_free(&avio_ctx);
    av_free(buffer);
    return gac;
  }

  // 打印检测到的格式（调试用）
  std::cout << "Detected format: " << fmt_ctx->iformat->name << std::endl;

  // 4. 找到音频流
  const AVCodec *codec = nullptr;
  int audio_idx =
      av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
  if (audio_idx < 0 || !codec) {
    avformat_close_input(&fmt_ctx);
    avio_context_free(&avio_ctx);
    av_free(buffer);
    return gac;
  }

  std::cout << "Audio codec: " << codec->name << std::endl;

  // 5. 创建解码器上下文
  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
  if (!codec_ctx) {
    avformat_close_input(&fmt_ctx);
    avio_context_free(&avio_ctx);
    av_free(buffer);
    return gac;
  }

  AVCodecParameters *codec_params = fmt_ctx->streams[audio_idx]->codecpar;
  if (avcodec_parameters_to_context(codec_ctx, codec_params) < 0) {
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
    avio_context_free(&avio_ctx);
    av_free(buffer);
    return gac;
  }

  // 设置多线程
  codec_ctx->thread_count = std::thread::hardware_concurrency();
  codec_ctx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;

  if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
    avio_context_free(&avio_ctx);
    av_free(buffer);
    return gac;
  }

  // 6. 准备解码结构
  AVPacket *packet = av_packet_alloc();
  AVFrame *frame = av_frame_alloc();
  if (!packet || !frame) {
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
    avio_context_free(&avio_ctx);
    av_free(buffer);
    return gac;
  }
  //   填充音频结构
  switch (codec_ctx->sample_fmt) {
  case AV_SAMPLE_FMT_U8:
  case AV_SAMPLE_FMT_U8P: {
    gac.spec.format = SDL_AUDIO_U8;
    break;
  }
  case AV_SAMPLE_FMT_S16:
  case AV_SAMPLE_FMT_S16P: {
    gac.spec.format = SDL_AUDIO_S16LE;
    break;
  }
  case AV_SAMPLE_FMT_S32:
  case AV_SAMPLE_FMT_S32P: {
    gac.spec.format = SDL_AUDIO_S32LE;
    break;
  }
  case AV_SAMPLE_FMT_FLT:
  case AV_SAMPLE_FMT_FLTP: {
    gac.spec.format = SDL_AUDIO_F32LE;
    break;
  }
  default: {
    gac.spec.format = SDL_AUDIO_S16LE;
    break;
  }
  }
  gac.spec.freq = codec_ctx->sample_rate;
  gac.spec.channels = codec_ctx->ch_layout.nb_channels;

  // 7. 解码循环
  while (av_read_frame(fmt_ctx, packet) >= 0) {
    if (packet->stream_index == audio_idx) {
      int ret = avcodec_send_packet(codec_ctx, packet);
      if (ret == 0) {
        while (true) {
          ret = avcodec_receive_frame(codec_ctx, frame);
          if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
          }
          if (ret < 0) {
            break;
          }

          // 获取 PCM 数据
          int channels = codec_ctx->ch_layout.nb_channels;
          int data_size = av_samples_get_buffer_size(
              nullptr, channels, frame->nb_samples, codec_ctx->sample_fmt, 1);

          if (data_size > 0) {
            size_t old_size = pcm_data.size();
            pcm_data.resize(old_size + data_size);
            memcpy(pcm_data.data() + old_size, frame->data[0], data_size);
          }

          av_frame_unref(frame);
        }
      }
    }
    av_packet_unref(packet);
  }

  // 8. 冲刷解码器
  avcodec_send_packet(codec_ctx, nullptr);
  while (true) {
    int ret = avcodec_receive_frame(codec_ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      break;
    }
    if (ret < 0) {
      break;
    }

    int channels = codec_ctx->ch_layout.nb_channels;
    int data_size = av_samples_get_buffer_size(
        nullptr, channels, frame->nb_samples, codec_ctx->sample_fmt, 1);

    if (data_size > 0) {
      size_t old_size = pcm_data.size();
      pcm_data.resize(old_size + data_size);
      memcpy(pcm_data.data() + old_size, frame->data[0], data_size);
    }

    av_frame_unref(frame);
  }
  gac.data = pcm_data;

  // 9. 清理资源
  av_frame_free(&frame);
  av_packet_free(&packet);
  avcodec_free_context(&codec_ctx);
  avformat_close_input(&fmt_ctx);
  avio_context_free(&avio_ctx);
  av_free(buffer);

  return gac;
}