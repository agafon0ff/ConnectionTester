#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcpserver.h"
#include "tcpsocket.h"
#include "udpsocket.h"

#include <QFontDatabase>
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QDebug>
#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(new JsonSettings(this)),
    m_scriptEditor(new ScriptEditor)
{
    ui->setupUi(this);
    m_scriptEditor->hide();

    loadFont();
    loadStyleSheet();
    setWindowIcon(QIcon(":/connection.ico"));

    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->tabWidgetCentral, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
    connect(ui->actionTcpServer, &QAction::triggered, [=]{addConnection(ConnectionTcpServer);});
    connect(ui->actionTcpSocket, &QAction::triggered, [=]{addConnection(ConnectionTcpSocket);});
    connect(ui->actionUdpSocket, &QAction::triggered, [=]{addConnection(ConnectionUdpSocket);});
    connect(ui->actionSessionSave, &QAction::triggered, this, &MainWindow::saveSession);
    connect(ui->actionSessionLoad, &QAction::triggered, this, &MainWindow::loadSession);
    connect(ui->menuRecentSessions, &QMenu::triggered, this, &MainWindow::onRecentSessionTriggered);

    connect(ui->menuScripts, &QMenu::aboutToShow, this, &MainWindow::onScriptsMenuRequested);
    connect(ui->actionScriptAdd, &QAction::triggered, this, &MainWindow::onScriptActionRequested);
    connect(ui->actionScriptEdit, &QAction::triggered, this, &MainWindow::onScriptActionRequested);
    connect(ui->actionScriptLoad, &QAction::triggered, this, &MainWindow::onScriptActionRequested);
    connect(ui->actionScriptSave, &QAction::triggered, this, &MainWindow::onScriptActionRequested);
    connect(ui->actionScriptRemove, &QAction::triggered, this, &MainWindow::onScriptActionRequested);

    loadSettings();
}

MainWindow::~MainWindow()
{
    delete m_settings;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    e->ignore();

    if (!checkSessionChanges()) return;

    closeAllTabs();
    saveSettings();

    e->accept();
}

void MainWindow::addConnection(int type)
{
    qDebug()<<"MainWindow::addConnection, type:"<<CONNECTION_TYPES.at(type);

    QJsonObject defaultSettings = m_settings->getJsonObject(KEY_DEFAULT);
    QJsonObject jSettings = defaultSettings.value(CONNECTION_TYPES.at(type)).toObject();
    NetSettingsStruct settings = m_settings->settingsFromJson(jSettings);
    createConnection(type, settings);
}

ConnectionWidget *MainWindow::createConnection(int type, const NetSettingsStruct &settings)
{
    qDebug()<<"MainWindow::addConnection, type:"<<CONNECTION_TYPES.at(type);

    NetConnection *netConnection = Q_NULLPTR;

    if (type == ConnectionTcpServer)
        netConnection = dynamic_cast<NetConnection*>(new TcpServer(this));
    else if (type == ConnectionTcpSocket)
        netConnection = dynamic_cast<NetConnection*>(new TcpSocket(this));
    else if (type == ConnectionUdpSocket)
        netConnection = dynamic_cast<NetConnection*>(new UdpSocket(this));

    if (!netConnection)
        return Q_NULLPTR;

    netConnection->setSettings(settings);

    ConnectionWidget *widget = new ConnectionWidget(this);
    widget->setType(netConnection->type());
    widget->setNetConnection(netConnection);
    widget->setSettings(settings);
    widget->setStyleString(m_styleString);
    widget->setScriptEditor(m_scriptEditor);

    connect(widget, &ConnectionWidget::defaultSettings, this, &MainWindow::setDefaultConnectionSettings);
    connect(widget, &ConnectionWidget::showScriptMenu, this, &MainWindow::onScriptsMenuRequested);

    ui->tabWidgetCentral->addTab(widget, netConnection->getTypeString());
    ui->tabWidgetCentral->setCurrentWidget(widget);

    return widget;
}

void MainWindow::onTabCloseRequested(int index)
{
    qDebug()<<"MainWindow::onTabCloseRequested"<<index;

    if (!checkSessionChanges()) return;
    closeConnectionTab(index);
}

void MainWindow::closeConnectionTab(int index)
{
    qDebug()<<"MainWindow::closeConnectionTab"<<index;

    ConnectionWidget *widget = dynamic_cast<ConnectionWidget*>(ui->tabWidgetCentral->widget(index));
    NetConnection *netConnection = widget->netConnection();

    netConnection->stop();
    widget->setNetConnection(Q_NULLPTR);
    ui->tabWidgetCentral->removeTab(index);

    delete widget;
    delete netConnection;
}

void MainWindow::setDefaultConnectionSettings(int type, const NetSettingsStruct &settings)
{
    QJsonObject defaultSettings = m_settings->getJsonObject(KEY_DEFAULT);
    QJsonObject jSettings = m_settings->jsonFromSettings(settings);
    defaultSettings.insert(CONNECTION_TYPES.at(type), jSettings);
    m_settings->setJsonObject(KEY_DEFAULT, defaultSettings);
}

