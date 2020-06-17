#include <iostream>
using namespace std;
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/spdlog.h>
namespace spd = spdlog;

int main()
{
	int n;

	// Console logger with color
    spd::set_level(spd::level::debug);
	auto console = spd::stdout_color_mt("logger");
	console->info("Welcome to spdlog!");
	console->error("Some error message with arg{}..", 1);
	console->debug("debug");
	console->warn("warn");

	// Create basic file logger (not rotated)
	auto my_logger = spd::basic_logger_mt("spdlog_test", "d:/logs/vs/spdlog_log.txt",false);
	my_logger->info("Some log message");
	my_logger->debug("debug");
	my_logger->warn("warning");
	my_logger->error("error 现在c++的框架使用也很方便了，感谢开源。");

	cout << endl;

	cout << "are you ok ?" << endl;
	cin >> n;

    return 0;
}