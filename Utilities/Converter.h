#pragma once

////////////////////////////////////////////////////////////////////////////
// Converter.h - A class exposes some conversion tools as static functions//
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package contains static functions to convert various 
* data types to string and vice-versa
*
* This is largely based on Prof. Fawcett's example code.
* The base64 encode and decode functions were copied off this site
* https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp
*/

#ifndef CONVERTER_H
#define CONVERTER_H

#include <iostream>
#include <iterator>
#include <objbase.h>
#include <sstream>
#include <string>
#include <vector>

namespace Utilities
{
	static const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	class Convert
	{
	public:
		template <typename T>
		inline static std::string ToString(T data)
		{
			std::ostringstream oss;
			oss << data;
			return oss.str();
		}

		template <typename T>
		inline static T FromString(std::string string)
		{
			T data;
			std::istringstream ss(string);
			ss >> data;
			return data;
		}

		inline static std::string GuidToString(GUID& guid)
		{
			std::ostringstream oss;
			oss.width(8);
			oss << std::hex << guid.Data1 << "-";
			oss.width(4);
			oss << std::hex << guid.Data2 << "-";
			oss.width(4);
			oss << std::hex << guid.Data3 << "-";
			oss.width(2);
			oss << std::hex << (unsigned int)guid.Data4[0]
				<< std::hex << (unsigned int)guid.Data4[1]
				<< "-"
				<< std::hex << (unsigned int)guid.Data4[2]
				<< std::hex << (unsigned int)guid.Data4[3]
				<< std::hex << (unsigned int)guid.Data4[4]
				<< std::hex << (unsigned int)guid.Data4[5]
				<< std::hex << (unsigned int)guid.Data4[6]
				<< std::hex << (unsigned int)guid.Data4[7];

			return oss.str();
		}

		inline static std::vector<std::string> StringToList(const std::string& source)
		{
			std::vector<std::string> list;
			std::string temp;
			size_t index = 0;
			do
			{
				while ((source[index] == ',' || source[index] == '\n') && index < source.length())
				{
					++index;
					if (temp.size() > 0)
					{
						list.push_back(temp);
						temp.clear();
					}
				}
				if (source[index] != '\0')
					temp += source[index];
				if (temp == "BODY:")
				{
					temp += source.substr(index + 1);
					break;
				}
			} while (index++ < source.size());

			if (temp.size() > 0)
				list.push_back(temp);
			return list;
		}

		inline static std::string ListToString(const std::vector<std::string>& elements, const char* const separator)
		{
			switch (elements.size())
			{
			case 0:
				return "";
			case 1:
				return elements[0];
			default:
				std::ostringstream os;
				std::copy(elements.begin(), elements.end() - 1, std::ostream_iterator<std::string>(os, separator));
				os << *elements.rbegin();
				return os.str();
			}
		}

		static inline bool is_base64(unsigned char c) {
			return (isalnum(c) || (c == '+') || (c == '/'));
		}

		static inline std::string Base64Encode(const char* bytes, size_t length) {
			std::string ret;
			int i = 0;
			int j = 0;
			char char_array_3[3];
			char char_array_4[4];

			while (length--) {
				char_array_3[i++] = *(bytes++);
				if (i == 3) {
					char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
					char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
					char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
					char_array_4[3] = char_array_3[2] & 0x3f;

					for (i = 0; (i < 4); i++)
						ret += base64_chars[char_array_4[i]];
					i = 0;
				}
			}

			if (i)
			{
				for (j = i; j < 3; j++)
					char_array_3[j] = '\0';

				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

				for (j = 0; (j < i + 1); j++)
					ret += base64_chars[char_array_4[j]];

				while ((i++ < 3))
					ret += '=';

			}

			return ret;

		}

		static inline std::string Base64Decode(std::string& encodedString) {
			int in_len = encodedString.size();
			int i = 0;
			int j = 0;
			int in_ = 0;
			char char_array_4[4], char_array_3[3];
			std::string ret;

			while (in_len-- && (encodedString[in_] != '=') && is_base64(encodedString[in_])) {
				char_array_4[i++] = encodedString[in_]; in_++;
				if (i == 4) {
					for (i = 0; i < 4; i++)
						char_array_4[i] = (char)base64_chars.find(char_array_4[i]);

					char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
					char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
					char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

					for (i = 0; (i < 3); i++)
						ret += char_array_3[i];
					i = 0;
				}
			}

			if (i) {
				for (j = 0; j < i; j++)
					char_array_4[j] = (char)base64_chars.find(char_array_4[j]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

				for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
			}

			return ret;
		}
	};
}
#endif // !CONVERTER_H
