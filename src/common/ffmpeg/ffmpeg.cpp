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

game_audio_cache
ffmpeg::decode_audio_pcm(const std::vector<uint8_t> &audio_data) {
  game_audio_cache gac;

  if (audio_data.empty()) {
    return gac;
  }

  // 1. 打开输入（直接从内存读取）
  AVIOContext *avio_ctx = nullptr;
  AVFormatContext *fmt_ctx = nullptr;

  uint8_t *buffer = (uint8_t *)av_malloc(audio_data.size());
  if (!buffer)
    return gac;
  memcpy(buffer, audio_data.data(), audio_data.size());

  avio_ctx = avio_alloc_context(buffer, audio_data.size(), 0, nullptr, nullptr,
                                nullptr, nullptr);
  if (!avio_ctx) {
    av_free(buffer);
    return gac;
  }

  fmt_ctx = avformat_alloc_context();
  fmt_ctx->pb = avio_ctx;

  if (avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr) < 0) {
    avformat_close_input(&fmt_ctx);
    avio_context_free(&avio_ctx);
    return gac;
  }

  if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
    avformat_close_input(&fmt_ctx);
    return gac;
  }

  // 2. 找到音频流
  int audio_idx =
      av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
  if (audio_idx < 0) {
    avformat_close_input(&fmt_ctx);
    return gac;
  }

  // 3. 创建解码器
  const AVCodec *codec =
      avcodec_find_decoder(fmt_ctx->streams[audio_idx]->codecpar->codec_id);
  if (!codec) {
    avformat_close_input(&fmt_ctx);
    return gac;
  }

  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
  avcodec_parameters_to_context(codec_ctx,
                                fmt_ctx->streams[audio_idx]->codecpar);
  avcodec_open2(codec_ctx, codec, nullptr);

  // 设置多线程解码
  codec_ctx->thread_count = std::thread::hardware_concurrency();

  // 4. 设置输出格式（统一转为交错格式）
  AVSampleFormat in_fmt = codec_ctx->sample_fmt;
  bool is_planar = av_sample_fmt_is_planar(in_fmt);
  AVSampleFormat out_fmt =
      is_planar ? av_get_packed_sample_fmt(in_fmt) : in_fmt;

  // 映射到SDL格式
  switch (out_fmt) {
  case AV_SAMPLE_FMT_U8:
    gac.spec.format = SDL_AUDIO_U8;
    break;
  case AV_SAMPLE_FMT_S16:
    gac.spec.format = SDL_AUDIO_S16LE;
    break;
  case AV_SAMPLE_FMT_S32:
    gac.spec.format = SDL_AUDIO_S32LE;
    break;
  case AV_SAMPLE_FMT_FLT:
    gac.spec.format = SDL_AUDIO_F32LE;
    break;
  default:
    gac.spec.format = SDL_AUDIO_S16LE;
    break;
  }
  gac.spec.freq = codec_ctx->sample_rate;
  gac.spec.channels = codec_ctx->ch_layout.nb_channels;

  // 5. 解码并转换
  AVPacket *packet = av_packet_alloc();
  AVFrame *frame = av_frame_alloc();
  std::vector<uint8_t> pcm_data;

  int channels = gac.spec.channels;
  int bytes_per_sample = av_get_bytes_per_sample(out_fmt);

  while (av_read_frame(fmt_ctx, packet) >= 0) {
    if (packet->stream_index != audio_idx) {
      av_packet_unref(packet);
      continue;
    }

    if (avcodec_send_packet(codec_ctx, packet) == 0) {
      while (avcodec_receive_frame(codec_ctx, frame) == 0) {
        int nb_samples = frame->nb_samples;
        size_t old_size = pcm_data.size();

        if (is_planar) {
          // 平面 → 交错（手动交织）
          pcm_data.resize(old_size + nb_samples * channels * bytes_per_sample);
          uint8_t *dst = pcm_data.data() + old_size;

          for (int s = 0; s < nb_samples; s++) {
            for (int c = 0; c < channels; c++) {
              memcpy(dst, frame->extended_data[c] + s * bytes_per_sample,
                     bytes_per_sample);
              dst += bytes_per_sample;
            }
          }
        } else {
          // 交错格式直接拷贝
          int size = nb_samples * channels * bytes_per_sample;
          pcm_data.resize(old_size + size);
          memcpy(pcm_data.data() + old_size, frame->data[0], size);
        }
        av_frame_unref(frame);
      }
    }
    av_packet_unref(packet);
  }

  // 6. 冲刷解码器
  avcodec_send_packet(codec_ctx, nullptr);
  while (avcodec_receive_frame(codec_ctx, frame) == 0) {
    // 同样的转换逻辑...
    int nb_samples = frame->nb_samples;
    size_t old_size = pcm_data.size();

    if (is_planar) {
      pcm_data.resize(old_size + nb_samples * channels * bytes_per_sample);
      uint8_t *dst = pcm_data.data() + old_size;
      for (int s = 0; s < nb_samples; s++) {
        for (int c = 0; c < channels; c++) {
          memcpy(dst, frame->extended_data[c] + s * bytes_per_sample,
                 bytes_per_sample);
          dst += bytes_per_sample;
        }
      }
    } else {
      int size = nb_samples * channels * bytes_per_sample;
      pcm_data.resize(old_size + size);
      memcpy(pcm_data.data() + old_size, frame->data[0], size);
    }
    av_frame_unref(frame);
  }

  // 7. 清理
  gac.data = std::move(pcm_data);
  av_frame_free(&frame);
  av_packet_free(&packet);
  avcodec_free_context(&codec_ctx);
  avformat_close_input(&fmt_ctx);
  av_freep(&avio_ctx->buffer);
  avio_context_free(&avio_ctx);

  return gac;
}