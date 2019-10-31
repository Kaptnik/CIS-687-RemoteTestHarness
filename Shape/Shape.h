// Filename		: Shape.h
// Summary		: Defines an abstract class for a shape. 
// Description	: This is an abstract class for a shape. It's implemented by another class, 
//				  Hexagon, that is then used by TestDriver to test against. It's also referenced
//				  by the TestDriver so it has an idea of the interface that it's calling, without
//				  worrying about the implementation.


#pragma once

#ifndef SHAPE_H
#define SHAPE_H

class Shape
{
public:
	virtual ~Shape() {}
	virtual double Area() = 0;
	virtual double Perimeter() = 0;
	virtual int NumberOfSides() = 0;
	virtual void SetLengthOfSide(double) = 0;
};

#endif // !SHAPE_H

