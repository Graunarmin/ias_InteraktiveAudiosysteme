#include "QCoreApplication"
#include <iostream>
#include "QUdpSocket"
#include <string>

#include "QTextStream"
#include "QDebug"

void doCpp()
{
    std::string name;
    std::cout << "StdLib: Please enter your name: ";
    std::getline(std::cin, name);
    std::cout << "Hello " << name << "\n";
}

void doQt()
{
    QTextStream qin(stdin);
    QTextStream qout(stdout);

    // Stream the string into the variable
    qout << "Qt: Please enter your name: ";
    // Then tell the operating system to "flush it out" to the console
    qout.flush();

    QString name = qin.readLine();

    qout << "Hello " << name << "\n";
    qout.flush();
}

void doMixed()
{
    QTextStream qin(stdin);
    qInfo() << "Your name: ";
    QString name = qin.readLine();
    qInfo() << "Hello" << name;

}