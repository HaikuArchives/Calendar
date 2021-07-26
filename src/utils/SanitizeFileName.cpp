/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "SanitizeFileName.h"

#include <regex>

BString SanitizeFileName(BString name) 
{	
	std::regex expression("[^\\w\\d]");
	return std::regex_replace(name.String(), expression, "_").c_str();
}
