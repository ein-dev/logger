//*********************************************************************************
// Definition and tuning of the application loggers.
// In real projects this code must be moved in separate header.
//*********************************************************************************

#pragma once

#include "./logger/logger.h"

// New filter implementation
namespace Logger {
	template <typename TStr, typename TFilterOpt>
	class TraceFilter {
	public:
		bool filter(const Level level, TStr& in, TStr& out) {return (level == Level::TRACE ? false : true);}
	};
};

// The simplest logger.
// Character data type - char; number of outs - 1.
namespace MinLogger {

	using Options = Logger::Options;

	template <int N> struct Item {};
	template <> struct Item<1> {
		typedef Logger::Out<Options::LogChar, Logger::AnyFilter, Logger::NullType, Logger::CoutSink, Logger::NullType> TData;
	};
	typedef Logger::NumMarkedList<1, Item>::T OutList;
};
using MLOG = Logger::LogEntry<MinLogger::Options, MinLogger::OutList>;

// Normal logger.
// Character data type - char; number of outs - 2.
namespace CharLogger {

	struct Options : public Logger::Options {
		static constexpr int deltaUTC = 3;
	};

	struct FileSinkOptions : public Logger::OptionsForStdFileSink {
		static constexpr int deltaUTC = 3;
		static constexpr const char* filename = "./myapp_log";
	};

	template <int N> struct Item {};
	template <> struct Item<1> {
		typedef Logger::Out<Options::LogChar, Logger::AnyFilter, Logger::NullType, Logger::CoutSink, Logger::NullType> TData;
	};
	template <> struct Item<2> {
		typedef Logger::Out<Options::LogChar, Logger::AnyFilter, Logger::NullType, Logger::StdFileSink, FileSinkOptions> TData;
	};
	typedef Logger::NumMarkedList<2, Item>::T OutList;
}
using ALOG = Logger::LogEntry<CharLogger::Options, CharLogger::OutList>;

// Normal logger.
// Character data type - wchar_t; number of outs - 2; uses user defined filter (TraceFilter)
namespace WCharLogger {

	struct Options : public Logger::Options {
		using LogChar = wchar_t;
		static constexpr int deltaUTC = 3;
	};

	struct FileSinkOptions : public Logger::OptionsForStdFileSink {
		static constexpr int deltaUTC = 3;
		static constexpr const char* filename = "./myapp_wlog";
	};


	template <int N> struct Item {};
	template <> struct Item<1> {
		typedef Logger::Out<Options::LogChar, Logger::TraceFilter, Logger::NullType, Logger::CoutSink, Logger::NullType> TData;
	};
	template <> struct Item<2> {
		typedef Logger::Out<Options::LogChar, Logger::AnyFilter, Logger::NullType, Logger::StdFileSink, FileSinkOptions> TData;
	};
	typedef Logger::NumMarkedList<2, Item>::T OutList;
}
using WLOG = Logger::LogEntry<WCharLogger::Options, WCharLogger::OutList>;
