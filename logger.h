#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <iostream>
#include <fstream>

class logger : public QObject
{ Q_OBJECT

public:
    logger();
    ~logger();
    void UIAction(QString msg); // Debug: records user input
    void systemMessage(QString msg); // System messages
    void errorMessage(QString msg); // Error messages
    void debugMessage(QString msg); // Debug messages
    void statMessage(QString msg); // Performance Statistics
    void msg(QString msg); // etc message

public slots:
    void makeLog(std::string input,std::string logMode);

private:
    void writeLogEntry(QString message, std::string messageType);
};

#endif // LOGGER_H
