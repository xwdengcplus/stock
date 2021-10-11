#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QSettings>
#include "datamodel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
protected:
    virtual bool eventFilter(QObject *o, QEvent *e);

    void getData();
    bool IsRequestOrNot();
private:
    QNetworkRequest request;
    QNetworkProxy naProxy;
    QNetworkAccessManager* naManager;
    QSettings *settings;
    DataModel *model;
    bool isFirst{true};
};
#endif // MAINWINDOW_H
