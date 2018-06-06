#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include <QLineEdit>

#include <qpa/qplatforminputcontext.h>

class Control: public QObject
{
    Q_OBJECT

public:
    Control(QObject *parent = 0);
    ~Control();

    QPlatformInputContext *imContext();

signals:
    void Commit(const QString &string);
    void ForwardKey(const QString &key);

public slots:
    void ShowKeyboard();
    void HideKeyboard();

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    QLineEdit *m_edit;
    QWindow *m_focusWindow;
    QPlatformInputContext *m_imContext;

    QString m_previousCommit;
};

#endif // CONTROL_H
