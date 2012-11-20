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
// Local
#include "Logger.h"
#include "AbstractAppender.h"

// Qt
#include <QCoreApplication>
#include <QReadWriteLock>
#include <QSemaphore>
#include <QDateTime>
#include <QIODevice>
#include <QTextCodec>

// STL
#include <iostream>


/**
 * \file Logger.h
 * \brief A file containing the description of Logger class and and additional useful macros for logging
 */


/**
 * \mainpage
 *
 * Logger is a simple way to write the history of your application lifecycle to any target logging device (which is
 * called Appender and may write to any target you will implement with it: console, text file, XML or something - you
 * choose) and to map logging message to a class, function, source file and line of code which it is called from.
 *
 * Some simple appenders (which may be considered an examples) are provided with the logger itself: see ConsoleAppender
 * and FileAppender documentation.
 *
 * It supports using it in a multithreaded applications, so ALL of its functions are thread safe.
 *
 * Simple usage example:
 * \code
 * #include <QCoreApplication>
 *
 * #include <Logger.h>
 * #include <ConsoleAppender.h>
 *
 * int main(int argc, char* argv[])
 * {
 *   QCoreApplication app(argc, argv);
 *   ...
 *   ConsoleAppender* consoleAppender = new ConsoleAppender();
 *   consoleAppender->setFormat("[%-7l] <%C> %m\n");
 *   Logger::registerAppender(consoleAppender);
 *   ...
 *   LOG_INFO("Starting the application");
 *   int result = app.exec();
 *   ...
 *   if (result)
 *     LOG_WARNING() << "Something went wrong." << "Result code is" << result;
 *
 *   return result;
 * }
 * \endcode
 *
 * Logger internally uses the lazy-initialized singleton object and needs no definite initialization, but you may
 * consider registering a log appender before calling any log recording functions or macros.
 *
 * The library design of Logger allows you to simply mass-replace all occurrences of qDebug and similiar calls with
 * similiar Logger macros (e.g. LOG_DEBUG)
 *
 * \note Logger uses a singleton class which must live through all the application life cycle and cleans it on the
 *       destruction of the QCoreApplication (or QApplication) instance. It needs a QCoreApplication instance to be
 *       created before any of the Logger's functions are called.
 *
 * \sa AbstractAppender
 * \sa LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_FATAL
 * \sa LOG_ASSERT
 */


/**
 * \def LOG_TRACE(...)
 *
 * \brief Writes the trace log record
 *
 * This macro is the convinient way to call Logger::write(). It uses the common preprocessor macros \c __FILE__,
 * \c __LINE__ and the standart Qt \c Q_FUNC_INFO macros to automatically determine the needed parameters to call
 * Logger::write().
 *
 * \note This and other (LOG_INFO() etc...) macros uses the variadic macro arguments to give convinient usage form for
 * the different versions of Logger::write() (using the QString or const char* argument or returning the QDebug class
 * instance). Not all compilers will support this. Please, consider reviewing your compiler documentation to ensure
 * it support __VA_ARGS__ macro.
 *
 * It is checked to work with GCC 4.4 or later.
 *
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_DEBUG(...)
 *
 * \brief Writes the debug log record
 *
 * This macro records the info log record using the Logger::write() function. It works identically to the LOG_TRACE()
 * macro.
 *
 * \sa LOG_TRACE()
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_INFO(...)
 *
 * \brief Writes the info log record
 *
 * This macro records the info log record using the Logger::write() function. It works identically to the LOG_TRACE()
 * macro.
 *
 * \sa LOG_TRACE()
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_WARNING(...)
 *
 * \brief Write the warning log record
 *
 * This macro records the warning log record using the Logger::write() function. It works identically to the LOG_TRACE()
 * macro.
 *
 * \sa LOG_TRACE()
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_ERROR(...)
 *
 * \brief Write the error log record
 * This macro records the error log record using the Logger::write() function. It works identically to the LOG_TRACE()
 * macro.
 *
 * \sa LOG_TRACE()
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_FATAL(...)
 *
 * \brief Write the fatal log record
 *
 * This macro records the fatal log record using the Logger::write() function. It works identically to the LOG_TRACE()
 * macro.
 *
 * \note Recording of the log record using the Logger::Fatal log level will lead to calling the STL abort()
 *       function, which will interrupt the running of your software and begin the writing of the core dump.
 *
 * \sa LOG_TRACE()
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_ASSERT
 *
 * \brief Check the assertion
 *
 * This macro is a convinient and recommended to use way to call Logger::writeAssert() function. It uses the
 * preprocessor macros (as the LOG_DEBUG() does) to fill the necessary arguments of the Logger::writeAssert() call. It
 * also uses undocumented but rather mature and stable \c qt_noop() function (which does nothing) when the assertion
 * is true.
 *
 * Example:
 * \code
 * bool b = checkSomething();
 * ...
 * LOG_ASSERT(b == true);
 * \endcode
 *
 * \sa Logger::writeAssert()
 */