void MainWindow::onScriptsMenuRequested()
{
    ConnectionWidget *widget = dynamic_cast<ConnectionWidget*>(ui->tabWidgetCentral->currentWidget());

    bool enableActions = widget ? true : false;
    ui->actionScriptAdd->setEnabled(enableActions);
    ui->actionScriptLoad->setEnabled(enableActions);
    ui->actionScriptEdit->setEnabled(enableActions);
    ui->actionScriptSave->setEnabled(enableActions);
    ui->actionDublicateScript->setEnabled(enableActions);
    ui->actionScriptRemove->setEnabled(enableActions);

    if(sender()->inherits("ConnectionWidget"))
        ui->menuScripts->exec(QCursor::pos());

    if (!enableActions) return;

    bool noScript = widget->currentScriptName().isEmpty();
    ui->actionScriptEdit->setEnabled(!noScript);
    ui->actionScriptSave->setEnabled(!noScript);
    ui->actionDublicateScript->setEnabled(!noScript);
    ui->actionScriptRemove->setEnabled(!noScript);
}

void MainWindow::onScriptActionRequested()
{
    ConnectionWidget *widget = dynamic_cast<ConnectionWidget*>(ui->tabWidgetCentral->currentWidget());
    if (!widget) return;

    QAction *action = dynamic_cast<QAction*>(sender());
    if (!action) return;

    if (action->objectName() == "actionScriptAdd")
        widget->onScriptAdd();
    else if (action->objectName() == "actionScriptLoad")
        widget->onScriptLoad();
    else if (action->objectName() == "actionScriptEdit")
        widget->onScriptEdit();
    else if (action->objectName() == "actionScriptSave")
        widget->onScriptSave();
    else if (action->objectName() == "actionDublicateScript")
        widget->onScriptDuplicate();
    else if (action->objectName() == "actionScriptRemove")
        widget->onScriptRemove();
}

void MainWindow::closeAllTabs()
{
    qDebug()<<"MainWindow::closeAllTabs";

    int tabsCount = ui->tabWidgetCentral->tabBar()->count();
    for (int i=0; i<tabsCount; ++i)
        closeConnectionTab(0);
}

bool MainWindow::checkSessionChanges()
{
    qDebug()<<"MainWindow::checkSessionChanges";
    bool result = true;
    bool isConnectionsChanged = false;
    int tabsCount = ui->tabWidgetCentral->tabBar()->count();
    for (int i=0; i<tabsCount; ++i)
    {
        ConnectionWidget *widget = dynamic_cast<ConnectionWidget*>(ui->tabWidgetCentral->widget(i));

        if (widget->isChanged())
        {
            isConnectionsChanged = true;
            break;
        }
    }

    if(isConnectionsChanged)
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle(tr("Session has been modified"));
        msgBox->setText(tr("Do you want to save session?"));
        msgBox->setIcon(QMessageBox::Question);

        QPushButton *saveButton = msgBox->addButton(tr("Save"), QMessageBox::YesRole);
        QPushButton *cancelButton = msgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);
        QPushButton *discardButton = msgBox->addButton(tr("Don't save"), QMessageBox::NoRole);

        saveButton->setIcon(QIcon(":/icon_ok.png"));
        cancelButton->setIcon(QIcon(":/icon_left.png"));
        discardButton->setIcon(QIcon(":/icon_cross.png"));
        discardButton->setMinimumWidth(100);

        msgBox->exec();

        if(msgBox->clickedButton() == saveButton)
        {
            result = saveSession();
        }
        else if(msgBox->clickedButton() == discardButton)
        {
            result = true;
        }
        else result = false;

        delete msgBox;
    }

    return result;
}

bool MainWindow::saveSession()
{
    qDebug()<<"MainWindow::saveSession";

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save session"),
        m_sessionsDir + "/" + "session.ctses",
        "*.ctses", 0, QFileDialog::DontUseNativeDialog);

    if (fileName.isEmpty()) return false;

    updateRecentSessions(fileName);

    QJsonObject jSession;
    QJsonArray jSessionTabs;
    int tabsCount = ui->tabWidgetCentral->tabBar()->count();

    for (int i=0; i<tabsCount; ++i)
    {
        ConnectionWidget *widget = dynamic_cast<ConnectionWidget*>(ui->tabWidgetCentral->widget(i));
        if (!widget) continue;

        NetConnection *netConnection = widget->netConnection();

        QJsonObject jConnection;
        jConnection.insert(KEY_CONNECTION_TYPE, CONNECTION_TYPES.at(netConnection->type()));
        jConnection.insert(KEY_SETTINGS, m_settings->jsonFromSettings(widget->settings()));

        QJsonArray jScriptsList;
        QStringList scriptNames = widget->scriptsNames();
        foreach (const QString &key, scriptNames)
        {
            QJsonObject jScript;
            jScript.insert(KEY_NAME, key);
            jScript.insert(KEY_TEXT, netConnection->scriptText(key));
            jScriptsList.append(jScript);
        }

        jConnection.insert(KEY_SCRIPTS, jScriptsList);
        jSessionTabs.append(jConnection);
        widget->applyChanges();
    }

    jSession.insert(KEY_CONNECTIONS, jSessionTabs);
    m_settings->saveJson(fileName, jSession);
    return true;
}

