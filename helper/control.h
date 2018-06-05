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

public slots:
    void ShowKeyboard();
    void HideKeyboard();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
    QLineEdit *m_edit;
    QPlatformInputContext *m_imContext;
};

#endif // CONTROL_H
