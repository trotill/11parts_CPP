/*
 * 11p_string.h
 *
 *  Created on: 26 дек. 2018 г.
 *      Author: root
 */

#ifndef SRC_ENGINE_LIB_11P_STRING_H_
#define SRC_ENGINE_LIB_11P_STRING_H_

#include "engine/basic.h"
#include "engine/memadmin.h"


template <class T>
string toString( T argument)
{
       string r;
       stringstream s;
       s << argument;
       r = s.str();
       return r;
}



template <typename V>
void pop_front(V & v)
{
    if (v.size()>0)
    v.erase(v.begin());
}

int HexToInt( string i );
int HexToInt( char * i );
u64 HexToU64( string i );

std::string string_format(const std::string fmt, ...);
std::string sf(const std::string fmt, ...) ;

std::string IntToStr(int i);
std::string intToString(int i);
eErrorTp IsNumeric(char* ccharptr);
eErrorTp ThisStrIsFloat(string & str, float & val);
eErrorTp ThisStrIsDouble(string & str, double & val);
eErrorTp ThisStrIsNum(string & str, u32 & val);
string removeSpaces(string input);
void strtolower(char * str);
void strtolower(string & str);
string byte_2_str(char* bytes, int size);
string delSpaces(string &str);
string delCh(string &str, char del);
string delIncorrectSymb(string &str);

void printhex_ex(u8 * buf,u32 len,u16 loop,u8 mode);
void printhex(u8 * buf,u32 len,u16 loop);
string printhex(u8 * buf,u32 len);
void printhex(eDebugTp set_level,eDebugTp debug_level,char * info,u8 * buf,u32 len,u16 loop);
string RandomCStr(string prefix);

eErrorTp PrepareForJsonC(string & json);
eErrorTp PrepareCMDForNode(string cmd,string json,string & out);
eErrorTp AddZeroToBuf(BufHandler * BufH);
eErrorTp AddCStringToBuf(BufHandler * BufH);
std::vector<std::string> split(const std::string &s, char delim);

eErrorTp ReplaceCharInStr(string & str,char lastch, char newch);
eErrorTp ReplaceCharInCharStr(char * str,char lastch, char newch);
string ReplaceSubString(const std::string& inputStr, const std::string& src, const std::string& dst);
eErrorTp regexMatch(string regex,string data);

#endif /* SRC_ENGINE_LIB_11P_STRING_H_ */
