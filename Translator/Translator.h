#pragma once

/////////////////////////////////////////////////////////////////////////
// Translator.h - Translates messages to/from managed and native types //
// ver 1.0                                                             //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018	   //
/////////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
* ---------------------
*  This C++\Cli Package contains one class, Translator.  It's purpose is to convert
*  managed messages from managed to native objects and vice versa.
*
*  Required Files:
* -----------------
*  Translator.h, Translater.cpp
*  Utilities.h
*  Required References:
* ----------------------
*  CommunicationFactory.dll
*  Socket.lib
*
* This is largely based of Prof. Fawcett's code, but hey, I spelled Translator correctly! :)
*/

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <iostream>
#include "..\Socket\ICommunication.h"
#include "..\Socket\Message.h"
#include "..\Socket\Socket.h"

namespace Translator {
	public ref class EndPoint
	{
	public:
		Sockets::EndPoint AsNativeEndPoint();
		static EndPoint^ FromNativeEndPoint(Sockets::EndPoint&);
		System::String^ AsString();
		property System::String^ Address;
		property int Port;
	};

	public enum MessageType { FILE_GET, FILE_LIST, FILE_MKDIR, FILE_POST, SIGNAL_READY, SIGNAL_SHUTDOWN, TEST_ACKNOWLEDGE, TEST_EXECUTE, TEST_REQUEST, USER_AUTHENTICATE, USER_REGISTER };

	public ref class Message
	{
	public:
		Sockets::Message* AsNativeMessage();
		static Message^ FromNativeMessage(Sockets::Message&);
		property System::String^ MessageId;
		property EndPoint^ Source;
		property EndPoint^ Destination;
		property MessageType Type;
		property System::DateTime^ Timestamp;
		property System::String^ Filename;
		property System::String^ Body;
		property int ContentLength;
	};

	public ref class Communication
	{
	public:
		Communication(size_t, int context);
		Message^ CollectMessage();
		void DeliverMessage(Message^);
		void StartListening();
	private:
		Sockets::ICommunication *_communication;
	};
}

#endif // !TRANSLATOR_H