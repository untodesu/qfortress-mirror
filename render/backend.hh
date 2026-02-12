#ifndef RENDER_BACKEND_HH
#define RENDER_BACKEND_HH
#pragma once

// Render backend is a lower-level abstraction
// over different graphics APIs (compat for OpenGL 3.3 and modern for SDL_GPU);
// this subsystem handles initialization and frame lifecycle for the chosen API

namespace render_backend
{
void init(void);
void init_late(void);
void shutdown(void);
void prepare(void);
void present(void);
} // namespace render_backend

namespace render_backend
{
std::string_view display_name(void);
SDL_WindowFlags window_flags(void);
} // namespace render_backend

#endif
