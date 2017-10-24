#include "logger.h"
#include "QCoreApplication"
#include "QDateTime"
#include "algorithm"
#include "string.h"

std::string directory; // global variable to hold directory, written in makeLog();
std::ofstream of; // output file
std::string mode; // log mode (debug, distribution, etc.) [[not implemented]]

logger::logger()
{

}

logger::~logger()
{

}

void logger::makeLog(std::string input, std::string logMode) // input is "Liburdi System Log: "
{
    mode = logMode;
    // CREATES A NEW LOG BASED ON TIMESTAMP (in a relative path)
    QString qdir = QCoreApplication::applicationDirPath(); // fetch directory
    std::string dir = qdir.toStdString(); // conv dir to std::string
    dir.append("/logs/"); // add log path to directory
    QString qtime = QDateTime::currentDateTime().toString(); // fetch time
    std::string time = qtime.toStdString(); //  time -> std::string

    std::string logname = "log-" + mode + "-";
    logname.append(time); // add date+time to filename
    dir.append(logname + ".txt"); // add filename to path
    directory = dir; // global

    // Write to log
    std::ofstream outputFile(dir); // make file
    if (outputFile.is_open())
    {
        outputFile << input;
        outputFile << time << std::endl;
        outputFile << "Working Directory: ";
        outputFile << dir << std::endl;

        if (mode == "debug")
        {
            outputFile << "Mode: DEBUG MODE\n" << std::endl;
        }
        else
        {
            outputFile << "Mode: NORMAL MODE\n" << std::endl;
        }
        outputFile << "--------------------------------------------------\n" << std::endl;
        outputFile.close();
    }

    else
    {
        qDebug("FAILED TO CREATE LOG FILE!");
    }
}

void logger::UIAction(QString msg)
{
    if(mode == "debug")
        writeLogEntry(msg, "UI ACTION");
}

void logger::systemMessage(QString msg)
{
    writeLogEntry(msg, "SYSTEM");
}

void logger::errorMessage(QString msg)
{
    writeLogEntry(msg, "ERROR");
}

void logger::debugMessage(QString msg)
{
    if(mode == "debug")
        writeLogEntry(msg, "DEBUG");
}

void logger::statMessage(QString msg)
{
    writeLogEntry(msg, "STAT");
}

void logger::msg(QString msg)
{
    writeLogEntry(msg, "MSG");
}

void logger::writeLogEntry(QString message, std::string messageType)
{
    QString qtime = QTime::currentTime().toString(); // fetch time
    std::string time = qtime.toStdString();

    std::string msg = message.toStdString();
    std::ofstream outputFile(directory, std::ios_base::app); // make file
    if (outputFile.is_open())
    {
        outputFile << "[" + messageType + "] " + time << std::endl;
        outputFile << msg << std::endl;
        outputFile << "\n";
        outputFile.close();
    }

    else
    {
        qDebug("FAILED TO WRITE LOG ENTRY!");
        qDebug(directory.std::string::c_str());
    }
}
