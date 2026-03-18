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

#include "ResourceRegistry.h"

class MessageBubble : public QWidget {
    Q_OBJECT

public:
    explicit MessageBubble(const QString &text, bool incoming, QWidget *parent = nullptr);

    enum MessageStatus { Sent, Read };
    void setTail(bool show);
    void setStatus(MessageStatus status);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_incoming;
    bool m_hasTail;
    QColor m_color;
    QLabel *m_textLabel;
    QLabel *m_timeLabel;
    QLabel *m_statusIconLabel;

    QPixmap getColoredIcon(const QString &path, const QColor &color, int size);
};

#endif // MESSAGEBUBBLE_H
