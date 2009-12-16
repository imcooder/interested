#ifndef COMMONS_H
#define COMMONS_H

#pragma once

template <class T>
class CNotificationInfoEqualHelper
{
public:
	static bool IsEqual(const T& t1, const T& t2)
	{
		return false;
	}
};

#endif // COMMONS_H