#include "logger.h"
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <thread>

void dwlog(int level, char const * fmt, char const * file, int line, char const * func, ...) {
	if (level < 1) return;

	FILE * f1 = stdout;

	// We require that the file of the source comes as the argument after fmt
	va_list ap;
	va_start(ap, func);
	// Here, we assume that this file is still called logger.cpp and lives in the root of the project
	int const npre = strlen(__FILE__) - 10;

	int const N1 = 8000;
	char buf1[N1];
	//int x = snprintf(buf1, N1, "## %s:%d [%s]\n%s\n", file+npre, line, func, fmt);
	int x = snprintf(buf1, N1, "### {%d} %s:%d\n%s\n\n", level, file+npre, line, fmt);
	if (x >= N1) {
		snprintf(buf1, N1, "[ERR IN LOG FORMAT]\n");
	}

	vfprintf(f1, buf1, ap);
	va_end(ap);
}

void break1() {
}
