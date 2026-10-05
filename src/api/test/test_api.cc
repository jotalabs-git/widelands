/*
 * Copyright (C) 2026 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "api/game_state.h"
#include "api/health_server.h"
#include "base/test.h"

#include <asio.hpp>

#include <chrono>
#include <sstream>
#include <string>
#include <thread>

namespace {

uint16_t wait_for_port(WidelandsApi::HealthServer& server) {
	for (int i = 0; i < 100; ++i) {
		if (server.port() != 0) {
			return server.port();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return 0;
}

std::string get(uint16_t port, const std::string& path) {
	asio::io_context io_context;
	asio::ip::tcp::socket socket(io_context);
	socket.connect(
	   asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port));

	const std::string request =
	   "GET " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
	asio::write(socket, asio::buffer(request));

	std::ostringstream response;
	std::array<char, 4096> buffer{};
	std::error_code error;
	for (;;) {
		const size_t bytes = socket.read_some(asio::buffer(buffer), error);
		if (bytes > 0) {
			response.write(buffer.data(), static_cast<std::streamsize>(bytes));
		}
		if (error == asio::error::eof) {
			break;
		}
		if (error) {
			throw std::system_error(error);
		}
	}
	return response.str();
}

bool has(const std::string& text, const std::string& expected) {
	return text.find(expected) != std::string::npos;
}

}  // namespace

TESTSUITE_START(local_api)

TESTCASE(health_endpoint_returns_system_info) {
	WidelandsApi::HealthServer server(0);
	server.start();
	const uint16_t port = wait_for_port(server);
	check_equal(port != 0, true);

	const std::string response = get(port, "/health");
	check_equal(has(response, "HTTP/1.1 200 OK"), true);
	check_equal(has(response, "\"status\":\"ok\""), true);
	check_equal(has(response, "\"service\":\"widelands-local-api\""), true);
	check_equal(has(response, "\"system\":{"), true);
	check_equal(has(response, "\"os\":"), true);
	check_equal(has(response, "\"architecture\":"), true);
	check_equal(has(response, "\"hardware_threads\":"), true);

	server.stop();
}

TESTCASE(game_endpoint_returns_published_snapshot) {
	WidelandsApi::publish_game_state(true, 154320, 128, 96, 3);

	WidelandsApi::HealthServer server(0);
	server.start();
	const uint16_t port = wait_for_port(server);
	check_equal(port != 0, true);

	const std::string response = get(port, "/api/v1/game");
	check_equal(has(response, "HTTP/1.1 200 OK"), true);
	check_equal(has(response, "\"running\":true"), true);
	check_equal(has(response, "\"time_ms\":154320"), true);
	check_equal(has(response, "\"width\":128"), true);
	check_equal(has(response, "\"height\":96"), true);
	check_equal(has(response, "\"players\":3"), true);

	server.stop();
	WidelandsApi::clear_game_state();
}

TESTCASE(unknown_endpoint_returns_404) {
	WidelandsApi::HealthServer server(0);
	server.start();
	const uint16_t port = wait_for_port(server);
	check_equal(port != 0, true);

	const std::string response = get(port, "/does-not-exist");
	check_equal(has(response, "HTTP/1.1 404 Not Found"), true);
	check_equal(has(response, "\"error\":\"not_found\""), true);

	server.stop();
}

TESTSUITE_END()
