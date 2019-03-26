#pragma once

#include <mutex>
#include <sstream>

enum {
	DEBUG,
	INFO,
	NONE,
	ERR,
	WARN,
};

class Log : public std::ostringstream {
public:
	Log();
	Log(int level);
	~Log();

private:
	static std::mutex print_mutex_;
	int level_;
};