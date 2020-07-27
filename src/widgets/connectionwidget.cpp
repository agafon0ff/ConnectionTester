#include "connectionwidget.h"
#include "ui_connectionwidget.h"
#include "scriptitemwidget.h"

#include <QDebug>
#include <QAction>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QListWidgetItem>

ConnectionWidget::ConnectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionWidget),
    m_netConnection(Q_NULLPTR),
    m_scriptEditor(Q_NULLPTR),
    m_menuScriptEdit(new QMenu(this)),
    m_isConnectionChanged(true)
{
    ui->setupUi(this);

    ui->splitterV->setSizes({800, 100});
    ui->splitterH->setSizes({800, 100});
    ui->btnStart->setText(KEY_START);
    ui->lineEditTcpServerPort->setValidator(new QIntValidator(10, 999999, this));
    ui->lineEditTcpSocketPort->setValidator(new QIntValidator(10, 999999, this));
    ui->lineEditUdpSocketPort->setValidator(new QIntValidator(10, 999999, this));
    ui->lineEditUdpSocketPortDst->setValidator(new QIntValidator(10, 999999, this));

    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(10);
    QFontMetrics metrics(font);
    ui->textEditOutput->setFont(font);
    ui->textEditOutput->setTabStopWidth(4 * metrics.width(' '));
    ui->textBrowserInput->setFont(font);
    ui->textBrowserInput->setTabStopWidth(4 * metrics.width(' '));

    connect(ui->btnClearInput, &QPushButton::clicked, ui->textBrowserInput, &QTextBrowser::clearHistory);
    connect(ui->btnClearInput, &QPushButton::clicked, ui->textBrowserInput, &QTextBrowser::clear);
    connect(ui->btnClearOutput, &QPushButton::clicked, ui->textEditOutput, &QTextEdit::clear);
    connect(ui->btnSend, &QPushButton::clicked, this, &ConnectionWidget::onSendClicked);
    connect(ui->btnScriptMenu, &QToolButton::pressed, this, &ConnectionWidget::showScriptMenu);

    connect(ui->checkBoxShowText, &QCheckBox::clicked, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->checkBoxShowHex, &QCheckBox::clicked, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->checkBoxAddNewLine, &QCheckBox::clicked, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->checkBoxAutoStart, &QCheckBox::clicked, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->lineEditTcpServerPort, &QLineEdit::editingFinished, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->lineEditTcpSocketHost, &QLineEdit::editingFinished, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->lineEditTcpSocketPort, &QLineEdit::editingFinished, this, &ConnectionWidget::onSettingsChanged);

    connect(ui->checkBoxUdpMulticast, &QCheckBox::clicked, ui->lineEditUdpSocketHost, &QLineEdit::setEnabled);
    connect(ui->checkBoxUdpMulticast, &QCheckBox::clicked, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->lineEditUdpSocketPort, &QLineEdit::editingFinished, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->lineEditUdpSocketPortDst, &QLineEdit::editingFinished, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->lineEditUdpSocketHost, &QLineEdit::editingFinished, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->lineEditUdpSocketHostDst, &QLineEdit::editingFinished, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->splitterH, &QSplitter::splitterMoved, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->splitterV, &QSplitter::splitterMoved, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->listWidgetScripts, &QListWidget::currentItemChanged, this, &ConnectionWidget::onCurrentItemChanged);

    m_menuScriptEdit->addAction(QIcon(":/icon_edit.png"),tr("Edit Script"),this, &ConnectionWidget::onScriptEdit);
    m_menuScriptEdit->addAction(QIcon(":/icon_save.png"),tr("Save Script"),this, &ConnectionWidget::onScriptSave);
    m_menuScriptEdit->addAction(QIcon(":/icon_copy.png"),tr("Duplicate Script"),this, &ConnectionWidget::onScriptDuplicate);
    m_menuScriptEdit->addAction(QIcon(":/icon_cross.png"),tr("Remove Script"),this, &ConnectionWidget::onScriptRemove);

    qDebug()<<"*ConnectionWidget";
}

