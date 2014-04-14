/*
  Copyright (c) 2012 Boris Moiseev (cyberbobs at gmail dot com)

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
#include "AbstractStringAppender.h"

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
 * Some simple appenders (which may be considered an examples) are provided with the Logger itself: see ConsoleAppender
 * and FileAppender documentation.
 *
 * It supports using it in a multithreaded applications, so all of its functions are thread safe.
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
 *   ConsoleAppender* consoleAppender = new ConsoleAppender;
 *   consoleAppender->setFormat("[%{type:-7}] <%{Function}> %{message}\n");
 *   logger->registerAppender(consoleAppender);
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
 * similiar Logger macros (e.g. LOG_DEBUG())
 *
 * \note Logger uses a singleton global instance which lives through all the application life cycle and self-destroys
 *       destruction of the QCoreApplication (or QApplication) instance. It needs a QCoreApplication instance to be
 *       created before any of the Logger's functions are called.
 *
 * \sa logger
 * \sa LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_FATAL
 * \sa LOG_ASSERT
 * \sa LOG_TRACE_TIME, LOG_DEBUG_TIME, LOG_INFO_TIME
 * \sa AbstractAppender
 */


/**
 * \def logger
 *
 * \brief Macro returning the current instance of Logger object
 *
 * If you haven't created a local Logger object it returns the same value as the Logger::globalInstance() functions.
 * This macro is a recommended way to get an access to the Logger instance used in current class.
 *
 * Example:
 * \code
 * ConsoleAppender* consoleAppender = new ConsoleAppender;
 * logger->registerAppender(consoleAppender);
 * \endcode
 *
 * \sa Logger::globalInstance()
 */


/**
 * \def LOG_TRACE
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
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_DEBUG
 *
 * \brief Writes the debug log record
 *
 * This macro records the info log record using the Logger::write() function. It works similiar to the LOG_TRACE()
 * macro.
 *
 * \sa LOG_TRACE()
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_INFO
 *
 * \brief Writes the info log record
 *
 * This macro records the info log record using the Logger::write() function. It works similiar to the LOG_TRACE()
 * macro.
 *
 * \sa LOG_TRACE()
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_WARNING
 *
 * \brief Write the warning log record
 *
 * This macro records the warning log record using the Logger::write() function. It works similiar to the LOG_TRACE()
 * macro.
 *
 * \sa LOG_TRACE()
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_ERROR
 *
 * \brief Write the error log record
 * This macro records the error log record using the Logger::write() function. It works similiar to the LOG_TRACE()
 * macro.
 *
 * \sa LOG_TRACE()
 * \sa Logger::LogLevel
 * \sa Logger::write()
 */


/**
 * \def LOG_FATAL
 *
 * \brief Write the fatal log record
 *
 * This macro records the fatal log record using the Logger::write() function. It works similiar to the LOG_TRACE()
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
 * \def LOG_TRACE_TIME
 *
 * \brief Logs the processing time of current function / code block
 *
 * This macro automagically measures the function or code of block execution time and outputs it as a Logger::Trace
 * level log record.
 *
 * Example:
 * \code
 * int foo()
 * {
 *   LOG_TRACE_TIME();
 *   ... // Do some long operations
 *   return 0;
 * } // Outputs: Function foo finished in <time> ms.
 * \endcode
 *
 * If you are measuring a code of block execution time you may also add a name of block to the macro:
 * \code
 * int bar(bool doFoo)
 * {
 *   LOG_TRACE_TIME();
 *
 *   if (doFoo)
 *   {
 *     LOG_TRACE_TIME("Foo");
 *     ...
 *   }
 *
 *   ...
 * }
 * // Outputs:
 * // "Foo" finished in <time1> ms.
 * // Function bar finished in <time2> ms.
 * \endcode
 *
 * \note Macro switches to logging the seconds instead of milliseconds when the execution time reaches 10000 ms.
 * \sa LOG_DEBUG_TIME, LOG_INFO_TIME
 */


