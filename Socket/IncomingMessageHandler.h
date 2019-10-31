#pragma once

////////////////////////////////////////////////////////////////////////////
// IncomingMessageHandler.h - A class that handles incoming messages	  //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package contains a handler class that is a functor, accepting a socket parameter
* An instance of this is passed to a Receiver's constructor
* This is largely based on Prof. Fawcett's example code.
*
* Required Files:
* ---------------
* Message.h, Socket.h, BlockingQueue.h
*/

#ifndef INCOMING_MESSAGE_HANDLER_H
#define INCOMING_MESSAGE_HANDLER_H

#include "..\BlockingQueue\BlockingQueue.h"
#include "ICommunication.h"
#include "Message.h"
#include "Socket.h"

class IncomingMessageHandler
{
public:
	void SetTestRequestQueue(BlockingQueue<Sockets::Message>*);
	void SetReadyWorkerQueue(BlockingQueue<Sockets::Message>*);
	void SetSendMessageQueue(BlockingQueue<Sockets::Message>*);	
	void SetRecvMessageQueue(BlockingQueue<Sockets::Message>*);
	void SetContext(Sockets::CONTEXT context);
	void operator()(Sockets::Socket);
private:
	Sockets::CONTEXT _context;
	std::string ReadMessage();
	bool ReceiveFile(Sockets::Message);
	Sockets::Message* BuildResponse(Sockets::Message&);
	BlockingQueue<Sockets::Message> *_testRequestQPtr, *_readyWorkerQPtr, *_sendMessageQPtr, *_recvMessageQPtr;
	Sockets::Socket* _socketPointer = nullptr;
};
#endif // !INCOMING_MESSAGE_HANDLER_H

