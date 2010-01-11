///////////////////////////////////////////////////////////////////////////////
//
// TESTRESULT.H
// 
// A TestResult is a collection of the history of some test runs.  Right now
// it just collects failures.
// 
///////////////////////////////////////////////////////////////////////////////



#ifndef TESTRESULT_H
#define TESTRESULT_H
#include <stdio.h>

class Failure;

class TestResult
{
public:
					TestResult (char* _fileName = 0); 
	virtual void	testsStarted ();
	virtual void	addFailure (const Failure& failure);
	virtual void	testsEnded ();

private:
	int				failureCount;
	char*			fileName;
	FILE*			fileStream;
};

#endif