/**
 * \def LOG_DEBUG_TIME
 *
 * \brief Logs the processing time of current function / code block
 *
 * This macro automagically measures the function or code of block execution time and outputs it as a Logger::Debug
 * level log record. It works similiar to LOG_TRACE_TIME() macro.
 *
 * \sa LOG_TRACE_TIME
 */


/**
 * \def LOG_INFO_TIME
 *
 * \brief Logs the processing time of current function / code block
 *
 * This macro automagically measures the function or code of block execution time and outputs it as a Logger::Info
 * level log record. It works similiar to LOG_TRACE_TIME() macro.
 *
 * \sa LOG_TRACE_TIME
 */


/**
 * \class Logger
 *
 * \brief Very simple but rather powerful component which may be used for logging your application activities.
 *
 * Global logger instance created on a first access to it (e.g. registering appenders, calling a LOG_DEBUG() macro
 * etc.) registers itself as a Qt default message handler and captures all the qDebug/qWarning/qCritical output.
 *
 * \note Qt 4 qDebug set of macro doesn't support capturing source function name, file name or line number so we
 *       recommend to use LOG_DEBUG() and other Logger macros instead.
 *
 * \sa logger
 * \sa [CuteLogger Documentation](index.html)
 */

class LogDevice : public QIODevice
{
  public:
    LogDevice(Logger* l)
      : m_logger(l),
        m_semaphore(1)
    {}

    void lock(Logger::LogLevel logLevel, const char* file, int line, const char* function, const char* category)
    {
      m_semaphore.acquire();

      if (!isOpen())
        open(QIODevice::WriteOnly);

      m_logLevel = logLevel;
      m_file = file;
      m_line = line;
      m_function = function;
      m_category = category;
    }

  protected:
    qint64 readData(char*, qint64)
    {
      return 0;
    }

    qint64 writeData(const char* data, qint64 maxSize)
    {
      if (maxSize > 0)
        m_logger->write(m_logLevel, m_file, m_line, m_function, m_category, QString::fromLocal8Bit(QByteArray(data, maxSize)));

      m_semaphore.release();
      return maxSize;
    }

  private:
    Logger* m_logger;
    QSemaphore m_semaphore;
    Logger::LogLevel m_logLevel;
    const char* m_file;
    int m_line;
    const char* m_function;
    const char* m_category;
};


// Forward declarations
static void cleanupLoggerGlobalInstance();

#if QT_VERSION >= 0x050000
static void qtLoggerMessageHandler(QtMsgType, const QMessageLogContext& context, const QString& msg);
#else
static void qtLoggerMessageHandler(QtMsgType type, const char* msg);
#endif

/**
 * \internal
 *
 * LoggerPrivate class implements the Singleton pattern in a thread-safe way. It contains a static pointer to the
 * global logger instance protected by QReadWriteLock
 */
class LoggerPrivate
{
  public:
    static Logger* globalInstance;
    static QReadWriteLock globalInstanceLock;

    QList<AbstractAppender*> appenders;
    QMutex loggerMutex;

    QMap<QString, bool> categories;
    QMultiMap<QString, AbstractAppender*> categoryAppenders;
    QString defaultCategory;

    LogDevice* logDevice;
};


// Static fields initialization
Logger* LoggerPrivate::globalInstance = 0;
QReadWriteLock LoggerPrivate::globalInstanceLock;


static void cleanupLoggerGlobalInstance()
{
  QWriteLocker locker(&LoggerPrivate::globalInstanceLock);

  delete LoggerPrivate::globalInstance;
  LoggerPrivate::globalInstance = 0;
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

  bool isDefaultCategory = QString::fromLatin1(context.category) == "default";
  Logger::globalInstance()->write(level, context.file, context.line, context.function, isDefaultCategory ? 0 : context.category, msg);
}

#else

static void qtLoggerMessageHandler(QtMsgType type, const char* msg)
{
  switch (type)
  {
    case QtDebugMsg:
      loggerInstance()->write(Logger::Debug, "", 0, "qDebug", msg, 0);
      break;
    case QtWarningMsg:
      loggerInstance()->write(Logger::Warning, "", 0, "qDebug", msg, 0);
      break;
    case QtCriticalMsg:
      loggerInstance()->write(Logger::Error, "", 0, "qDebug", msg, 0);
      break;
    case QtFatalMsg:
      loggerInstance()->write(Logger::Fatal, "", 0, "qDebug", msg, 0);
      break;
  }
}
#endif


