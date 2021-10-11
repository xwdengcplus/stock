#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QEvent>
#include <QDebug>
#include <QTableView>
#include <QTextCodec>
#include <QPushButton>
#include <QLineEdit>
#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "stock", "stock");
    //qDebug() << settings->fileName();

    setCentralWidget(new QWidget());
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(2);
    main_layout->setSpacing(2);
    centralWidget()->setLayout(main_layout);
    QTableView *table = new QTableView;
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    model = new DataModel;
    model->SetHeader({"名称","涨幅","现价"});
    table->setModel(model);
    table->setColumnWidth(0, 100);
    table->setColumnWidth(1, 70);
    table->setColumnWidth(2, 90);
    main_layout->addWidget(table);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(5);
    layout->setSpacing(5);
    QLineEdit *edit = new QLineEdit;
    QPushButton *add = new QPushButton("+");
    QPushButton *minus = new QPushButton("-");
    QPushButton *setting = new QPushButton;
    QPushButton *refresh = new QPushButton;
    add->setEnabled(false);
    edit->setFixedWidth(100);
    add->setFixedSize(40,30);
    minus->setFixedSize(40,30);
    setting->setIcon(QIcon(":/setting.png"));
    setting->setStyleSheet("QPushButton{border:none;}");
    refresh->setIcon(QIcon(":/refresh.png"));
    refresh->setStyleSheet("QPushButton{border:none;}");
    layout->addWidget(edit);
    layout->addWidget(add);
    layout->addWidget(minus);
    layout->addStretch();
    layout->addWidget(setting);
    layout->addWidget(refresh);
    main_layout->addLayout(layout);
    setFixedSize(270,400);

    //qDebug()<<"QSslSocket="<<QSslSocket::sslLibraryBuildVersionString();

    connect(edit, &QLineEdit::textChanged, this, [=](const QString &text){
        if (text.size() == 6)
            add->setEnabled(true);
        else
            add->setEnabled(false);
    });

    connect(add, &QPushButton::clicked, this, [=](bool k){
        QString text = edit->text();
        if (text.size() == 6){
            QString prefix = "s_sz";

            if (text.startsWith("5") ||text.startsWith("6"))
            {
                prefix = "s_sh";
            }
            auto tmp = settings->value("common/code").toStringList();
            tmp.append(prefix+text);
            settings->setValue("common/code",tmp);
            settings->sync();
            isFirst = true;
            edit->clear();
            getData();
        }
    });

    connect(minus, &QPushButton::clicked, this, [=](bool k){
        auto code = settings->value("common/code").toStringList();
        QStringList val;
        for (auto i=0;i<code.size();i++) {
            if (i != table->currentIndex().row())
                val << code.at(i);
        }
        settings->setValue("common/code",val);
        settings->sync();
        isFirst = true;
        getData();
    });


    //setWindowFlags(Qt::FramelessWindowHint);
    move(this->screen()->geometry().width()-270, 0);

    naManager = new QNetworkAccessManager(this);
    auto proxy_switch = settings->value("proxy/switch").toBool();
    auto proxy_host = settings->value("proxy/host").toString();
    auto proxy_port = settings->value("proxy/port").toUInt();

    // config
    QDialog *dlg = new QDialog;
    dlg->setFixedSize(260, 100);
    QVBoxLayout *dlg_ly = new QVBoxLayout;
    QHBoxLayout *proxy = new QHBoxLayout;
    QCheckBox *py_switch = new QCheckBox("代理");
    QLineEdit *py_host = new QLineEdit("127.0.0.1");
    QLineEdit *py_port = new QLineEdit("1080");
    proxy->addWidget(py_switch);
    proxy->addWidget(new QLabel("IP"));
    proxy->addWidget(py_host, 10);
    proxy->addWidget(new QLabel("端口"));
    proxy->addWidget(py_port, 5);
//    QHBoxLayout *fresh = new QHBoxLayout;
//    QCheckBox *fsh_auto = new QCheckBox("自动刷新");
//    QLineEdit *fsh_time = new QLineEdit;
//    fresh->addWidget(fsh_auto);
//    fresh->addWidget(fsh_time);
//    fresh ->addWidget(new QLabel("间隔(秒)"));
    dlg_ly->addLayout(proxy);
