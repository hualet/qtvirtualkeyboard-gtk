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
      m_focusWindow(new QWindow),
      m_imContext(nullptr)
{
    QGuiApplicationPrivate::focus_window = m_focusWindow;

    m_edit->installEventFilter(this);
    imContext()->setFocusObject(m_edit);
    QGuiApplicationPrivate::focus_window->installEventFilter(this);

    connect(qApp, &QApplication::focusObjectChanged,
            this, [this](QObject *o) {
        if (o != m_edit) {
            imContext()->setFocusObject(m_edit);
        }
    });
    connect(qApp, &QApplication::focusWindowChanged,
            this, [this](QWindow *w) {
        if (w != m_focusWindow) {
            QGuiApplicationPrivate::focus_window = m_focusWindow;
        }
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

bool Control::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_edit) {
        if (event->type() == QEvent::InputMethod) {
            QInputMethodEvent *im = static_cast<QInputMethodEvent*>(event);
            if (!im->commitString().isEmpty() && m_previousCommit != im->commitString()) {
                m_previousCommit = im->commitString();
                emit Commit(im->commitString());
            }
        }
    } else if (watched == m_focusWindow) {
        if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *key = static_cast<QKeyEvent*>(event);
            if (!key->text().isEmpty()) {
                emit Commit(key->text());
            } else if (key->key() == Qt::Key_Backspace) {
                emit Backspace();
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

QPlatformInputContext *Control::imContext()
{
    if (!m_imContext)
        m_imContext = QGuiApplicationPrivate::platformIntegration()->inputContext();

    return m_imContext;
}
