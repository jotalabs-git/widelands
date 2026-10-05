/*
 * Copyright (C) 2026 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "api/game_state.h"

#include <mutex>

namespace WidelandsApi {
namespace {

std::mutex g_game_state_mutex;
GameStateSnapshot g_game_state;

}  // namespace

void publish_game_state(bool running,
                        uint32_t time_ms,
                        int32_t map_width,
                        int32_t map_height,
                        uint8_t players) {
	std::lock_guard<std::mutex> lock(g_game_state_mutex);
	g_game_state.running = running;
	g_game_state.time_ms = time_ms;
	g_game_state.map_width = map_width;
	g_game_state.map_height = map_height;
	g_game_state.players = players;
}

void clear_game_state() {
	std::lock_guard<std::mutex> lock(g_game_state_mutex);
	g_game_state = GameStateSnapshot{};
}

GameStateSnapshot game_state_snapshot() {
	std::lock_guard<std::mutex> lock(g_game_state_mutex);
	return g_game_state;
}

}  // namespace WidelandsApi
