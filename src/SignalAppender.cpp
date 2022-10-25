#include "SignalAppender.h"

/**
 * \class SignalAppender signalappender.h signalappender.cpp
 *
 * \brief Simple appender that sends the log records to the writeMessage signal.
 */

//! Constructs the new file appender assigned to file with the given name.
SignalAppender::SignalAppender()
  : QObject()
  , AbstractStringAppender()
{
}

SignalAppender::~SignalAppender() {}

//! \fn void writeMessage(const QString& message)
//!
//! LOG_DEBUG etc. will send the message to the appropriate slot.
//!
//! \code
//!   auto infoWidget = new QPlainTextEdit(this);
//!
//!   auto signalAppender = new SignalAppender();
//!   signalAppender->setFormat("[%{file}] [%{type:-7}] <%{Function}>
//!   Line:%{line} %{message}"); signalAppender->setDetailsLevel(Logger::Debug);
//!   cuteLogger->registerAppender(signalAppender);
//!   connect(signalAppender, &SignalAppender::write, infoWidget,
//!   &QPlainTextEdit::appendPlainText);
//! \endcode

//! Write the log to writeMessage signal.
/**
 * \sa AbstractStringAppender::format()
 */
void
SignalAppender::append(const QDateTime& timeStamp,
                       Logger::LogLevel logLevel,
                       const char* file,
                       int line,
                       const char* function,
                       const QString& category,
                       const QString& message)
{
  auto string = formattedString(
    timeStamp, logLevel, file, line, function, category, message);
  emit writeMessage(string);
}
