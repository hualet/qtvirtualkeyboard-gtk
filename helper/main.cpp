#include <QGuiApplication>
#include <QDBusConnection>

#include "control.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    Control c;

    QDBusConnection::sessionBus().registerService("com.deepin.VirtualKeyboard");
    QDBusConnection::sessionBus().registerObject("/com/deepin/VirtualKeyboard",
                                                 "com.deepin.VirtualKeyboard", &c,
                                                 QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);

    return a.exec();
}
