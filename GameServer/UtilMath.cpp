#include "pch.h"
#include "UtilMath.h"

const float UtilMath::PIE = 3.141592653589793238462643383279502884197169399375105820974944f;
const float UtilMath::PIE2 = PIE * 2.0f;
const float UtilMath::DegToRad = UtilMath::PIE / 180;
const float UtilMath::RadToDeg = 180 / UtilMath::PIE;

bool UtilMath::IsZero(const Protocol::PosInfo& vec, bool calcZ /*= false*/)
{
	bool result = true;
	result &= (0.f == vec.x());
	result &= (0.f == vec.y());
	if(true == calcZ)
		result &= (0.f == vec.z());
	return result;
}

Protocol::PosInfo UtilMath::Plus(const Protocol::PosInfo& a, const Protocol::PosInfo& b, bool calcZ /*= false*/)
{
	Protocol::PosInfo result;
	result.set_x(a.x() + b.x());
	result.set_y(a.y() + b.y());
	float z = (true == calcZ) ? (a.z() + b.z()) : 0.f;
	result.set_z(z);
	return result;
}

Protocol::PosInfo UtilMath::Minus(const Protocol::PosInfo& a, const Protocol::PosInfo& b, bool calcZ /*= false*/)
{
	Protocol::PosInfo result;
	result.set_x(a.x() - b.x());
	result.set_y(a.y() - b.y());
	float z = (true == calcZ) ? (a.z() - b.z()) : 0.f;
	result.set_z(z);
	return result;
}

Protocol::PosInfo UtilMath::Multi(const Protocol::PosInfo& a, float value, bool calcZ /*= false*/)
{
	Protocol::PosInfo result;
	result.set_x(a.x() * value);
	result.set_y(a.y() * value);
	float z = (true == calcZ) ? (a.z() * value) : 0.f;
	result.set_z(z);
	return result;
}

Protocol::PosInfo UtilMath::Divide(const Protocol::PosInfo& a, float value, bool calcZ /*= false*/)
{
	ASSERT_CRASH(0.f != value);
	
	Protocol::PosInfo result;
	result.set_x(a.x() / value);
	result.set_y(a.y() / value);
	float z = (true == calcZ) ? (a.z() / value) : 0.f;
	result.set_z(z);
	return result;
}

float UtilMath::Length(Protocol::PosInfo vec, bool calcZ /*= false*/)
{
	if (false == calcZ)
	{
		vec.set_z(0.f);
	}
	
	return std::sqrtf((vec.x() * vec.x()) + (vec.y() * vec.y()) + (vec.z() * vec.z()));
}

Protocol::PosInfo UtilMath::Normalize(const Protocol::PosInfo& vec, bool calcZ /*= false*/)
{
	float len = UtilMath::Length(vec, calcZ);
	if (0 == len)
		return Protocol::PosInfo();
	
	Protocol::PosInfo result = UtilMath::Divide(vec, len, calcZ);
	return result;
}

Protocol::PosInfo UtilMath::Lerp(float ratio, const Protocol::PosInfo& start, const Protocol::PosInfo& dest, bool calcZ /*= false*/)
{
	Protocol::PosInfo result;
	ratio = std::clamp(ratio, 0.f, 1.f);
	result.set_x(start.x() * ratio + dest.x() * (1.f - ratio));
	result.set_y(start.y() * ratio + dest.y() * (1.f - ratio));
	float z = (true == calcZ) ? (start.z() * ratio + dest.z() * (1.f - ratio)) : 0.f;
	result.set_z(z);
	return result;
}

float UtilMath::GetAngleRad(const Protocol::PosInfo& dir)
{
	Protocol::PosInfo temp = UtilMath::Normalize(dir);
	temp.set_y(-temp.y());
	float result = std::acosf(temp.x());
	
	if (0.f < temp.y())
	{
		result = UtilMath::PIE2 - result;
	}
	
	return result;
}

float UtilMath::GetAngleDegree(const Protocol::PosInfo& dir)
{
	return GetAngleRad(dir) * RadToDeg;
}

