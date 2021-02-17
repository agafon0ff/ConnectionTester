#include "connectionwidget.h"
#include "ui_connectionwidget.h"
#include "scriptitemwidget.h"
#include "serialport.h"

#include <QtGlobal>
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
    ui->lineEditTcpServerPort->setValidator(new QIntValidator(1, 999999, this));
    ui->lineEditTcpSocketPort->setValidator(new QIntValidator(1, 999999, this));
    ui->lineEditUdpSocketPort->setValidator(new QIntValidator(1, 999999, this));
    ui->lineEditUdpSocketPortDst->setValidator(new QIntValidator(1, 999999, this));

    ui->comboBoxPortName->addItems(SerialPort::availablePorts());
    ui->comboBoxBaudrate->addItems({"1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200"});

    QFont font = QApplication::font();
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(9);
    QFontMetrics metrics(font);

    ui->textEditOutput->setFont(font);
    ui->textBrowserInput->setFont(font);

#if QT_VERSION > 0x050906
    int tabStopDistance = 4 * metrics.horizontalAdvance(' ');
    ui->textEditOutput->setTabStopDistance(tabStopDistance);
    ui->textBrowserInput->setTabStopDistance(tabStopDistance);
#else
    int tabStopDistance = 4 * metrics.width(' ');
    ui->textEditOutput->setTabStopWidth(tabStopDistance);
    ui->textBrowserInput->setTabStopWidth(tabStopDistance);
#endif

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

    connect(ui->comboBoxPortName, &QComboBox::currentTextChanged, this, &ConnectionWidget::onSettingsChanged);
    connect(ui->comboBoxBaudrate, &QComboBox::currentTextChanged, this, &ConnectionWidget::onSettingsChanged);

    connect(ui->listWidgetScripts, &QListWidget::currentItemChanged, this, &ConnectionWidget::onCurrentItemChanged);
    connect(ui->listWidgetScripts, &QListWidget::itemDoubleClicked, this, &ConnectionWidget::onItemDoubleClicked);

    m_menuScriptEdit->addAction(QIcon(":/icon_edit.png"),tr("Edit Script"),this, &ConnectionWidget::onScriptEdit);
    m_menuScriptEdit->addAction(QIcon(":/icon_save.png"),tr("Save Script"),this, &ConnectionWidget::onScriptSave);
    m_menuScriptEdit->addAction(QIcon(":/icon_copy.png"),tr("Duplicate Script"),this, &ConnectionWidget::onScriptDuplicate);
    m_menuScriptEdit->addAction(QIcon(":/icon_cross.png"),tr("Remove Script"),this, &ConnectionWidget::onScriptRemove);

    qDebug() << Q_FUNC_INFO;
}

ConnectionWidget::~ConnectionWidget()
{
    qDebug() << Q_FUNC_INFO;
    delete ui;
}

void ConnectionWidget::setType(int type)
{
    qDebug() << Q_FUNC_INFO<<type;

    if(type < 0 || type >= ui->stackedWidgetSettings->count())
        return;

    m_type = type;
    ui->stackedWidgetSettings->setCurrentIndex(type);
}

void ConnectionWidget::setNetConnection(NetConnection *pointer)
{
    qDebug() << Q_FUNC_INFO;

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
    connect(m_netConnection, &NetConnection::clearText, ui->textBrowserInput, &QTextBrowser::clear);
}

void ConnectionWidget::setSettings(const QJsonObject &settings)
{
    qDebug() << Q_FUNC_INFO;

    if (!m_netConnection)
        return;

    if (m_type == ConnectionTcpServer)
    {
        ui->lineEditTcpServerPort->setText(QString::number(settings.value(KEY_PORT).toInt()));
    }
    else if (m_type == ConnectionTcpSocket)
    {
        ui->lineEditTcpSocketHost->setText(settings.value(KEY_HOST).toString());
        ui->lineEditTcpSocketPort->setText(QString::number(settings.value(KEY_PORT).toInt()));
    }
    else if (m_type == ConnectionUdpSocket)
    {
        ui->lineEditUdpSocketHost->setText(settings.value(KEY_HOST).toString());
        ui->lineEditUdpSocketHostDst->setText(settings.value(KEY_HOST_DST).toString());
        ui->lineEditUdpSocketPort->setText(QString::number(settings.value(KEY_PORT).toInt()));
        ui->lineEditUdpSocketPortDst->setText(QString::number(settings.value(KEY_PORT_DST).toInt()));
        ui->checkBoxUdpMulticast->setChecked(settings.value(KEY_USE_MULTICAST).toBool());

        ui->lineEditUdpSocketHost->setEnabled(ui->checkBoxUdpMulticast->isChecked());
    }
    else if (m_type == ConnectionSerialPort)
    {
        ui->comboBoxPortName->setCurrentText(settings.value(KEY_SERIAL_PORT).toString());
        ui->comboBoxBaudrate->setCurrentText(QString::number(settings.value(KEY_BAUDRATE).toInt()));
    }

    ui->checkBoxAutoStart->setChecked(settings.value(KEY_AUTO_START).toBool());
    ui->checkBoxShowText->setChecked(settings.value(KEY_SHOW_TEXT).toBool());
    ui->checkBoxShowHex->setChecked(settings.value(KEY_SHOW_HEX).toBool());
    ui->checkBoxAddNewLine->setChecked(settings.value(KEY_ADD_NEW_LINE).toBool());

    if (ui->checkBoxAutoStart->isChecked())
        m_netConnection->start();

    m_scriptsDir = settings.value(KEY_SCRIPTS_DIR).toString();
    ui->splitterH->restoreState(QByteArray::fromBase64(settings.value(KEY_SPLITTER_H).toString().toLocal8Bit()));
    ui->splitterV->restoreState(QByteArray::fromBase64(settings.value(KEY_SPLITTER_V).toString().toLocal8Bit()));
}

