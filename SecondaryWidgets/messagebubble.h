#ifndef MESSAGEBUBBLE_H
#define MESSAGEBUBBLE_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTime>
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QPushButton>

#include "ResourceRegistry.h"

class MessageBubble : public QWidget {
    Q_OBJECT

public:
    enum BubbleType { Standard, Reply, Forward };
    enum AttachmentType { NoAttachment, PhotoAttachment, VideoAttachment, FileAttachment, PollAttachment };

    struct MessageData {
        QString text;
        BubbleType type = Standard;
        QString replyPreview;
        bool replyClickable = false;
        int replyToMessageId = -1;
        QString forwardSource;
        bool forwardSourceClickable = false;
        AttachmentType attachmentType = NoAttachment;
        QString attachmentTitle;
        QString attachmentSubtitle;
    };

    explicit MessageBubble(const QString &text, bool incoming, QWidget *parent = nullptr);
    explicit MessageBubble(const MessageData &data, bool incoming, QWidget *parent = nullptr);

    enum MessageStatus { Sent, Read };
    void setTail(bool show);
    void setStatus(MessageStatus status);

signals:
    void replyClicked();
    void forwardSourceClicked();
    void attachmentClicked();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_incoming;
    bool m_hasTail;
    QColor m_color;
    QLabel *m_textLabel;
    QLabel *m_timeLabel;
    QLabel *m_statusIconLabel;

    QWidget *createReplyHeader(const MessageData &data);
    QWidget *createForwardHeader(const MessageData &data);
    QWidget *createAttachmentWidget(const MessageData &data);
    QTextEdit *createTextView(const QString &text);
    QPixmap getColoredIcon(const QString &path, const QColor &color, int size);
};

#endif // MESSAGEBUBBLE_H