ConnectionWidget::~ConnectionWidget()
{
    qDebug()<<"~ConnectionWidget";
    delete ui;
}

void ConnectionWidget::setType(int type)
{
    qDebug()<<"ConnectionWidget::setType: "<<type;

    if(type < 0 || type >= ui->stackedWidgetSettings->count())
        return;

    m_type = type;
    ui->stackedWidgetSettings->setCurrentIndex(type);
}

void ConnectionWidget::setNetConnection(NetConnection *pointer)
{
    qDebug()<<"ConnectionWidget::setNetConnection";

    if(!pointer)return;

    m_netConnection = pointer;

    connect(m_netConnection, &NetConnection::started,
            [=]{ui->btnStart->setIcon(QIcon(":/icon_stop.png")); ui->btnStart->setText(KEY_STOP);});

    connect(m_netConnection, &NetConnection::stopped,
            [=]{ui->btnStart->setIcon(QIcon(":/icon_play.png")); ui->btnStart->setText(KEY_START);});

    connect(ui->btnStart, &QPushButton::clicked,
            [=]{ui->btnStart->text() == KEY_START ? m_netConnection->start() : m_netConnection->stop();});

    connect(m_netConnection, &NetConnection::status, this, &ConnectionWidget::setStatusMessage);
    connect(m_netConnection, &NetConnection::datagram, this, &ConnectionWidget::setDatagram);
}

void ConnectionWidget::setSettings(const NetSettingsStruct &settings)
{
    qDebug()<<"ConnectionWidget::setSettings";

    if (!m_netConnection)
        return;

    if (m_type == ConnectionTcpServer)
    {
        ui->lineEditTcpServerPort->setText(QString::number(settings.port));
    }
    else if (m_type == ConnectionTcpSocket)
    {
        ui->lineEditTcpSocketHost->setText(settings.host);
        ui->lineEditTcpSocketPort->setText(QString::number(settings.port));
    }
    else if (m_type == ConnectionUdpSocket)
    {
        ui->lineEditUdpSocketHost->setText(settings.host);
        ui->lineEditUdpSocketHostDst->setText(settings.hostDst);
        ui->lineEditUdpSocketPort->setText(QString::number(settings.port));
        ui->lineEditUdpSocketPortDst->setText(QString::number(settings.portDst));
        ui->checkBoxUdpMulticast->setChecked(settings.useMulticast);

        ui->lineEditUdpSocketHost->setEnabled(ui->checkBoxUdpMulticast->isChecked());
    }

    ui->checkBoxAutoStart->setChecked(settings.autoStart);
    ui->checkBoxShowText->setChecked(settings.showText);
    ui->checkBoxShowHex->setChecked(settings.showHex);
    ui->checkBoxAddNewLine->setChecked(settings.addNewLine);

    if (ui->checkBoxAutoStart->isChecked())
        m_netConnection->start();

    m_scriptsDir = settings.scriptsDir;
    ui->splitterH->setSizes(QList<int>() << settings.splitterH.first << settings.splitterH.second);
    ui->splitterV->setSizes(QList<int>() << settings.splitterV.first << settings.splitterV.second);
}

NetSettingsStruct ConnectionWidget::settings()
{
    NetSettingsStruct netSettings = m_netConnection->settings();
    netSettings.splitterH = TwoNumbers(ui->splitterH->sizes().at(0),
                                       ui->splitterH->sizes().at(1));
    netSettings.splitterV = TwoNumbers(ui->splitterV->sizes().at(0),
                                       ui->splitterV->sizes().at(1));

    return netSettings;
}

bool ConnectionWidget::isChanged()
{
    return m_isConnectionChanged;
}

void ConnectionWidget::applyChanges()
{
    m_isConnectionChanged = false;
}

