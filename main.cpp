#include "mainwindow.h"
#include "hashtable.hpp"
#include <QApplication>
#include "avltree3.hpp"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
