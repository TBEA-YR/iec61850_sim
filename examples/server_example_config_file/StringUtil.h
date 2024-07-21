#pragma once
#include <vector>
#include <string>

class StringUtil {
public:
	static void getBooleanValue(const std::string &value, bool& defaultValue);
	static void stringToList(const std::string &value, std::vector<std::string>& vec);
	static void SplitString(const std::string& str, std::vector<std::string>& ret_, const std::string& sep);
	static std::string Trim(std::string& str);
	static std::string m_replace(const std::string &strSrc, const std::string& oldStr, const std::string& newStr, int count = -1);
	static char* strToChar(const std::string &strSrc);
	static std::string IntToString(int nNum);
	//static bool startsWith(long num, char* ch);
};