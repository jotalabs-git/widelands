/*
 * Copyright (C) 2026 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "api/health_server.h"

#include <asio.hpp>

#include <array>
#include <chrono>
#include <sstream>
#include <string>
#include <thread>

#include "api/game_state.h"
#include "base/log.h"
#include "build_info.h"

namespace WidelandsApi {
namespace {

constexpr const char* kBindAddress = "127.0.0.1";

const char* operating_system() {
#if defined(_WIN32)
	return "windows";
#elif defined(__APPLE__)
	return "macos";
#elif defined(__linux__)
	return "linux";
#elif defined(__FreeBSD__)
	return "freebsd";
#else
	return "unknown";
#endif
}

const char* architecture() {
#if defined(__x86_64__) || defined(_M_X64)
	return "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
	return "arm64";
#elif defined(__i386__) || defined(_M_IX86)
	return "x86";
#elif defined(__arm__) || defined(_M_ARM)
	return "arm";
#else
	return "unknown";
#endif
}

std::string health_json(uint16_t port) {
	std::ostringstream out;
	out << "{"
	    << "\"status\":\"ok\","
	    << "\"service\":\"widelands-local-api\","
	    << "\"version\":\"" << build_id() << "\","
	    << "\"build_type\":\"" << build_type() << "\","
	    << "\"system\":{"
	    << "\"os\":\"" << operating_system() << "\","
	    << "\"architecture\":\"" << architecture() << "\","
	    << "\"hardware_threads\":" << std::thread::hardware_concurrency()
	    << "},"
	    << "\"api\":{"
	    << "\"address\":\"" << kBindAddress << "\","
	    << "\"port\":" << port
	    << "}"
	    << "}";
	return out.str();
}

std::string game_json() {
	const GameStateSnapshot state = game_state_snapshot();
	std::ostringstream out;
	out << "{"
	    << "\"running\":" << (state.running ? "true" : "false") << ","
	    << "\"time_ms\":" << state.time_ms << ","
	    << "\"map\":{"
	    << "\"width\":" << state.map_width << ","
	    << "\"height\":" << state.map_height
	    << "},"
	    << "\"players\":" << static_cast<unsigned>(state.players)
	    << "}";
	return out.str();
}

std::string response(int status, const std::string& reason, const std::string& body) {
	std::ostringstream out;
	out << "HTTP/1.1 " << status << " " << reason << "\r\n"
	    << "Content-Type: application/json; charset=utf-8\r\n"
	    << "Content-Length: " << body.size() << "\r\n"
	    << "Connection: close\r\n"
	    << "Cache-Control: no-store\r\n"
	    << "\r\n"
	    << body;
	return out.str();
}

void handle_connection(asio::ip::tcp::socket& socket, uint16_t port) {
	std::array<char, 4096> buffer{};
	std::string request;
	std::error_code error;
	socket.non_blocking(true, error);

	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
	while (std::chrono::steady_clock::now() < deadline && request.find("\r\n") == std::string::npos) {
		const size_t bytes = socket.read_some(asio::buffer(buffer), error);
		if (!error) {
			request.append(buffer.data(), bytes);
			continue;
		}
		if (error == asio::error::would_block || error == asio::error::try_again) {
			error.clear();
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}
		break;
	}

	std::istringstream input(request);
	std::string method;
	std::string path;
	input >> method >> path;

	std::string http_response;
	if (method == "GET" && path == "/health") {
		http_response = response(200, "OK", health_json(port));
	} else if (method == "GET" && path == "/api/v1/game") {
		http_response = response(200, "OK", game_json());
	} else {
		http_response = response(404, "Not Found", "{\"error\":\"not_found\"}");
	}

	error.clear();
	asio::write(socket, asio::buffer(http_response), error);
}

}  // namespace

HealthServer::HealthServer(uint16_t port) : port_(port) {
}

HealthServer::~HealthServer() {
	stop();
}

void HealthServer::start() {
	bool expected = false;
	if (!running_.compare_exchange_strong(expected, true)) {
		return;
	}
	server_thread_ = std::thread(&HealthServer::run, this);
}

void HealthServer::stop() {
	running_ = false;
	if (server_thread_.joinable()) {
		server_thread_.join();
	}
}

void HealthServer::run() {
	try {
		asio::io_context io_context;
		const asio::ip::tcp::endpoint endpoint(asio::ip::make_address(kBindAddress), port_);
		asio::ip::tcp::acceptor acceptor(io_context, endpoint);
		acceptor.non_blocking(true);

		log_info("Local API listening on http://%s:%u\n", kBindAddress, port_);

		while (running_) {
			asio::ip::tcp::socket socket(io_context);
			std::error_code error;
			acceptor.accept(socket, error);

			if (!error) {
				handle_connection(socket, port_);
				continue;
			}

			if (error == asio::error::would_block || error == asio::error::try_again) {
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
				continue;
			}

			if (running_) {
				log_warn("Local API accept failed: %s\n", error.message().c_str());
			}
		}
	} catch (const std::exception& e) {
		log_err("Local API failed to start: %s\n", e.what());
		running_ = false;
	}
}

}  // namespace WidelandsApi