//! Construct the instance of Logger
/**
 * If you're only using one global instance of logger you wouldn't probably need to use this constructor manually.
 * Consider using [logger](@ref logger) macro instead to access the logger instance
 */
Logger::Logger()
  : d_ptr(new LoggerPrivate)
{
  Q_D(Logger);

  d->logDevice = new LogDevice(this);
}


Logger::Logger(const QString& defaultCategory)
  : d_ptr(new LoggerPrivate)
{
  Q_D(Logger);
  d->logDevice = new LogDevice(this);

  logCategoryToGlobal(defaultCategory);
  setDefaultCategory(defaultCategory);
}


//! Destroy the instance of Logger
/**
 * You probably wouldn't need to use this function directly. Global instance of logger will be destroyed automatically
 * at the end of your QCoreApplication execution
 */
Logger::~Logger()
{
  Q_D(Logger);

  // Cleanup appenders
  QMutexLocker appendersLocker(&d->loggerMutex);
  qDeleteAll(d->appenders);
  qDeleteAll(d->categoryAppenders);

  // Cleanup device
  delete d->logDevice;
  appendersLocker.unlock();

  delete d_ptr;
}


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


//! Returns the global instance of Logger
/**
 * In a most cases you shouldn't use this function directly. Consider using [logger](@ref logger) macro instead.
 *
 * \sa logger
 */
Logger* Logger::globalInstance()
{
  Logger* result = 0;
  {
    QReadLocker locker(&LoggerPrivate::globalInstanceLock);
    result = LoggerPrivate::globalInstance;
  }

  if (!result)
  {
    QWriteLocker locker(&LoggerPrivate::globalInstanceLock);
    LoggerPrivate::globalInstance = new Logger;

#if QT_VERSION >= 0x050000
    qInstallMessageHandler(qtLoggerMessageHandler);
#else
    qInstallMsgHandler(qtLoggerMessageHandler);
#endif
    qAddPostRoutine(cleanupLoggerGlobalInstance);
    result = LoggerPrivate::globalInstance;
  }

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
  Q_D(Logger);

  QMutexLocker locker(&d->loggerMutex);

  if (!d->appenders.contains(appender))
    d->appenders.append(appender);
  else
    std::cerr << "Trying to register appender that was already registered" << std::endl;
}


void Logger::registerCategoryAppender(const QString& category, AbstractAppender* appender)
{
  Q_D(Logger);

  QMutexLocker locker(&d->loggerMutex);

  if (!d->categoryAppenders.values().contains(appender))
    d->categoryAppenders.insert(category, appender);
  else
    std::cerr << "Trying to register appender that was already registered" << std::endl;
}


void Logger::setDefaultCategory(const QString& category)
{
  Q_D(Logger);

  QMutexLocker locker(&d->loggerMutex);

  d->defaultCategory = category;
}


QString Logger::defaultCategory() const
{
  Q_D(const Logger);
  return d->defaultCategory;
}


