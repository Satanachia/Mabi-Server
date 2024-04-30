#pragma once

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "ProtoBuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "ProtoBuf\\Release\\libprotobuf.lib")
#endif // _DEBUG

#include "CorePch.h"