////////////////////////////////////////////////////////////////////////////
// IncomingMessageHandler.cpp - A class that handles incoming messages	  //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IncomingMessageHandler.h"
#include "..\Utilities\Credentials.h"
#include "..\Utilities\Converter.h"
#include "..\Utilities\Directory.h"
#include "..\Utilities\File.h"
#include "..\Utilities\Logger.h"
#include <objbase.h>
#include <fstream>

using namespace Sockets;

const int BLOCK_SIZE = 102400;

// Set the test request queue
void IncomingMessageHandler::SetTestRequestQueue(BlockingQueue<Message>* queuePointer)
{
	_testRequestQPtr = queuePointer;
}

// Set the ready worker queue
void IncomingMessageHandler::SetReadyWorkerQueue(BlockingQueue<Sockets::Message>* queuePointer)
{
	_readyWorkerQPtr = queuePointer;
}

// Set the sender queue
void IncomingMessageHandler::SetSendMessageQueue(BlockingQueue<Sockets::Message>* queuePointer)
{
	_sendMessageQPtr = queuePointer;
}

void IncomingMessageHandler::SetRecvMessageQueue(BlockingQueue<Sockets::Message>* queuePointer)
{
	_recvMessageQPtr = queuePointer;
}

void IncomingMessageHandler::SetContext(Sockets::CONTEXT context)
{
	_context = context;
}

// Read a message from a socket as a string
std::string IncomingMessageHandler::ReadMessage()
{
	std::string temp, message;
	char* buffer = new char[BLOCK_SIZE];
	size_t blockSize;
	if (_socketPointer->IsValidState())
	{
		blockSize = _socketPointer->ReceiveStream(BLOCK_SIZE, buffer);
		if (blockSize > 0)
		{
			temp = buffer;
			message += temp.substr(0, blockSize);
		}
	}
	delete(buffer);
	return message;
}

// Receive a file if send on the socket
bool IncomingMessageHandler::ReceiveFile(Message message)
{
	size_t filesize = message.GetContentLength();
	if (filesize == 0)
		return false;

	std::string downloadLocation;

	downloadLocation = message.GetFilename();
	// If acting as the server, prefix the repository
	if (_context == Sockets::CONTEXT::SERVER)
	{
		if (downloadLocation.substr(0, 12) == "TestResults_")
			downloadLocation = "TESTRESULTS/" + downloadLocation;
		else
			downloadLocation = "REPOSITORY/" + downloadLocation;
	}

	std::ofstream outstream(downloadLocation, std::ios::binary);
	if (!outstream.good())
	{
		return false;
	}
	std::string body = message.GetBody();
	std::string decodedBody = Utilities::Convert::Base64Decode(body);
	outstream.write(decodedBody.c_str(), decodedBody.length());
	outstream.flush();
	outstream.close();
	return true;
}

Message* IncomingMessageHandler::BuildResponse(Message& incomingMessage)
{
	Message *response = new Message();
	response->SetSource(incomingMessage.GetDestination());
	response->SetDestination(incomingMessage.GetSource());
	return response;
}

