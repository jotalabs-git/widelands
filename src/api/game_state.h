/*
 * Copyright (C) 2026 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef WL_API_GAME_STATE_H
#define WL_API_GAME_STATE_H

#include <cstdint>

namespace WidelandsApi {

struct GameStateSnapshot {
	bool running{false};
	uint32_t time_ms{0};
	int32_t map_width{0};
	int32_t map_height{0};
	uint8_t players{0};
};

void publish_game_state(bool running,
                        uint32_t time_ms,
                        int32_t map_width,
                        int32_t map_height,
                        uint8_t players);
void clear_game_state();
GameStateSnapshot game_state_snapshot();

}  // namespace WidelandsApi

#endif  // WL_API_GAME_STATE_H
