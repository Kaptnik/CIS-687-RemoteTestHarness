/////////////////////////////////////////////////////////////////////////
// Sockets.cpp - C++ wrapper for Win32 socket api                      //
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

#include "stdafx.h"
#include "Socket.h"
#include "..\Utilities\converter.h"
#include <iostream>
#include <exception>
#include <sstream>
#include <thread>
#include <memory>
#include <functional>
#include <exception>

using namespace Sockets;

/////////////////////////////////////////////////////////////////////////////
// SocketSystem class members

//----< constructor starts up sockets by loading winsock lib >---------------

SocketSystem::SocketSystem()
{
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}
//-----< destructor frees winsock lib >--------------------------------------

SocketSystem::~SocketSystem()
{
	WSACleanup();
}

/////////////////////////////////////////////////////////////////////////////
// Socket class members

//----< constructor sets TCP protocol and Stream mode >----------------------

Socket::Socket(IPVersion ipVersion) : _ipVersion(ipVersion)
{
	InitializeHints();
}
//----< promotes Win32 socket to Socket >------------------------------------
/*
*  You have to set ip version if you want IP6 after promotion, e.g.:
*     s.ipVer() = IP6;
*/
Socket::Socket(SOCKET sock) : _socket(sock)
{
	_ipVersion = IP4;
	InitializeHints();
}
//----< transfer socket ownership with move constructor >--------------------

Socket::Socket(Socket&& s)
{
	_socket = s._socket;
	s._socket = INVALID_SOCKET;
	_ipVersion = s._ipVersion;
	InitializeHints(&s._hints);
}
//----< transfer socket ownership with move assignment >---------------------

Socket& Socket::operator=(Socket&& s)
{
	if (this == &s) return *this;
	_socket = s._socket;
	s._socket = INVALID_SOCKET;
	_ipVersion = s._ipVersion;
	InitializeHints(&s._hints);
	return *this;
}

//----< close connection >---------------------------------------------------

void Socket::Close()
{
	if (_socket != INVALID_SOCKET)
		closesocket(_socket);
}
//----< tells receiver there will be no more sends from this socket >--------

bool Socket::ShutDownSend()
{
	return ShutdownSocket(SD_SEND);
}

//----< tells receiver this socket won't call receive anymore >--------------

bool Socket::ShutDownReceive()
{
	return ShutdownSocket(SD_RECEIVE);
}
//----< tells receiver there will be no more sends or recvs >----------------

bool Socket::ShutDown()
{
	return ShutdownSocket();
}

bool Socket::ShutdownSocket(int how /* = SD_BOTH */)
{
	shutdown(_socket, how);
	if (_socket != INVALID_SOCKET)
		return true;
	return false;
}

//----< destructor closes socket handle >------------------------------------

Socket::~Socket() {
	ShutDown();
	Close();
}
//----< send buffer >--------------------------------------------------------
/*
*  - bytes must be less than or equal to the size of buffer
*  - doesn't return until requested number of bytes have been sent
*/
bool Socket::SendBuffer(size_t bytes, byte* buffer)
{
	size_t bytesSent = 0, bytesLeft = bytes;
	byte* pBuf = buffer;
	while (bytesLeft > 0)
	{
		bytesSent = send(_socket, pBuf, bytesLeft, 0);
		if (_socket == INVALID_SOCKET || bytesSent == 0)
			return false;
		bytesLeft -= bytesSent;
		pBuf += bytesSent;
	}
	return true;
}
//----< recv buffer >--------------------------------------------------------
/*
*  - bytes must be less than or equal to the size of buffer
*  - doesn't return until buffer has been filled with requested bytes
*/
bool Socket::ReceiveBuffer(size_t bytes, byte* buffer)
{
	size_t bytesRecvd = 0, bytesLeft = bytes;
	byte* pBuf = buffer;
	while (bytesLeft > 0)
	{
		bytesRecvd = recv(_socket, pBuf, bytesLeft, 0);
		if (_socket == INVALID_SOCKET || bytesRecvd == 0)
			return false;
		bytesLeft -= bytesRecvd;
		pBuf += bytesRecvd;
	}
	return true;
}
//----< attempt to send specified number of bytes, but may not send all >----
/*
* returns number of bytes actually sent
*/
size_t Socket::SendStream(size_t bytes, byte* pBuf)
{
	return send(_socket, pBuf, bytes, 0);
}
//----< attempt to recv specified number of bytes, but may not send all >----
/*
* returns number of bytes actually received
*/
size_t Socket::ReceiveStream(size_t bytes, byte* pBuf)
{
	return recv(_socket, pBuf, bytes, 0);
}
//----< returns bytes available in recv buffer >-----------------------------