// Overloading the () operator to make this a functor
void IncomingMessageHandler::operator()(Socket socket)
{
	std::string logMessage;
	_socketPointer = &socket;
	while (_socketPointer->IsValidState())
	{
		// Read a message from the socket
		std::string messageString = ReadMessage();
		if (messageString.length() == 0 || messageString.length() >= BLOCK_SIZE)
			break;

		// Deserialize it to a message object
		Message *message = new Message(messageString);
		std::string body = message->GetBody();
		body.erase(std::remove(body.begin(), body.end(), '\n'), body.end());
		message->SetBody(body);
		// If message type is to get a file, respond with one.
		if (message->GetType() == MessageType::FILE_GET)
		{
			std::string filepath = "REPOSITORY" + message->GetBody();

			std::string fileContents = Utilities::File::GetFileContents(filepath);
			Message* file = BuildResponse(*message);
			file->SetType(MessageType::FILE_POST);
			file->SetBody(fileContents);
			file->SetContentLength(fileContents.length());
			file->SetFilename(message->GetFilename());
			logMessage = "Sending file " + filepath + " to " + message->GetSource().AsString();
			Utilities::Logger::Log(logMessage.c_str());
			_sendMessageQPtr->EnQueue(*file);
			continue;
		}

		// If it's a mkdir request, create a directory
		else if (message->GetType() == MessageType::FILE_MKDIR)
		{
			std::string directory = "REPOSITORY" + message->GetBody();
			CreateDirectory(directory.c_str(), NULL);
			continue;
		}

		// If it's a posted file, download it
		else if (message->GetType() == MessageType::FILE_POST)
		{
			logMessage = "Received a file (" + message->GetFilename() + ") with MessageID (" + message->GetMessageId() + ") from " + message->GetSource().AsString();
			Utilities::Logger::Log(logMessage.c_str());
			ReceiveFile(*message);
			continue;
		}
		// If it's a request to list a directory
		else if (message->GetType() == MessageType::FILE_LIST)
		{
			if (_context == Sockets::CONTEXT::SERVER)
			{
				std::string directory = "REPOSITORY" + message->GetBody();
				Message* list = BuildResponse(*message);
				list->SetType(MessageType::FILE_LIST);
				list->SetBody(Utilities::Convert::ListToString(Utilities::Directory::GetFileSystemEntries(directory), ","));
				list->SetContentLength(0);
				_sendMessageQPtr->EnQueue(*list);
				continue;
			}
		}
		// If it's a test request, queue it up in the
		// TestRequestQ, and send an acknowledgement back
		// to the client
		else if (message->GetType() == MessageType::TEST_REQUEST)
		{
			if (message->GetSource().GetUriHost() == "")
				break;
			logMessage = "Received a test request (" + message->GetMessageId() + ") from " + message->GetSource().AsString();
			Utilities::Logger::Log(logMessage.c_str());
			_testRequestQPtr->EnQueue(*message);
			Message *ack = BuildResponse(*message);
			ack->SetType(MessageType::TEST_ACKNOWLEDGE);
			ack->SetBody(message->GetMessageId());
			logMessage = "Sending acknowledgement of test request to " + message->GetSource().AsString();
			Utilities::Logger::Log(logMessage.c_str());
			_sendMessageQPtr->EnQueue(*ack);
			continue;
		}

		// If it's a worker signaling they're ready,
		// Queue them up in the ReadyWorkerQ
		else if (message->GetType() == MessageType::SIGNAL_READY)
		{
			logMessage = "Worker process on " + message->GetSource().AsString() + " is ready";
			Utilities::Logger::Log(logMessage.c_str());
			_readyWorkerQPtr->EnQueue(*message);
			continue;
		}
		// If it's a SHUTDOWN message from self, stop
		else if (message->GetType() == MessageType::SIGNAL_SHUTDOWN)
			break;

		else if (message->GetType() == MessageType::USER_AUTHENTICATE)
		{
			if (_context == Sockets::CONTEXT::SERVER)
			{
				std::vector<std::string> credentials = Utilities::Convert::StringToList(message->GetBody());
				std::string username, password;
				for (std::string kvp : credentials)
				{
					size_t index = kvp.find_first_of(':');
					if (index == kvp.length())
						continue;
					std::string key = kvp.substr(0, index), value = kvp.substr(index + 1, kvp.length() - index);
					if (key == "username")
						username = value;
					if (key == "password")
						password = value;
				}

				Utilities::User *user = Utilities::Credentials::Login("authentication.txt", username, password);
				Message *response = BuildResponse(*message);
				response->SetType(MessageType::USER_AUTHENTICATE);
				if (user != nullptr)
				{
					std::string body = "authenticated:1,fullname:" + user->Fullname;
					if (user->IsAdmin)
						body += ",isadmin:1";
					response->SetBody(body);
				}
				_sendMessageQPtr->EnQueue(*response);
				continue;
			}
		}
		else if (message->GetType() == MessageType::USER_REGISTER)
		{
			Utilities::User user;
			for (std::string kvp : Utilities::Convert::StringToList(message->GetBody()))
			{
				size_t index = kvp.find_first_of(':');
				if (index == kvp.length())
					continue;
				std::string key = kvp.substr(0, index), value = kvp.substr(index + 1, kvp.length() - index);
				if (key == "username")
					user.Username = value;
				else if (key == "password")
					user.Password = value;
				else if (key == "isadmin")
				{
					if (value == "0")
						user.IsAdmin = false;
					else
						user.IsAdmin = true;
				}
				else if (key == "fullname")
					user.Fullname = value;
			}

			Utilities::Credentials::RegisterUser("authentication.txt", user);
			continue;
		}
		if (_context != Sockets::CONTEXT::SERVER)
			_recvMessageQPtr->EnQueue(*message);
	}
}