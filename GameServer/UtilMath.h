#pragma once
#include "Protocol.pb.h"

class UtilMath
{
public:
	static const float PIE;
	static const float PIE2;
	static const float DegToRad;
	static const float RadToDeg;
	
	static bool IsZero(const Protocol::PosInfo& vec, bool calcZ = false);
	static Protocol::PosInfo Plus(const Protocol::PosInfo& a, const Protocol::PosInfo& b, bool calcZ = false);
	static Protocol::PosInfo Minus(const Protocol::PosInfo& a, const Protocol::PosInfo& b, bool calcZ = false);
	static Protocol::PosInfo Multi(const Protocol::PosInfo& a, float value, bool calcZ = false);
	static Protocol::PosInfo Divide(const Protocol::PosInfo& a, float value, bool calcZ = false);
	
	static float Length(Protocol::PosInfo vec, bool calcZ = false);
	static Protocol::PosInfo Normalize(const Protocol::PosInfo& vec, bool calcZ = false);

	static Protocol::PosInfo Lerp(float ratio, const Protocol::PosInfo& start, const Protocol::PosInfo& dest, bool calcZ = false);

	static float GetAngleRad(const Protocol::PosInfo& dir);
	static float GetAngleDegree(const Protocol::PosInfo& dir);
	
};

