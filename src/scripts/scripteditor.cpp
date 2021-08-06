#include "scripteditor.h"
#include "ui_scripteditor.h"
#include <QDebug>
#include <QAction>

ScriptEditor::ScriptEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScriptEditor),
    m_menuTemplates(new QMenu(this))
{
    ui->setupUi(this);
    readTemplatesFile();

    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);

#ifdef Q_OS_WIN
    font.setPointSize(10);
#else
    font.setPointSize(11);
#endif // Q_OS_WIN

    QFontMetrics metrics(font);
    ui->textEdit->setFont(font);

#if QT_VERSION > 0x050906
    ui->textEdit->setTabStopDistance(4 * metrics.horizontalAdvance(' '));
#else
    ui->textEdit->setTabStopWidth(4 * metrics.width(' '));
#endif


    m_highlighter = new Highlighter(ui->textEdit->document());

    connect(ui->btnOk, &QPushButton::clicked, this, &ScriptEditor::onBtnOkClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, [=]{reject();});
    connect(ui->btnClear, &QPushButton::clicked, this, &ScriptEditor::clear);
    connect(ui->btnTemplates, &QPushButton::clicked, this, &ScriptEditor::onBtnTemplatesClicked);
}

ScriptEditor::~ScriptEditor()
{
    delete m_menuTemplates;
    delete ui;
    delete m_highlighter;
}

void ScriptEditor::setScriptName(const QString &text)
{
    ui->lineEditScriptName->setText(text);
}

void ScriptEditor::setScriptText(const QString &text)
{
    ui->textEdit->setText(text);
}

void ScriptEditor::clear()
{
    ui->lineEditScriptName->clear();
    ui->textEdit->clear();
}

void ScriptEditor::onBtnOkClicked()
{
    m_scriptName = ui->lineEditScriptName->text();

    if (m_scriptName.isEmpty())
        return;

    m_scriptText = ui->textEdit->toPlainText();
    accept();
}

void ScriptEditor::onBtnTemplatesClicked()
{
    QAction *action = m_menuTemplates->exec(mapToGlobal(ui->btnTemplates->geometry().bottomLeft()));
    if (!action) return;

    if(m_templatesMap.contains(action->text()))
        ui->textEdit->append(m_templatesMap.value(action->text()));
}

void ScriptEditor::readTemplatesFile()
{
    QString templates;
    QFile file(":/templates.js");
    if (file.open(QIODevice::ReadOnly))
    {
        templates = QString::fromUtf8(file.readAll());
        file.close();
    }

    if (templates.isEmpty())
        return;

    templates.remove("\r");
    QStringList lines = templates.split("\n");
    m_templatesMap.clear();
    m_menuTemplates->clear();
    QString key, value;

    for (QString line: qAsConst(lines))
    {
        if(line.contains("/* "))
        {
            if(!key.isEmpty())
            {
                m_templatesMap.insert(key, value);
                m_menuTemplates->addAction(key);

                key.clear();
                value.clear();
            }

            line.remove("/* ");
            line.remove(" */");
            key = line;
        }
        else value.append(line + "\n");
    }

    m_templatesMap.insert(key, value);
    m_menuTemplates->addAction(key);
}

// ********************************************************************************

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    QStringList keywords = { "class", "var", "let", "new", "return", "if",
                             "else", "break", "true", "false", "function"};

    for (const QString &key: keywords)
        addRule("\\b" + key + "\\b", QBrush(QColor(0x9b, 0xc3, 0xd4)));

    QStringList operators = {"=", "==", "!=", "<", "<=", ">", ">=","\\+", "-", "\\*", "/", "//", "\\%", "\\*\\*",
                            "\\+=", "-=", "\\*=", "/=", "\\%=","\\^", "\\|", "\\&", "\\~", ">>", "<<" };

    for (const QString &key: operators)
        addRule(key, QBrush(QColor(0xcf, 0xc5, 0xd3)));

    addRule("\\b[A-Za-z0-9_]+(?=\\()", QBrush(QColor(0xdd, 0xca, 0x7e)));
    addRule("\\b[+-]?[0-9]+[lL]?\\b", QBrush(QColor(0xcf, 0xa7, 0x90)));
    addRule("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b", QBrush(QColor(0xcf, 0xa7, 0x90)));
    addRule("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b", QBrush(QColor(0xcf, 0xa7, 0x90)));
    addRule("//[^\n]*", QBrush(QColor(0x56, 0x77, 0x90)));
    addRule("\"([^\"\"]*)\"", QBrush(QColor(0xa6, 0xd4, 0x9b)));
    addRule("'([^'']*)'", QBrush(QColor(0xa6, 0xd4, 0x9b)));

    multiLineCommentFormat.setForeground(Qt::red);
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void Highlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule: qAsConst(highlightingRules))
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                    + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}

void Highlighter::addRule(const QString &regx, const QBrush &brush)
{
    HighlightingRule rule;
    QTextCharFormat textCharFormat;
    textCharFormat.setForeground(brush);
    textCharFormat.setFontWeight(QFont::Medium);
    rule.pattern = QRegExp(regx);
    rule.format = textCharFormat;
    highlightingRules.append(rule);
}

