//////////////////////////////////////////////////////////////////////
// Hexagon.cpp - An implementation of the Shape abstract class		//
// ver 1.0															//
// Karthik Umashankar, CSE687 - Object Oriented Design, Summer 2018 //
//////////////////////////////////////////////////////////////////////

/*
* Package Operations:
* -------------------
* Apart from implementing the methods of the Shape class,
* it also exposes a Factory method for creating an instance of itself.
* This factory method is called GetInstance, and it's what the TestDriver
* expects when it loads the DLL.
*
* Required Files:
* ---------------
* Shape.h
*/


#include "stdafx.h"
#include "Shape.h"
#include <cmath>

class __declspec(dllexport) RegularHexagon : public Shape
{
public:
	virtual ~RegularHexagon() {}
	double Area() {
		return 3 * std::sqrt(3) * pow(_lengthOfSide,2) / 2;
	}
	double Perimeter() {
		return _lengthOfSide * NumberOfSides();
	}
	int NumberOfSides() {
		return 6;
	}
	void SetLengthOfSide(double length)
	{
		_lengthOfSide = length;
	}
private:
	double _lengthOfSide = 0;
};

extern "C" __declspec(dllexport) Shape* GetInstance() { return new RegularHexagon(); }