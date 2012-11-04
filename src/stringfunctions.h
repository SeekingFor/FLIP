#ifndef _string_functions_
#define _string_functions_

#include <string>
#include <vector>
#include <sstream>

namespace StringFunctions
{

inline const bool Convert(const std::string &input, std::string &output)
{
	output=input;
	return true;
}

/**
	\brief Converts a string into any other type
	\param input[in] string to convert
	\param output[out] output type
	\return true if conversion was successful, false if it was not
*/
template <class T>
inline const bool Convert(const std::string &input, T &output)
{
	std::istringstream i(input);
	if(i >> output)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
	\brief Converter from any type into a string
	\param input[in] data to convert
	\param output[out] string output
	\return true if conversion was successful, false if it was not
*/
template <class T>
inline const bool Convert(const T &input, std::string &output)
{
	std::ostringstream o;
	o << input;
	output=o.str();
	return true;	
}

/**
	\brief Replaces occurences of a string with another string

	\param input string to search in
	\param find string to find in input string
	\param replace string to replace find string with
*/
std::string Replace(const std::string &input, const std::string &find, const std::string &replace);

/**
	\brief Splits a string into pieces separated by a delimeter
	
	If the delimiter is not found within the string, the output vector will contain 1 element with the entire input string
	\param str string to split
	\param delim delimeter to split at
	\param[out] vector containing parts
*/
void Split(const std::string &str, const std::string &delim, std::vector<std::string> &output);

/**
	\brief Splits a string into pieces separated by one or more delimeters
	
	The delimiter string contains individual character delimieters
	\param str string to split
	\param delim string containing individual characters to split at
	\param[out] vector containing parts
*/
void SplitMultiple(const std::string &str, const std::string &delim, std::vector<std::string> &output);

/**
	\brief Trims whitespace from beginning to end of string
*/
std::string TrimWhitespace(const std::string &str);

/**
	\brief Trims trailing whitespace in a string
*/
std::string TrimTrailingWhitespace(const std::string &str);

/**
	\brief Converts a string to upper case
	\param str string to convert to upper case
	\param[out] string converted to upper case
*/
void UpperCase(const std::string &str, std::string &output);

/**
	\brief Converts a string to lower case
	\param str string to convert to lower case
	\param[out] string converted to lower case
*/
void LowerCase(const std::string &str, std::string &output);

/**
	\brief Decodes a URI encoded string
	\param aSrc string that is URI encoded
	\return URI decoded input string
*/
std::string UriDecode(const std::string & sSrc);

/**
	\brief URI Encodes a string
	\param aSrc string to encode
	\return input string URI encoded
*/
std::string UriEncode(const std::string & sSrc);

};

#endif	// _string_functions_
