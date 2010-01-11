
#include "stdafx.h"
#include "Currency.h"


CString CCurrency::Format(NumberFormat fmt, LCID lcid)
{
	TCHAR		szNum[32];					// Must hold between 922337203685477.5807 and -922337203685477.5808
	TCHAR*		psz		= szNum + 31;		// Set psz to last char
	__int64		ii		= m_cy.int64 < 0 ? -m_cy.int64 : m_cy.int64;
	int			nDec	= 4;				// Four decimals max

	*psz = 0;								// Set terminating null
	do
	{
		--psz;								// Move back
		if(nDec-- == 0)
			*psz = _T('.');					// Decimal point
		else
		{
			unsigned lsd = (unsigned)(ii % 10);		// Get least significant
													// digit
			ii /= 10;								// Prepare for next most
													// significant digit
			*psz = _T('0') + lsd;					// Place the digit
		}
	} while(ii != 0);

	// Check if we need to add trailing zeroes
	if(nDec >= 0)
	{
		while(nDec-- > 0)
			*--psz = _T('0');
		*--psz = _T('.');
	}

	if(m_cy.int64 < 0)
		*--psz = _T('-');

	if(fmt == Number)
	{
		TCHAR	szFmt[32];

		GetNumberFormat(LOCALE_USER_DEFAULT, NULL, psz, NULL, szFmt, 32);
		return CString(szFmt);
	}
	
	if(fmt == Currency)
	{
		TCHAR	szFmt[32];

		GetCurrencyFormat(LOCALE_USER_DEFAULT, NULL, psz, NULL, szFmt, 32);
		return CString(szFmt);
	}

	return CString(psz);
}
