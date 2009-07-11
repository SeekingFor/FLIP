#ifndef _base64_
#define _base64_

#include <string>
#include <vector>

class Base64
{
public:
	static const bool Encode(std::vector<unsigned char> &data, std::string &encoded);
	static const bool Decode(const std::string &encoded, std::vector<unsigned char> &data);
};

#endif	// _base64_
