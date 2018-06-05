#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>

class Control: public QObject
{
    Q_OBJECT

public:
    Control(QObject *parent = 0);
    ~Control();

signals:
    void commitString(const QString &string);

public slots:
    void showKeyboard();
    void hideKeyboard();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
};

#endif // CONTROL_H
