#ifndef CLIENT_MODERN_EXPERIMENTAL_HH
#define CLIENT_MODERN_EXPERIMENTAL_HH
#pragma once

namespace experimental
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void update_late(void);
void render(SDL_GPUCommandBuffer* command_buffer);
} // namespace experimental

#endif
