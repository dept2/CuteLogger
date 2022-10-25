#ifndef SIGNALAPPENDER_H
#define SIGNALAPPENDER_H

#include <AbstractStringAppender.h>
#include <QObject>

class SignalAppender
  : public QObject
  , public AbstractStringAppender
{
  Q_OBJECT
public:
  SignalAppender();
  ~SignalAppender();

signals:
  //! This signal is sent whenever a message is sent from the code.
  void writeMessage(const QString& message);

  // AbstractAppender interface
protected:
  //! appends the formatted message and sends it off using the writeMessage
  //! signal.
  void append(const QDateTime& timeStamp,
              Logger::LogLevel logLevel,
              const char* file,
              int line,
              const char* function,
              const QString& category,
              const QString& message) override;
};

#endif // SIGNALAPPENDER_H