/**
 * \class Logger
 *
 * \brief Very simple but rather powerful component which may be used for logging your application activities.
 */

class LogDevice : public QIODevice
{
  public:
    LogDevice()
      : m_semaphore(1)
    {}

    void lock(Logger::LogLevel logLevel, const char* file, int line, const char* function)
    {
      m_semaphore.acquire();

      if (!isOpen())
        open(QIODevice::WriteOnly);

      m_logLevel = logLevel;
      m_file = file;
      m_line = line;
      m_function = function;
    }

  protected:
    qint64 readData(char*, qint64)
    {
      return 0;
    }

    qint64 writeData(const char* data, qint64 maxSize)
    {
      if (maxSize > 0)
        Logger::write(m_logLevel, m_file, m_line, m_function, QString::fromLocal8Bit(QByteArray(data, maxSize)));

      m_semaphore.release();
      return maxSize;
    }

  private:
    QSemaphore m_semaphore;
    Logger::LogLevel m_logLevel;
    const char* m_file;
    int m_line;
    const char* m_function;
};


// Forward declarations
static void cleanupLoggerPrivate();

#if QT_VERSION >= 0x050000
static void qtLoggerMessageHandler(QtMsgType, const QMessageLogContext& context, const QString& msg);
#else
static void qtLoggerMessageHandler(QtMsgType type, const char* msg);
#endif

/**
 * \internal
 *
 * LoggerPrivate class implements the Singleton pattern in a thread-safe way. It uses a static pointer to itself
 * protected by QReadWriteLock
 *
 * The appender list inside the LoggerPrivate class is also protected by QReadWriteLock so this class could be safely
 * used in a multi-threaded application.
 */
class LoggerPrivate
{
  public:
    static LoggerPrivate* m_self;
    static QReadWriteLock m_selfLock;

    static LoggerPrivate* instance()
    {
      LoggerPrivate* result = 0;
      {
        QReadLocker locker(&m_selfLock);
        result = m_self;
      }

      if (!result)
      {
        QWriteLocker locker(&m_selfLock);
        m_self = new LoggerPrivate;

#if QT_VERSION >= 0x050000
        qInstallMessageHandler(qtLoggerMessageHandler);
#else
        qInstallMsgHandler(qtLoggerMessageHandler);
#endif
        qAddPostRoutine(cleanupLoggerPrivate);
        result = m_self;
      }

      return result;
    }


    LoggerPrivate()
      : m_logDevice(0)
    {}


    ~LoggerPrivate()
    {
      // Cleanup appenders
      QReadLocker appendersLocker(&m_appendersLock);
      foreach (AbstractAppender* appender, m_appenders)
        delete appender;

      // Cleanup device
      QReadLocker deviceLocker(&m_logDeviceLock);
      delete m_logDevice;
    }


    void registerAppender(AbstractAppender* appender)
    {
      QWriteLocker locker(&m_appendersLock);

      if (!m_appenders.contains(appender))
        m_appenders.append(appender);
      else
        std::cerr << "Trying to register appender that was already registered" << std::endl;
    }


    LogDevice* logDevice()
    {
      LogDevice* result = 0;
      {
        QReadLocker locker(&m_logDeviceLock);
        result = m_logDevice;
      }

      if (!result)
      {
        QWriteLocker locker(&m_logDeviceLock);
        m_logDevice = new LogDevice;
        result = m_logDevice;
      }

      return result;
    }


