#include <stdio.h>
#include "MathTest.h"
#include "MyException.h"


bool MathTest::startTesting()
{
	try
	{
		PlusTest();
	}
	catch(MyException myEx)
	{
		const char* ch = myEx.what();
	}
	return true;
}

void MathTest::PlusTest()
{
	if((2+2) != 4)
	{
		MyException myEx(new std::string("PlusTestFailed"));
		throw myEx;
	}
}