#pragma once

#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

namespace StringUtilities
{
	public ref class Convert
	{
	public:
		static std::string* ManagedStringToNative(System::String^ string)
		{
			std::string* nativeString = new std::string();

			if (string != nullptr)
			{
				for (int i = 0; i < string->Length; i++)
				{
					nativeString->push_back((char)string[i]);
				}
			}
			return nativeString;
		}

		static System::String^ NativeStringToManaged(std::string& string)
		{
			if (string.empty())
				return nullptr;
			System::Text::StringBuilder^ stringBuilder = gcnew System::Text::StringBuilder();
			for (auto character : string)
			{
				stringBuilder->Append((wchar_t)character);
			}
			return stringBuilder->ToString();
		}
	};
}
#endif // !UTILITIES_H

