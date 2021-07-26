/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */


#include <regex>

const char* SanitizeFileName(char* name) 
{	
	std::regex expression("[^\\w\\d]");
	return std::regex_replace(name, expression, "_").c_str();
}
