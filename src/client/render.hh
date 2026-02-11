#ifndef CLIENT_RENDER_HH
#define CLIENT_RENDER_HH
#pragma once

// This subsystem is implemented as a separate library
// so the build system can avoid recompiling every client
// source just to produce a second binary with a different renderer

namespace render
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void update_late(void);
void begin_frame(void);
void end_frame(void);
void render(void);
void layout(void);
} // namespace render

namespace render
{
SDL_WindowFlags window_flags(void);
} // namespace render

#endif
