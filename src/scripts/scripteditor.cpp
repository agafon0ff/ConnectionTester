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
    font.setPointSize(10);
    QFontMetrics metrics(font);
    ui->textEdit->setFont(font);

#if QT_VERSION > 0x050906
    ui->textEdit->setTabStopDistance(4 * metrics.horizontalAdvance(' '));
#else
    ui->textEdit->setTabStopWidth(4 * metrics.width(' '));
#endif


    m_highlighter = new Highlighter(ui->textEdit->document());

    connect(ui->btnOk, &QPushButton::clicked, this, &ScriptEditor::onBtnOkClicked);
    connect(ui->btnCancel, &QPushButton::clicked, [=]{reject();});
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

    foreach (QString line, lines)
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

    foreach (const QString &key, keywords)
        addRule("\\b" + key + "\\b", QBrush("#9BC3D4"));

    QStringList operators = {"=", "==", "!=", "<", "<=", ">", ">=","\\+", "-", "\\*", "/", "//", "\\%", "\\*\\*",
                            "\\+=", "-=", "\\*=", "/=", "\\%=","\\^", "\\|", "\\&", "\\~", ">>", "<<" };

    foreach (const QString &key, operators)
        addRule(key, QBrush("#CFC5d3"));

    addRule("\\b[A-Za-z0-9_]+(?=\\()", QBrush("#ddca7e"));
    addRule("\\b[+-]?[0-9]+[lL]?\\b", QBrush("#CFA790"));
    addRule("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b", QBrush("#CFA790"));
    addRule("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b", QBrush("#CFA790"));
    addRule("//[^\n]*", QBrush("#567790"));
    addRule("\"([^\"\"]*)\"", QBrush("#A6D49B"));
    addRule("'([^'']*)'", QBrush("#A6D49B"));

    multiLineCommentFormat.setForeground(Qt::red);
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
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

