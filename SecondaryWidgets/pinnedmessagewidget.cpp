#include "pinnedmessagewidget.h"
#include "ResourceRegistry.h"
#include <QMouseEvent>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>

PinnedMessageWidget::PinnedMessageWidget(const QString &text, QWidget *parent)
    : QWidget(parent)
{
    // ГЛАВНОЕ: Этот флаг заставляет QWidget рисовать фон из StyleSheet
    setAttribute(Qt::WA_StyledBackground, true);

    setObjectName("pinnedWidget");
    setFixedHeight(50);

    // Чисто белый непрозрачный фон и заметная рамка снизу
    setStyleSheet(
        "QWidget#pinnedWidget {"
        "  background-color: white;"
        "  border: 1px solid rgba(0, 0, 0, 0.1);" // Тонкая рамка вокруг всего блока
        "  border-radius: 12px;"                  // Скругление как у сообщений
        "}"
        );

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 0, 15, 0);
    layout->setSpacing(10);

    // 1. Иконка PIN
    m_pinIcon = new QLabel(this);
    m_pinIcon->setFixedSize(20, 20);
    layout->addWidget(m_pinIcon);

    // 2. Акцентная полоска (синяя)
    QWidget *accent = new QWidget(this);
    accent->setFixedSize(3, 24);
    accent->setStyleSheet("background-color: #4e54c8; border-radius: 1px;");
    layout->addWidget(accent);

    // 3. Текст
    m_textLabel = new QLabel(text, this);
    m_textLabel->setFont(QFont("Roboto", 11, QFont::Medium));
    m_textLabel->setStyleSheet("color: #4e54c8; background: transparent;");
    layout->addWidget(m_textLabel, 1);

    // 4. Кнопка закрытия
    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFixedSize(30, 30);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setIconSize(QSize(24, 24));
    m_closeBtn->setStyleSheet(
        "QPushButton { border: none; background: transparent; border-radius: 15px; }"
        "QPushButton:hover { background: #f5f5f5; }"
        );
    connect(m_closeBtn, &QPushButton::clicked, this, &PinnedMessageWidget::unpinRequested);
    layout->addWidget(m_closeBtn);
}

void PinnedMessageWidget::updateText(const QString &text) {
    if (text.isEmpty()) return;

    QFontMetrics metrics(m_textLabel->font());
    // Берем ширину родителя минус отступы (20 + 20 на края и ~80 на иконки/кнопку)
    int parentW = parentWidget() ? parentWidget()->width() : 400;
    int availableWidth = parentW - 120;

    QString elided = metrics.elidedText(text, Qt::ElideRight, availableWidth);
    m_textLabel->setText(elided);
}

void PinnedMessageWidget::animateShow(int topBarHeight) {
    this->show();
    int sidePadding = 20;
    int targetY = topBarHeight + 5;

    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setDuration(400);
    // Вылетает из-под топ-бара (начинает с Y = topBarHeight - 50)
    anim->setStartValue(QRect(sidePadding, topBarHeight - 50, parentWidget()->width() - (sidePadding * 2), 50));
    anim->setEndValue(QRect(sidePadding, targetY, parentWidget()->width() - (sidePadding * 2), 50));
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void PinnedMessageWidget::animateHide(int topBarHeight) {
    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setDuration(300);
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(0, 0, parentWidget()->width(), 50)); // Уезжает вверх
    anim->setEasingCurve(QEasingCurve::InCubic);

    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        this->hide();
        emit animationFinished();
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void PinnedMessageWidget::setPinIcon(QPixmap pix)
{
    m_pinIcon->setPixmap(pix);
}

void PinnedMessageWidget::setCloseBtnIcon(QIcon icon)
{
    m_closeBtn->setIcon(icon);
}

void PinnedMessageWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) emit clicked();
    QWidget::mousePressEvent(event);
}
