#include "stdafx.h"
#include "AddOffset.h"


ULONG AddOffset(ULONG nCurrent, ULONG nAdd)
{
	struct foobar
	{
		char    foo;
		long    bar;
	};
	ULONG	nAlign = offsetof(foobar, bar),
			nRet,
			nMod;

	nRet = nCurrent + nAdd;
	nMod = nRet % nAlign;

	if(nMod)
		nRet += nAlign - nMod;

	return nRet;
}
