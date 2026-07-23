#include "video_ui_system.h"
#include "SDL3/SDL_cpuinfo.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_render.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/window/window.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/mastering_display_metadata.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
#include <string>

static uint64_t video_start;
static SDL_Texture *video_texture;

static AVFormatContext *ic = NULL;

static const AVCodec *audio_codec = NULL;
static const AVCodec *video_codec = NULL;
static AVCodecContext *audio_context = NULL;
static AVCodecContext *video_context = NULL;
static AVPacket *pkt = NULL;
static AVFrame *frame = NULL;
static double f_pts = -1.0;
static double g_pts = -1.0;

static int audio_stream;
static int video_stream;

static SDL_FRect dst;
static bool vertical;

static SDL_AudioStream *stream;

struct SwsContextContainer {
  struct SwsContext *context;
};

static SDL_PixelFormat GetTextureFormat(enum AVPixelFormat format) {
  switch (format) {
  case AV_PIX_FMT_RGB8:
    return SDL_PIXELFORMAT_RGB332;
  case AV_PIX_FMT_RGB444:
    return SDL_PIXELFORMAT_XRGB4444;
  case AV_PIX_FMT_RGB555:
    return SDL_PIXELFORMAT_XRGB1555;
  case AV_PIX_FMT_BGR555:
    return SDL_PIXELFORMAT_XBGR1555;
  case AV_PIX_FMT_RGB565:
    return SDL_PIXELFORMAT_RGB565;
  case AV_PIX_FMT_BGR565:
    return SDL_PIXELFORMAT_BGR565;
  case AV_PIX_FMT_RGB24:
    return SDL_PIXELFORMAT_RGB24;
  case AV_PIX_FMT_BGR24:
    return SDL_PIXELFORMAT_BGR24;
  case AV_PIX_FMT_0RGB32:
    return SDL_PIXELFORMAT_XRGB8888;
  case AV_PIX_FMT_0BGR32:
    return SDL_PIXELFORMAT_XBGR8888;
  case AV_PIX_FMT_NE(RGB0, 0BGR):
    return SDL_PIXELFORMAT_RGBX8888;
  case AV_PIX_FMT_NE(BGR0, 0RGB):
    return SDL_PIXELFORMAT_BGRX8888;
  case AV_PIX_FMT_RGB32:
    return SDL_PIXELFORMAT_ARGB8888;
  case AV_PIX_FMT_RGB32_1:
    return SDL_PIXELFORMAT_RGBA8888;
  case AV_PIX_FMT_BGR32:
    return SDL_PIXELFORMAT_ABGR8888;
  case AV_PIX_FMT_BGR32_1:
    return SDL_PIXELFORMAT_BGRA8888;
  case AV_PIX_FMT_YUV420P:
    return SDL_PIXELFORMAT_IYUV;
  case AV_PIX_FMT_YUYV422:
    return SDL_PIXELFORMAT_YUY2;
  case AV_PIX_FMT_UYVY422:
    return SDL_PIXELFORMAT_UYVY;
  case AV_PIX_FMT_NV12:
    return SDL_PIXELFORMAT_NV12;
  case AV_PIX_FMT_NV21:
    return SDL_PIXELFORMAT_NV21;
  case AV_PIX_FMT_P010:
    return SDL_PIXELFORMAT_P010;
  default:
    return SDL_PIXELFORMAT_UNKNOWN;
  }
}

static SDL_Colorspace GetFrameColorspace(AVFrame *frame) {
  SDL_Colorspace colorspace = SDL_COLORSPACE_SRGB;

  if (frame && frame->colorspace != AVCOL_SPC_RGB) {
    colorspace = (SDL_Colorspace)SDL_DEFINE_COLORSPACE(
        SDL_COLOR_TYPE_YCBCR, frame->color_range, frame->color_primaries,
        frame->color_trc, frame->colorspace, frame->chroma_location);
  }
  return colorspace;
}

