/*
 * base64.h
 *
 *  Created on: 10 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ALGO_BASE64_H_
#define SRC_ALGO_BASE64_H_


#include "engine/basic.h"

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string base64_decode(std::string const& encoded_string);

#endif /* SRC_ALGO_BASE64_H_ */