void Socket::InitializeHints(struct addrinfo* hints)
{
	ZeroMemory(&_hints, sizeof(_hints));
	if (hints == nullptr)
	{		
		_hints.ai_family = AF_UNSPEC;
		_hints.ai_socktype = SOCK_STREAM;
		_hints.ai_protocol = IPPROTO_TCP;
	}
	else
	{
		_hints.ai_family = hints->ai_family;
		_hints.ai_socktype = hints->ai_socktype;
		_hints.ai_protocol = hints->ai_protocol;
	}
}

/////////////////////////////////////////////////////////////////////////////
// SocketConnector class members

//----< constructor inherits its base Socket's Win32 _socket member >--------

SocketConnector::SocketConnector(): Socket()
{
	_hints.ai_family = AF_UNSPEC;
}
//----< move constructor transfers ownership of Win32 _socket member >-------

SocketConnector::SocketConnector(SocketConnector&& s) : Socket()
{
	_socket = s._socket;
	s._socket = INVALID_SOCKET;
	_ipVersion = s._ipVersion;
	InitializeHints(&s._hints);
}
//----< move assignment transfers ownership of Win32 _socket member >--------

SocketConnector& SocketConnector::operator=(SocketConnector&& s)
{
	if (this == &s) return *this;
	_socket = s._socket;
	s._socket = INVALID_SOCKET;
	_ipVersion = s._ipVersion;
	InitializeHints(&s._hints);
	return *this;
}
//----< destructor announces destruction if Verbose(true) >------------------

SocketConnector::~SocketConnector()
{
}
//----< request to connect to ip and port >----------------------------------

bool SocketConnector::Connect(const std::string& ip, size_t port)
{
	size_t uport = htons((u_short)port);
	std::string sPort =Utilities::Convert::ToString<size_t>(uport);

	// Resolve the server address and port
	const char* pTemp = ip.c_str();
	_result = getaddrinfo(pTemp, sPort.c_str(), &_hints, &_ai_result);  // was DEFAULT_PORT
	if (_result != 0) {
		return false;
	}

	// Attempt to connect to an address until one succeeds
	for (_pointer = _ai_result; _pointer != NULL; _pointer = _pointer->ai_next) {

		char ipstr[INET6_ADDRSTRLEN];
		void *addr;
		char *ipver;

		// get pointer to address - different fields in IPv4 and IPv6:

		if (_pointer->ai_family == AF_INET) { // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)_pointer->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = const_cast<char*>("IPv4");
		}
		else { // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)_pointer->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = const_cast<char*>("IPv6");
		}

		// convert the IP to a string and print it:
		inet_ntop(_pointer->ai_family, addr, ipstr, sizeof ipstr);
		//printf("\n  %s: %s", ipver, ipstr);

		// Create a SOCKET for connecting to server
		_socket = socket(_pointer->ai_family, _pointer->ai_socktype, _pointer->ai_protocol);
		if (_socket == INVALID_SOCKET) {
			int error = WSAGetLastError();
			return false;
		}

		_result = connect(_socket, _pointer->ai_addr, (int)_pointer->ai_addrlen);
		if (_result == SOCKET_ERROR) {
			_socket = INVALID_SOCKET;
			int error = WSAGetLastError();
			continue;
		}
		break;
	}

	freeaddrinfo(_ai_result);

	if (_socket == INVALID_SOCKET) {
		int error = WSAGetLastError();
		return false;
	}
	return true;
}

