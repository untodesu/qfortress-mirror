#ifndef CLIENT_MODERN_EXPERIMENTAL_HH
#define CLIENT_MODERN_EXPERIMENTAL_HH
#pragma once

namespace experimental
{
void init(void);
void init_late(void);
void shutdown(void);
void render(SDL_GPURenderPass* render_pass);
} // namespace experimental

#endif
