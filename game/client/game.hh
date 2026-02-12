#ifndef GAME_CLIENT_GAME_HH
#define GAME_CLIENT_GAME_HH
#pragma once

namespace client_game
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void update_late(void);
void fixed_update(void);
void fixed_update_late(void);
void layout(void);
} // namespace client_game

#endif