void ConnectionWidget::onScriptLoad()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open script"),
        m_scriptsDir, "*.js", 0, QFileDialog::DontUseNativeDialog);

    if (fileName.isEmpty()) return;

    QFileInfo fInfo(fileName);
    if(m_scriptsDir != fInfo.dir().path())
    {
        m_scriptsDir = fInfo.dir().path();
        onSettingsChanged();
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;

    QString scriptText = QString::fromUtf8(file.readAll());
    file.close();

    int step = 0;

    for (step; step<scriptText.size(); ++step)
        if(scriptText.at(step) == "\n")
            break;

    QString scriptName = scriptText.mid(0, step);
    if (scriptName.contains("/* ") && scriptName.contains(" */"))
    {
        if(scriptText.size() > step + 1)
            scriptText.remove(0, step + 1);

        scriptName.remove("/* ");
        scriptName.remove(" */");
    }
    else
    {
        QFileInfo fInfo(fileName);
        scriptName = fInfo.fileName();
        scriptName.remove(".js");
    }

    if (!scriptName.isEmpty() && !scriptText.isEmpty())
        addScriptItem(scriptName, scriptText);
}

void ConnectionWidget::onScriptSave()
{
    qDebug()<<"ConnectionWidget::onScriptSave";

    QListWidgetItem *listItem = ui->listWidgetScripts->currentItem();
    ScriptItemWidget *itemWidget = dynamic_cast<ScriptItemWidget*>(ui->listWidgetScripts->itemWidget(listItem));

    if(!itemWidget || !listItem) return;

    QString scriptName = itemWidget->name();
    QString scriptText = m_netConnection->scriptText(scriptName);

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save script"),
        m_scriptsDir + "/" + "script_" + scriptName + ".js",
        "*.js", 0, QFileDialog::DontUseNativeDialog);

    if (fileName.isEmpty()) return;

    QFileInfo fInfo(fileName);
    if(m_scriptsDir != fInfo.dir().path())
    {
        m_scriptsDir = fInfo.dir().path();
        onSettingsChanged();
    }

    scriptText.prepend("/* " + scriptName + " */\n");

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) return;

    file.write(scriptText.toUtf8());
    file.close();
}

void ConnectionWidget::onScriptAdd()
{
    if (!m_scriptEditor) return;

    m_scriptEditor->clear();
    if (m_scriptEditor->exec() != QDialog::Accepted)
        return;

    QString scriptText = m_scriptEditor->scriptText();
    QString scriptName = m_scriptEditor->scriptName();

    if (scriptName.isEmpty()) return;
    addScriptItem(scriptName, scriptText);
}

void ConnectionWidget::onScriptEdit()
{
    if (!m_scriptEditor) return;

    QListWidgetItem *listItem = ui->listWidgetScripts->currentItem();
    ScriptItemWidget *itemWidget = dynamic_cast<ScriptItemWidget*>(ui->listWidgetScripts->itemWidget(listItem));

    if(!itemWidget || !listItem) return;

    QString scriptName = itemWidget->name();
    QString scriptText = m_netConnection->scriptText(scriptName);

    m_scriptEditor->setScriptName(scriptName);
    m_scriptEditor->setScriptText(scriptText);

    if (m_scriptEditor->exec() != QDialog::Accepted) return;
    if(m_scriptEditor->scriptName().isEmpty()) return;

    m_netConnection->removeScript(scriptName);
    scriptName = m_scriptEditor->scriptName();
    scriptText = m_scriptEditor->scriptText();

    itemWidget->setName(scriptName);
    listItem->setText(scriptName);

    ScriptItem *script = m_netConnection->addScript(scriptName, scriptText);
    connect(itemWidget, &ScriptItemWidget::startClicked, script, &ScriptItem::startScript);
    connect(itemWidget, &ScriptItemWidget::stopClicked, script, &ScriptItem::stopScript);
    connect(script, &ScriptItem::started, itemWidget, &ScriptItemWidget::onStarted);
    connect(script, &ScriptItem::stopped, itemWidget, &ScriptItemWidget::onStopped);

    m_isConnectionChanged = true;
}

