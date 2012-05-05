#pragma once
#include <stdio.h>
#include "IUnitTest.h"
#include "MyException.h"


class MathTest:public IUnitTest
{
public:
	virtual bool startTesting(); 
private:
	static void PlusTest() throw(MyException);
	static void MinusTest() throw(MyException);

};
