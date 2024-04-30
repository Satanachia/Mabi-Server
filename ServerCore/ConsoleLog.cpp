#include "pch.h"
#include "ConsoleLog.h"

ConsoleLog::ConsoleLog()
{
	setlocale(LC_ALL, "ko_KR.UTF-8");
	_stdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
	_stdErr = ::GetStdHandle(STD_ERROR_HANDLE);
	_pushOnlyValue = true;
}

ConsoleLog::~ConsoleLog()
{

}



void ConsoleLog::WriteStdOut(Color color, const WCHAR* format, ...)
{
	if (nullptr == format)
		return;

	//color로 색상 변경
	SetColor(true, color);

	//가변인자를 이용한 출력
	va_list ap;
	va_start(ap, format);
	::vwprintf(format, ap);
	va_end(ap);
	
	fflush(stdout);

	//기존 색상으로 복귀
	SetColor(true, Color::WHITE);
}

void ConsoleLog::WriteStdErr(Color color, const WCHAR* format, ...)
{
	WCHAR buffer[BUFFER_SIZE];

	if (format == nullptr)
		return;

	SetColor(false, color);

	va_list ap;
	va_start(ap, format);
	::vswprintf_s(buffer, BUFFER_SIZE, format, ap);
	va_end(ap);

	::fwprintf_s(stderr, buffer);
	fflush(stderr);

	SetColor(false, Color::WHITE);
}

void ConsoleLog::SetColor(bool stdOut, Color color)
{
	static WORD SColors[]
	{
		0,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		FOREGROUND_RED | FOREGROUND_INTENSITY,
		FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
	};

	::SetConsoleTextAttribute(stdOut ? _stdOut : _stdErr, SColors[static_cast<int32>(color)]);
}