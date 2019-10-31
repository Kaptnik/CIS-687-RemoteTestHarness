#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
// Directory.h - A class that exposes directory helper functions such as listing files  //
// ver 1.0																				//
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018						//
//////////////////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package contains static functions to list the contents
* of a directory, .
*
*/

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <windows.h>
#include <vector>
#pragma comment(lib, "User32.lib")

namespace Utilities
{
	class Directory
	{
	public:
		inline static std::vector<std::string> GetFileSystemEntries(std::string path)
		{
			std::vector<std::string> entries;
			path += "*";
			WIN32_FIND_DATA findData;
			HANDLE handle = INVALID_HANDLE_VALUE;
			handle = FindFirstFile(path.c_str(), &findData);
			if (handle == INVALID_HANDLE_VALUE)
				return entries;

			do
			{
				std::string entry = findData.cFileName;
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					entry = "directory:" + entry;
				else
					entry = "file:" + entry;
				entries.push_back(entry);
			} while (FindNextFile(handle, &findData) != 0);
			FindClose(handle);

			return entries;
		}
	};
}

#endif // !DIRECTORY_H
