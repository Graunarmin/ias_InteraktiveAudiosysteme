#include "QCoreApplication"
#include <iostream>
#include "QUdpSocket"
#include <string>

#include "QTextStream"
#include "QDebug"
/// Tutorial: https://www.youtube.com/watch?v=smQms-2yJYc&list=PLUbFnGajtZlXbrbdlraCe3LMC_YH5abao&index=5

/*!
 * \fn doCpp()
 *
 * Writes a string to console and reads in user input the standard library way.
 */
void doCpp()
{
    std::string name;
    std::cout << "StdLib: Please enter your name: ";
    std::getline(std::cin, name);
    std::cout << "Hello " << name << "\n";
}

/*!
 * \fn doQt()
 *
 * Writes a string to console and reads in user input the Qt-way.
 */
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

/*!
 * \fn doMixed()
 *
 * Uses QTextStream and QDebug to write output and read input
 */
void doMixed()
{
    QTextStream qin(stdin);
    qInfo() << "Your name: ";
    QString name = qin.readLine();
    qInfo() << "Hello" << name;

}
