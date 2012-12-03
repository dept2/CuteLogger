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
#include "ConsoleAppender.h"

// STL
#include <iostream>


/**
 * \class ConsoleAppender
 *
 * \brief ConsoleAppender is the simple appender that writes the log records to the std::cerr output stream.
 */


ConsoleAppender::ConsoleAppender()
  : AbstractStringAppender(),
    m_ignoreEnvPattern(false)
{}


QString ConsoleAppender::format() const
{
  const QString envPattern = QString::fromLocal8Bit(qgetenv("QT_MESSAGE_PATTERN"));
  return (m_ignoreEnvPattern || envPattern.isEmpty()) ? AbstractStringAppender::format() : (envPattern + "\n");
}


void ConsoleAppender::ignoreEnvironmentPattern(bool ignore)
{
  m_ignoreEnvPattern = ignore;
}


//! Writes the log record to the std::cerr stream.
/**
 * \sa AbstractStringAppender::format()
 */
void ConsoleAppender::append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                             const char* function, const QString& message)
{
  std::cerr << qPrintable(formattedString(timeStamp, logLevel, file, line, function, message));
}
