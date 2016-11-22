#include "files.h"

std::vector<char> gulp(std::string fname) {
	std::vector<char> ret;
	std::ifstream ifs(fname, std::ios::binary | std::ios::ate);
	auto n1 = ifs.tellg();
	ret.reserve(n1);
	ifs.seekg(0);
	ifs.read(ret.data(), n1);
	return ret;
}