QJsonObject ConnectionWidget::settings()
{
    QJsonObject settings = m_netConnection->settings();
    settings.insert(KEY_SPLITTER_H, QString::fromLocal8Bit(ui->splitterH->saveState().toBase64()));
    settings.insert(KEY_SPLITTER_V, QString::fromLocal8Bit(ui->splitterV->saveState().toBase64()));

    return settings;
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

    for (step = 0; step<scriptText.size(); ++step)
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
    qDebug() << Q_FUNC_INFO;

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

    if (type == StatusType::StatusOk)
        color = "#5BC791";
    else if (type == StatusType::StatusError)
        color = "#CD6E6E";
    else if (type == StatusType::StatusConsole)
        color = "#D9D98B";
    else if (type == StatusType::StatusOutput)
        color = "#B7CEE5";
    else
    {
        ui->textBrowserInput->append(message);
        return;
    }

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
    Q_UNUSED(pos);
    m_menuScriptEdit->exec(QCursor::pos());
}

void ConnectionWidget::onSettingsChanged()
{
    if (!m_netConnection) return;

    QJsonObject settings;
    settings.insert(KEY_AUTO_START, ui->checkBoxAutoStart->isChecked());
    settings.insert(KEY_SHOW_TEXT, ui->checkBoxShowText->isChecked());
    settings.insert(KEY_SHOW_HEX, ui->checkBoxShowHex->isChecked());
    settings.insert(KEY_ADD_NEW_LINE, ui->checkBoxAddNewLine->isChecked());
    settings.insert(KEY_SCRIPTS_DIR, m_scriptsDir);
    settings.insert(KEY_SPLITTER_H, QString::fromLocal8Bit(ui->splitterH->saveState().toBase64()));
    settings.insert(KEY_SPLITTER_V, QString::fromLocal8Bit(ui->splitterV->saveState().toBase64()));

    if (m_type == ConnectionTcpServer)
    {
        settings.insert(KEY_PORT, ui->lineEditTcpServerPort->text().toInt());
    }
    else if (m_type == ConnectionTcpSocket)
    {
        settings.insert(KEY_PORT, ui->lineEditTcpSocketPort->text().toInt());
        settings.insert(KEY_HOST, ui->lineEditTcpSocketHost->text());
    }
    else if (m_type == ConnectionUdpSocket)
    {
        settings.insert(KEY_PORT, ui->lineEditUdpSocketPort->text().toInt());
        settings.insert(KEY_HOST, ui->lineEditUdpSocketHost->text());
        settings.insert(KEY_PORT_DST, ui->lineEditUdpSocketPortDst->text().toInt());
        settings.insert(KEY_HOST_DST, ui->lineEditUdpSocketHostDst->text());
        settings.insert(KEY_USE_MULTICAST, ui->checkBoxUdpMulticast->isChecked());
    }
    else if (m_type == ConnectionSerialPort)
    {
        settings.insert(KEY_SERIAL_PORT, ui->comboBoxPortName->currentText());
        settings.insert(KEY_BAUDRATE, ui->comboBoxBaudrate->currentText().toInt());
    }

    m_netConnection->setSettings(settings);
    emit defaultSettings(m_type, settings);
}

void ConnectionWidget::onSendClicked()
{
    if(!m_netConnection)
        return;

    QString text = ui->textEditOutput->toPlainText();
    QString host = m_netConnection->settings().value(KEY_HOST_DST).toString();
    quint16 port = m_netConnection->settings().value(KEY_PORT_DST).toInt();
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

void ConnectionWidget::onItemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    onScriptEdit();
}
