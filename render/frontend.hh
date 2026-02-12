#ifndef RENDER_FRONTEND_HH
#define RENDER_FRONTEND_HH
#pragma once

// Render frontend is a higher-level gamesystem-like abstraction
// that encapsulates literally everything related to rendering the
// game world and UI to the screen; this subsystem is compiled in
// the same module as the render_backend but works somewhat independently
// and is basically an answer to "what is inside render_everything() call"

namespace render_frontend
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void update_late(void);
void render(void);
void layout(void);
} // namespace render_frontend

#endif
