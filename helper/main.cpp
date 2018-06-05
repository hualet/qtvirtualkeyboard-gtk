#include <QApplication>
#include <QDBusConnection>

#include "control.h"
#include "virtualkeyboard.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    Control c;
    VirtualKeyboard vk(&c);

    bool result;
    QDBusConnection session = QDBusConnection::sessionBus();

    result = session.registerService("com.deepin.VirtualKeyboard");
    if (!result) {
        qWarning() << "register service failed";
        return -1;
    }

    result = session.registerObject("/com/deepin/VirtualKeyboard", &c);
    if (!result) {
        qWarning() << "register object failed";
        return -1;
    }

    return a.exec();
}
