/*
  Copyright (c) 2010 Boris Moiseev (cyberbobs at gmail dot com)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1
  as published by the Free Software Foundation and appearing in the file
  LICENSE.LGPL included in the packaging of this file.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
*/
#ifndef LOGGER_H
#define LOGGER_H

// Qt
#include <QString>
#include <QDebug>
#include <QDateTime>

// Local
#include "CuteLogger_global.h"
class AbstractAppender;


#define LOG_TRACE(...)       Logger::write(Logger::Trace,   __FILE__, __LINE__, Q_FUNC_INFO, ##__VA_ARGS__)
#define LOG_DEBUG(...)       Logger::write(Logger::Debug,   __FILE__, __LINE__, Q_FUNC_INFO, ##__VA_ARGS__)
#define LOG_INFO(...)        Logger::write(Logger::Info,    __FILE__, __LINE__, Q_FUNC_INFO, ##__VA_ARGS__)
#define LOG_WARNING(...)     Logger::write(Logger::Warning, __FILE__, __LINE__, Q_FUNC_INFO, ##__VA_ARGS__)
#define LOG_ERROR(...)       Logger::write(Logger::Error,   __FILE__, __LINE__, Q_FUNC_INFO, ##__VA_ARGS__)
#define LOG_FATAL(...)       Logger::write(Logger::Fatal,   __FILE__, __LINE__, Q_FUNC_INFO, ##__VA_ARGS__)

#define LOG_TRACE_TIME(...)  LoggerTimingHelper loggerTimingHelper(Logger::Trace, __FILE__, __LINE__, Q_FUNC_INFO, ##__VA_ARGS__)
#define LOG_DEBUG_TIME(...)  LoggerTimingHelper loggerTimingHelper(Logger::Debug, __FILE__, __LINE__, Q_FUNC_INFO, ##__VA_ARGS__)
#define LOG_INFO_TIME(...)   LoggerTimingHelper loggerTimingHelper(Logger::Info,  __FILE__, __LINE__, Q_FUNC_INFO, ##__VA_ARGS__)

#define LOG_ASSERT(cond) ((!(cond)) ? Logger::writeAssert(__FILE__, __LINE__, Q_FUNC_INFO, #cond) : qt_noop())


class CUTELOGGERSHARED_EXPORT Logger
{
  public:
    //! Describes the possible severity levels of the log records
    enum LogLevel
    {
      Trace,   //!< Trace level. Can be used for mostly unneeded records used for internal code tracing.
      Debug,   //!< Debug level. Useful for non-necessary records used for the debugging of the software.
      Info,    //!< Info level. Can be used for informational records, which may be interesting for not only developers.
      Warning, //!< Warning. May be used to log some non-fatal warnings detected by your application.
      Error,   //!< Error. May be used for a big problems making your application work wrong but not crashing.
      Fatal    //!< Fatal. Used for unrecoverable errors, crashes the application right after the log record is written.
    };

    static QString levelToString(LogLevel logLevel);
    static LogLevel levelFromString(const QString& s);

    static void registerAppender(AbstractAppender* appender);

    static void write(const QDateTime& timeStamp, LogLevel logLevel, const char* file, int line, const char* function,
                      const QString& message);

    static void write(LogLevel logLevel, const char* file, int line, const char* function, const QString& message);

    static void write(LogLevel logLevel, const char* file, int line, const char* function, const char* message, ...);

    static QDebug write(LogLevel logLevel, const char* file, int line, const char* function);

    static void writeAssert(const char* file, int line, const char* function, const char* condition);
};


class CUTELOGGERSHARED_EXPORT LoggerTimingHelper
{
  public:
    inline explicit LoggerTimingHelper(Logger::LogLevel logLevel, const char* file, int line, const char* function,
                                       const QString& block = QString())
      : m_logLevel(logLevel),
        m_file(file),
        m_line(line),
        m_function(function),
        m_block(block)
    {
      m_time.start();
    }

    ~LoggerTimingHelper();

  private:
    QTime m_time;
    Logger::LogLevel m_logLevel;
    const char* m_file;
    int m_line;
    const char* m_function;
    QString m_block;
};


#endif // LOGGER_H
