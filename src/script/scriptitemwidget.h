#ifndef SCRIPTITEMWIDGET_H
#define SCRIPTITEMWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>

class ScriptItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScriptItemWidget(QWidget *parent = nullptr);

private:

    QWidget *m_centralWidget;
    QLabel *m_labelName;
    QToolButton *m_btnStart;
    bool m_isStarted;

    void resizeEvent(QResizeEvent *);

signals:
    void startClicked();
    void stopClicked();

public slots:
    void initWidget();

    void setName(const QString &name);
    QString name() const;

    void onStarted();
    void onStopped();

    void setActive(bool state);
};

#endif // SCRIPTITEMWIDGET_H
