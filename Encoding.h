#ifndef ENCODING_H__
#define ENCODING_H__

#include <string>

namespace enc
{
std::string base64_encode(const unsigned char* input, int length);
}

#endif