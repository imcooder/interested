
#include "TestResult.h"
#include "Failure.h"

#include <stdio.h>


TestResult::TestResult (char* _fileName)
	: failureCount (0),
	fileName(_fileName)
{
}


void TestResult::testsStarted () 
{
	if(fileName != NULL)
	{
		fileStream = fopen(fileName, "w");
	}
	else
	{
		fileStream = stdout;
	}
}


void TestResult::addFailure (const Failure& failure) 
{
	fprintf (fileStream, "%s%s%s%s%ld%s%s\n",
		"Failure: \"",
		failure.message.asCharString (),
		"\" " ,
		"line ",
		failure.lineNumber,
		" in ",
		failure.fileName.asCharString ());
		
	failureCount++;
}


void TestResult::testsEnded () 
{
	if (failureCount > 0)
		fprintf (fileStream, "There were %ld failures\n", failureCount);
	else
		fprintf (fileStream, "There were no test failures\n");

	if(fileName != NULL && fileStream != NULL)
	{
		fflush(fileStream);
		fclose(fileStream);
		fileStream = NULL;
	}
}
