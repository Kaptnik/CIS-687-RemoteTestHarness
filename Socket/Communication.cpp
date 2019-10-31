/////////////////////////////////////////////////////////////////////////////
// Communication.cpp - A class that encapsulates both a Sender and Receiver//
// ver 1.0																   //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		   //
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Communication.h"
#include "IncomingMessageHandler.h"

using namespace Sockets;

// Constructor to set port to listen on
Communication::Communication(size_t port) : _receiver(port) 
{
	ProcessContext = Sockets::CONTEXT::SERVER;
}

// Constructor to set port to listen on & the context
Communication::Communication(size_t port, Sockets::CONTEXT context) : _receiver(port)
{ 
	this->ProcessContext = context;
}

// Start the sender and the receiver
void Communication::Start()
{
	IncomingMessageHandler *handler = new IncomingMessageHandler();
	handler->SetContext(ProcessContext);
	handler->SetSendMessageQueue(_sender.GetQueue());
	handler->SetRecvMessageQueue(_receiver.GetQueue());
	handler->SetTestRequestQueue(_testRequestQPtr);
	handler->SetReadyWorkerQueue(_readyWorkerQPtr);

	_sender.Start();
	_receiver.Start(*handler);
}

// Stop the sender and the receiver
void Communication::Stop()
{
	_receiver.Stop();
	_sender.Stop();
}

// Set the test request queue
void Communication::SetTestRequestQueue(BlockingQueue<Message>* queuePointer)
{
	_testRequestQPtr = queuePointer;
}

// Set the ready worker queue
void Communication::SetReadyWorkerQueue(BlockingQueue<Message>* queuePointer)
{
	_readyWorkerQPtr = queuePointer;
}

// Fetch a pointer to the receiver queue
BlockingQueue<Message>* Communication::GetReceiverQueue()
{
	return _receiver.GetQueue();
}

// Queue up a message in the sender's blocking queue
void Communication::DeliverMessage(Message message)
{
	_sender.DeliverMessage(message);
}

// DeQueue a message from the receiver's blocking queue
Message Communication::CollectMessage()
{
	return _receiver.CollectMessage();
}