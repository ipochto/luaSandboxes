#include <iostream>
#include <format>

namespace log_
{
	template<typename... Args>
	void error(std::format_string<Args...> fmt, Args&&... args) {
		std::cerr << "[error] " << std::format(fmt, std::forward<Args>(args)...) << "\n";
	}
}
