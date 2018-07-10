#include "Scheduler.h"
#include <future>
#include <iostream>
#include <iomanip>
#include <ctime>

class Client {
	std::unique_ptr<Tool::Scheduler> scheduler;
public:
	void SetScheduler(const char* cronExpression) {
		try {
			scheduler.reset(new Tool::Scheduler());
			//char cronExpression[256] = "0 0 * ? * *";
			scheduler->Cron(cronExpression, std::bind(&Client::Function, this));
		}
		catch (const std::exception& e) {
			scheduler.reset();
			std::cerr << e.what() << std::endl;
		}
	}

	void Function() {
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::cout << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;
	}
};

int main(int argc, char* argv[]) {
	std::cout << argv[1] << std::endl;
	auto client = new(std::nothrow) Client();
	client->SetScheduler(argv[1]);
	std::promise<void>().get_future().wait();
	return 0;
}