void SocketConnector::InitializeHints(struct addrinfo* hints)
{
	ZeroMemory(&_hints, sizeof(_hints));
	if (hints == nullptr)
	{
		_hints.ai_family = AF_UNSPEC;
		_hints.ai_socktype = SOCK_STREAM;
		_hints.ai_protocol = IPPROTO_TCP;
	}
	else
	{
		_hints.ai_family = hints->ai_family;
		_hints.ai_socktype = hints->ai_socktype;
		_hints.ai_protocol = hints->ai_protocol;
	}
}

/////////////////////////////////////////////////////////////////////////////
// SocketListener class members

//----< constructs SocketListener, specifying type of protocol to use >------

SocketListener::SocketListener(size_t port, IPVersion ipVersion) : Socket(ipVersion), _port(port)
{
	_socket = INVALID_SOCKET;
}
//----< move constructor transfers ownership of Win32 _socket member >-------

SocketListener::SocketListener(SocketListener&& s) : Socket()
{
	_socket = s._socket;
	s._socket = INVALID_SOCKET;
	_ipVersion = s._ipVersion;
	InitializeHints(&s._hints);
}
//----< move assignment transfers ownership of Win32 _socket member >--------

SocketListener& SocketListener::operator=(SocketListener&& s)
{
	if (this == &s) return *this;
	_socket = s._socket;
	s._socket = INVALID_SOCKET;
	_ipVersion = s._ipVersion;
	InitializeHints(&s._hints);
	return *this;
}
//----< destructor announces destruction if Verbal(true) >-------------------

SocketListener::~SocketListener()
{
}
//----< binds SocketListener to a network adddress on local machine >--------

bool SocketListener::Bind()
{
	// Resolve the server address and port

	size_t uport = htons((u_short)_port);
	std::string sPort =Utilities::Convert::ToString<size_t>(uport);
	_result = getaddrinfo(NULL, sPort.c_str(), &_hints, &_ai_result);
	if (_result != 0) {
		return false;
	}

	// Iterate through all results and bind to first available or all, depending on else condition, below

	for (auto pResult = _ai_result; pResult != NULL; pResult = pResult->ai_next)
	{
		// Create a SOCKET for connecting to server

		_socket = socket(pResult->ai_family, pResult->ai_socktype, pResult->ai_protocol);
		if (_socket == INVALID_SOCKET) {
			int error = WSAGetLastError();
			continue;
		}

		// Setup the TCP listening socket
		_result = bind(_socket, pResult->ai_addr, (int)pResult->ai_addrlen);
		if (_result == SOCKET_ERROR) {
			int error = WSAGetLastError();
			_socket = INVALID_SOCKET;
			continue;
		}
		else
		{
			//break;  // bind to first available
			continue;   // bind to all
		}
	}
	freeaddrinfo(_ai_result);
	return true;
}
//----< put SocketListener in listen mode, doesn't block >-------------------

bool SocketListener::Listen()
{
	_result = listen(_socket, SOMAXCONN);
	if (_result == SOCKET_ERROR) {
		int error = WSAGetLastError();
		_socket = INVALID_SOCKET;
		return false;
	}
	return true;
}
//----< accepts incoming requrests to connect - blocking call >--------------

Socket SocketListener::Accept()
{
	SOCKET sock = accept(_socket, NULL, NULL);
	Socket clientSocket = sock;    // uses Socket(::SOCKET) promotion ctor
	if (!clientSocket.IsValidState()) {
		_acceptFailed = true;
		int error = WSAGetLastError();
		return clientSocket;
	}
	return clientSocket;
}

//----< request SocketListener to stop accepting connections >---------------
void SocketListener::Stop()
{
	_stop.exchange(true);
}

void SocketListener::InitializeHints(struct addrinfo* hints)
{
	ZeroMemory(&_hints, sizeof(_hints));
	if (hints == nullptr)
	{		
		_hints.ai_socktype = SOCK_STREAM;
		_hints.ai_protocol = IPPROTO_TCP;
		_hints.ai_flags = AI_PASSIVE;
		if (_ipVersion == IPVersion::IP6)
			_hints.ai_family = AF_INET6;
		else
			_hints.ai_family = AF_INET;
	}
	else
	{
		_hints.ai_socktype = hints->ai_socktype;
		_hints.ai_protocol = hints->ai_protocol;
		_hints.ai_family = hints->ai_family;
		_hints.ai_flags = hints->ai_flags;
	}
}

