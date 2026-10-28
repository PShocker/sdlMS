#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_CURSOR_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_CURSOR_RENDER_SYSTEM_H_
namespace sdlms {
class CursorRenderSystem {
 private:
  static void RenderCursor();
  static void RenderHand();

 public:
  static bool Render();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_CURSOR_RENDER_SYSTEM_H_