void ConnectionWidget::onScriptDuplicate()
{
    QListWidgetItem *listItem = ui->listWidgetScripts->currentItem();
    ScriptItemWidget *itemWidget = dynamic_cast<ScriptItemWidget*>(ui->listWidgetScripts->itemWidget(listItem));

    if(!itemWidget || !listItem) return;

    QString scriptName = itemWidget->name();
    QString scriptText = m_netConnection->scriptText(scriptName);

    if (scriptName.isEmpty()) return;
    addScriptItem(scriptName + '_', scriptText);
}

void ConnectionWidget::onScriptRemove()
{
    QListWidgetItem *currItem = ui->listWidgetScripts->currentItem();
    ScriptItemWidget *itemWidget = dynamic_cast<ScriptItemWidget*>(ui->listWidgetScripts->itemWidget(currItem));

    if(!currItem || !itemWidget)
        return;

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(tr("Delete script"));
    msgBox->setText(tr("Confirm script deletion:") + "      \n\"" + itemWidget->name() + "\"");
    msgBox->setIconPixmap(QPixmap(":/icon_cross.png"));
    QPushButton *okButton = msgBox->addButton(tr("OK"), QMessageBox::AcceptRole);
    QPushButton *cancelButton = msgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);

    okButton->setIcon(QIcon(":/icon_ok.png"));
    cancelButton->setIcon(QIcon(":/icon_cross.png"));

    if(msgBox->exec() == QMessageBox::AcceptRole)
    {
        m_netConnection->removeScript(itemWidget->name());
        ui->listWidgetScripts->removeItemWidget(currItem);
        itemWidget->disconnect();
        delete itemWidget;
        delete currItem;
        m_isConnectionChanged = true;

    }

    delete msgBox;

}

void ConnectionWidget::addScriptItem(const QString &name, const QString &text)
{
    ScriptItemWidget *itemWidget = new ScriptItemWidget(this);
    itemWidget->setName(name);
    itemWidget->setStyleSheet(m_styleString);
    itemWidget->initWidget();
    itemWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(itemWidget, &ScriptItemWidget::customContextMenuRequested,
            this, &ConnectionWidget::onScriptContextMenuRequested);

    QListWidgetItem *listItem = new QListWidgetItem(name);
    listItem->setSizeHint(itemWidget->sizeHint());
    listItem->setText(name);

    ui->listWidgetScripts->addItem(listItem);
    ui->listWidgetScripts->setItemWidget(listItem, itemWidget);

    ScriptItem *script = m_netConnection->addScript(name, text);
    connect(itemWidget, &ScriptItemWidget::startClicked, script, &ScriptItem::startScript);
    connect(itemWidget, &ScriptItemWidget::stopClicked, script, &ScriptItem::stopScript);
    connect(script, &ScriptItem::started, itemWidget, &ScriptItemWidget::onStarted);
    connect(script, &ScriptItem::stopped, itemWidget, &ScriptItemWidget::onStopped);

    m_isConnectionChanged = true;
}

QStringList ConnectionWidget::scriptsNames()
{
    QStringList result;

    for (int i=0; i<ui->listWidgetScripts->count(); ++i)
    {
        QListWidgetItem *currItem = ui->listWidgetScripts->item(i);
        ScriptItemWidget *itemWidget = dynamic_cast<ScriptItemWidget*>(ui->listWidgetScripts->itemWidget(currItem));
        if (!itemWidget) continue;

        result.append(itemWidget->name());
    }

    return result;
}

QString ConnectionWidget::currentScriptName()
{
    QString result;

    QListWidgetItem *item = ui->listWidgetScripts->currentItem();
    if (item) result = item->text();

    return result;
}

void ConnectionWidget::setStyleString(const QString &styleStr)
{
    m_styleString = styleStr;
    if (!m_styleString.isEmpty())
        setStyleSheet(m_styleString);
}

