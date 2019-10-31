/////////////////////////////////////////////////////////////////////////////
// Receiver.cpp - A class that receives messages on a socket, and queues it//
// ver 1.0																   //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		   //
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Receiver.h"

using namespace Sockets;

// Initialize listener
Receiver::Receiver(size_t port) : _listener(port) {}

// Stop the listener
void Receiver::Stop()
{
	_listener.Stop();
}

// DeQueue from the blocking queue
Message Receiver::CollectMessage()
{
	return _receiveQueue.DeQueue();
}

// Return a pointer to the blocking queue
BlockingQueue<Message>* Receiver::GetQueue()
{
	return &_receiveQueue;
}