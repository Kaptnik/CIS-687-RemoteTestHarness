#pragma once

////////////////////////////////////////////////////////////////////////////
// Receiver.h - A class that receives messages on a socket, and queues it //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package contains a Receiver class that receives messages
* on a socket and queues them up in a BlockingQueue.
*
* This is largely based on Prof. Fawcett's example code.
*
* Required Files:
* ---------------
* Message.h, Socket.h, BlockingQueue.h
*/

#ifndef RECEIVER_H
#define RECEIVER_H

#include "..\BlockingQueue\BlockingQueue.h"
#include "Message.h"
#include "Socket.h"


class Receiver
{
public:
	Receiver(size_t);
	template<typename CallableObject>
	void Start(CallableObject& co);
	void Stop();
	Sockets::Message CollectMessage();
	BlockingQueue<Sockets::Message>* GetQueue();
private:
	BlockingQueue<Sockets::Message> _receiveQueue;
	Sockets::SocketListener _listener;
};

template<typename CallableObject>
void Receiver::Start(CallableObject& callableObject)
{
	_listener.Start(callableObject);
}

#endif // !RECEIVER_H