static AVCodecContext *OpenVideoStream(AVFormatContext *ic, int stream,
                                       const AVCodec *codec) {
  AVStream *st = ic->streams[stream];
  AVCodecParameters *codecpar = st->codecpar;
  AVCodecContext *context;
  const AVCodecHWConfig *config = nullptr;
  int result;

  SDL_Log("Video stream: %s %dx%d", avcodec_get_name(codec->id),
          codecpar->width, codecpar->height);

  context = avcodec_alloc_context3(NULL);
  if (!context) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "avcodec_alloc_context3 failed");
    return NULL;
  }

  result =
      avcodec_parameters_to_context(context, ic->streams[stream]->codecpar);
  if (result < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "avcodec_parameters_to_context failed: %s",
                 av_err2str(result));
    avcodec_free_context(&context);
    return NULL;
  }
  context->pkt_timebase = ic->streams[stream]->time_base;

  result = avcodec_open2(context, codec, NULL);
  if (result < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open codec %s: %s",
                 avcodec_get_name(context->codec_id), av_err2str(result));
    avcodec_free_context(&context);
    return NULL;
  }

  return context;
}

static AVCodecContext *OpenAudioStream(AVFormatContext *ic, int stream,
                                       const AVCodec *codec) {
  AVStream *st = ic->streams[stream];
  AVCodecParameters *codecpar = st->codecpar;
  AVCodecContext *context;
  int result;

  SDL_Log("Audio stream: %s %d channels, %d Hz", avcodec_get_name(codec->id),
          codecpar->ch_layout.nb_channels, codecpar->sample_rate);

  context = avcodec_alloc_context3(NULL);
  if (!context) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "avcodec_alloc_context3 failed");
    return NULL;
  }

  result =
      avcodec_parameters_to_context(context, ic->streams[stream]->codecpar);
  if (result < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "avcodec_parameters_to_context failed: %s",
                 av_err2str(result));
    avcodec_free_context(&context);
    return NULL;
  }
  context->pkt_timebase = ic->streams[stream]->time_base;

  result = avcodec_open2(context, codec, NULL);
  if (result < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open codec %s: %s",
                 avcodec_get_name(context->codec_id), av_err2str(result));
    avcodec_free_context(&context);
    return NULL;
  }
  return context;
}

void video_ui_system::close() {
  if (video_context) {
    video_start = 0;
    f_pts = -1;
  }
  if (video_texture) {
    SDL_DestroyTexture(video_texture);
    video_texture = nullptr;
  }
  if (audio_context) {
    if (stream) {
      SDL_UnbindAudioStream(stream);
      SDL_DestroyAudioStream(stream);
      stream = nullptr;
    }
  }
  av_frame_free(&frame);
  av_packet_free(&pkt);
  avcodec_free_context(&audio_context);
  avcodec_free_context(&video_context);
  avformat_close_input(&ic);
}

void video_ui_system::open(const std::string &path) {
  auto it = std::ranges::find(system::render_systems, &render);
  if (it != system::render_systems.end()) {
    system::render_systems.erase(it);
    close();
  }

  auto result = avformat_open_input(&ic, path.c_str(), NULL, NULL);
  if (result < 0) {
    std::abort();
  }
  video_stream =
      av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, &video_codec, 0);
  if (video_stream >= 0) {
    video_context = OpenVideoStream(ic, video_stream, video_codec);
    if (!video_context) {
      std::abort();
    }
    // 设置多线程解码
    video_context->thread_count = SDL_GetNumLogicalCPUCores();
    video_context->thread_type = FF_THREAD_FRAME;
  }
  audio_stream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, video_stream,
                                     &audio_codec, 0);
  if (audio_stream >= 0) {
    audio_context = OpenAudioStream(ic, audio_stream, audio_codec);
    if (!audio_context) {
      std::abort();
    }
    // 设置多线程解码
    audio_context->thread_count = SDL_GetNumLogicalCPUCores();
    audio_context->thread_type = FF_THREAD_FRAME;
  }

  pkt = av_packet_alloc();
  if (!pkt) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "av_packet_alloc failed");
    std::abort();
  }
  frame = av_frame_alloc();
  if (!frame) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "av_frame_alloc failed");
    std::abort();
  }

  it = std::ranges::find(system::render_systems, &cursor_render_system::render);
  if (it != system::render_systems.end()) {
    system::render_systems.insert(it, render);
  }
}

