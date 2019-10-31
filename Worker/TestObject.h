
////////////////////////////////////////////////////////////////////////////////////////////////////
// TestObject.h - Class that'll hold the deserialized XML for each test within the test request.  //
// ver 1.0																						  //
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
*
* Since the test request contains a TestDriver and a series of TestedCodes, this file declares
* the class that can hold that data, and uses the boost::serialization library to deserialize
* the XML into an object instance.
* It also provides a static method to deserialize XML into a list of test objects.
*
* Most of the code on this file was inspired by the boost documentation at
* https://www.boost.org/doc/libs/1_67_0/libs/serialization/doc/serialization.html
*/

#ifndef TESTOBJECT_H
#define TESTOBJECT_H

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <iostream>
#include <fstream>

class TestObject
{
	friend class boost::serialization::access;
	std::string TestDriver;
	std::list<std::string> TestedCodes;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		ar	& BOOST_SERIALIZATION_NVP(TestDriver)
			& BOOST_SERIALIZATION_NVP(TestedCodes);
	}

public:
	TestObject() {};
	TestObject(std::string driver, std::list<std::string> codes) :
		TestDriver(driver), TestedCodes(codes) {};

	static std::list<TestObject> DeserializeXml(const char* pathToXml)
	{
		std::ifstream inputFile(pathToXml);
		assert(inputFile.good());
		boost::archive::xml_iarchive archive(inputFile, boost::archive::no_header);
		std::list<TestObject> testObjects;
		archive >> BOOST_SERIALIZATION_NVP(testObjects);

		return testObjects;
	}

	static std::list<TestObject> DeserializeXmlString(std::string xmlString, std::string requestPath)
	{
		std::ofstream outFile;
		outFile.open(requestPath, std::ios::out);
		outFile << xmlString;
		outFile.close();
		return DeserializeXml(requestPath.c_str());
	}

	std::string GetTestDriver()
	{
		return TestDriver;
	}

	std::list<std::string> GetTestedCodes()
	{
		return TestedCodes;
	}
};

BOOST_CLASS_IMPLEMENTATION(TestObject, boost::serialization::object_serializable);
BOOST_CLASS_IMPLEMENTATION(std::list<std::string>, boost::serialization::object_serializable);
BOOST_CLASS_IMPLEMENTATION(std::list<TestObject>, boost::serialization::object_serializable);

BOOST_CLASS_TRACKING(TestObject, boost::serialization::track_never);
BOOST_CLASS_TRACKING(std::list<std::string>, boost::serialization::track_never);
BOOST_CLASS_TRACKING(std::list<TestObject>, boost::serialization::track_never);

#endif // !TESTOBJECT_H