    void write(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line, const char* function,
               const QString& message)
    {
      QReadLocker locker(&m_appendersLock);

      if (!m_appenders.isEmpty())
      {
        foreach (AbstractAppender* appender, m_appenders)
          appender->write(timeStamp, logLevel, file, line, function, message);
      }
      else
      {
        // Fallback
        QString result = QString(QLatin1String("[%1] <%2> %3")).arg(Logger::levelToString(logLevel), -7)
                                                               .arg(function).arg(message);

        std::cerr << qPrintable(result) << std::endl;
      }

      if (logLevel == Logger::Fatal)
        abort();
    }


    void write(Logger::LogLevel logLevel, const char* file, int line, const char* function, const QString& message)
    {
      write(QDateTime::currentDateTime(), logLevel, file, line, function, message);
    }


    void write(Logger::LogLevel logLevel, const char* file, int line, const char* function, const char* message)
    {
      write(logLevel, file, line, function, QString(message));
    }


    QDebug write(Logger::LogLevel logLevel, const char* file, int line, const char* function)
    {
      LogDevice* d = logDevice();
      d->lock(logLevel, file, line, function);
      return QDebug(d);
    }


    void writeAssert(const char* file, int line, const char* function, const char* condition)
    {
      write(Logger::Fatal, file, line, function, QString("ASSERT: \"%1\"").arg(condition));
    }

  private:
    QList<AbstractAppender*> m_appenders;
    QReadWriteLock m_appendersLock;

    LogDevice* m_logDevice;
    QReadWriteLock m_logDeviceLock;
};

// Static fields initialization
LoggerPrivate* LoggerPrivate::m_self = 0;
QReadWriteLock LoggerPrivate::m_selfLock;


static void cleanupLoggerPrivate()
{
  QWriteLocker locker(&LoggerPrivate::m_selfLock);

  delete LoggerPrivate::m_self;
  LoggerPrivate::m_self = 0;
}


#if QT_VERSION >= 0x050000
static void qtLoggerMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
  Logger::LogLevel level;
  switch (type)
  {
    case QtDebugMsg:
      level = Logger::Debug;
      break;
    case QtWarningMsg:
      level = Logger::Warning;
      break;
    case QtCriticalMsg:
      level = Logger::Error;
      break;
    case QtFatalMsg:
      level = Logger::Fatal;
      break;
  }

  Logger::write(level, context.file, context.line, context.function, msg);
}

#else

static void qtLoggerMessageHandler(QtMsgType type, const char* msg)
{
  switch (type)
  {
    case QtDebugMsg:
      LOG_DEBUG(msg);
      break;
    case QtWarningMsg:
      LOG_WARNING(msg);
      break;
    case QtCriticalMsg:
      LOG_ERROR(msg);
      break;
    case QtFatalMsg:
      LOG_FATAL(msg);
      break;
  }
}
#endif


//! Converts the LogLevel enum value to its string representation
/**
 * \param logLevel Log level to convert
 *
 * \sa LogLevel
 * \sa levelFromString()
 */
QString Logger::levelToString(Logger::LogLevel logLevel)
{
  switch (logLevel)
  {
    case Trace:
      return QLatin1String("Trace");
    case Debug:
      return QLatin1String("Debug");
    case Info:
      return QLatin1String("Info");
    case Warning:
      return QLatin1String("Warning");
    case Error:
      return QLatin1String("Error");
    case Fatal:
      return QLatin1String("Fatal");
  }

  return QString();
}


//! Converts the LogLevel string representation to enum value
/**
 * Comparation of the strings is case independent. If the log level string provided cannot be understood
 * Logger::Debug is returned.
 *
 * \param s String to be decoded
 *
 * \sa LogLevel
 * \sa levelToString()
 */
Logger::LogLevel Logger::levelFromString(const QString& s)
{
  QString str = s.trimmed().toLower();

  LogLevel result = Debug;

  if (str == QLatin1String("trace"))
    result = Trace;
  else if (str == QLatin1String("debug"))
    result = Debug;
  else if (str == QLatin1String("info"))
    result = Info;
  else if (str == QLatin1String("warning"))
    result = Warning;
  else if (str == QLatin1String("error"))
    result = Error;
  else if (str == QLatin1String("fatal"))
    result = Fatal;

  return result;
}


