#pragma once

////////////////////////////////////////////////////////////////////////////
// Communication.h - A class that encapsulates both a Sender and Receiver //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* This package contains a Communication class that encapsulates 
* both a sender and a receiver. The communication happens through
* messages defined by the Message class.
*
* This is largely based on Prof. Fawcett's example code.
*
* Required Files:
* ---------------
* Message.h, Sender.h, Receiver.h
*/


#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "ICommunication.h"
#include "Message.h"
#include "Receiver.h"
#include "Sender.h"

namespace Sockets {
	class Communication: public ICommunication
	{
	public:
		Communication(size_t);
		Communication(size_t, Sockets::CONTEXT);
		void Start();
		void Stop();
		void DeliverMessage(Message);
		Message CollectMessage();
		BlockingQueue<Message>* GetReceiverQueue();
		void SetTestRequestQueue(BlockingQueue<Message>*);
		void SetReadyWorkerQueue(BlockingQueue<Message>*);
	private:
		Sender _sender;
		Receiver _receiver;
		BlockingQueue<Message> *_testRequestQPtr, *_readyWorkerQPtr;
		SocketSystem _socketSystem;
	};

	inline ICommunication* ICommunication::CreateInstance(size_t port)
	{
		ICommunication* communication = new Communication(port);
		return communication;
	}
}
#endif // COMMUNICATION_H