#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "jsonsettings.h"
#include "connectionwidget.h"
#include "scripteditor.h"
#include "global.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    JsonSettings *m_settings;
    ScriptEditor *m_scriptEditor;
    QString m_styleString;

    void closeEvent(QCloseEvent *e);

private slots:
    void addConnection(int type);
    ConnectionWidget *createConnection(int type, const NetSettingsStruct &settings);
    void onTabCloseRequested(int index);

    void setDefaultConnectionSettings(int type, const NetSettingsStruct &settings);
    void onScriptsMenuRequested();
    void onScriptActionRequested();

    void closeAllTabs();

    void saveSession();
    void loadSession();

    void saveSettings();
    void loadSettings();
    void loadFont();
    void loadStyleSheet();
};

#endif // MAINWINDOW_H
