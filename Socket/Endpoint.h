#pragma once

////////////////////////////////////////////////////////////////////////////
// EndPoint.h - A class that defines an endpoint for a message			  //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package contains an EndPoint class can be used to
* identify both the source and the destination of a message.
*
* This is largely based on Prof. Fawcett's example code.
*
*/


#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <iostream>
#include <exception>

namespace Sockets 
{
	class EndPoint
	{
	public:
		EndPoint();
		EndPoint(const EndPoint&);
		void operator=(const EndPoint&);
		EndPoint(const std::string& address, size_t port) : _address(address), _port(port) {}
		EndPoint(const std::string&);
		std::string GetUriHost() const;
		size_t GetPort() const;
		std::string AsString();
		const char SEPARATOR = ':';
	private:
		std::string _address;
		size_t _port;
	};
}
#endif // ENDPOINT_H
