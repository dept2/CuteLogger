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
#include "AbstractStringAppender.h"

// Qt
#include <QReadLocker>
#include <QWriteLocker>
#include <QDateTime>
#include <QRegExp>


/**
 * \class AbstractStringAppender
 *
 * \brief The AbstractStringAppender class provides a convinient base for appenders working with plain text formatted
 *        logs.
 *
 * AbstractSringAppender is the simple extension of the AbstractAppender class providing the convinient way to create
 * custom log appenders working with a plain text formatted log targets.
 *
 * It have the formattedString() protected function that formats the logging arguments according to a format set with
 * setFormat().
 *
 * This class can not be directly instantiated because it contains pure virtual function inherited from AbstractAppender
 * class.
 *
 * For more detailed description of customizing the log output format see the documentation on the setFormat() function.
 */


const char formattingMarker = '%';


//! Constructs a new string appender object
AbstractStringAppender::AbstractStringAppender()
  : m_format(QLatin1String("%t{yyyy-MM-ddTHH:mm:ss.zzz} [%-7l] <%c> %m\n"))
{}


//! Returns the current log format string.
/**
 * The default format is set to "%t{yyyy-MM-ddTHH:mm:ss.zzz} [%-7l] <%C> %m\n". You can set a different log record
 * format using the setFormat() function.
 *
 * \sa setFormat(const QString&)
 */
QString AbstractStringAppender::format() const
{
  QReadLocker locker(&m_formatLock);
  return m_format;
}


//! Sets the logging format for writing strings to the log target with this appender.
/**
 * The string format seems to be very common to those developers who have used a standart sprintf function.
 *
 * Log output format is a simple QString with the special markers (starting with % sign) which will be replaced with
 * it's internal meaning when writing a log record.
 *
 * Controlling marker begins with the percent sign (%) which is followed by (optional) field width argument, the
 * (necessary) single-letter command (which describes, what will be put to log record instead of marker, and an
 * additional formatting argument (in the {} brackets) supported for some of the log commands.
 *
 * Field width argument works almost identically to the \c QString::arg() \c fieldWidth argument (and uses it
 * internally). For example, \c "%-7l" will be replaced with the left padded debug level of the message
 * (\c "Debug  ") or something. For the more detailed description of it you may consider to look to the Qt
 * Reference Documentation.
 *
 * Supported marker commands are:
 *   \arg \c %t - timestamp. You may specify your custom timestamp format using the {} brackets after the marker,
 *           timestamp format here will be similiar to those used in QDateTime::toString() function. For example,
 *           "%t{dd-MM-yyyy, HH:mm}" may be replaced with "17-12-2010, 20:17" depending on current date and time.
 *           The default format used here is "HH:mm:ss.zzz".
 *   \arg \c %l - Log level. Possible log levels are shown in the Logger::LogLevel enumerator.
 *   \arg \c %L - Uppercased log level.
 *   \arg \c %F - Full source file name (with path) of the file that requested log recording. Uses the \c __FILE__
 *           preprocessor macro.
 *   \arg \c %f - Short file name (with stripped path).
 *   \arg \c %i - Line number in the source file. Uses the \c __LINE__ preprocessor macro.
 *   \arg \c %C - Name of function that called on of the LOG_* macros. Uses the \c Q_FUNC_INFO macro provided with
 *           Qt.
 *   \arg \c %c - [EXPERIMENTAL] Similiar to the %C, but the function name is stripped using stripFunctionName
 *   \arg \c %m - The log message sent by the caller.
 *   \arg \c %% - Convinient marker that is replaced with the single \c % mark.
 *
 * \note Format doesn't add \c '\\n' to the end of the format line. Please consider adding it manually.
 *
 * \sa format()
 * \sa stripFunctionName()
 * \sa Logger::LogLevel
 */
void AbstractStringAppender::setFormat(const QString& format)
{
  QWriteLocker locker(&m_formatLock);
  m_format = format;
}


//! Strips the long function signature (as added by Q_FUNC_INFO macro)
/**
 * The string processing drops the returning type, arguments and template parameters of function. It is definitely
 * useful for enchancing the log output readability.
 * \return stripped function name
 */
QString AbstractStringAppender::stripFunctionName(const char* name)
{
  return QString::fromLatin1(qCleanupFuncinfo(name));
}