#ifdef TEST_SOCKETS

//----< test stub >----------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// Server's client handler class
// - must be callable object so we've built as a functor
// - passed to SocketListener::start(CallObject& co)
// - Client handling thread starts by calling operator()

class ClientHandler
{
public:
	void operator()(Socket& _socket);
	bool testStringHandling(Socket& _socket);
	bool testBufferHandling(Socket& _socket);
};

//----< Client Handler thread starts running this function >-----------------

void clearBuffer(Socket::byte* buffer, size_t BufLen)
{
	for (size_t i = 0; i < BufLen; ++i)
		buffer[i] = '\0';
}

void ClientHandler::operator()(Socket& _socket)
{
	while (true)
	{
		// interpret test command

		std::string command = Socket::removeTerminator(_socket.ReceiveString());
		Show::write("\n  server rcvd command: " + command);
		if (command == "Done")
		{
			Show::write("\n  server sent : " + command);
			_socket.SendString(command);
			break;
		}
		if (command.size() == 0)
		{
			Show::write("\n  client connection closed");
			break;
		}
		//Show::write("\n  server recvd: " + command);

		if (command == "TEST_STRING_HANDLING")
		{
			if (testStringHandling(_socket))
				Show::write("\n  ----String Handling test passed\n");
			else
				Show::write("\n  ----String Handling test failed\n");
			continue; // go back and get another command
		}
		if (command == "TEST_BUFFER_HANDLING")
		{
			if (testBufferHandling(_socket))
				Show::write("\n  ----Buffer Handling test passed\n");
			else
				Show::write("\n  ----Buffer Handling test failed\n");
			continue;  // get another command
		}
	}

	// we get here if command isn't requesting a test, e.g., "TEST_STOP"

	Show::write("\n");
	Show::write("\n  ClientHandler socket connection closing");
	_socket.ShutDown();
	_socket.Close();
	Show::write("\n  ClientHandler thread terminating");
}

//----< test string handling >-----------------------------------------------
/*
*   Creates strings, sends to server, then reads strings server echos back.
*/
bool ClientHandler::testStringHandling(Socket& _socket)
{
	Show::title("String handling test on server");

	while (true)
	{
		std::string str = Socket::removeTerminator(_socket.ReceiveString());
		if (_socket == INVALID_SOCKET)
			return false;
		if (str.size() > 0)
		{
			//Show::write("\n  bytes recvd at server: " + toString(str.size() + 1));
			Show::write("\n  server rcvd : " + str);

			if (_socket.SendString(str))
			{
				Show::write("\n  server sent : " + str);
			}
			else
			{
				return false;
			}
			if (str == "TEST_END")
				break;
		}
		else
		{
			break;
		}
	}
	_socket.SendString("TEST_STRING_HANDLING_END");
	Show::write("\n  End of string handling test in ClientHandler");
	return true;
}

//----< test buffer handling >-----------------------------------------------
/*
*   Creates buffers, sends to server, then reads buffers server echos back.
*/
bool ClientHandler::testBufferHandling(Socket& _socket)
{
	Show::title("Buffer handling test on server");
	const size_t BufLen = 20;
	Socket::byte buffer[BufLen];
	bool ok;

	while (true)
	{
		ok = _socket.ReceiveBuffer(BufLen, buffer);
		if (_socket == INVALID_SOCKET)
			return false;
		if (ok)
		{
			std::string temp;
			for (size_t i = 0; i < BufLen; ++i)
				temp += buffer[i];
			//Show::write("\n  bytes recvd at server: " + toString(BufLen));
			Show::write("\n  server rcvd : " + temp);

			buffer[BufLen - 1] = '\0';
			if (_socket.Send(BufLen, buffer))
			{
				Show::write("\n  server sent : " + std::string(buffer));
			}
			else
			{
				Show::write("\n  server send failed");
				return false;
			}
			if (temp.find("TEST_END") != std::string::npos)
			{
				//std::string out = "TEST_END";
				//_socket.send(out.size(), (Socket::byte*)out.c_str());
				//Show::write("\n  server sent : " + out);
				break;
			}
		}
		else
		{
			break;
		}
	}
	Show::write("\n  End of buffer handling test in ClientHandler");
	::Sleep(4000);
	return true;
}

