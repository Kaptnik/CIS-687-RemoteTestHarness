////////////////////////////////////////////////////////////////////////////
// Sender.cpp - A class that sends messages on a socket                   //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Sender.h"
#include <fstream>

using namespace Sockets;

// Default constructor
Sender::Sender(){}

Sender::~Sender()
{
	if (_sendThread.joinable())
		_sendThread.join();
}

// Create a thread that dequeues from the blocking queue
// and sends a message to an endpoint given by the message
// destination
void Sender::Start()
{
	auto threadProc = [&]()
	{
		while (true)
		{			
			Message *message = new Message();
			*message = _sendQueue.DeQueue();
			if (message->GetType() == MessageType::SIGNAL_SHUTDOWN)
			{
				break;
			}			
			if (!Connect(message->GetDestination()))
			{
				std::cout << "Couldn't connect to " << message->GetDestination().AsString() << std::endl;
				continue;
			}
			std::string *serializedMessage = new std::string();
			*serializedMessage = message->ToString();
			delete message;
			_connector.SendBuffer(serializedMessage->length(), (char*)(serializedMessage->c_str()));
			delete serializedMessage;
			_connector.ShutDown();
		}
	};
	std::thread thread(threadProc);
	_sendThread = std::move(thread);
}

// Stop by sending a shutdown message to self
void Sender::Stop()
{
	Message message;
	message.SetType(MessageType::SIGNAL_SHUTDOWN);
	DeliverMessage(message);
}

// Connect to an endpoint (Required before sending message)
bool Sender::Connect(EndPoint endPoint)
{
	return _connector.Connect(endPoint.GetUriHost(), endPoint.GetPort());
}

// Send a message to an endpoint
void Sender::DeliverMessage(Message message)
{
	_sendQueue.EnQueue(message);
}

// Return a pointer to the blocking queue
BlockingQueue<Sockets::Message>* Sender::GetQueue()
{
	return &_sendQueue;
}