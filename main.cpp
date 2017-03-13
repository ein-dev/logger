//*********************************************************************************
// Definition and tuning of the application loggers.
// In real projects this code must be moved in separate header.
//*********************************************************************************
#include "./my_logger.h"

int main ()
{
	std::string s = "Test msg";
	std::wstring ws = L"Test msg";

	MLOG::trace() << "MLOG " << s << MLOG::CV::HEX << 777 << " " << MLOG::CV::DEC <<= 888;
	ALOG::trace() << "ALOG " << s << ALOG::CV::HEX << 777 << " " << ALOG::CV::DEC <<= 888;
	WLOG::trace() << L"WLOG " << ws << WLOG::CV::HEX << 777 << L" " << WLOG::CV::DEC <<= 888;
	WLOG::debug() << L"WLOG " << ws << WLOG::CV::HEX << 777 << L" " << WLOG::CV::DEC <<= 888;

	return 0;
}
