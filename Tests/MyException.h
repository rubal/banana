#pragma once
#include <exception>
#include <String>
using namespace std;


class MyException: public exception
{
	string *errorDescription;
public:
	MyException(string * str)
	{
		this->errorDescription = str;
	}
	~MyException()
	{
		if(this->errorDescription != NULL)
			delete this->errorDescription;
	}
	virtual const char* what() const throw()
	{
		return errorDescription->c_str();
	}
};

