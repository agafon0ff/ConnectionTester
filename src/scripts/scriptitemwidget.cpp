#include "scriptitemwidget.h"
#include <QHBoxLayout>
#include <QVariant>
#include <QStyle>
#include <QDebug>

ScriptItemWidget::ScriptItemWidget(QWidget *parent) : QWidget(parent),
    m_centralWidget(new QWidget(this)),
    m_labelName(new QLabel(m_centralWidget)),
    m_btnStart(new QToolButton(m_centralWidget)),
    m_isStarted(false)
{

}

void ScriptItemWidget::resizeEvent(QResizeEvent *)
{
    m_centralWidget->setGeometry(0,0,width(), height());
}

void ScriptItemWidget::initWidget()
{
    QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setContentsMargins(0, 0, 0, 4);

    m_labelName->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    m_labelName->setIndent(5);
    horizontalLayout->addWidget(m_labelName);

    m_btnStart->setMinimumSize(QSize(25, 25));
    m_btnStart->setMaximumSize(QSize(25, 25));
    m_btnStart->setIcon(QIcon(":/icon_play.png"));
    horizontalLayout->addWidget(m_btnStart);

    connect(m_btnStart, &QToolButton::clicked, this, [=]{m_isStarted ? emit stopClicked() : emit startClicked();});
}

void ScriptItemWidget::setName(const QString &name)
{
    m_labelName->setText(name);
}

QString ScriptItemWidget::name() const
{
    return m_labelName->text();
}

void ScriptItemWidget::onStarted()
{
    m_btnStart->setIcon(QIcon(":/icon_stop.png"));
    m_isStarted = true;
}

void ScriptItemWidget::onStopped()
{
    m_btnStart->setIcon(QIcon(":/icon_play.png"));
    m_isStarted = false;
}

void ScriptItemWidget::setActive(bool state)
{
    m_centralWidget->setProperty("active", state);
    m_centralWidget->style()->unpolish(m_centralWidget);
    m_centralWidget->style()->polish(m_centralWidget);
    m_centralWidget->update();
}
