#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
// Credentials.h - A class that exposes functions to register users and authenticate them//
// ver 1.0																				 //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018						 //
///////////////////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package contains static functions to register a user, or to 
* authenticate a registered user.
*/

#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include "Converter.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace Utilities
{
	struct User
	{
	public:
		std::string Username;
		std::string Password;
		std::string Fullname;
		bool IsAdmin;
	};

	class Credentials
	{
	public:
		inline static User* Login(std::string credentialsFile, std::string username, std::string password)
		{
			std::ifstream file(credentialsFile);
			if (!file.good())
				return nullptr;
			std::string line;
			while (file >> line)
			{
				std::string decodedLine =Utilities::Convert::Base64Decode(line);
				std::vector<std::string> credentials =Utilities::Convert::StringToList(decodedLine);
				User *user = new User();
				for (std::string kvp : credentials)
				{
					size_t index = kvp.find_first_of(':');
					if (index == kvp.length())
						continue;
					std::string key = kvp.substr(0, index), value = kvp.substr(index + 1, kvp.length() - index);
					if (key == "username")
						user->Username = value;
					else if (key == "password")
						user->Password = value;
					else if (key == "isadmin")
					{
						if (value == "0")
							user->IsAdmin = false;
						else
							user->IsAdmin = true;
					}
					else if (key == "fullname")
						user->Fullname = value;
				}
				if (user->Username == username && user->Password == password)
					return user;
			}
			file.close();
			return nullptr;
		}

		inline static void RegisterUser(std::string credentialsFile, User& user)
		{
			std::string credentials =
				"username:" + user.Username +
				",password:" + user.Password +
				",isadmin:" + (user.IsAdmin ? "1" : "0") +
				",fullname:" + user.Fullname;

			std::ofstream file(credentialsFile, std::ios_base::app);
			if(file.good())
				file << std::endl << Utilities::Convert::Base64Encode(credentials.c_str(), credentials.length()) << std::endl;
			file.close();
		}
	};
}
#endif // !CREDENTIALS_H