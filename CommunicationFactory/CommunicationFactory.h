#pragma once

////////////////////////////////////////////////////////////////////////////
// CommunicationFactory.h - A factory class for the communication object  //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package provides import and export declarations for the communication class
*
* This is largely based on Prof. Fawcett's example code.
*
*/

#include <string>
#include "..\Socket\ICommunication.h"

#ifdef IN_DLL
#define DLL_DECL __declspec(dllexport)
#else
#define DLL_DECL __declspec(dllimport)
#endif

namespace Sockets
{
	extern "C" {
		struct CommunicationFactory {
			static DLL_DECL ICommunication* CreateInstance(size_t port);
		};
	}
}