//! Registers the appender to write the log records to
/**
 * On the log writing call (using one of the macros or the write() function) Logger traverses through the list of
 * the appenders and writes a log records to the each of them. Please, look through the AbstractAppender
 * documentation to understand the concept of appenders.
 *
 * If no appenders was added to Logger, it falls back to logging into the \c std::cerr STL stream.
 *
 * \param appender Appender to register in the Logger
 *
 * \note Logger takes ownership on the appender and it will delete it on the application exit. According to this,
 *       appenders must be created on heap to prevent double destruction of the appender.
 *
 * \sa AbstractAppender
 */
void Logger::registerAppender(AbstractAppender* appender)
{
  LoggerPrivate::instance()->registerAppender(appender);
}


//! Writes the log record
/**
 * Writes the log records with the supplied arguments to all the registered appenders.
 *
 * \note It is not recommended to call this function directly. Instead of this you can just call one of the macros
 *       (LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_FATAL) that will supply all the needed
 *       information to this function.
 *
 * \param timeStamp - the time stamp of the record
 * \param logLevel - the log level of the record
 * \param file - the name of the source file that requested the log record
 * \param line - the line of the code of source file that requested the log record
 * \param function - name of the function that requested the log record
 * \param message - log message
 *
 * \note Recording of the log record using the Logger::Fatal log level will lead to calling the STL abort()
 *       function, which will interrupt the running of your software and begin the writing of the core dump.
 *
 * \sa LogLevel
 * \sa LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_FATAL
 * \sa AbstractAppender
 */
void Logger::write(const QDateTime& timeStamp, LogLevel logLevel, const char* file, int line, const char* function,
                   const QString& message)
{
  LoggerPrivate::instance()->write(timeStamp, logLevel, file, line, function, message);
}


/**
 * This is the overloaded function provided for the convinience. It behaves identically to the above function.
 *
 * This function uses the current timestamp obtained with \c QDateTime::currentDateTime().
 *
 * \sa write()
 */
void Logger::write(LogLevel logLevel, const char* file, int line, const char* function, const QString& message)
{
  LoggerPrivate::instance()->write(logLevel, file, line, function, message);
}


/**
 * This is the overloaded function provided for the convinience. It behaves identically to the above function.
 *
 * This function uses the current timestamp obtained with \c QDateTime::currentDateTime(). Also it supports writing
 * <tt>const char*</tt> instead of \c QString and converts it internally using the \c QString::fromAscii(). If you
 * want this function to support the non-ascii strings, you will need to setup the codec using the
 * \c QTextCodec::setCodecForCStrings()
 *
 * \sa write()
 */
void Logger::write(LogLevel logLevel, const char* file, int line, const char* function, const char* message, ...)
{
  va_list va;
  va_start(va, message);
  LoggerPrivate::instance()->write(logLevel, file, line, function, QString().vsprintf(message,va));
  va_end(va);
}


/**
 * This is the overloaded function provided for the convinience. It behaves identically to the above function.
 *
 * This function doesn't accept any log message as argument. It returns the \c QDebug object that can be written
 * using the stream functions. For example, you may like to write:
 * \code
 * LOG_DEBUG() << "This is the size" << size << "of the element" << elementName;
 * \endcode
 * instead of writing
 * \code
 * LOG_DEBUG(QString(QLatin1String("This is the size %1x%2 of the element %3"))
 *           .arg(size.x()).arg(size.y()).arg(elementName));
 * \endcode
 *
 * Please consider reading the Qt Reference Documentation for the description of the QDebug class usage syntax.
 *
 * \note This overload is definitely more pleasant to use than the first write() overload, but it behaves definitely
 *       slower than all the above overloads.
 *
 * \sa write()
 */
QDebug Logger::write(LogLevel logLevel, const char* file, int line, const char* function)
{
  return LoggerPrivate::instance()->write(logLevel, file, line, function);
}


//! Writes the assertion
/**
 * This function writes the assertion record using the write() function.
 *
 * The assertion record is always written using the Logger::Fatal log level which leads to the abortation of the
 * program and generation of the core dump (if supported).
 *
 * The message written to the appenders will be identical to the \c condition argument prefixed with the
 * <tt>ASSERT:</tt> notification.
 *
 * \note It is not recommended to call this function directly. Instead of this you can just call the LOG_ASSERT
 *       macro that will supply all the needed information to this function.
 *
 * \sa LOG_ASSERT
 * \sa write()
 */
void Logger::writeAssert(const char* file, int line, const char* function, const char* condition)
{
  LoggerPrivate::instance()->writeAssert(file, line, function, condition);
}