static SDL_AudioFormat GetAudioFormat(int format) {
  switch (format) {
  case AV_SAMPLE_FMT_U8:
  case AV_SAMPLE_FMT_U8P:
    return SDL_AUDIO_U8;
  case AV_SAMPLE_FMT_S16:
  case AV_SAMPLE_FMT_S16P:
    return SDL_AUDIO_S16;
  case AV_SAMPLE_FMT_S32:
  case AV_SAMPLE_FMT_S32P:
    return SDL_AUDIO_S32;
  case AV_SAMPLE_FMT_FLT:
  case AV_SAMPLE_FMT_FLTP:
    return SDL_AUDIO_F32;
  default:
    /* Unsupported */
    return SDL_AUDIO_UNKNOWN;
  }
}

static void InterleaveAudio(AVFrame *frame, const SDL_AudioSpec *spec) {
  int c, n;
  int samplesize = SDL_AUDIO_BYTESIZE(spec->format);
  int framesize = SDL_AUDIO_FRAMESIZE(*spec);
  Uint8 *data = (Uint8 *)SDL_malloc(frame->nb_samples * framesize);
  /* This could be optimized with SIMD and not allocating memory each time */
  for (c = 0; c < spec->channels; ++c) {
    const Uint8 *src = frame->data[c];
    Uint8 *dst = data + c * samplesize;
    for (n = frame->nb_samples; n--;) {
      SDL_memcpy(dst, src, samplesize);
      src += samplesize;
      dst += framesize;
    }
  }
  SDL_PutAudioStreamData(stream, data, frame->nb_samples * framesize);
  SDL_free(data);
}

static bool IsPlanarAudioFormat(int format) {
  switch (format) {
  case AV_SAMPLE_FMT_U8P:
  case AV_SAMPLE_FMT_S16P:
  case AV_SAMPLE_FMT_S32P:
  case AV_SAMPLE_FMT_FLTP:
  case AV_SAMPLE_FMT_DBLP:
  case AV_SAMPLE_FMT_S64P:
    return true;
  default:
    return false;
  }
}

static void HandleAudioFrame(AVFrame *frame) {
  SDL_AudioSpec spec = {GetAudioFormat(frame->format),
                        frame->ch_layout.nb_channels, frame->sample_rate};
  if (stream == nullptr) {
    stream = SDL_CreateAudioStream(&spec, NULL);
    SDL_BindAudioStream(audio_game_instance::device_id, stream);
  }
  if (frame->ch_layout.nb_channels > 1 && IsPlanarAudioFormat(frame->format)) {
    InterleaveAudio(frame, &spec);
  } else {
    SDL_PutAudioStreamData(stream, frame->data[0],
                           frame->nb_samples * SDL_AUDIO_FRAMESIZE(spec));
  }
}

static SDL_PropertiesID CreateVideoTextureProperties(AVFrame *frame,
                                                     SDL_PixelFormat format,
                                                     int access) {
  AVFrameSideData *pSideData;
  SDL_PropertiesID props;
  int width = frame->width;
  int height = frame->height;
  SDL_Colorspace colorspace = GetFrameColorspace(frame);

  /* ITU-R BT.2408-6 recommends using an SDR white point of 203 nits, which is
   * more likely for game content */
  static const float k_flSDRWhitePoint = 203.0f;
  float flMaxLuminance = k_flSDRWhitePoint;

  if (frame->hw_frames_ctx) {
    AVHWFramesContext *frames =
        (AVHWFramesContext *)(frame->hw_frames_ctx->data);

    width = frames->width;
    height = frames->height;
    if (format == SDL_PIXELFORMAT_UNKNOWN) {
      format = GetTextureFormat(frames->sw_format);
    }
  } else {
    if (format == SDL_PIXELFORMAT_UNKNOWN) {
      format = GetTextureFormat((AVPixelFormat)frame->format);
    }
  }

  props = SDL_CreateProperties();
  SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_COLORSPACE_NUMBER,
                        colorspace);
  pSideData =
      av_frame_get_side_data(frame, AV_FRAME_DATA_MASTERING_DISPLAY_METADATA);
  if (pSideData) {
    AVMasteringDisplayMetadata *pMasteringDisplayMetadata =
        (AVMasteringDisplayMetadata *)pSideData->data;
    flMaxLuminance = (float)pMasteringDisplayMetadata->max_luminance.num /
                     pMasteringDisplayMetadata->max_luminance.den;
  } else if (SDL_COLORSPACETRANSFER(colorspace) ==
             SDL_TRANSFER_CHARACTERISTICS_PQ) {
    /* The official definition is 10000, but PQ game content is often mastered
     * for 400 or 1000 nits */
    flMaxLuminance = 1000.0f;
  }
  if (flMaxLuminance > k_flSDRWhitePoint) {
    SDL_SetFloatProperty(props, SDL_PROP_TEXTURE_CREATE_SDR_WHITE_POINT_FLOAT,
                         k_flSDRWhitePoint);
    SDL_SetFloatProperty(props, SDL_PROP_TEXTURE_CREATE_HDR_HEADROOM_FLOAT,
                         flMaxLuminance / k_flSDRWhitePoint);
  }
  SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER, format);
  SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_ACCESS_NUMBER, access);
  SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER, width);
  SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER, height);

  return props;
}

