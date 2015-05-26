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
#include "FileAppender.h"

// Qt
#include <QDir>

// STL
#include <iostream>

/**
 * \class FileAppender
 *
 * \brief Simple appender that writes the log records to the plain text file.
 */


//! Constructs the new file appender assigned to file with the given name.
FileAppender::FileAppender(const QString& fileName)
{
  setFileName(fileName);
}

//! Constructs a new file appender, which takes care of log file rotation.
/**
 * Creates a log file with the prefix <tt>logFilePrefix</tt>, the current timestamp and the filename extension '.log',
 * for example: prefix_2013-10-10_11.55.17.log
 *
 * If the directory <tt>directory</tt> does not exist it will be created.
 * If there are more than <tt>maxNrOfLogFiles</tt> log files in the directory <tt>directory</tt>,
 * the oldest log files are deleted.
 *
 * \param logFilePrefix - a common prefix for all log files
 * \param directory - the directory where to save the log files
 * \param maxNrOfLogFiles - the maximum number of log files to keep
 */
FileAppender::FileAppender(const QString& logFilePrefix, const QString& directory, quint32 maxNrOfLogFiles)
{
  if(maxNrOfLogFiles < 1)
  {
    std::cerr << "<FileAppender::FileAppender> 'maxNrOfLogFiles' has to be greater than '1'" << std::endl;
    return;
  }

  QDir dir(directory);

  if(!dir.exists() && !dir.mkpath(directory))
  {
    std::cerr << "<FileAppender::FileAppender> Cannot create the log file directory " << qPrintable(directory) << std::endl;
    return;
  }

  // get current log file count
  dir.setSorting(QDir::Name | QDir::Reversed);
  dir.setFilter(QDir::Files);
  dir.setNameFilters(QStringList() << logFilePrefix + "*.log");
  QFileInfoList logFiles = dir.entryInfoList();

  // keep only the latest 'maxNrOfLogFiles - 1' files and delete all others
  for(qint32 i = maxNrOfLogFiles - 1; i < logFiles.count(); ++i)
  {
    bool ret = QFile::remove(logFiles.at(i).absoluteFilePath());
    Q_ASSERT(ret);
  }

  setFileName(directory + QDir::separator() + logFilePrefix + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss") + ".log");
}

FileAppender::~FileAppender()
{
  closeFile();
}


//! Returns the name set by setFileName() or to the FileAppender constructor.
/**
 * \sa setFileName()
 */
QString FileAppender::fileName() const
{
  QMutexLocker locker(&m_logFileMutex);
  return m_logFile.fileName();
}


//! Sets the name of the file. The name can have no path, a relative path, or an absolute path.
/**
 * \sa fileName()
 */
void FileAppender::setFileName(const QString& s)
{
  QMutexLocker locker(&m_logFileMutex);
  if (m_logFile.isOpen())
    m_logFile.close();

  m_logFile.setFileName(s);
}


bool FileAppender::openFile()
{
  bool isOpen = m_logFile.isOpen();
  if (!isOpen)
  {
    isOpen = m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    if (isOpen)
      m_logStream.setDevice(&m_logFile);
    else
      std::cerr << "<FileAppender::append> Cannot open the log file " << qPrintable(m_logFile.fileName()) << std::endl;
  }
  return isOpen;
}


//! Write the log record to the file.
/**
 * \sa fileName()
 * \sa AbstractStringAppender::format()
 */
void FileAppender::append(const QDateTime& timeStamp, Logger::LogLevel logLevel, const char* file, int line,
                          const char* function, const QString& category, const QString& message)
{
  QMutexLocker locker(&m_logFileMutex);

  if (openFile())
  {
    m_logStream << formattedString(timeStamp, logLevel, file, line, function, category, message);
    m_logStream.flush();
    m_logFile.flush();
  }
}


void FileAppender::closeFile()
{
  QMutexLocker locker(&m_logFileMutex);
  m_logFile.close();
}
