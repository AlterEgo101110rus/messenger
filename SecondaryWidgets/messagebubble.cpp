#include "messagebubble.h"

MessageBubble::MessageBubble(const QString &text, bool incoming, QWidget *parent)
    : QWidget(parent), m_incoming(incoming), m_hasTail(true)
{
    m_color = m_incoming ? QColor("#ffffff") : QColor("#c2c5ff");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    // Оставляем ограничение, чтобы пузырь плотно облегал контент
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    // 1. ТЕКСТ СООБЩЕНИЯ (Используем QTextEdit для продвинутого переноса)
    QTextEdit *textEdit = new QTextEdit(text);
    textEdit->setReadOnly(true);
    textEdit->setUndoRedoEnabled(false);
    textEdit->setFrameShape(QFrame::NoFrame);
    textEdit->setObjectName("msgTextEdit");
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setContextMenuPolicy(Qt::NoContextMenu);
    textEdit->setFocusPolicy(Qt::NoFocus);

    // РАЗРЫВ В ЛЮБОМ МЕСТЕ (решает проблему длинных строк без пробелов)
    textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textEdit->document()->setDocumentMargin(0);

    QFont font("Roboto", 13);
    textEdit->setFont(font);
    textEdit->setStyleSheet("background: transparent; border: none; color: #222;");

    // Расчет размеров
    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(text);
    int maxWidth = 450;

    if (textWidth < maxWidth) {
        textEdit->setFixedWidth(textWidth + 4);
    } else {
        textEdit->setFixedWidth(maxWidth);
    }

    // подгон высоты виджета под содержимое текста
    textEdit->document()->setTextWidth(textEdit->width());
    int textHeight = static_cast<int>(textEdit->document()->size().height());
    textEdit->setFixedHeight(textHeight);

    mainLayout->addWidget(textEdit);

    // 2. НИЖНЯЯ ПАНЕЛЬ (ВРЕМЯ + СТАТУС)
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setSpacing(1);
    statusLayout->addStretch();

    m_timeLabel = new QLabel(QTime::currentTime().toString("HH:mm"));
    m_timeLabel->setStyleSheet("font-size: 11px; color: rgba(0,0,0,0.4); background:transparent;");
    statusLayout->addWidget(m_timeLabel);

    if (!m_incoming) {
        m_statusIconLabel = new QLabel();
        m_statusIconLabel->setFixedSize(16, 16);
        setStatus(Sent);
        statusLayout->addWidget(m_statusIconLabel);
    } else {
        m_statusIconLabel = nullptr;
    }

    mainLayout->addLayout(statusLayout);
    mainLayout->setAlignment(statusLayout, Qt::AlignRight);

    // отступы для хвостика
    int sidePadding = 22;
    int normalPadding = 12;
    if (m_incoming) mainLayout->setContentsMargins(sidePadding, 8, normalPadding, 4);
    else mainLayout->setContentsMargins(normalPadding, 8, sidePadding, 4);
}

void MessageBubble::setTail(bool show) {
    if (m_hasTail != show) {
        m_hasTail = show;
        update();
    }
}

void MessageBubble::setStatus(MessageStatus status) {
    if (!m_statusIconLabel) return;

    QString iconPath = (status == Read) ? Icons::CheckDouble : Icons::CheckOne;
    // Красим галочки в тот же полупрозрачный черный, что и время
    QColor statusColor = (status == Read) ? QColor("#4e54c8") : QColor(0, 0, 0, 80);

    m_statusIconLabel->setPixmap(getColoredIcon(iconPath, statusColor, 20));
}

QPixmap MessageBubble::getColoredIcon(const QString &path, const QColor &color, int size) {
    qreal dpr = devicePixelRatioF();

    QPixmap pixmap(size * dpr, size * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QSvgRenderer renderer(path);

    int margin = 2;
    QRectF renderRect(margin, margin, size - 2 * margin, size - 2 * margin);

    renderer.render(&painter, renderRect);

    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return pixmap;
}

void MessageBubble::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_color);

    qreal r = 16;      // Радиус обычных углов
    qreal tailW = 10;  // Ширина хвоста
    qreal w = width();
    qreal h = height();

    QPainterPath path;

    if (m_incoming) {
        // --- ВХОДЯЩЕЕ (Хвостик СЛЕВА СНИЗУ) ---
        path.moveTo(tailW + r, 0);
        path.lineTo(w - r, 0);                         // Верхний край
        path.quadTo(w, 0, w, r);                       // Верхний правый угол
        path.lineTo(w, h - r);                         // Правый край
        path.quadTo(w, h, w - r, h);                   // Нижний правый угол
        path.lineTo(tailW + r, h);                     // Нижний край до хвоста

        if (m_hasTail) {
            // Плавный закругленный хвост
            path.quadTo(tailW, h, 0, h);               // Вылет хвоста в 0
            path.quadTo(tailW, h - 2, tailW, h - r);   // Возврат вверх по дуге
        } else {
            path.quadTo(tailW, h, tailW, h - r);       // Обычный угол без хвоста
        }

        path.lineTo(tailW, r);                         // Левый край
        path.quadTo(tailW, 0, tailW + r, 0);           // Верхний левый угол
    }
    else {
        // --- ИСХОДЯЩЕЕ (Хвостик СПРАВА СНИЗУ) ---
        qreal r = 16;      // Радиус углов
        qreal tailW = 10;  // Ширина зоны хвоста

        // Рисуем от верхнего левого угла по часовой стрелке
        path.moveTo(r, 0);
        path.lineTo(w - tailW - r, 0);               // Верхний край
        path.quadTo(w - tailW, 0, w - tailW, r);     // Верхний правый угол
        path.lineTo(w - tailW, h - r);               // Правый край (до начала хвоста)

        if (m_hasTail) {
            // Плавный хвост вылетает вправо до самого края (w)
            path.quadTo(w - tailW, h, w, h);
            path.quadTo(w - tailW, h, w - tailW - r, h); // Возвращаемся на нижнюю линию
        } else {
            // Если хвоста нет — просто обычный скругленный угол
            path.quadTo(w - tailW, h, w - tailW - r, h);
        }

        path.lineTo(r, h);                           // Нижний край
        path.quadTo(0, h, 0, h - r);                 // Нижний левый угол
        path.lineTo(0, r);                           // Левый край
        path.quadTo(0, 0, r, 0);                     // Замыкаем в верхнем левом углу
    }

    painter.drawPath(path);
}
