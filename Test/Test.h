// Filename		: Test.h
// Summary		: Defines an abstract class for a Test. 
// Description	: This is an abstract class, ITest, that contains the method that returns a bool value,
//				  and takes no arguments (as listed in the project requirements).
//				  I've added an additional method called LoadDLL, which will load the DLLs that each
//				  TestDriver is supposed to test. Since the list of DLLs is passed to the Executive package
//				  on startup, there is no way I can load a TestDriver and its dependencies in one go.


#pragma once

#ifndef TEST_H
#define TEST_H

class ITest
{
public:
	virtual ~ITest() {}
	virtual bool Test() = 0;
	virtual void LoadDLL(const char*, const char*) = 0;
};

#endif // !TEST_H

