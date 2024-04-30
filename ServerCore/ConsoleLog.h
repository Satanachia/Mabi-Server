#pragma once
#include "JobQueue.h"

/*----------------------------------------
							ConsoleLog
----------------------------------------*/

enum class Color
{
	BLACK,
	WHITE,
	RED,
	GREEN,
	BLUE,
	YELLOW,
};

class ConsoleLog : public JobQueue
{
	enum { BUFFER_SIZE = 4096 };

public:
	ConsoleLog();
	~ConsoleLog();

	ConsoleLog(const ConsoleLog& _Other) = delete;
	ConsoleLog(ConsoleLog&& _Other) noexcept = delete;
	ConsoleLog& operator=(const ConsoleLog& _Other) = delete;
	ConsoleLog& operator=(const ConsoleLog&& _Other) noexcept = delete;

public:
	template <typename... Args>
	void ReserveMsg(Color color, const WCHAR* str, Args... args)
	{
#if _DEBUG
		std::shared_ptr<ConsoleLog> thisSPtr = std::static_pointer_cast<ConsoleLog>(shared_from_this());
		DoASync([thisSPtr, color, str, args...]()
		{
			thisSPtr->WriteStdOut(color, str, args...);
		});
#endif 
	}

public:
	//메세지 출력
	void		WriteStdOut(Color color, const WCHAR* str, ...);
	//에러 메세지 출력
	void		WriteStdErr(Color color, const WCHAR* str, ...);
	
private:
	void		SetColor(bool stdOut, Color color);

private:
	HANDLE		_stdOut;
	HANDLE		_stdErr;
};

