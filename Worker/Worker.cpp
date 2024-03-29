////////////////////////////////////////////////////////////////////////////
// Worker.cpp - Defines the entry point for the worker application.		  //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* This program is spawned by a TestHarness server. On startup, it reguisters
* with the server, and waits for EXECUTE requests. It parses the XML, serializes it to a 
* list of TestObject objects, and for each of the objects, Loads the 
* required DLLs, and calls the Test() function
* It logs the results to screen, as well as returning it back to the server
*/

#include "stdafx.h"
#include "..\Utilities\converter.h"
#include "TestObject.h"
#include "..\Test\Test.h"
#include "..\Socket\Endpoint.h"
#include "..\Socket\Communication.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <objbase.h>
#include <sstream>


using namespace std;
using namespace Sockets;

typedef ITest*(__stdcall* TestDriverInstance)();
const size_t MIN_PORT_VALUE = 1;
const size_t MAX_PORT_VALUE = 65536;
const size_t BUFFER_SIZE = 1048576;
static size_t PORT_NUMBER = 0;
static bool IS_REGISTERED = false;

// Strip the path and the extension from a xmlString.
// So if you pass C:\foo\bar.txt, you'd just get back
// "bar"

string GetFilenameWithoutExtension(string filename)
{
	size_t index = filename.find_last_of("\\/");
	if (index != string::npos)
	{
		filename.erase(0, index + 1);
	}

	index = filename.rfind('.');
	if (index != string::npos)
	{
		filename.erase(index);
	}

	return filename;
}

// Prints the test result.
// Prints the current timestamp
// Then prints the TestDriver & TestedCodes used to execute the test
// followed by the status of the test
string GetTestResult(TestObject testObj, bool result)
{
	ostringstream oss;
	auto now = chrono::system_clock::now();
	auto now_c = chrono::system_clock::to_time_t(now);
	oss << "[" << std::put_time(std::localtime(&now_c), "%c") << "] ";

	oss << "Test using driver " << testObj.GetTestDriver() << " and tested codes ( ";
	for (auto const& iterator : testObj.GetTestedCodes())
		oss << iterator << " ";
	oss << ") was " << (result ? "successful" : "unsuccessful");

	return oss.str();
}

// Executes a single test object
// This is where the magic happens, so to speak.
// The test driver is loaded, and an object of TestDriver
// is created using the Factory "GetTestInstance" method.
// It then uses the object to load the "TestedCodes" libraries.
// Once done, it invokes the Test() function, and passes the baton
// on to GetTestResult()
string ProcessTestObject(TestObject testObj)
{
	HINSTANCE testInstance = LoadLibrary(("REPOSITORY" + testObj.GetTestDriver()).c_str());
	if (testInstance == nullptr)
	{
		string message = "Failure loading library " + testObj.GetTestDriver();
		cout << message << endl;
		return message;
	}

	TestDriverInstance tdInstance = (TestDriverInstance)GetProcAddress(testInstance, "GetTestInstance");
	if (tdInstance == nullptr)
	{
		string message = "Failure running GetProcAddress on function GetTestInstance";
		cout << message << endl;
		return message;
	}

	ITest *test = tdInstance();
	for (auto const& iterator : testObj.GetTestedCodes())
		test->LoadDLL(GetFilenameWithoutExtension(iterator).c_str(), ("REPOSITORY" + iterator).c_str());

	string result = GetTestResult(testObj, test->Test());
	cout << result << endl;
	return result;
}


// Send a READY message to the server
void RegisterWithServer(Communication& communication)
{
	Message* message = new Message();
	message->SetSource(*(new EndPoint("localhost", PORT_NUMBER)));
	message->SetDestination(*(new EndPoint("localhost:9000")));
	message->SetType(MessageType::SIGNAL_READY);
	communication.DeliverMessage(*message);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		cout << "Usage " << argv[0] << " <port>" << endl;
		cout << "\t<port>\tSpecifies the port number to listen on." << endl;
		return 1;
	}	
	size_t port = Utilities::Convert::FromString<size_t>(argv[1]);
	PORT_NUMBER = port;
	cout << "Binding on port " << argv[1] << endl;

	if (port < MIN_PORT_VALUE || port > MAX_PORT_VALUE)
	{
		cout << "Invalid port value " << argv[1] << ". Choose between " << MIN_PORT_VALUE << " and " << MAX_PORT_VALUE << "." << endl;
		return 1;
	}

	// Prof. Fawcett's SocketSystem initialized the windows socket libraries and frees them
	// When socket system goes out of scope.
	SocketSystem socketSystem;
	Communication comm(port);
	comm.ProcessContext = Sockets::CONTEXT::WORKER;
	comm.Start();

	// While the communication object takes care of
	// incoming requests, just while around,
	// forcing re-registration with server after a test request
	// has been executed
	while (true)
	{
		if (!IS_REGISTERED)
		{
			cout << "Registering with server" << endl;
			RegisterWithServer(comm);
			IS_REGISTERED = true;

			Message message = comm.CollectMessage();
			try
			{		
				if (message.GetType() == MessageType::TEST_EXECUTE)
				{
					std::string requestFile = "REPOSITORY/TESTREQUESTS/TestRequest_" + message.GetMessageId() + ".xml";
					std::string xmlString = message.GetBody();
					xmlString.erase(std::remove(xmlString.begin(), xmlString.end(), '\n'), xmlString.end());
					xmlString.erase(std::remove(xmlString.begin(), xmlString.end(), '\r'), xmlString.end());
					list<TestObject> testObjects = TestObject::DeserializeXmlString(xmlString, requestFile);
					int count = 0;
					for (auto const& iterator : testObjects) 
					{
						std::string result = ProcessTestObject(iterator);
						std::string resultFile = "REPOSITORY/TESTRESULTS/TestResults_" + message.GetMessageId() + ".txt";
						std::ofstream outFile;
						outFile.open(resultFile, ios::out);
						outFile << result;
						outFile.close();
					}
				}
			}
			catch (std::exception e)
			{
				cout << "Caught exception while running test\n" << e.what() << endl;
				cout << "Incoming request\n" << message.ToString() << endl;
			}
			IS_REGISTERED = false;
		}
		Sleep(5000);
	}
    return 0;
}