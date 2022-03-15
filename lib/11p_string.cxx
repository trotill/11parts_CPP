/*
 * 11p_string.cxx
 *
 *  Created on: 26 дек. 2018 г.
 *      Author: root
 */


#include "11p_string.h"

//string format = sf

std::string sf(const std::string fmt, ...) {
    int size = 100;
    std::string str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {
            str.resize(n);
            return str;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
    return str;
}


std::string string_format(const std::string fmt, ...) {
    int size = 100;
    std::string str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {
            str.resize(n);
            return str;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
    return str;
}


eErrorTp IsNumeric(char* ccharptr)
{
	u8 fsepcnt=0;
	char* ccharptr_CharacterList=ccharptr;
    for ( ; *ccharptr_CharacterList; ccharptr_CharacterList++)
    {
    	if (*ccharptr_CharacterList==FLOAT_SEPARATOR) fsepcnt++;

        if ((((*ccharptr_CharacterList < '0') || (*ccharptr_CharacterList > '9'))&&( *ccharptr_CharacterList!=FLOAT_SEPARATOR)&&( *ccharptr_CharacterList!='-')&&( *ccharptr_CharacterList!='+'))||fsepcnt>1)
        {

        	return ERROR; // false
        }
    }


    return NO_ERROR; // true
}

eErrorTp ThisStrIsDouble(string & str, double & val)
{
	if (IsNumeric((char*)str.c_str())==NO_ERROR)
	{
		val=(double)atof(str.c_str());
		return NO_ERROR;
	}
	else
	return ERROR;
}

eErrorTp ThisStrIsFloat(string & str, float & val)
{
	if (IsNumeric((char*)str.c_str())==NO_ERROR)
	{
		val=atof(str.c_str());
		return NO_ERROR;
	}
	else
	return ERROR;
}

eErrorTp ThisStrIsNum(string & str, u32 & val)
{
	if (IsNumeric((char*)str.c_str())==NO_ERROR)
	{
		val=atoi(str.c_str());
		return NO_ERROR;
	}
	else
	return ERROR;
}






std::string IntToStr(int i)
{
	return intToString(i);
}

std::string intToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}


void strtolower(char * str)
{
	for (char *ch = str; *ch;ch++)
	{
		*ch=tolower(*ch);
	}
}

void strtolower(string & str)
{
	std::transform(str.begin(), str.end(), str.begin(),
	    [](unsigned char c){ return std::tolower(c); });
}

string removeSpaces(string input)
{
  input.erase(std::remove(input.begin(),input.end(),' '),input.end());
  return input;
}

char hexb[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',   'B','C','D','E','F'};

string byte_2_str(char* bytes, int size) {
  string str;
  for (int i = 0; i < size; ++i) {
    const char ch = bytes[i];
    str.append(&hexb[(ch  & 0xF0) >> 4], 1);
    str.append(&hexb[ch & 0xF], 1);
  }
  return str;
}

string delSpaces(string &str)
{
   str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
   return str;
}

string delCh(string &str, char del)
{
   str.erase(std::remove(str.begin(), str.end(), del), str.end());
   return str;
}

string delIncorrectSymb(string &str)
{
   char del;

   for (int i=0;i<(int)str.size();i++)
   {
	   if ((str[i]<32)|(str[i]>127))
	   {
		   del=str[i];
		   str.erase(std::remove(str.begin(), str.end(), del), str.end());
		   i=-1;
	   }
   }
   return str;
}

eErrorTp ReplaceCharInStr(string & str,char lastch, char newch)
{
	u32 i;
	u32 size=str.size();
	for (i=0;i<size;i++)
		if (str[i]==lastch) str[i]=newch;

	return NO_ERROR;
}

eErrorTp ReplaceCharInCharStr(char * str,char lastch, char newch)
{
	u32 i;
	u32 size=(u32)strlen(str);
	for (i=0;i<size;i++)
		if (str[i]==lastch) str[i]=newch;

	return NO_ERROR;
}

void printhex_ex(u8 * buf,u32 len,u16 loop,u8 mode)
{
 u32 i;
 u16 k=0;
u8 buf2[10];
 if (mode==0)
 {
	 cout << "---------------------------------------------------" << endl;
	 return;
 }
 if (mode==1)
  {
	 cout << "---------------------------------------------------" << endl;
	 return;
  }

 if (len<loop) loop=len;
 cout << "  ";
 for (i=0;i<len;i++){
	 snprintf((char*)buf2,10,"[0x%02x]",buf[i]);
	cout << buf2;
	k++;
	if (k==loop) {
	  cout << endl;
	  if (i!=(len-1))
	  cout << "  ";
	  k=0;
	}
 }
 if (k!=0) cout << endl;

}

void printhex(eDebugTp set_level,eDebugTp debug_level,char * info,u8 * buf,u32 len,u16 loop)
{
 if (set_level>debug_level)
	 return;

 u32 i;
 u16 k=0;
u8 buf2[10];
 if (len<loop) loop=len;
 cout << "-S-------["<< info <<"]-------------------------------------------" << endl;
 for (i=0;i<len;i++){
	 snprintf((char*)buf2,10,"[0x%02x]",buf[i]);
	cout << buf2;
	k++;
	if (k==loop) {
	  cout << endl;
	  k=0;
	}
 }
 if (k!=0) cout << endl;
 cout << "-F-------["<< info <<"]-------------------------------------------" << endl;

}

void printhex(u8 * buf,u32 len,u16 loop)
{
 u32 i;
 u16 k=0;
u8 buf2[10];
 if (len<loop) loop=len;
 cout << "-S--------------------------------------------------" << endl;
 for (i=0;i<len;i++){
	 snprintf((char*)buf2,10,"[0x%02x]",buf[i]);
	cout << buf2;
	k++;
	if (k==loop) {
	  cout << endl;
	  k=0;
	}
 }
 if (k!=0) cout << endl;
 cout << "-F--------------------------------------------------" << endl;

}

string printhex(u8 * buf,u32 len)
{
 u32 i;
 u16 k=0;
 u8 buf2[10];
 //if (len<loop) loop=len;
 string result="";

 for (i=0;i<len;i++){
	 result+=string_format("[0x%02x]",buf[i]);
	 //result+=buf2;
	 //k++;
	//if (k==loop) {
	 // k=0;
	//}
 }
 //if (k!=0) cout << endl;

 return result;

}

string RandomCStr(string prefix) {

    //srand(TIME(NULL));
	u32 unival=rand();
	for (u32 n=0;n<prefix.length();n++){
		unival+=prefix[n]+rand();
	}
 //   u32 r=rand();
    return string_format("%s%08x",prefix.c_str(),unival);
}



eErrorTp PrepareCMDForNode(string cmd,string json,string & out)
{
	stringstream ss;
	ss << "{\"req\":" << cmd << "," << cmd << ":" << json << "}";

	out=ss.str();

	//replace(out.begin(),out.end(),'\'','"');
	delCh(out,'\r');
	delCh(out,'\n');

	return NO_ERROR;
}

string ReplaceSubString(const std::string& inputStr, const std::string& src, const std::string& dst)
{
    std::string result(inputStr);

    size_t pos = result.find(src);
    while(pos != std::string::npos) {
        result.replace(pos, src.size(), dst);
        pos = result.find(src, pos);
    }

    return result;
}
eErrorTp PrepareForJsonC(string & json)
{


	replace(json.begin(),json.end(),'\'','"');
	delCh(json,'\r');
	delCh(json,'\n');

	return NO_ERROR;
}

int HexToInt( string i ) {
  int decimalValue;
  sscanf(i.c_str(), "%x", &decimalValue);
  return decimalValue;
}

u64 HexToU64( string i ) {
    std::istringstream converter(i);
    u64 decimalValue;
    converter >> std::hex >> decimalValue;
    return decimalValue;
}

int HexToInt( char * i ) {
  int decimalValue;
  sscanf(i, "%x", &decimalValue);
  return decimalValue;
}

eErrorTp regexMatch(string regex,string data){
	const std::regex numRegex(regex);
	std::smatch match;
	if (std::regex_match(data, match, numRegex)) {
		return NO_ERROR;
	}
	return ERROR;
}

std::vector<std::string> &__split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    __split(s, delim, elems);
    return elems;
}
