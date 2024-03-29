////////////////////////////////////////////////////////////////////////////
// Server.cpp - A server program that handles client requests and workers //
// as part of the test harness											  //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\Socket\Communication.h"
#include "..\Utilities\Converter.h"
#include "..\Utilities\Logger.h"

#include <windows.h>

using namespace Sockets;

const byte NUMBER_OF_WORKERS = 2;

// Spawns a new process, creating a worker that listens on specified port number
PROCESS_INFORMATION* CreateWorker(std::string location, size_t port)
{
	STARTUPINFO si;
	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION();
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(pi, sizeof(*pi));

	if (!CreateProcess(
		NULL,		
		(LPSTR)(location + " " + Utilities::Convert::ToString<size_t>(port)).c_str(),
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		pi
	))
	{
		return nullptr;
	}
	return pi;
}

int main(int argc, char** argv)
{
	// Initialize the socket system 
	SocketSystem ss;

	// Setup a string for logging messages
	std::string logMessage;

	// Listen on port 9000
	size_t port = 9000;
	if (argc == 2)
		port = Utilities::Convert::FromString<size_t>(argv[1]);

	logMessage = "Binding to port " + Utilities::Convert::ToString<size_t>(port);
	Utilities::Logger::Log(logMessage.c_str());

	// Setup two way communication
	Communication communication(port);
	BlockingQueue<Message> *readyWorkers = new BlockingQueue<Message>();
	BlockingQueue<Message> *testRequests = communication.GetReceiverQueue();

	// Setup the incoming message handler
	Utilities::Logger::Log("Setting up BlockingQueues to handle incoming requests");
	communication.SetTestRequestQueue(testRequests);
	communication.SetReadyWorkerQueue(readyWorkers);
		
	// Start all communication
	communication.Start();

	// Spawn several worker processes
	logMessage = "Spawning " + Utilities::Convert::ToString<int>(NUMBER_OF_WORKERS) + " Worker.exe processes to handle test requests";
	Utilities::Logger::Log(logMessage.c_str());
	PROCESS_INFORMATION* processes[NUMBER_OF_WORKERS];

	for (int index = 0; index < NUMBER_OF_WORKERS; ++index)
	{
		processes[index] = CreateWorker("Worker.exe", port + (index + 1)*100);
	}

	while (true)
	{
		// Wait for a test request, and then wait for a ready worker,
		// and pass on the test reqeust to the worker.
		Message requestMessage = testRequests->DeQueue();
		Message testServer = readyWorkers->DeQueue();		
		requestMessage.SetType(MessageType::TEST_EXECUTE);
		requestMessage.SetDestination(testServer.GetSource());
		requestMessage.SetSource(EndPoint("localhost:9000"));
		logMessage = "Sending request (" + requestMessage.GetMessageId() + ") to worker '" + testServer.GetSource().AsString() + "'.";
		Utilities::Logger::Log(logMessage.c_str());
		communication.DeliverMessage(requestMessage);
	}

    return 0;
}