//    dlg_ly->addLayout(fresh);
    dlg->setLayout(dlg_ly);
    py_switch->setCheckState(settings->value("proxy/switch").toBool()? Qt::Checked : Qt::Unchecked);
    py_host->setText(settings->value("proxy/host").toString());
    py_port->setText(settings->value("proxy/port").toString());


    naProxy.setPort(proxy_port);
    naProxy.setHostName(proxy_host);
    naProxy.setType(proxy_switch ? QNetworkProxy::Socks5Proxy : QNetworkProxy::NoProxy);
    naManager->setProxy(naProxy);

    connect(py_switch, &QCheckBox::stateChanged, this, [=](int k){
        bool py_sh = (k!=0);
        settings->setValue("proxy/switch", py_sh);
        settings->sync();
        if (py_sh)
            naProxy.setType(QNetworkProxy::Socks5Proxy);
        else
            naProxy.setType(QNetworkProxy::NoProxy);

    });
    connect(py_host, &QLineEdit::textChanged, this, [=](const QString &k){
        settings->setValue("proxy/host", k);
        settings->sync();
    });
    connect(py_port, &QLineEdit::textChanged, this, [=](const QString &k){
        settings->setValue("proxy/port", k);
        settings->sync();
    });

    connect(setting, &QPushButton::clicked, this, [=](bool k){
        Q_UNUSED(k)
        dlg->exec();
        this->showNormal();
    });

    connect(refresh, &QPushButton::clicked, this, [=](bool k){
        Q_UNUSED(k)
        getData();
    });

    connect(naManager, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply){
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(!statusCode.isValid()) {
            //qDebug() <<"-----" << statusCode;
            return;
        }

        QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        if(!reason.isValid())
            return;

        QNetworkReply::NetworkError err = reply->error();
        if(err != QNetworkReply::NoError) {
            //qDebug() << err;
            return;
        }
        else {
            auto text = QTextCodec::codecForName("GBK")->toUnicode(reply->readAll());
            auto group = text.split(";");
            QList<QStringList> data;
            for(auto v : group) {
                auto content = v.split("\"");
                if (content.length() > 1) {
                    auto info = content[1].split( "~");
                    if (info.size() > 6) {
                        QStringList tmp{info[1], info[5], info[3] };
                        data.append(tmp);
                    }
                }
            }
            model->UpdateData(data);
        }
    });

    QWidget::installEventFilter(this);
}

MainWindow::~MainWindow()
{
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if( watched == this )
    {
        if(QEvent::WindowDeactivate == event->type())
        {
            //setFixedSize(1,1);
            this->showMinimized();
            return true;
        } else if (QEvent::WindowActivate == event->type())
        {
            //setFixedSize(250,400);
            this->showNormal();
            getData();
            return true;
        }
        else
        {
            return false ;
        }
    }
    return false;
}

void MainWindow::getData()
{
    if (isFirst || IsRequestOrNot()) {
        if (isFirst) {
            isFirst = false;
        }

        QString rand("0.");
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        for(int i=0; i<8; i++)
        {
            rand+=QString::number(qrand()%10);
        }
        auto code = settings->value("common/code");
        if (code.toStringList().size() == 0)
            return;

        QUrl url = QString("https://qt.gtimg.cn/r=%1&q=%2").arg(rand).arg(code.toStringList().join(","));
        request.setUrl(url);
        //qDebug() << url;
        naManager->get(request);
    }
}

bool MainWindow::IsRequestOrNot()
{
    QTime t = QTime::currentTime();

    auto hour = t.hour();
    auto minute = t.minute();
    if (hour < 9 || hour > 14 || hour == 12) {
        return false;
    }
    if (hour == 9) {
        if (minute <= 29) {
            return false;
        } else {
            return true;
        }
    } else if (hour == 11) {
        if (minute <= 30) {
            return true;
        } else {
            return false;
        }
    }
    return true;
}
