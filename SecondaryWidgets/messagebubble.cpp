#include "messagebubble.h"

#include <QFrame>

MessageBubble::MessageBubble(const QString &text, bool incoming, QWidget *parent)
    : MessageBubble([&text]() {
          MessageData data;
          data.text = text;
          return data;
      }(), incoming, parent)
{
}

MessageBubble::MessageBubble(const MessageData &data, bool incoming, QWidget *parent)
    : QWidget(parent), m_incoming(incoming), m_hasTail(true)
{
    m_color = m_incoming ? QColor("#ffffff") : QColor("#c2c5ff");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    if (data.type == Reply) {
        if (QWidget *replyHeader = createReplyHeader(data)) {
            mainLayout->addWidget(replyHeader);
        }
    } else if (data.type == Forward) {
        if (QWidget *forwardHeader = createForwardHeader(data)) {
            mainLayout->addWidget(forwardHeader);
        }
    }

    if (QWidget *attachmentWidget = createAttachmentWidget(data)) {
        mainLayout->addWidget(attachmentWidget);
    }

    QTextEdit *textEdit = createTextView(data.text);
    mainLayout->addWidget(textEdit);

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

    int sidePadding = 22;
    int normalPadding = 12;
    if (m_incoming) {
        mainLayout->setContentsMargins(sidePadding, 8, normalPadding, 4);
    } else {
        mainLayout->setContentsMargins(normalPadding, 8, sidePadding, 4);
    }
}

