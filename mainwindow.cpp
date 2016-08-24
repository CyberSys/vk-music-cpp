#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QUrlQuery>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // configure UI
    ui->setupUi(this);
    statusBar()->setSizeGripEnabled(false);

    // configure all connections
    connect(statusBar(), SIGNAL(messageChanged(QString)), this, SLOT(statusMessageChangedSlot(QString)));
    connect(this, SIGNAL(stateChangedSignal(State_t)), this, SLOT(stateChangedSlot(State_t)));

    // set initial state
    setState(NotStarted);
    setStatus("Starting...");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::stateChangedSlot(State_t iNewState)
{
    QPushButton *reconnectButton;
    switch (iNewState)
    {
        case NotStarted:
            requestToken();
            break;
        case TokenRequested: // nothing to be done here
            break;

        case TokenRecvd:
        break;

        case TokenFailed:
            cleanMainWidget();
            reconnectButton = new QPushButton("RECONNECT", ui->_mainWidget);
            connect (reconnectButton, SIGNAL(clicked(bool)), this, SLOT(requestToken()));
            reconnectButton->show();
        break;

        case AudioListRequested:
        case AudioListRcvd:
        case AudioListFailed:
        case AudioListDisplayed:
            qDebug()<< "Not implemented state was set: " << iNewState;
    }
}

void MainWindow::cleanMainWidget()
{
    while ( QWidget* w = ui->_mainWidget->findChild<QWidget*>() ) delete w;
}

// Token-related methods and slots


void MainWindow::requestToken()
{
    setState(TokenRequested);
    _authWebView = new QWebEngineView(ui->_mainWidget);
    connect(_authWebView, SIGNAL(loadStarted()), this, SLOT(tokenViewLoadStartedSlot()));
    connect(_authWebView, SIGNAL(loadProgress(int)), this, SLOT(tokenViewloadProgressSlot(int)));
    connect(_authWebView, SIGNAL(loadFinished(bool)), this, SLOT(tokenViewLoadFinishedSlot(bool)));
    connect(_authWebView, SIGNAL(urlChanged(QUrl)), this, SLOT(tokenViewUrlChangedSlot(QUrl)));

    _authWebView->load(QUrl("https://oauth.vk.com/authorize"
                    "?client_id=5601291"
                    "&display=page"
                    "&redirect_uri=https://oauth.vk.com/blank.html"
                    "&scope=friends"
                    "&response_type=token"
                    "&v=5.53"));
    //_authWebView->load(QUrl("https://blahblah.bl"));
    _authWebView->show();
}

void MainWindow::tokenViewLoadFinishedSlot(bool iResult)
{
    if (!iResult)
    {
        setStatus("Cannot connect to "+_authWebView->url().host());
        setState(TokenFailed);
    }
    else
    {
        setStatus("Loaded "+ _authWebView->url().host());
    }
}

void MainWindow::tokenViewUrlChangedSlot(const QUrl &url)
{
    qDebug()<<"New url: " << url.toString();
    QUrlQuery aQuery(url);
    if (aQuery.hasQueryItem("access_token"))
    {
        setStatus( "voila! we got the token!");
        _token = aQuery.queryItemValue("access_token");
        setState(TokenRecvd);
    }
    if(aQuery.hasQueryItem("error"))
    {
        _lastError = aQuery.queryItemValue("error");
        setStatus( "Error: " + _lastError);
        setState(TokenFailed);
    }
}
