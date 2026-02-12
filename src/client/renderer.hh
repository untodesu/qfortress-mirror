#ifndef CLIENT_RENDERER_HH
#define CLIENT_RENDERER_HH
#pragma once

namespace renderer::backend
{
void init(void);
void init_late(void);
void shutdown(void);
void prepare(void);
void present(void);
} // namespace renderer::backend

namespace renderer::backend
{
SDL_WindowFlags window_flags(void);
} // namespace renderer::backend

namespace renderer
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void update_late(void);
void render(void);
void layout(void);
} // namespace renderer

#endif
