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
#include <optional>
#include <string>
#include <vector>

namespace WidelandsApi {

struct PlayerStateSnapshot {
	uint8_t id{0};
	std::string name;
	std::string tribe;
	uint8_t team{0};
	bool defeated{false};
};

enum class ExternalPlayerCommandType : uint8_t {
	kBuildFlag = 1,
};

struct ExternalPlayerCommand {
	uint64_t id{0};
	ExternalPlayerCommandType type{ExternalPlayerCommandType::kBuildFlag};
	uint8_t player_id{0};
	int32_t x{0};
	int32_t y{0};
};

struct GameStateSnapshot {
	bool running{false};
	uint32_t time_ms{0};
	int32_t map_width{0};
	int32_t map_height{0};
	uint8_t players{0};
	std::vector<PlayerStateSnapshot> player_states;
};

void publish_game_state(bool running,
                        uint32_t time_ms,
                        int32_t map_width,
                        int32_t map_height,
                        uint8_t players,
                        std::vector<PlayerStateSnapshot> player_states = {});
void clear_game_state();
GameStateSnapshot game_state_snapshot();

uint64_t enqueue_external_player_command(ExternalPlayerCommand command);
std::optional<ExternalPlayerCommand> pop_external_player_command();
void clear_external_player_commands();

}  // namespace WidelandsApi

#endif  // WL_API_GAME_STATE_H
