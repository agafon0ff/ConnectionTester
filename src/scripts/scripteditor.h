#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDialog>
#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>

namespace Ui {
class ScriptEditor;
}

class Highlighter;

class ScriptEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = Q_NULLPTR);
    ~ScriptEditor();

    inline QString scriptName() const {return m_scriptName;}
    inline QString scriptText() const {return m_scriptText;}
    void setScriptName(const QString &text);
    void setScriptText(const QString &text);
    void clear();

private:
    Ui::ScriptEditor *ui;
    Highlighter* m_highlighter;
    QString m_scriptName;
    QString m_scriptText;
    QMap<QString, QString> m_templatesMap;

private slots:
    void onBtnOkClicked();
    void onComboTemplatesActivated(const QString &key);
    void readTemplatesFile();
};

// ********************************************************************************

class QTextDocument;

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;

    void addRule(const QString &regx, const QBrush &brush);
};


#endif // SCRIPTEDITOR_H
