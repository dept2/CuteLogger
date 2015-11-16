#include <QCoreApplication>

#include <Logger.h>
#include <ConsoleAppender.h>

int main(int argc, char* argv[])
{
//    QCoreApplication app(argc, argv);
    // ...
    ConsoleAppender* consoleAppender = new ConsoleAppender();
    consoleAppender->setFormat("[%{type}] <%{function}> %{message}\n");
    logger->registerAppender(consoleAppender);
    // ...
    LOG_INFO("Starting the application");
//    int result = app.exec();
    // ...
//    if (result)
    LOG_WARNING() << "Something went wrong.";

//    return result;
    return EXIT_SUCCESS;
}
