
#pragma once

class CCurrency
{
private:
	CY	m_cy;

public:
	CCurrency(const CY& cy) : m_cy(cy) {}
	CCurrency(const CCurrency& cy) : m_cy(cy) {}

	CCurrency& operator=(const CY& cy);
	CCurrency& operator=(const CCurrency& cy);

	bool operator==(const CCurrency& cy) const	{ return m_cy.int64 == cy.m_cy.int64;	}
	bool operator!=(const CCurrency& cy) const	{ return m_cy.int64 != cy.m_cy.int64;	}
	bool operator< (const CCurrency& cy) const	{ return m_cy.int64 > cy.m_cy.int64;	}
	bool operator> (const CCurrency& cy) const	{ return m_cy.int64 < cy.m_cy.int64;	}
	bool operator<=(const CCurrency& cy) const	{ return m_cy.int64 <= cy.m_cy.int64;	}
	bool operator>=(const CCurrency& cy) const	{ return m_cy.int64 >= cy.m_cy.int64;	}
	
	bool operator! () const { return !m_cy.int64; }

	CCurrency	operator+(const CCurrency& cy) const;
	CCurrency	operator-(const CCurrency& cy) const;
	CCurrency	operator-() const;
	CCurrency	operator*(int nMultiplier) const;
	CCurrency	operator/(int nDivider) const;

	const CCurrency&	operator+=(const CCurrency& cy)	{ m_cy.int64 += cy.m_cy.int64; return *this;	}
	const CCurrency&	operator-=(const CCurrency& cy)	{ m_cy.int64 -= cy.m_cy.int64; return *this;	}
	const CCurrency&	operator*=(int nMultiplier)		{ m_cy.int64 *= nMultiplier; return *this;		}
	const CCurrency&	operator/=(int nDivider)		{ m_cy.int64 /= nDivider; return *this;			}

	operator CY() const			{ return m_cy; }
	operator double() const		{ return ((double)m_cy.int64) / 10000;	}
	operator float() const		{ return ((float)m_cy.int64) / 10000;	}

	enum NumberFormat
	{
		None,
		Number,
		Currency
	};

	CString Format(NumberFormat fmt = None, LCID lcid = LOCALE_SYSTEM_DEFAULT);
};


inline CCurrency& CCurrency::operator=(const CY &cy)
{
	m_cy = cy;
	return *this;
}


inline CCurrency& CCurrency::operator=(const CCurrency &cy)
{
	if(this != &cy)
		m_cy = cy.m_cy;
	return *this;
}


inline CCurrency CCurrency::operator +(const CCurrency& cy) const
{
	CY	val;

	val.int64 = m_cy.int64 + cy.m_cy.int64;

	return val;
}


inline CCurrency CCurrency::operator -(const CCurrency& cy) const
{
	CY	val;

	val.int64 = m_cy.int64 - cy.m_cy.int64;

	return val;
}


inline CCurrency CCurrency::operator *(int nMultiplier) const
{
	CY	val;

	val.int64 = m_cy.int64 * nMultiplier;

	return val;
}


inline CCurrency CCurrency::operator /(int nDivider) const
{
	CY	val;

	val.int64 = m_cy.int64 / nDivider;

	return val;
}


// CCurrency::operator-
//
//		Unary opertor -
//
inline CCurrency CCurrency::operator-() const
{
	CY	val;

	val.int64 = -m_cy.int64;

	return val;
}