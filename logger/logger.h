/****************************************************************************
**
** Copyright (C) 2017 Dmitry Kuznetsov.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <memory>
#include <mutex>
#include <atomic>
#include <ctime>

#if __cplusplus < 201103L
	#error "Your compiler must support c++11 features."
#endif

namespace Logger {

// Logger message priorities.
enum class Level { TRACE = 0, DEBUG, INFO, WARN, ERROR, FATAL };

// May be useful for template tricks.
class NullType {};

// String constant definitions and tools to extract them.
// It supports a few character data types.
struct DefStr {
	// String constant descriptor.
	// It contains a few "const T*" field (a field for each supported character data type).
	// (where T must be correct template parameter of std::basic_string<T>).
	struct Descriptor {
		const char* char_str;
		const wchar_t* wchar_str;
	};

	// Returns corresponding to TChar field of Descriptor.
	// It's common version therefore get-method is empty.
	// Below you can find the specialization for each supported characher data type.
	template <typename TChar>
	struct Extractor {
		static constexpr const TChar* get(const DefStr::Descriptor* item) {return nullptr;}
	};

	// Parser of compound string constant.
	// Its parse-method returns nth (n) component of TChar string constant (s).
	// It's very simple parser. Compomets of the input string (s) must have equal length (Len) 
	// and be separeted by one space character.
	template <typename TChar, int Len>
	struct Parser {
		static std::basic_string<TChar> parse(const TChar* s, const int n)
		{
			auto all = std::basic_string<TChar>(s);
			int pos = n * (Len + 1);
			return (pos < all.length() ? all.substr(pos, Len) : std::basic_string<TChar>());
		}
	};

	//Definitions of simple string constants.
	static constexpr Descriptor empty = {"", L""};
	static constexpr Descriptor space = {" ", L" "};
	static constexpr Descriptor colon = {":", L":"};
	static constexpr Descriptor under = {"_", L"_"};
	static constexpr Descriptor zero = {"0", L"0"};
	static constexpr Descriptor openSqBracket = {"[", L"["};
	static constexpr Descriptor closeSqBracket = {"]", L"]"};
	static constexpr Descriptor lrArrow = {"=>", L"=>"};
	static constexpr Descriptor rlArrow = {"<=", L"<="};

	//Definitions of compound string constants.
	static constexpr Descriptor levels = {
		"TRACE DEBUG INFO  WARN  ERROR FATAL",
		L"TRACE DEBUG INFO  WARN  ERROR FATAL"};
	static constexpr Descriptor months = {
		"Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec",
		L"Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec"};
};
//Specialization for char data type
template <>
struct DefStr::Extractor<char> {
	static constexpr const char* get(const DefStr::Descriptor* item) {return item->char_str;}
};

//Specialization for wchar_t data type
template <>
struct DefStr::Extractor<wchar_t> {
	static constexpr const wchar_t* get(const DefStr::Descriptor* item) {return item->wchar_str;}
};

//Wrapper for convenient using of Extractor.
template <typename TChar>
constexpr const TChar* str(const DefStr::Descriptor& item) {
	return DefStr::Extractor<TChar>::get(&item);
}
//Converts Level ID to string
template <typename TChar>
std::basic_string<TChar> strLevel(const Level id)
{
	constexpr auto all = str<TChar>(DefStr::levels);
	std::basic_string<TChar> result = DefStr::Parser<TChar, 5>::parse(all, static_cast<int>(id));
	return result;
}
//Converts month number (1..12) to string
template <typename TChar>
std::basic_string<TChar> strMonth(const int n)
{
	constexpr auto all = str<TChar>(DefStr::months);
	std::basic_string<TChar> result = DefStr::Parser<TChar, 3>::parse(all, n);
	return result;
}

//Provides numeric and string presentation of a date and time.
template <typename TChar = char>
class DateTime {
public:
	//Constructor. 
	// deltaUTC - diffrence from UTC time zone (in hours)
	// t - initial value (number of seconds since 00:00:00 1 Jan 1970 UTC).
	// Zero value makes use the current system time.
	DateTime(int deltaUTC = 0, const std::time_t t = 0) 
	: m_time(t)
	{
		if (t == 0) {
			std::time(&m_time);
		}
		m_time += deltaUTC * SEC_IN_HOUR;
	}

	//Returns time in numeric presentation.
	//hour - 0..23, minute - 0..59, seconds - 0..59
	void time(int& hour, int& minute, int& second)
	{
		int secInCurrentDay = m_time % SEC_IN_DAY;
		int secInCurrentHour = secInCurrentDay % SEC_IN_HOUR;
		hour = secInCurrentDay / SEC_IN_HOUR;
		minute = (secInCurrentHour / SEC_IN_MINUTE);
		second = (secInCurrentHour % SEC_IN_MINUTE);
	}

	//Returns date in numeric presentation
	// year - 1970+, month - 1,,12, day - 1..31
	void date(int& year, int& month, int& day)
	{	
		int totalDays = m_time / SEC_IN_DAY;
		int overDays = 0;
		calcYear(totalDays, year, overDays);
		calcMonth(overDays, isLeapYear(year), month, overDays);
		day = overDays + 1;
	}
	
	//Returns time in string presentation (for example: 23:15:07) 
	std::basic_string<TChar> strTime()
	{
		int h = 0, m = 0, s = 0;
		time(h, m, s);
		constexpr auto zero = str<TChar>(DefStr::zero)[0];
		constexpr auto colon = str<TChar>(DefStr::colon);

		std::basic_stringstream<TChar> ss;
		ss << std::setw(2) << std::setfill(zero) << h << colon;
		ss << std::setw(2) << std::setfill(zero) << m << colon;
		ss << std::setw(2) << std::setfill(zero) << s;
		return ss.str();
	}

	// Returns date in string presentation (for example: 2016 Sep 15)
	// If noSpace = true character '_' be used instead ' '.
	std::basic_string<TChar> strDate(const bool noSpace = false)
	{
		int y = 0, m = 0, d = 0;
		date(y, m, d);

		constexpr auto zero = str<TChar>(DefStr::zero)[0];
		constexpr auto space = str<TChar>(DefStr::space);
		constexpr auto under =  str<TChar>(DefStr::under);
		std::basic_string<TChar> strSpace = noSpace ? under : space;

		std::basic_stringstream<TChar> ss;
		ss << y << strSpace;
		ss << monthNumToStr(m) << strSpace;
		ss << std::setw(2) << std::setfill(zero) << d;
		return ss.str();
	}

private:
	std::time_t m_time;

	static const time_t SEC_IN_DAY = 24 * 60 * 60;
	static const time_t SEC_IN_HOUR = 60 * 60;
	static const time_t SEC_IN_MINUTE = 60;
	static const int BASE_YEAR = 1970;

	// Converts month number (m = 1..12) to triliteral name (for example: Jan).
	std::basic_string<TChar> monthNumToStr(const int m)
	{
		return ((m <= 0 || m > 12) ? std::basic_string<TChar>() : strMonth<TChar>(m - 1));
	}

	// Returns whether the year is leap.
	bool isLeapYear(const int year)
	{
		return ((year % 4 == 0 && year % 100 != 0 || year % 400 == 0) ? true : false);
	}

	// Returns the number of days in the year.
	int daysInYear(const int year)
	{
		return (isLeapYear(year) ? 366 : 365);
	}

	// Return the number of days in the month.
	// isLeap = true for leap years
	int daysInMonth(const int month, const bool isLeap)
	{
		static const int numDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		if (month <= 0 || month > 12)
			return 0;
		if (month == 2 && isLeap) 
			return numDays[month - 1] + 1;
		return numDays[month - 1];
	}

	// Takes the number of days (totalDays) from 00:00:00 01 Jan 1970 and calculate
	// year and remainder of days in current year (returns by year and overDays parameters).
	void calcYear(const int totalDays, int& year, int& overDays)
	{
		int sumDays = 0;
		int currentYear = BASE_YEAR;
		int daysInCurrentYear = 0;
		while (true) {
			daysInCurrentYear = daysInYear(currentYear);
			if (sumDays + daysInCurrentYear <= totalDays) {
				sumDays += daysInCurrentYear;
				++currentYear;
			} else {
				year = currentYear;
				overDays = totalDays - sumDays;
				return;
			}
		}
	}

	// Takes the number of days (totalDays) from begining of a year and calculate
	// month and remainder of days in current month (returns by month and overDays parameters).
	// isLeap = true for leap years
	void calcMonth(const int totalDays, const bool isLeap,  int& month, int& overDays)
	{
		int sumDays = 0;
		int daysInCurrentMonth = 0;
		for (int currentMonth = 1; currentMonth <= 12; ++currentMonth) {
			daysInCurrentMonth = daysInMonth(currentMonth, isLeap);
			if (sumDays + daysInCurrentMonth <= totalDays) {
				sumDays += daysInCurrentMonth;
			} else {
				month = currentMonth;
				overDays = totalDays - sumDays;
				return;
			}
		}
	}
};

// Compile time list containing items of any data types. 
// head - items's data, tail - List, tail of the last item - NullList.
template <typename THead, typename TTail>
struct List {
	using TailType = TTail;
	THead head;
	TTail tail;
};

using NullList = List<NullType, NullType>;

// List definition is very inconvenient besause it's nested data structure.
// However in case of the items look like Item<int> NumMakredList solve the problem.
// It takes list size (N) and item class name (TItem).
template <int N, template<int> class TItem>
struct NumMarkedList {
	using T = List<typename TItem<N>::TData, typename NumMarkedList<N-1, TItem>::T>;
};

template<template<int> class TItem>
struct NumMarkedList<0, TItem> {
	using T = NullList;
};

// Each logger must be able to output messages.
// This class is common implementation of an output strategy.
// It defines a logger out as pair of a filter and a sink.
// Filter can let pass any message or not.
// Sink implements same kind of output (for example, to file or to std::cout). 
// Below you can find a few trvial samples of filters and sinks.
template <
	typename TChar, // character data type
	template<typename TStr, typename TOpt> class TFilter, //filter
	typename TFilterOpt, //filter options
	template<typename TStr, typename TOpt> class TSink, //sink
	typename TSinkOpt //sink options
>
class Out {
public:
	using LogString = std::basic_string<TChar>;

	void send(const Level level, LogString msg)
	{
		LogString filteredMsg;
		if (m_filter.filter(level, msg, filteredMsg)) {
			m_sink.sink(level, msg);
		} else {
			if (!filteredMsg.empty()) {
				m_sink.sink(level, filteredMsg);
			}
		}
	}

private:
	TFilter<LogString, TFilterOpt> m_filter;
	TSink<LogString, TSinkOpt> m_sink;
};

// Implements recursive enumeration of logger out's list.
template <typename TStr, typename TList>
class OutListRunner {
public:
	static void run(const Level level, TStr msg, TList& list)
	{
		list.head.send(level, msg);
		OutListRunner<TStr, typename TList::TailType>::run(level, msg, list.tail);
	}
};

template<typename TStr>
class OutListRunner<TStr, NullList> {
public:
	static void run(const Level level, TStr msg, NullList& list) {}
};

// Methods to free logger instances
enum class DeleteMethod {
	AT_EXIT = 0,
	DELIBERATE_MEMORY_LEAK
};

// Default logger options. Can be redefined by iheritance if it's necessery.
struct Options {
	using LogChar = char;
	static constexpr DeleteMethod deleteMethod = DeleteMethod::AT_EXIT;
	static constexpr int deltaUTC = 0;
	static constexpr bool noLock = true;
	static constexpr bool printTime = true;
	static constexpr bool printDate = true;
};

// Main logger class. Implements as Meyers' singletone.
// Takes options structure (TOptions) and list of outs (TOutList)
template <typename TOptions, typename TOutList>
class Logger {
public:
	using LogChar = typename TOptions::LogChar;
	using LogString = std::basic_string<LogChar>;

	static Logger* instance()
	{
		static std::once_flag flag;
		std::call_once(flag, create);
		return m_instance;
	}
	
	void log(const Level level, LogString msg)
	{
		if (TOptions::noLock) {
			OutListRunner<LogString, TOutList>::run(level, msg, m_sinkList);
			return;
		}
		std::lock_guard<std::mutex> lock(m_mutex);
		OutListRunner<LogString, TOutList>::run(level, msg, m_sinkList);
	}

private:
	Logger(){}
	~Logger(){}
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	static void create()
	{
		m_instance = new Logger<TOptions, TOutList>;
		if (TOptions::deleteMethod == DeleteMethod::AT_EXIT) {
			std::atexit([](){delete m_instance;});
		}
	}

	TOutList m_sinkList;
	std::lock_guard<std::mutex>::mutex_type m_mutex;

	static Logger* m_instance;
	static std::mutex m_createMutex;
};

template <typename TOptions, typename TOutList> 
Logger<TOptions, TOutList>* Logger<TOptions, TOutList>::m_instance(nullptr);

template <typename TOptions, typename TOutList> 
std::mutex Logger<TOptions, TOutList>::m_createMutex;

enum class ControlValue {
	NL = 0, //insert new line
	DEC, OCT, HEX //switch numeric output format
};

//Accumulates message and send it to the logger
template <typename TOptions, typename TOutList>
class MessageAccumulator {
public:
	using LogChar = typename TOptions::LogChar;
	using LogString = std::basic_string<LogChar>;
	using LogStrStream = std::basic_stringstream<LogChar>;

	MessageAccumulator(const Level level) : m_level(level)
	{
		m_ss << std::showbase << std::boolalpha;
	}

	// Append a value to the message and send it if isLast = true
	template <typename TValue>
	void append(TValue value, bool isLast)
	{
		Appender<LogStrStream, TValue> a;
		a.append(m_ss, value);
		if (isLast) {
			Logger<TOptions, TOutList>::instance()->log(m_level, additionMsg() + m_ss.str());
		}
	}

	// Calls before sending message to add some extra information (priority level, date, time etc.)
	LogString additionMsg()
	{
		auto level = strLevel<LogChar>(m_level);
		constexpr auto space = str<LogChar>(DefStr::space);
		constexpr auto openBr = str<LogChar>(DefStr::openSqBracket);
		constexpr auto closeBr = str<LogChar>(DefStr::closeSqBracket);
		DateTime<LogChar> dt(TOptions::deltaUTC);

		auto result = openBr + level + closeBr + space;
		if (TOptions::printDate) {
			result += (openBr + dt.strDate() + closeBr + space);
		}
		if (TOptions::printTime) {
			result += (openBr + dt.strTime() + closeBr + space);
		}
		return result;
	}

private:
	Level m_level;
	LogStrStream m_ss;

	//Appends a value to the message.
	// Fits for any data types can be used with std::basic_stringstream.
	template <typename TStrStream, typename TValue>
	struct Appender {
		void append(TStrStream& ss, TValue value)
		{
			ss << value;
		}
	};

	//Specialization for ControlValue
	template <typename TStrStream>
	struct Appender<TStrStream, ControlValue> {
		void append(TStrStream& ss, ControlValue value)
		{
			if (value == ControlValue::NL) {
				ss << std::endl;
			} else if (value == ControlValue::DEC) {
				ss << std::dec;
			} else if (value == ControlValue::OCT) {
				ss << std::oct;
			} else if (value == ControlValue::HEX) {
				ss << std::hex;
			}
		}
	};
};

// Logger entry data type.
template <typename TOptions, typename TOutList>
using Entry = std::unique_ptr<MessageAccumulator<TOptions, TOutList>>;

// Creates an logger entry. Calls for each new message.
template <typename TOptions, typename TOutList>
Entry<TOptions, TOutList> createLogEntry(const Level id)
{
	return Entry<TOptions, TOutList>(new typename Entry<TOptions, TOutList>::element_type(id));
}

//Operator to send the part of message to the message accumulator.
template <typename TOptions, typename TOutList, typename TValue>
Entry<TOptions, TOutList> operator<<(Entry<TOptions, TOutList> ma, TValue value)
{
	ma->append(value, false);
	return ma;
}

// Operator to send the last part of message.
// It makes the message accumulator to send the whole message to the logger.
template <typename TOptions, typename TOutList, typename TValue>
void operator<<=(Entry<TOptions, TOutList> ma, TValue value)
{
	ma->append(value, true);
}

// Wrapper class for convenient using the logger.
template <typename TOptions, typename TOutList>
struct LogEntry {

	using CV = ControlValue;

	static Entry<TOptions, TOutList> log(const Level id)
	{
		return createLogEntry<TOptions, TOutList>(id);
	}

	static Entry<TOptions, TOutList> trace() { return log(Level::TRACE);}
	static Entry<TOptions, TOutList> debug() { return log(Level::DEBUG);}
	static Entry<TOptions, TOutList> info() { return log(Level::INFO);}
	static Entry<TOptions, TOutList> warn() { return log(Level::WARN);}
	static Entry<TOptions, TOutList> error() { return log(Level::ERROR);}
	static Entry<TOptions, TOutList> fatal() { return log(Level::FATAL);}
};

//=============================================================================
// A few trivial filters. If it's necessery 
// you can make own filters like these ones.
//=============================================================================

//Lets pass nothing
template <typename TStr, typename TFilterOpt>
class NoneFilter {
public:
	bool filter(const Level level, TStr& in, TStr& out) {return false;}
};

// Lets pass anything
template <typename TStr, typename TFilterOpt>
class AnyFilter {
public:
	bool filter(const Level level, TStr& in, TStr& out) {return true;}
};

//=============================================================================
// A few trivial sinks. If it's necessery 
// you can make own sinks like these ones.
//=============================================================================

//Outputs to std::cout
template <typename TStr, typename TSinkOpt> 
class CoutSink {
public:
	void sink(const Level level, TStr& msg)
	{
		do_sink(msg);
	}

private:
	void do_sink(std::string msg)
	{
		std::cout << msg << std::endl;
	}

	void do_sink(std::wstring msg)
	{
		std::wcout << msg << std::endl;
	}
};

//Default options for StdFileSink (see below). Can be redefined by inheritance if it's necessery.
struct OptionsForStdFileSink {
	static constexpr const char* filename = "./log";
	static constexpr bool clearIfExist = true;
	static constexpr int deltaUTC = 0;
	static constexpr bool addDateTimeToFilename = true;
};

//Outputs to file
template <typename TStr, typename TSinkOpt> 
class StdFileSink {
public:
	StdFileSink()
	{
		DateTime<char> dt(TSinkOpt::deltaUTC);
		std::string filename = TSinkOpt::filename + 
			(TSinkOpt::addDateTimeToFilename ? "-" + dt.strDate(true) + "-" + dt.strTime() : "");

		std::ios_base::openmode mode = std::ofstream::out;
		mode |= (TSinkOpt::clearIfExist ? std::ofstream::trunc : std::ofstream::app);

		m_ofs.open(filename, mode);
	}

	void sink(const Level level, TStr& msg)
	{
		m_ofs << msg << std::endl;
	}

private:
	using OutFileStream = std::basic_ofstream<typename TStr::value_type>;
	OutFileStream m_ofs;
};

};