static bool GetTextureForMemoryFrame(AVFrame *frame, SDL_Texture **texture) {
  static const char *SWS_CONTEXT_CONTAINER_PROPERTY = "SWS_CONTEXT_CONTAINER";

  int texture_width = 0, texture_height = 0;
  SDL_PixelFormat texture_format = SDL_PIXELFORMAT_UNKNOWN;
  SDL_PixelFormat frame_format = GetTextureFormat((AVPixelFormat)frame->format);

  if (*texture) {
    SDL_PropertiesID props = SDL_GetTextureProperties(*texture);
    texture_format = (SDL_PixelFormat)SDL_GetNumberProperty(
        props, SDL_PROP_TEXTURE_FORMAT_NUMBER, SDL_PIXELFORMAT_UNKNOWN);
    texture_width =
        (int)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0);
    texture_height =
        (int)SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0);
  }
  if (!*texture || texture_width != frame->width ||
      texture_height != frame->height ||
      (frame_format != SDL_PIXELFORMAT_UNKNOWN &&
       texture_format != frame_format) ||
      (frame_format == SDL_PIXELFORMAT_UNKNOWN &&
       texture_format != SDL_PIXELFORMAT_ARGB8888)) {
    if (*texture) {
      SDL_DestroyTexture(*texture);
    }

    SDL_PropertiesID props;
    if (frame_format == SDL_PIXELFORMAT_UNKNOWN) {
      props = CreateVideoTextureProperties(frame, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING);
    } else {
      props = CreateVideoTextureProperties(frame, frame_format,
                                           SDL_TEXTUREACCESS_STREAMING);
    }
    *texture = SDL_CreateTextureWithProperties(window::renderer, props);
    SDL_DestroyProperties(props);
    if (!*texture) {
      return false;
    }

    if (frame_format == SDL_PIXELFORMAT_UNKNOWN ||
        SDL_ISPIXELFORMAT_ALPHA(frame_format)) {
      SDL_SetTextureBlendMode(*texture, SDL_BLENDMODE_BLEND);
    } else {
      SDL_SetTextureBlendMode(*texture, SDL_BLENDMODE_NONE);
    }
    SDL_SetTextureScaleMode(*texture, SDL_SCALEMODE_LINEAR);
  }

  switch (frame_format) {
  case SDL_PIXELFORMAT_UNKNOWN: {
    SDL_PropertiesID props = SDL_GetTextureProperties(*texture);
    struct SwsContextContainer *sws_container =
        (struct SwsContextContainer *)SDL_GetPointerProperty(
            props, SWS_CONTEXT_CONTAINER_PROPERTY, NULL);
    if (!sws_container) {
      sws_container =
          (struct SwsContextContainer *)SDL_calloc(1, sizeof(*sws_container));
      if (!sws_container) {
        return false;
      }
      auto FreeSwsContextContainer = [](void *userdata, void *value) {
        struct SwsContextContainer *sws_container =
            (struct SwsContextContainer *)value;
        if (sws_container->context) {
          sws_freeContext(sws_container->context);
        }
        SDL_free(sws_container);
      };
      SDL_SetPointerPropertyWithCleanup(props, SWS_CONTEXT_CONTAINER_PROPERTY,
                                        sws_container, FreeSwsContextContainer,
                                        NULL);
    }
    sws_container->context = sws_getCachedContext(
        sws_container->context, frame->width, frame->height,
        (AVPixelFormat)frame->format, frame->width, frame->height,
        AV_PIX_FMT_BGRA, SWS_POINT, NULL, NULL, NULL);
    if (sws_container->context) {
      uint8_t *pixels[4];
      int pitch[4];
      if (SDL_LockTexture(*texture, NULL, (void **)&pixels[0], &pitch[0])) {
        sws_scale(sws_container->context, (const uint8_t *const *)frame->data,
                  frame->linesize, 0, frame->height, pixels, pitch);
        SDL_UnlockTexture(*texture);
      }
    } else {
      SDL_SetError("Can't initialize the conversion context");
      return false;
    }
    break;
  }
  case SDL_PIXELFORMAT_IYUV:
    if (frame->linesize[0] > 0 && frame->linesize[1] > 0 &&
        frame->linesize[2] > 0) {
      SDL_UpdateYUVTexture(*texture, NULL, frame->data[0], frame->linesize[0],
                           frame->data[1], frame->linesize[1], frame->data[2],
                           frame->linesize[2]);
    } else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 &&
               frame->linesize[2] < 0) {
      SDL_UpdateYUVTexture(
          *texture, NULL,
          frame->data[0] + frame->linesize[0] * (frame->height - 1),
          -frame->linesize[0],
          frame->data[1] +
              frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1),
          -frame->linesize[1],
          frame->data[2] +
              frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1),
          -frame->linesize[2]);
    }
    break;
  default:
    if (frame->linesize[0] < 0) {
      SDL_UpdateTexture(*texture, NULL,
                        frame->data[0] +
                            frame->linesize[0] * (frame->height - 1),
                        -frame->linesize[0]);
    } else {
      SDL_UpdateTexture(*texture, NULL, frame->data[0], frame->linesize[0]);
    }
    break;
  }
  return true;
}