void Logger::logCategoryToGlobal(const QString& category, bool logToGlobal)
{
  Q_D(Logger);

  if (this == globalInstance())
  {
    QMutexLocker locker(&d->loggerMutex);
    d->categories.insert(category, logToGlobal);
  }
  else
  {
    globalInstance()->logCategoryToGlobal(category, logToGlobal);
  }
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
 * \param category - logging category (0 for default category)
 * \param message - log message
 * \param passedFromLocal - log message was initiated from local logger instance, but was not written and passed to the global instance
 *
 * \note Recording of the log record using the Logger::Fatal log level will lead to calling the STL abort()
 *       function, which will interrupt the running of your software and begin the writing of the core dump.
 *
 * \sa LogLevel
 * \sa LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_FATAL
 * \sa AbstractAppender
 */
void Logger::write(const QDateTime& timeStamp, LogLevel logLevel, const char* file, int line, const char* function, const char* category,
                   const QString& message, bool passedFromLocal)
{
  Q_D(Logger);

  QMutexLocker locker(&d->loggerMutex);

  QString logCategory = QString::fromLatin1(category);
  if (logCategory.isNull() && !d->defaultCategory.isNull())
    logCategory = d->defaultCategory;

  bool wasWritten = false;
  bool isGlobalInstance = this == globalInstance();
  bool linkedToGlobal = isGlobalInstance && d->categories.value(logCategory, false);

  if (!logCategory.isNull())
  {
    QList<AbstractAppender*> appenders = d->categoryAppenders.values(logCategory);
    if (appenders.length() == 0)
    {
      if (logCategory != d->defaultCategory && !linkedToGlobal)
        std::cerr << "No appenders assotiated with category " << qPrintable(logCategory) << std::endl;
    }
    else
    {
      foreach (AbstractAppender* appender, appenders)
        appender->write(timeStamp, logLevel, file, line, function, logCategory, message);
      wasWritten = true;
    }
  }

  // the default category is linked to the main logger appenders
  // global logger instance also writes all linked categories to the main appenders
  if (logCategory.isNull() || logCategory == d->defaultCategory || linkedToGlobal)
  {
    if (!d->appenders.isEmpty())
    {
      foreach (AbstractAppender* appender, d->appenders)
        appender->write(timeStamp, logLevel, file, line, function, logCategory, message);
      wasWritten = true;
    }
    else
    {
      std::cerr << "No appenders registered with logger" << std::endl;
    }
  }

  // local logger instances send category messages to the global instance
  if (!logCategory.isNull() && !isGlobalInstance)
    globalInstance()->write(timeStamp, logLevel, file, line, function, logCategory.toLatin1(), message, !wasWritten);

  if (!wasWritten && passedFromLocal)
  {
    // Fallback
    QString result = QString(QLatin1String("[%1] <%2> %3")).arg(levelToString(logLevel), -7)
                     .arg(function).arg(message);
    std::cerr << qPrintable(result) << std::endl;
  }

  if (logLevel == Logger::Fatal)
    abort();
}


/**
 * This is the overloaded function provided for the convinience. It behaves similiar to the above function.
 *
 * This function uses the current timestamp obtained with \c QDateTime::currentDateTime().
 *
 * \sa write()
 */
void Logger::write(LogLevel logLevel, const char* file, int line, const char* function, const char* category,
                   const QString& message)
{
  write(QDateTime::currentDateTime(), logLevel, file, line, function, category, message);
}


/**
 * This is the overloaded function provided for the convinience. It behaves similiar to the above function.
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
QDebug Logger::write(LogLevel logLevel, const char* file, int line, const char* function, const char* category)
{
  Q_D(Logger);

  d->logDevice->lock(logLevel, file, line, function, category);
  return QDebug(d->logDevice);
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
  write(Logger::Fatal, file, line, function, 0, QString("ASSERT: \"%1\"").arg(condition));
}


Logger* loggerInstance()
{
  return Logger::globalInstance();
}


LoggerTimingHelper::~LoggerTimingHelper()
{
  QString message;
  if (m_block.isEmpty())
    message = QString(QLatin1String("Function %1 finished in ")).arg(AbstractStringAppender::stripFunctionName(m_function));
  else
    message = QString(QLatin1String("\"%1\" finished in ")).arg(m_block);

  int elapsed = m_time.elapsed();
  if (elapsed >= 10000)
    message += QString(QLatin1String("%1 s.")).arg(elapsed / 1000);
  else
    message += QString(QLatin1String("%1 ms.")).arg(elapsed);

  m_logger->write(m_logLevel, m_file, m_line, m_function, 0, message);
}


void CuteMessageLogger::write(const char* msg, ...) const
{
  va_list va;
  va_start(va, msg);
  m_l->write(m_level, m_file, m_line, m_function, m_category, QString().vsprintf(msg, va));
  va_end(va);
}


void CuteMessageLogger::write(const QString& msg) const
{
  m_l->write(m_level, m_file, m_line, m_function, m_category, msg);
}


QDebug CuteMessageLogger::write() const
{
  return m_l->write(m_level, m_file, m_line, m_function, m_category);
}
