/*
 * Copyright (C) 2026 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef WL_API_HEALTH_SERVER_H
#define WL_API_HEALTH_SERVER_H

#include <atomic>
#include <cstdint>
#include <thread>

namespace WidelandsApi {

class HealthServer {
public:
	explicit HealthServer(uint16_t port = 7391);
	~HealthServer();

	HealthServer(const HealthServer&) = delete;
	HealthServer& operator=(const HealthServer&) = delete;

	void start();
	void stop();
	[[nodiscard]] uint16_t port() const {
		return bound_port_;
	}

private:
	void run();

	const uint16_t port_;
	std::atomic<uint16_t> bound_port_{0};
	std::atomic<bool> running_{false};
	std::thread server_thread_;
};

}  // namespace WidelandsApi

#endif  // WL_API_HEALTH_SERVER_H