static bool GetTextureForFrame(AVFrame *frame, SDL_Texture **texture) {
  return GetTextureForMemoryFrame(frame, texture);
}

static void DisplayVideoTexture() {
  if (vertical) {
    SDL_RenderTextureRotated(window::renderer, video_texture, NULL, &dst, 0.0,
                             NULL, SDL_FLIP_VERTICAL);
  } else {
    SDL_RenderTexture(window::renderer, video_texture, NULL, &dst);
  }
}

static void DisplayVideoFrame() { DisplayVideoTexture(); }

static void SetVideoFrame(AVFrame *frame) {
  dst.w = (float)frame->width;
  dst.h = (float)frame->height;
  if (frame->linesize[0] < 0) {
    vertical = true;
  } else {
    vertical = false;
  }
}

static void HandleVideoFrame(AVFrame *frame) {
  /* Quick and dirty PTS handling */
  if (!video_start) {
    video_start = SDL_GetTicks();
  }
  /* Update the video texture */
  if (!GetTextureForFrame(frame, &video_texture)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Couldn't get texture for frame: %s", SDL_GetError());
    return;
  }
  SetVideoFrame(frame);
  DisplayVideoFrame();
}

bool video_ui_system::render() {
  double now = (double)(window::dt_now - video_start) / 1000.0;
  // pts控制帧率
  if (now >= g_pts && ic != NULL) {
    auto result = av_read_frame(ic, pkt);
    if (result >= 0) {
      if (pkt->stream_index == audio_stream) {
        avcodec_send_packet(audio_context, pkt);
      } else if (pkt->stream_index == video_stream) {
        avcodec_send_packet(video_context, pkt);
      }
      av_packet_unref(pkt);
    } else if (result == AVERROR_EOF) {
      if (audio_context) {
        avcodec_flush_buffers(audio_context);
      }
      if (video_context) {
        avcodec_flush_buffers(video_context);
        video_start = 0;
        f_pts = -1;
      }
      int ret = av_seek_frame(ic, -1, 0, AVSEEK_FLAG_BACKWARD);
    }
  }

  if (audio_context) {
    auto r = avcodec_receive_frame(audio_context, frame);
    if (r >= 0) {
      HandleAudioFrame(frame);
    }
  }
  if (video_context) {
    bool render = false;
    while (avcodec_receive_frame(video_context, frame) >= 0) {
      g_pts = ((double)frame->pts * video_context->pkt_timebase.num) /
              video_context->pkt_timebase.den;
      if (f_pts < 0.0) {
        f_pts = g_pts;
      }
      g_pts -= f_pts;
      HandleVideoFrame(frame);
      render = true;
    }
    if (!render) {
      DisplayVideoFrame();
    }
  }
  return true;
}