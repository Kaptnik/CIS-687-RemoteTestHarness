////////////////////////////////////////////////////////////////////////////
// Test.cpp - An implementation of the ITest abstract class				  //
// ver 1.0																  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018		  //
////////////////////////////////////////////////////////////////////////////

/*
* Apart from implementing the methods of the ITest class,
* it also exposes a Factory method for creating an instance of itself.
* This factory method is called GetInstance, and it's what the TestDriver
* expects when it loads the DLL.
* The output of the Test() function depends on both the TestedCodes, Vehicle & Shape,
* being available. It creates an instance of both vehicle and shape once the DLLs are loaded,
* and uses those objects to run some rudimentary tests.
* The DLLs are freed when the TestDriver object goes out of scope (in its destructor).
*/

#include "stdafx.h"
#include "..\Vehicle\Vehicle.h"
#include "..\Shape\Shape.h"
#include "string.h"
#include <assert.h>
#include <exception>
#include <list>

using namespace std;

typedef Vehicle*(CALLBACK *VehicleInstance)();
typedef Shape*(CALLBACK *ShapeInstance)();


class __declspec(dllexport) TestDriver : ITest
{
public:
	virtual ~TestDriver();
	bool Test();
	void LoadDLL(const char*, const char*);
private:
	FARPROC GetInstanceFromDLL(const char*);
	Vehicle * _bicycle;
	Shape * _shape;
	list<HMODULE> _modules;
};

extern "C" __declspec(dllexport) TestDriver* GetTestInstance() { return new TestDriver(); }

// Free up any DLLs loaded when the object goes out of scope
TestDriver::~TestDriver()
{
	for (auto const& iterator : _modules)
		FreeModule(iterator);
}

// Load any DLLs the test driver depends on.
// This instance depends on Vehicle.dll & Shape.dll
void TestDriver::LoadDLL(const char* name, const char* path)
{
	if (strcmp(name,"Vehicle") == 0)
	{
		VehicleInstance vInstance = (VehicleInstance)(GetInstanceFromDLL((path)));
		if (vInstance == nullptr)
			throw new std::exception("Error initializing Vehicle Instance. Couldn't find GetInstance method");
		_bicycle = vInstance();
	}

	if (strcmp(name, "Shape") ==0)
	{
		ShapeInstance sInstance = (ShapeInstance)(GetInstanceFromDLL((path)));
		if (sInstance == nullptr)
			throw new std::exception("Error initializing Shape Instance. Couldn't find GetInstance method");

		_shape = sInstance();
	}
}

// Once the DLLs are loaded, it creates an object of each class.
// And uses them to run some tests that should always pass.
bool TestDriver::Test()
{
	if (_bicycle == nullptr || _shape == nullptr)
		throw new std::exception("Test() called without loading the DLLs required");

	if (_bicycle->ModeOfSteering().compare("Handlebar") == 0 && !(_bicycle->NumberOfWheels() == 2))
		return false;
	if (_bicycle->PowerSource().compare("Rider") == 0 && _bicycle->LicenseRequiredToDrive())
		return false;

	_shape->SetLengthOfSide(6);

	if (_shape->Perimeter() > 6 * _shape->NumberOfSides())
		return false;

	if (_shape->Area() < 54)
		return false;

	return true;
}

// Given a DLL path, it loads it into memoryand returns a 
// function pointer to the Factory method "GetInstance".
// It's a private method used by the LoadDLL function.
FARPROC TestDriver::GetInstanceFromDLL(const char* dllPath)
{
	HMODULE module = LoadLibrary(dllPath);
	if (module != nullptr)
	{
		_modules.push_back(module);
		return GetProcAddress(module, "GetInstance");
	}
	return nullptr;
}