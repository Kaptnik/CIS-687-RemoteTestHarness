#pragma once

#ifndef SOCKETS_H
#define SOCKETS_H
/////////////////////////////////////////////////////////////////////////
// Sockets.h - C++ wrapper for Win32 socket api                        //
// ver 5.2                                                             //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016           //
// CST 4-187, Syracuse University, 315 443-3948, jfawcett@twcny.rr.com //
//---------------------------------------------------------------------//
// Jim Fawcett (c) copyright 2015                                      //
// All rights granted provided this copyright notice is retained       //
//---------------------------------------------------------------------//
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  Provides four classes that wrap the Winsock API:
*  Socket:
*  - provides all the functionality necessary to handle server clients
*  - created by SocketListener after accepting a request
*  - usually passed to a client handling thread
*  SocketConnector:
*  - adds the ability to connect to a server
*  SocketListener:
*  - adds the ability to listen for connections on a dedicated thread
*  - instances of this class are the only ones influenced by ipVer().
*    clients will use whatever protocol the server provides.
*  SocketSystem:
*  - Loads and unloads winsock2 library.
*  - Declared once at beginning of execution
*
*  Required Files:
*  ---------------
*  Sockets.h, Sockets.cpp,
*  Logger.h, Logger.cpp,
*  Utilities.h, Utililties.cpp,
*  WindowsHelpers.h, WindowsHelpers.cpp
*
*  Maintenance History:
*  --------------------
*  ver 5.2.1 : 10 Jun 2018
*  - Renamed class and method names to not have abbreviations
*  ver 5.2 : 05 Oct 2017
*  - changed Socket::recvString to append the terminating character,
*    newline by default
*  - added removeTerminator method to remove the newly added terminator
*    character
*  - modified error logging displays
*  ver 5.1 : 10 Apr 16
*  - Added sendStream and recvStream to support sending and receiving
*    file streams.  These simply wrap the native sockets send and recv.
*  ver 5.0 : 04 Mar 16
*  - Fixed bugs in Socket test stub, essentially stealing fixes from
*    ClientTest.cpp and ServerTest.cpp
*  - Didn't change any code in the Socket library itself
*  ver 4.9 : 04 Mar 16
*  - Added a single write statement in Socket::Listener::accept()
*  ver 4.8 : 22 Feb 16
*  - Replaced verbose I/O with Logger I/O
*  - Replaced ApplicationHelpers package with Utilities package
*  ver 4.7 : 04 Apr 15
*  - removed testBlockHandling declaration from Socket.cpp IncomingMessageHandler.
*    The implementation had already been removed, I just forgot the declaration.
*  - added test for INVALID_SOCKET in Socket::recvString.  The omission was
*    reported by Huanming Fang.  Thanks Huanming.
*  ver 4.6 : 30 Mar 15
*  - minor modification to comments, above, and in Socket class implem.
*  ver 4.5 : 30 Mar 15
*  - moved SocketListener::start(...) from cpp to h file since it is a
*    template method.
*  - renamed ClientProc to IncomingMessageHandler
*  - removed Block operations to avoid binding Socket package to
*    FileSystem package.  Will add buffer operations to the
*    FileSystem::File class to match the Socket buffer operations.
*  - gave IncomingMessageHandler a command interpreter to select a test process
*    - test string tranfers
*    - test buffer transfers
*    - client sends a string to select test mode
*    - test modes are (string, buffer, and stop)
*  - Created a Verbose class in AppHelpers package that locks stream io.
*    That helps to keep server and client io text from intermingling.
*    You can turn verbose mode off which silences output that isn't
*    marked "always".
*  - Fixed again the bug which prevented communicating with anything other
*    than the loopback by adding hints.ai_flags = AI_PASSIVE to
*    SocketListener member data.
*  - added more testing
*  ver 4.4 : 27 Mar 15
*  - minor changes to comments
*  - moved IncomingMessageHandler into test stub
*  ver 4.3 : 26 Mar 15
*  - fixed bug noticed by Tarun Rajput
*  - used '0' as terminator.  Should have been '\0'
*  ver 4.2 : 26 Mar 15
*  - several small changes to the Socket class interface
*  ver 4.1 : 25 Mar 15
*  - fixed connection bug that prevented connecting to anything
*    other than a loopback (localhost, 127.0.0.1, ::1) by
*    adding winsock code to SocketConnector().
*  - removed low-level code from ClientProc
*    (server's client handler callable object)
*    replaced with code written to Socket interface
*  Ver 4.0 : 24 Mar 15
*  - first release of total redesign - had a known bug (see ver 4.1)
*/
/*
* ToDo:
* - make SocketSystem a reference counted instance of Socket
* - write buffered recv which efficiently returns string or line
*   - reads and concatenates everything available into circular buffer
*   - parses out first string or line and moves start of buffer pointer
*     to begining of next
* -----------------------------------------------------------------------
*  Wait for The next items until Students have submitted their code
* -----------------------------------------------------------------------
* - build front end, e.g., Sender and Receiver classes
* - implement message facility: message class, sendMsg and recvMsg
* - Test and Display packages
*/

#ifndef WIN32_LEAN_AND_MEAN  // prevents duplicate includes of core parts of windows.h in winsock2.h
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>     // Windows sockets, ver 2
#include <Windows.h>      // Windnows API
#include <WS2tcpip.h>     // support for IPv6 and other things
#include <IPHlpApi.h>     // ip helpers

#include <vector>
#include <string>
#include <atomic>

#pragma warning(disable:4522)
#pragma comment(lib, "Ws2_32.lib")

namespace Sockets
{
	/////////////////////////////////////////////////////////////////////////////
	// SocketSystem class - manages loading and unloading Winsock library

	class SocketSystem
	{
	public:
		SocketSystem();
		~SocketSystem();
	private:
		int result;
		WSADATA wsaData;
	};

	/////////////////////////////////////////////////////////////////////////////
	// Socket class
	// - used by server for client handling
	// - base for SocketConnector and SocketListener classes

	class Socket
	{
	public:
		enum IPVersion { IP4, IP6 };
		using byte = char;

		// disable copy construction and assignment
		Socket(const Socket& s) = delete;
		Socket& operator=(const Socket& s) = delete;

		Socket(IPVersion ipver = IP4);
		Socket(SOCKET);
		Socket(Socket&& s);
		operator SOCKET() { return _socket; }
		Socket& operator=(Socket&& s);
		virtual ~Socket();

		bool SendBuffer(size_t bytes, byte* buffer);
		bool ReceiveBuffer(size_t bytes, byte* buffer);
		size_t SendStream(size_t bytes, byte* buffer);
		size_t ReceiveStream(size_t bytes, byte* buffer);
		bool ShutDownSend();
		bool ShutDownReceive();
		bool ShutDown();
		void Close();

		bool IsValidState() { return _socket != INVALID_SOCKET; }

	protected:
		WSADATA _wsaData;
		SOCKET _socket;
		struct addrinfo *_ai_result = NULL, *_pointer = NULL, _hints;
		int _result;
		IPVersion _ipVersion = IP4;

	private:
		void InitializeHints(struct addrinfo * = nullptr);
		bool ShutdownSocket(int how = SD_BOTH);
	};

	/////////////////////////////////////////////////////////////////////////////
	// SocketConnector class
	// - supports connecting to a SocketListener

	class SocketConnector : public Socket
	{
	public:
		SocketConnector(const SocketConnector& s) = delete;
		SocketConnector& operator=(const SocketConnector& s) = delete;

		SocketConnector();
		SocketConnector(SocketConnector&& s);
		SocketConnector& operator=(SocketConnector&& s);
		virtual ~SocketConnector();

		bool Connect(const std::string& ip, size_t port);
	private:
		void InitializeHints(struct addrinfo * = nullptr);
	};

	/////////////////////////////////////////////////////////////////////////////
	// SocketListener class
	// - listens for incoming connections
	// - each connection is handled on its own thread

	class SocketListener : public Socket
	{
	public:
		SocketListener(const SocketListener& s) = delete;
		SocketListener& operator=(const SocketListener& s) = delete;

		SocketListener(size_t port, IPVersion ipv = IP4);
		SocketListener(SocketListener&& s);
		SocketListener& operator=(SocketListener&& s);
		virtual ~SocketListener();

		template<typename CallableObject>
		bool Start(CallableObject& co);
		void Stop();
	private:
		bool Bind();
		bool Listen();
		Socket Accept();
		std::atomic<bool> _stop = false;
		size_t _port;
		bool _acceptFailed = false;
		void InitializeHints(struct addrinfo * = nullptr);
	};

	//----< SocketListener start function runs listener on its own thread >------
	/*
	*  - Accepts Callable Object that defines the operations
	*    to handle client requests.
	*  - You will find an example Callable Object, ClientProc,
	*    used in the test stub below
	*/
	template<typename CallableObject>
	bool SocketListener::Start(CallableObject& co)
	{
		if (!Bind())
		{
			return false;
		}

		if (!Listen())
		{
			return false;
		}
		// listen on a dedicated thread so server's main thread won't block

		std::thread listenThread(
			[&]()
		{
			while (!_acceptFailed)
			{
				if (_stop.load())
					break;

				// Accept a client socket - blocking call

				Socket clientSocket = Accept();    // uses move ctor

				if (!clientSocket.IsValidState()) {
					continue;
				}

				std::thread clientThread(co, std::move(clientSocket));
				clientThread.detach();  // detach - listener won't access thread again
			}
		}
		);
		listenThread.detach();
		return true;
	}
}
#endif