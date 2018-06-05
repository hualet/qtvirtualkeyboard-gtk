#include "control.h"

#include <QApplication>

#include <QEvent>
#include <QDebug>
#include <QLineEdit>

#include <private/qguiapplication_p.h>

#include <qpa/qplatformintegration.h>


Control::Control(QObject *parent)
    :QObject(parent),
      m_edit(new QLineEdit),
      m_imContext(nullptr)
{
    imContext()->setFocusObject(m_edit);

    qApp->installEventFilter(this);
    connect(qApp, &QGuiApplication::focusObjectChanged,
            this, [this](QObject *) {
        imContext()->setFocusObject(m_edit);
    });
}

Control::~Control()
{

}

void Control::ShowKeyboard()
{
    qDebug() << "ShowKeyboard";

    imContext()->showInputPanel();
}

void Control::HideKeyboard()
{
    qDebug() << "HideKeyboard";

    imContext()->hideInputPanel();
}

bool Control::event(QEvent *event)
{
    if (event->type() == QEvent::InputMethod) {
        QInputMethodEvent *im = static_cast<QInputMethodEvent*>(event);
        if (!im->commitString().isEmpty())
            emit Commit(im->commitString());
    }

    return QObject::event(event);
}

QPlatformInputContext *Control::imContext()
{
    if (!m_imContext)
        m_imContext = QGuiApplicationPrivate::platformIntegration()->inputContext();

    return m_imContext;
}
