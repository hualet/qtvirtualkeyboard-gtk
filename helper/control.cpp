#include "control.h"

#include <QGuiApplication>

#include <QEvent>

#include <private/qguiapplication_p.h>

#include <qpa/qplatformintegration.h>
#include <qpa/qplatforminputcontext.h>

Control::Control(QObject *parent)
    :QObject(parent)
{
    qApp->installEventFilter(this);
    connect(qApp, &QGuiApplication::focusObjectChanged,
            this, [this](QObject *) {
        QGuiApplicationPrivate::platformIntegration()->inputContext()->setFocusObject(this);
    });
}

Control::~Control()
{

}

void Control::showKeyboard()
{
    QGuiApplicationPrivate::platformIntegration()->inputContext()->showInputPanel();
}

void Control::hideKeyboard()
{
    QGuiApplicationPrivate::platformIntegration()->inputContext()->hideInputPanel();
}

bool Control::event(QEvent *event)
{
    if (event->type() == QEvent::InputMethod) {
        QInputMethodEvent *im = static_cast<QInputMethodEvent*>(event);
        if (!im->commitString().isEmpty())
            emit commitString(im->commitString());
    }

    return QObject::event(event);
}
