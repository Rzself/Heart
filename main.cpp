#include "Heart.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Heart w;
    w.show();

    return a.exec();
}
