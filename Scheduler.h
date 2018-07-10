#pragma once

#include "ccronexpr.h"
#include <thread>
#include <functional>
#include <exception>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace Tool {

	class Scheduler {
	private:
		cron_expr expr{ 0 };

		std::condition_variable cv;
		mutable std::mutex mu;
		bool terminate = false;
		std::thread thread;
	public:
		~Scheduler() {
			{
				std::lock_guard<std::mutex> lock(mu);
				terminate = true;
			}
			cv.notify_all();
			if (thread.joinable()) {
				thread.join();
			}
		}
	public:
		void Cron(const std::string& cronExpression, std::function<void(void)> func) {
			const char* err = NULL;
			cron_parse_expr(cronExpression.c_str(), &expr, &err);
			if (err) {
				throw std::invalid_argument("CronExpression invalid");
			}
			thread = std::thread([&, func]() {
				bool spurious = false;
				while (true) {
					const time_t next = cron_next(&expr, time(NULL));
					if (next < 0) {
						std::cout << "Scheduler Done" << std::endl;
						break;
					}
					if (!spurious) {
						std::stringstream ss;
						ss << "Next schedule at " << std::put_time(std::localtime(&next), "%Y.%m.%d %a %H:%M:%S");
						std::cout << ss.str() << std::endl;
					}
					std::unique_lock<std::mutex> lock(mu);
					std::cv_status status = cv.wait_until(lock, std::chrono::system_clock::from_time_t(next));
					if (status == std::cv_status::timeout) {
						spurious = false;
						func();
					}
					else {
						if (terminate) {
							spurious = false;
							break;
						}
						else {
							spurious = true;
						}
					}
				}
				std::cout << "Scheduler Stopped" << std::endl;
			});
		}
	};
}
