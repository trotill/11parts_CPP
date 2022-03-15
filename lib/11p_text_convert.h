/*
 * text_convert.h
 *
 *  Created on: 26 апр. 2018 г.
 *      Author: root
 */

#ifndef TEXT_CONVERT_H_
#define TEXT_CONVERT_H_

#include <engine/types.h>
#include <engine/basic.h>

int convert_utf8_to_windows1251(const char* utf8, char* windows1251, size_t n);

#endif /* TEXT_CONVERT_H_ */