// The function was backported from Qt5 sources (qlogging.h)
QByteArray AbstractStringAppender::qCleanupFuncinfo(const char* name)
{
  QByteArray info(name);

  // Strip the function info down to the base function name
  // note that this throws away the template definitions,
  // the parameter types (overloads) and any const/volatile qualifiers.

  if (info.isEmpty())
      return info;

  int pos;

  // skip trailing [with XXX] for templates (gcc)
  pos = info.size() - 1;
  if (info.endsWith(']')) {
      while (--pos) {
          if (info.at(pos) == '[')
              info.truncate(pos);
      }
  }

  // operator names with '(', ')', '<', '>' in it
  static const char operator_call[] = "operator()";
  static const char operator_lessThan[] = "operator<";
  static const char operator_greaterThan[] = "operator>";
  static const char operator_lessThanEqual[] = "operator<=";
  static const char operator_greaterThanEqual[] = "operator>=";

  // canonize operator names
  info.replace("operator ", "operator");

  // remove argument list
  forever {
      int parencount = 0;
      pos = info.lastIndexOf(')');
      if (pos == -1) {
          // Don't know how to parse this function name
          return info;
      }

      // find the beginning of the argument list
      --pos;
      ++parencount;
      while (pos && parencount) {
          if (info.at(pos) == ')')
              ++parencount;
          else if (info.at(pos) == '(')
              --parencount;
          --pos;
      }
      if (parencount != 0)
          return info;

      info.truncate(++pos);

      if (info.at(pos - 1) == ')') {
          if (info.indexOf(operator_call) == pos - (int)strlen(operator_call))
              break;

          // this function returns a pointer to a function
          // and we matched the arguments of the return type's parameter list
          // try again
          info.remove(0, info.indexOf('('));
          info.chop(1);
          continue;
      } else {
          break;
      }
  }

  // find the beginning of the function name
  int parencount = 0;
  int templatecount = 0;
  --pos;

  // make sure special characters in operator names are kept
  if (pos > -1) {
      switch (info.at(pos)) {
      case ')':
          if (info.indexOf(operator_call) == pos - (int)strlen(operator_call) + 1)
              pos -= 2;
          break;
      case '<':
          if (info.indexOf(operator_lessThan) == pos - (int)strlen(operator_lessThan) + 1)
              --pos;
          break;
      case '>':
          if (info.indexOf(operator_greaterThan) == pos - (int)strlen(operator_greaterThan) + 1)
              --pos;
          break;
      case '=': {
          int operatorLength = (int)strlen(operator_lessThanEqual);
          if (info.indexOf(operator_lessThanEqual) == pos - operatorLength + 1)
              pos -= 2;
          else if (info.indexOf(operator_greaterThanEqual) == pos - operatorLength + 1)
              pos -= 2;
          break;
      }
      default:
          break;
      }
  }

  while (pos > -1) {
      if (parencount < 0 || templatecount < 0)
          return info;

      char c = info.at(pos);
      if (c == ')')
          ++parencount;
      else if (c == '(')
          --parencount;
      else if (c == '>')
          ++templatecount;
      else if (c == '<')
          --templatecount;
      else if (c == ' ' && templatecount == 0 && parencount == 0)
          break;

      --pos;
  }
  info = info.mid(pos + 1);

  // remove trailing '*', '&' that are part of the return argument
  while ((info.at(0) == '*')
         || (info.at(0) == '&'))
      info = info.mid(1);

  // we have the full function name now.
  // clean up the templates
  while ((pos = info.lastIndexOf('>')) != -1) {
      if (!info.contains('<'))
          break;

      // find the matching close
      int end = pos;
      templatecount = 1;
      --pos;
      while (pos && templatecount) {
          register char c = info.at(pos);
          if (c == '>')
              ++templatecount;
          else if (c == '<')
              --templatecount;
          --pos;
      }
      ++pos;
      info.remove(pos, end - pos + 1);
  }

  return info;
}


//! Returns the string to record to the logging target, formatted according to the format().
/**
 * \sa format()
 * \sa setFormat(const QString&)
 */
QString AbstractStringAppender::formattedString(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file,
                                                int line, const char* function, const QString& message) const
{
  QString f = format();
  const int size = f.size();

  QString result;

  int i = 0;
  while (i < f.size())
  {
    QChar c = f.at(i);

    // We will silently ignore the broken % marker at the end of string
    if (c != QLatin1Char(formattingMarker) || (i + 1) == size)
    {
      result.append(c);
    }
    else
    {
      QChar command = f.at(++i);

      // Check for the padding instruction
      int fieldWidth = 0;
      if (command.isDigit() || command.category() == QChar::Punctuation_Dash)
      {
        int j = 1;
        while ((i + j) < size && f.at(i + j).isDigit())
          j++;
        fieldWidth = f.mid(i, j).toInt();

        i += j;
        command = f.at(i);
      }

      // Log record chunk to insert instead of formatting instruction
      QString chunk;

      // Time stamp
      if (command == QLatin1Char('t'))
      {
        if (f.at(i + 1) == QLatin1Char('{'))
        {
          int j = 1;
          while ((i + 2 + j) < size && f.at(i + 2 + j) != QLatin1Char('}'))
            j++;

          if ((i + 2 + j) < size)
          {
            chunk = timeStamp.toString(f.mid(i + 2, j));

            i += j;
            i += 2;
          }
        }

        if (chunk.isNull())
          chunk = timeStamp.toString(QLatin1String("HH:mm:ss.zzz"));
      }

      // Log level
      else if (command == QLatin1Char('l'))
        chunk = Logger::levelToString(logLevel);

      // Uppercased log level
      else if (command == QLatin1Char('L'))
        chunk = Logger::levelToString(logLevel).toUpper();

      // Filename
      else if (command == QLatin1Char('F'))
        chunk = QLatin1String(file);

      // Filename without a path
      else if (command == QLatin1Char('f'))
        chunk = QString(QLatin1String(file)).section('/', -1);

      // Source line number
      else if (command == QLatin1Char('i'))
        chunk = QString::number(line);

      // Function name, as returned by Q_FUNC_INFO
      else if (command == QLatin1Char('C'))
        chunk = QString::fromLatin1(function);

      // Stripped function name
      else if (command == QLatin1Char('c'))
        chunk = stripFunctionName(function);

      // Log message
      else if (command == QLatin1Char('m'))
        chunk = message;

      // We simply replace the double formatting marker (%) with one
      else if (command == QLatin1Char(formattingMarker))
        chunk = QLatin1Char(formattingMarker);

      // Do not process any unknown commands
      else
      {
        chunk = QLatin1Char(formattingMarker);
        chunk.append(command);
      }

      result.append(QString(QLatin1String("%1")).arg(chunk, fieldWidth));
    }

    ++i;
  }

  return result;
}