QWidget *MessageBubble::createReplyHeader(const MessageData &data) {
    if (data.replyPreview.isEmpty()) {
        return nullptr;
    }

    QWidget *replyWidget = new QWidget(this);
    replyWidget->setStyleSheet(
        "background: rgba(78, 84, 200, 0.10);"
        "border-radius: 12px;"
        );

    QHBoxLayout *layout = new QHBoxLayout(replyWidget);
    layout->setContentsMargins(10, 8, 12, 8);
    layout->setSpacing(8);

    QFrame *accent = new QFrame(replyWidget);
    accent->setFixedWidth(3);
    accent->setStyleSheet("background: #4e54c8; border-radius: 1px;");
    layout->addWidget(accent);

    QPushButton *previewButton = new QPushButton(data.replyPreview, replyWidget);
    previewButton->setFlat(true);
    previewButton->setFont(QFont("Roboto", 11, QFont::Medium));
    previewButton->setCursor(data.replyClickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    previewButton->setEnabled(data.replyClickable);
    previewButton->setStyleSheet(
        "QPushButton {"
        "  text-align: left;"
        "  border: none;"
        "  background: transparent;"
        "  color: #3942c0;"
        "  padding: 0;"
        "}"
        "QPushButton:disabled { color: #3942c0; }"
        "QPushButton:hover:!disabled { color: #2e05a8; }"
        );
    previewButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(previewButton, &QPushButton::clicked, this, &MessageBubble::replyClicked);
    layout->addWidget(previewButton);

    return replyWidget;
}

QWidget *MessageBubble::createForwardHeader(const MessageData &data) {
    if (data.forwardSource.isEmpty()) {
        return nullptr;
    }

    QWidget *forwardWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(forwardWidget);
    layout->setContentsMargins(2, 0, 2, 0);
    layout->setSpacing(1);

    QLabel *caption = new QLabel("Переслано от", forwardWidget);
    caption->setStyleSheet("color: rgba(0,0,0,0.45); font-size: 11px; font-weight: 600; background: transparent;");
    layout->addWidget(caption);

    QPushButton *sourceButton = new QPushButton(data.forwardSource, forwardWidget);
    sourceButton->setFlat(true);
    sourceButton->setCursor(data.forwardSourceClickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    sourceButton->setEnabled(data.forwardSourceClickable);
    sourceButton->setStyleSheet(
        "QPushButton {"
        "  text-align: left;"
        "  border: none;"
        "  background: transparent;"
        "  color: #4e54c8;"
        "  font-size: 13px;"
        "  font-weight: 700;"
        "  padding: 0;"
        "}"
        "QPushButton:disabled { color: #4e54c8; }"
        "QPushButton:hover:!disabled { color: #3c42aa; }"
        );
    connect(sourceButton, &QPushButton::clicked, this, &MessageBubble::forwardSourceClicked);
    layout->addWidget(sourceButton);

    return forwardWidget;
}

QWidget *MessageBubble::createAttachmentWidget(const MessageData &data) {
    if (data.attachmentType == NoAttachment) {
        return nullptr;
    }

    QWidget *attachmentWidget = new QWidget(this);
    attachmentWidget->setStyleSheet(
        "background: rgba(0, 0, 0, 0.05);"
        "border-radius: 14px;"
        );

    QHBoxLayout *layout = new QHBoxLayout(attachmentWidget);
    layout->setContentsMargins(10, 10, 12, 10);
    layout->setSpacing(10);

    QLabel *badge = new QLabel(attachmentWidget);
    badge->setFixedSize(42, 42);
    badge->setAlignment(Qt::AlignCenter);
    badge->setStyleSheet(
        "background: rgba(78, 84, 200, 0.14);"
        "border-radius: 12px;"
        "color: #4e54c8;"
        "font-size: 11px;"
        "font-weight: 700;"
        );

    QString badgeText = "Файл";
    switch (data.attachmentType) {
    case PhotoAttachment: badgeText = "Фото"; break;
    case VideoAttachment: badgeText = "Видео"; break;
    case FileAttachment: badgeText = "Файл"; break;
    case PollAttachment: badgeText = "Опрос"; break;
    default: break;
    }
    badge->setText(badgeText);
    layout->addWidget(badge);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);

    QString title = data.attachmentTitle.isEmpty() ? badgeText : data.attachmentTitle;
    QLabel *titleLabel = new QLabel(title, attachmentWidget);
    titleLabel->setStyleSheet("color: #222; font-size: 13px; font-weight: 700; background: transparent;");
    textLayout->addWidget(titleLabel);

    QString subtitleText = data.attachmentSubtitle;
    if (subtitleText.isEmpty()) {
        subtitleText = "Вложение доступно для просмотра";
    }

    QPushButton *subtitleButton = new QPushButton(subtitleText, attachmentWidget);
    subtitleButton->setFlat(true);
    subtitleButton->setCursor(Qt::PointingHandCursor);
    subtitleButton->setStyleSheet(
        "QPushButton {"
        "  text-align: left;"
        "  border: none;"
        "  background: transparent;"
        "  color: rgba(0,0,0,0.55);"
        "  font-size: 12px;"
        "  padding: 0;"
        "}"
        "QPushButton:hover { color: #4e54c8; }"
        );
    connect(subtitleButton, &QPushButton::clicked, this, &MessageBubble::attachmentClicked);
    textLayout->addWidget(subtitleButton);

    layout->addLayout(textLayout, 1);

    return attachmentWidget;
}

QTextEdit *MessageBubble::createTextView(const QString &text) {
    QTextEdit *textEdit = new QTextEdit(text, this);
    textEdit->setReadOnly(true);
    textEdit->setUndoRedoEnabled(false);
    textEdit->setFrameShape(QFrame::NoFrame);
    textEdit->setObjectName("msgTextEdit");
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit->setContextMenuPolicy(Qt::NoContextMenu);
    textEdit->setFocusPolicy(Qt::NoFocus);
    textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textEdit->document()->setDocumentMargin(0);

    QFont font("Roboto", 13);
    textEdit->setFont(font);
    textEdit->setStyleSheet("background: transparent; border: none; color: #222;");

    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(text);
    int maxWidth = 450;

    if (textWidth < maxWidth) {
        textEdit->setFixedWidth(textWidth + 4);
    } else {
        textEdit->setFixedWidth(maxWidth);
    }

    textEdit->document()->setTextWidth(textEdit->width());
    int textHeight = static_cast<int>(textEdit->document()->size().height());
    textEdit->setFixedHeight(textHeight);

    return textEdit;
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

    qreal r = 16;
    qreal tailW = 10;
    qreal w = width();
    qreal h = height();

    QPainterPath path;

    if (m_incoming) {
        path.moveTo(tailW + r, 0);
        path.lineTo(w - r, 0);
        path.quadTo(w, 0, w, r);
        path.lineTo(w, h - r);
        path.quadTo(w, h, w - r, h);
        path.lineTo(tailW + r, h);

        if (m_hasTail) {
            path.quadTo(tailW, h, 0, h);
            path.quadTo(tailW, h - 2, tailW, h - r);
        } else {
            path.quadTo(tailW, h, tailW, h - r);
        }

        path.lineTo(tailW, r);
        path.quadTo(tailW, 0, tailW + r, 0);
    } else {
        path.moveTo(r, 0);
        path.lineTo(w - tailW - r, 0);
        path.quadTo(w - tailW, 0, w - tailW, r);
        path.lineTo(w - tailW, h - r);

        if (m_hasTail) {
            path.quadTo(w - tailW, h, w, h);
            path.quadTo(w - tailW, h, w - tailW - r, h);
        } else {
            path.quadTo(w - tailW, h, w - tailW - r, h);
        }

        path.lineTo(r, h);
        path.quadTo(0, h, 0, h - r);
        path.lineTo(0, r);
        path.quadTo(0, 0, r, 0);
    }

    painter.drawPath(path);
}