//----< test string handling - server echos back client sent string >--------

void clientTestStringHandling(Socket& si)
{
	std::string command = "TEST_STRING_HANDLING";
	si.SendString(command);
	Show::write("\n  client sent : " + command);

	for (size_t i = 0; i < 5; ++i)
	{
		std::string text = "Hello World " + std::string("#") + Conv<size_t>::toString(i + 1);
		si.SendString(text);
		Show::write("\n  client sent : " + text);
	}
	command = "TEST_END";
	si.SendString(command);
	Show::write("\n  client sent : " + command);

	while (true)
	{
		std::string str = Socket::removeTerminator(si.ReceiveString());
		if (str.size() == 0)
		{
			Show::write("\n  client detected closed connection");
			break;
		}
		Show::write("\n  client recvd: " + str);
		if (str == "TEST_END")
		{
			Show::write("\n  End of string handling test in client");
			break;
		}
	}
}
//----< test buffer handling - server echos back client sent buffer >--------

void clientTestBufferHandling(Socket& si)
{
	std::string command = "TEST_BUFFER_HANDLING";
	si.SendString(command);
	Show::write("\n  client sent : " + command);

	const int BufLen = 20;
	Socket::byte buffer[BufLen];

	for (size_t i = 0; i < 5; ++i)
	{
		std::string text = "Hello World " + std::string("#") + Conv<size_t>::toString(i + 1);
		for (size_t i = 0; i < BufLen; ++i)
		{
			if (i < text.size())
				buffer[i] = text[i];
			else
				buffer[i] = '.';
		}
		buffer[BufLen - 1] = '\0';
		si.Send(BufLen, buffer);
		Show::write("\n  client sent : " + std::string(buffer));
	}
	std::string text = "TEST_END";
	for (size_t i = 0; i < BufLen; ++i)
	{
		if (i < text.size())
			buffer[i] = text[i];
		else
			buffer[i] = '.';
	}
	buffer[BufLen - 1] = '\0';
	si.Send(BufLen, buffer);
	Show::write("\n  client sent : " + std::string(buffer));

	bool ok;
	std::string collector;
	while (true)
	{
		if (si.bytesWaiting() == 0)
			break;
		ok = si.ReceiveBuffer(BufLen, buffer);
		if (!ok)
		{
			Show::write("\n  client unable to receive");
			break;
		}
		std::string str(buffer);
		collector += str;
		if (str.size() == 0)
		{
			Show::write("\n  client detected closed connection");
			break;
		}
		Show::write("\n  client rcvd : " + str);
		if (collector.find("TEST_END") != std::string::npos)
		{
			Show::write("\n  End of buffer handling test in client");
			break;
		}
	}
}
//----< demonstration >------------------------------------------------------

int main(int argc, char* argv[])
{
	Show::attach(&std::cout);
	Show::start();
	Show::title("Testing Sockets", '=');

	try
	{
		SocketSystem ss;
		SocketConnector si;
		SocketListener sl(9070, Socket::IP6);
		ClientHandler cp;
		sl.start(cp);

		while (!si.connect("localhost", 9070))
		{
			Show::write("\n  client waiting to connect");
			::Sleep(100);
		}

		Show::title("Starting string test on client");
		clientTestStringHandling(si);

		////////////////////////////////////////////////////
		// This buffer handling test doesn't work yet.
		// I'll fix when time permits.
		//
		// Show::title("Starting buffer test on client");
		// clientTestBufferHandling(si);

		si.SendString("TEST_STOP");

		Show::write("\n\n  client calling send shutdown\n");
		si.ShutDownSend();
		sl.stop();
	}
	catch (std::exception& ex)
	{
		std::cout << "\n  Exception caught:";
		std::cout << "\n  " << ex.what() << "\n\n";
	}
}

#endif
