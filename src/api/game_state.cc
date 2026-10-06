/*
 * Copyright (C) 2026 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "api/game_state.h"

#include <deque>
#include <mutex>
#include <utility>

namespace WidelandsApi {
namespace {

std::mutex g_game_state_mutex;
GameStateSnapshot g_game_state;
std::deque<ExternalPlayerCommand> g_external_commands;
uint64_t g_next_external_command_id = 1;

}  // namespace

void publish_game_state(bool running,
                        uint32_t time_ms,
                        int32_t map_width,
                        int32_t map_height,
                        uint8_t players,
                        std::vector<PlayerStateSnapshot> player_states) {
	std::lock_guard<std::mutex> lock(g_game_state_mutex);
	g_game_state.running = running;
	g_game_state.time_ms = time_ms;
	g_game_state.map_width = map_width;
	g_game_state.map_height = map_height;
	g_game_state.players = players;
	g_game_state.player_states = std::move(player_states);
}

void clear_game_state() {
	std::lock_guard<std::mutex> lock(g_game_state_mutex);
	g_game_state = GameStateSnapshot{};
	g_external_commands.clear();
}

GameStateSnapshot game_state_snapshot() {
	std::lock_guard<std::mutex> lock(g_game_state_mutex);
	return g_game_state;
}

uint64_t enqueue_external_player_command(ExternalPlayerCommand command) {
	std::lock_guard<std::mutex> lock(g_game_state_mutex);
	command.id = g_next_external_command_id++;
	g_external_commands.push_back(command);
	return command.id;
}

std::optional<ExternalPlayerCommand> pop_external_player_command() {
	std::lock_guard<std::mutex> lock(g_game_state_mutex);
	if (g_external_commands.empty()) {
		return std::nullopt;
	}
	ExternalPlayerCommand command = g_external_commands.front();
	g_external_commands.pop_front();
	return command;
}

void clear_external_player_commands() {
	std::lock_guard<std::mutex> lock(g_game_state_mutex);
	g_external_commands.clear();
}

}  // namespace WidelandsApi
