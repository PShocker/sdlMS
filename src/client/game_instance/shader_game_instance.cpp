// #include "shader_game_instance.h"
// #include "src/client/window/window.h"
// #include <iterator>

// constexpr unsigned char greyscale_dx[] = {
// #embed "../../../shaders/bin/grayscale.frag.dxil"
// };

// constexpr unsigned char greyscale_msl[] = {
// #embed "../../../shaders/bin/grayscale.frag.msl"
// };

// constexpr unsigned char greyscale_spv[] = {
// #embed "../../../shaders/bin/grayscale.frag.spv"
// };

// typedef struct {
//   shader_enum type;
//   const unsigned char *dxil_shader_source;
//   unsigned int dxil_shader_source_len;
//   const unsigned char *msl_shader_source;
//   unsigned int msl_shader_source_len;
//   const unsigned char *spirv_shader_source;
//   unsigned int spirv_shader_source_len;
//   int num_samplers;
//   int num_uniform_buffers;
//   SDL_GPUShader *shader;
//   SDL_GPURenderState *state;
// } ShaderData;

// static ShaderData shaders[] = {
//     {
//         shader_enum::grayscale,
//         greyscale_dx,
//         sizeof(greyscale_dx),
//         greyscale_msl,
//         sizeof(greyscale_msl),
//         greyscale_spv,
//         sizeof(greyscale_spv),
//         1,
//         0,
//         NULL,
//         NULL,
//     },
// };

// bool shader_game_instance::init() {
//   auto device = SDL_GetGPURendererDevice(window::renderer);
//   if (!device) {
//     SDL_Log("Couldn't get GPU device");
//     return false;
//   }
//   auto formats = SDL_GetGPUShaderFormats(device);
//   if (formats == SDL_GPU_SHADERFORMAT_INVALID) {
//     SDL_Log("Couldn't get supported shader formats: %s", SDL_GetError());
//     return false;
//   }
//   for (int i = 0; i < std::size(shaders); ++i) {
//     auto *data = &shaders[i];
//     SDL_GPUShaderCreateInfo info = {};
//     SDL_GPURenderStateCreateInfo createinfo = {};
//     if (formats & SDL_GPU_SHADERFORMAT_SPIRV) {
//       SDL_Log("SDL_GPU_SHADERFORMAT_SPIRV");
//       info.format = SDL_GPU_SHADERFORMAT_SPIRV;
//       info.code = data->spirv_shader_source;
//       info.code_size = data->spirv_shader_source_len;
//     } else if (formats & SDL_GPU_SHADERFORMAT_DXIL) {
//       SDL_Log("SDL_GPU_SHADERFORMAT_DXIL");
//       info.format = SDL_GPU_SHADERFORMAT_DXIL;
//       info.code = data->dxil_shader_source;
//       info.code_size = data->dxil_shader_source_len;
//     } else if (formats & SDL_GPU_SHADERFORMAT_MSL) {
//       SDL_Log("SDL_GPU_SHADERFORMAT_MSL");
//       info.format = SDL_GPU_SHADERFORMAT_MSL;
//       info.code = data->msl_shader_source;
//       info.code_size = data->msl_shader_source_len;
//     } else {
//       SDL_Log("No supported shader format found");
//       return false;
//     }
//     info.num_samplers = data->num_samplers;
//     info.num_uniform_buffers = data->num_uniform_buffers;
//     info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
//     data->shader = SDL_CreateGPUShader(device, &info);
//     if (!data->shader) {
//       SDL_Log("Couldn't create shader: %s", SDL_GetError());
//       return false;
//     }
//     createinfo.fragment_shader = data->shader;
//     data->state = SDL_CreateGPURenderState(window::renderer, &createinfo);
//     if (!data->state) {
//       SDL_Log("Couldn't create render state: %s", SDL_GetError());
//       return false;
//     }
//   }
//   return true;
// }

// void shader_game_instance::start(shader_enum type) {
//   SDL_SetGPURenderState(window::renderer, shaders[(int)type].state);
// }

// void shader_game_instance::end() {
//   SDL_SetGPURenderState(window::renderer, NULL);
// }