void MainWindow::loadSession()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open session"),
        m_sessionsDir, "*.ctses", 0, QFileDialog::DontUseNativeDialog);

    if (fileName.isEmpty()) return;

    qDebug()<<"MainWindow::loadSession"<<fileName;

    openSession(fileName);
}

void MainWindow::openSession(const QString &path)
{
    if (!checkSessionChanges()) return;
    updateRecentSessions(path);

    QJsonObject jSession = m_settings->loadJson(path);
    if (!jSession.contains(KEY_CONNECTIONS)) return;

    closeAllTabs();
    QJsonArray jSessionTabs = jSession.value(KEY_CONNECTIONS).toArray();
    foreach (const QJsonValue &jValue, jSessionTabs)
    {
        QJsonObject jConnection = jValue.toObject();
        QJsonObject jSettings = jConnection.value(KEY_SETTINGS).toObject();
        QString type = jConnection.value(KEY_CONNECTION_TYPE).toString();

        ConnectionWidget *widget = createConnection(CONNECTION_TYPES.indexOf(type),
                                                    m_settings->settingsFromJson(jSettings));

        QJsonArray jScriptsList = jConnection.value(KEY_SCRIPTS).toArray();
        foreach (const QJsonValue &jSriptValue, jScriptsList)
        {
            QJsonObject jScript = jSriptValue.toObject();
            QString name = jScript.value(KEY_NAME).toString();
            QString text = jScript.value(KEY_TEXT).toString();
            widget->addScriptItem(name, text);
        }

        widget->applyChanges();
    }
}

void MainWindow::updateRecentSessions(const QString &path)
{
    qDebug()<<"MainWindow::updateRecentSessions" << path;

    if(!path.isEmpty())
    {
        QFileInfo fInfo(path);
        m_sessionsDir = fInfo.dir().path();

        if (m_recentSessions.contains(path))
            m_recentSessions.removeOne(path);

        m_recentSessions.prepend(path);

        if(m_recentSessions.size() > 10)
            m_recentSessions.removeLast();
    }

    foreach (QAction *action, m_recentSessionActions)
        ui->menuRecentSessions->removeAction(action);

    m_recentSessionActions.clear();

    foreach (const QString &session, m_recentSessions)
        m_recentSessionActions.append(ui->menuRecentSessions->addAction(session));
}

void MainWindow::onRecentSessionTriggered(QAction *action)
{
    if (!action) return;

    qDebug()<<"MainWindow::onRecentSessionTriggered" << action->text();

    openSession(action->text());
}

void MainWindow::saveSettings()
{
    QMap<QString, QVariant> parameters;
    parameters.insert(KEY_GEOMETRY_MAIN, QString::fromLocal8Bit(saveGeometry().toBase64()));
    parameters.insert(KEY_GEOMETRY_EDITOR, QString::fromLocal8Bit(m_scriptEditor->saveGeometry().toBase64()));
    parameters.insert(KEY_SESSIONS_DIR, m_sessionsDir);
    parameters.insert(KEY_RECENT_SESSIONS, m_recentSessions);
    m_settings->setParameters(parameters);
}

void MainWindow::loadSettings()
{
    restoreGeometry(QByteArray::fromBase64(m_settings->getParameter(KEY_GEOMETRY_MAIN).toString().toLocal8Bit()));
    m_scriptEditor->restoreGeometry(QByteArray::fromBase64(m_settings->getParameter(KEY_GEOMETRY_EDITOR).toString().toLocal8Bit()));
    m_sessionsDir = m_settings->getParameter(KEY_SESSIONS_DIR).toString();
    m_recentSessions = m_settings->getParameter(KEY_RECENT_SESSIONS).toStringList();
    updateRecentSessions();
}

void MainWindow::loadFont()
{
#ifdef Q_OS_WIN
    int id = QFontDatabase::addApplicationFont(":/Calibri.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont  font = QFont(family,10,QFont::Medium);
    font.setPixelSize(15);
    QApplication::setFont(font);
#endif

#ifdef Q_OS_UNIX
    int id = QFontDatabase::addApplicationFont(":/Ubuntu-L.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont  font = QFont(family,10,QFont::Normal);
    font.setPixelSize(13);
    QApplication::setFont(font);
#endif
}

void MainWindow::loadStyleSheet()
{
    m_styleString.clear();
    QFile file(":/style.css");
    if(file.open(QIODevice::ReadOnly))
    {
        m_styleString.append(file.readAll());
        file.close();
    }

    if(m_styleString.isEmpty())
        return;

    setStyleSheet(m_styleString);
    m_scriptEditor->setStyleSheet(m_styleString);
}
