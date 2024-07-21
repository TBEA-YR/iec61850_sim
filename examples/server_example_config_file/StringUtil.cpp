
#include "StringUtil.h"
#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;

void StringUtil::getBooleanValue(const std::string &strValue, bool& bDefaultValue)
{
	if (!strValue.empty()) {
		bDefaultValue = stoi(strValue) == 0 ? false : true;
	}
}

void StringUtil::stringToList(const std::string &value, std::vector<std::string>& vec)
{
	istringstream iss(value);
	string item;
	while (getline(iss, item, ',')) {
		vec.push_back(item);
	}

	// 打印结果
// 	for (const auto& item1 : vec) {
// 		cout << item1 << endl;
// 	}
}

void StringUtil::SplitString(const std::string& str, std::vector<std::string>& ret_, const std::string& sep)
{
	if (str.empty())
	{
		return;
	}

	string tmp;
	string::size_type pos_begin = 0;//str.find_first_not_of(sep);
	string::size_type comma_pos = 0;

	while (pos_begin != string::npos)
	{
		comma_pos = str.find(sep, pos_begin);
		if (comma_pos != string::npos)
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
			pos_begin = comma_pos;
		}

		ret_.push_back(tmp);
	}
}


std::string StringUtil::Trim(std::string& str)
{
	str.erase(0, str.find_first_not_of(" \t\r\n"));
	str.erase(str.find_last_not_of(" \t\r\n") + 1);
	return str;
}

std::string StringUtil::m_replace(const std::string &strSrc,const std::string& oldStr, const std::string& newStr, int count)
{
	string strRet = strSrc;
	size_t pos = 0;
	int l_count = 0;
	if (-1 == count) // replace all
		count = strRet.size();
	while ((pos = strRet.find(oldStr, pos)) != string::npos)
	{
		strRet.replace(pos, oldStr.size(), newStr);
		if (++l_count >= count) break;
		pos += newStr.size();
	}
	return strRet;
}


char* StringUtil::strToChar(const std::string &strSrc)
{
	int len = strSrc.length();
	char* result = new char[len + 1];
	for (int i = 0; i < len; ++i)
	{
		result[i] = strSrc[i];
	}
	result[len] = '\0';

	return result;

}

std::string StringUtil::IntToString(int nNum)
{
	std::stringstream newstr;
	newstr << nNum;
	return newstr.str();
}


//bool StringUtil::startsWith(long num, char* ch) {
//	std::stringstream ss;
//	ss << num;
//	std::string numStr = ss.str();
//	if (numStr.length() < 3) {
//		return false; // 长度小于3位时，直接返回false
//	}
//	// 获取前三位字符
//	std::string firstThreeChars = numStr.substr(0, 3);
//	// 使用strncmp进行比较
//	return strncmp(firstThreeChars.c_str(), &ch, 1) == 0;
//}