void ConnectionWidget::setStatusMessage(const QString &message, int type)
{
    QString color = "#eee";

    if (type == StatusOk)
        color = "#5BC791";
    else if (type == StatusError)
        color = "#CD6E6E";
    else if (type == StatusConsole)
        color = "#D9D98B";
    else if (type == StatusOutput)
        color = "#B7CEE5";

    ui->textBrowserInput->append("<font color=\"" + color + "\">" + message + "</font>");
}

void ConnectionWidget::setDatagram(const QByteArray &data, const QString &host, quint16 port)
{
    QString id = host + ":" + QString::number(port);

    if(ui->checkBoxShowText->isChecked())
        setStatusMessage(id + ": " + QString::fromUtf8(data), StatusInput);

    if(ui->checkBoxShowHex->isChecked())
    {
        QString inputHex;
        for (int i=0; i<data.count(); ++i)
            inputHex.append(QString("|%1").arg(static_cast<quint8>(data.at(i)),0,16));

        inputHex.append("|");
        setStatusMessage(id + ":", StatusInput);
        setStatusMessage(inputHex, StatusInput);
    }

    if(ui->checkBoxAddNewLine->isChecked())
        ui->textBrowserInput->append("");
}

void ConnectionWidget::onScriptContextMenuRequested(const QPoint &pos)
{
    m_menuScriptEdit->exec(QCursor::pos());
}

void ConnectionWidget::onSettingsChanged()
{
    if (!m_netConnection) return;

    NetSettingsStruct settings;
    settings.autoStart = ui->checkBoxAutoStart->isChecked();
    settings.showText = ui->checkBoxShowText->isChecked();
    settings.showHex = ui->checkBoxShowHex->isChecked();
    settings.addNewLine = ui->checkBoxAddNewLine->isChecked();
    settings.scriptsDir = m_scriptsDir;
    settings.splitterH = TwoNumbers(ui->splitterH->sizes().at(0), ui->splitterH->sizes().at(1));
    settings.splitterV = TwoNumbers(ui->splitterV->sizes().at(0), ui->splitterV->sizes().at(1));

    if (m_type == ConnectionTcpServer)
    {
        settings.port = static_cast<quint16>(ui->lineEditTcpServerPort->text().toInt());
    }
    else if (m_type == ConnectionTcpSocket)
    {
        settings.port = static_cast<quint16>(ui->lineEditTcpSocketPort->text().toInt());
        settings.host = ui->lineEditTcpSocketHost->text();
    }
    else if (m_type == ConnectionUdpSocket)
    {
        settings.port = static_cast<quint16>(ui->lineEditUdpSocketPort->text().toInt());
        settings.host = ui->lineEditUdpSocketHost->text();
        settings.portDst = static_cast<quint16>(ui->lineEditUdpSocketPortDst->text().toInt());
        settings.hostDst = ui->lineEditUdpSocketHostDst->text();
        settings.useMulticast = ui->checkBoxUdpMulticast->isChecked();
    }

    m_netConnection->setSettings(settings);
    emit defaultSettings(m_type, settings);
}

void ConnectionWidget::onSendClicked()
{
    if(!m_netConnection)
        return;

    QString text = ui->textEditOutput->toPlainText();
    QString host = m_netConnection->settings().hostDst;
    quint16 port = m_netConnection->settings().portDst;
    m_netConnection->sendDatagram(text.toUtf8(), host, port);

    setStatusMessage("← " + text, StatusOutput);
}

void ConnectionWidget::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ScriptItemWidget *currWidget = dynamic_cast<ScriptItemWidget*>(ui->listWidgetScripts->itemWidget(current));
    if (currWidget) currWidget->setActive(true);

    ScriptItemWidget *prewWidget = dynamic_cast<ScriptItemWidget*>(ui->listWidgetScripts->itemWidget(previous));
    if (prewWidget) prewWidget->setActive(false);
}
