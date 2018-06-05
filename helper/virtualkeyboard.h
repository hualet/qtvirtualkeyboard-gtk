/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -a virtualkeyboard -c VirtualKeyboard ../interface/com.deepin.VirtualKeyboard.xml
 *
 * qdbusxml2cpp is Copyright (C) 2016 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.VirtualKeyboard
 */
class VirtualKeyboard: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.VirtualKeyboard")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.VirtualKeyboard\">\n"
"    <method name=\"ShowKeyboard\"/>\n"
"    <method name=\"HideKeyboard\"/>\n"
"    <signal name=\"Commit\">\n"
"      <arg type=\"s\" name=\"String\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    VirtualKeyboard(QObject *parent);
    virtual ~VirtualKeyboard();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void HideKeyboard();
    void ShowKeyboard();
Q_SIGNALS: // SIGNALS
    void Commit(const QString &String);
};

#endif