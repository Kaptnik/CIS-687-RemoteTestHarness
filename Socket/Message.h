#pragma once

////////////////////////////////////////////////////////////////////////////
// Message.h - A class that defines the construct of a message			  //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package contains a Message class that provides:
*	1. Source and Destination address
*	2. Message type
*	3. Time-Date
*	4. Body
* and a few other properties like content length, etc.
*
* This is based on the requirements for the project at 
* https://ecs.syr.edu/faculty/fawcett/handouts/CSE687-OnLine/SynchronousLectures/Project3.htm
*
* Required Files:
* ---------------
* Endpoint.h
*/

#ifndef MESSAGE_H
#define MESSAGE_H

#include "Endpoint.h"
#include <chrono>

namespace Sockets
{
	enum MessageType { FILE_GET, FILE_LIST, FILE_MKDIR, FILE_POST, SIGNAL_READY, SIGNAL_SHUTDOWN, TEST_ACKNOWLEDGE, TEST_EXECUTE, TEST_REQUEST, USER_AUTHENTICATE, USER_REGISTER };

	class Message
	{
	public:
		using timestamp = std::chrono::system_clock::time_point;
		Message();
		Message(std::string);
		Message(EndPoint, EndPoint);
		Message(std::string, std::string);
		std::string GetMessageId();
		EndPoint GetSource();
		EndPoint GetDestination();
		MessageType GetType();
		timestamp GetTimestamp();
		std::string GetBody();
		size_t GetContentLength();
		std::string GetFilename();
		void SetMessageId(std::string);
		void SetSource(EndPoint);
		void SetDestination(EndPoint);
		void SetType(MessageType);
		void SetTimestamp(timestamp);
		void SetBody(std::string);
		void SetContentLength(size_t);
		void SetFilename(std::string);
		std::string ToString();
	private:
		std::string _messageId;
		EndPoint _source;
		EndPoint _destination;
		MessageType _type;
		timestamp _datetime;
		std::string _filename;
		std::string _body;
		size_t _contentLength;
	};
}

#endif // ! MESSAGE_H

