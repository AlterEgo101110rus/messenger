#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QSvgRenderer>
#include <QProxyStyle>
#include <QKeyEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>
#include <QShowEvent>
#include <QHash>

#include "messagebubble.h"
#include "pinnedmessagewidget.h"
#include "ResourceRegistry.h"

class MenuIconStyle : public QProxyStyle {
public:
    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override {
        // Если меню спрашивает "какой размер иконок мне рисовать?", отвечаем — 36!
        if (metric == QStyle::PM_SmallIconSize || metric == QStyle::PM_LargeIconSize)
            return 24;
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

class ChatWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChatWidget(QWidget *parent = nullptr);
    enum ChatStatus { Online, Offline, Typing };
    void setChatStatus(ChatStatus status); // Главный метод для внешнего управления статусом диалога

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // Функция для добавления сообщения (можно перегрузить для "моих" и "чужих")
    void addMessage(const QString &text, bool isIncoming = false);
    void addMessage(const MessageBubble::MessageData &data, bool isIncoming = false);
    void showContextMenu(const QPoint &pos);
    void animateTyping(); // Слот для таймера

private:
    QListWidget *listWidget;
    QListWidgetItem *m_topSpacerItem = nullptr;
    QListWidgetItem *m_bottomSpacerItem = nullptr;
    PinnedMessageWidget *pinnedWidget;
    QSvgRenderer pattern;
    QBrush m_backgroundPattern; // Храним готовую кисть с узором
    QTextEdit *messageEdit;
    QWidget *inputBlurOverlay = nullptr;
    QWidget *inputBar;
    QWidget *replyPreviewBar = nullptr;
    QPushButton *replyPreviewButton = nullptr;
    QPushButton *replyCancelButton = nullptr;
    QWidget *topBar;
    QLabel *statusLabel; // Переносим сюда
    QPushButton *sendButton;

    // логика для аватаров
    bool m_lastWasIncoming = false;
    QWidget* m_lastMessageContainer = nullptr;

    QTimer *m_typingTimer;
    int m_typingStep = 0;
    ChatStatus m_currentStatus = Online;
    bool m_pinnedWidgetAnimated = false;
    bool m_initialLayoutApplied = false;
    int m_pendingReplyMessageId = -1;
    int m_nextMessageId = 1;
    QHash<int, QListWidgetItem *> m_messageItems;

    void updateBackgroundPattern();
    void updateListContentInsets(int bottomInsetOverride = -1);
    void updateFirstMessageOffset();
    int replyPreviewHeight() const;
    QString buildMessagePreview(const MessageBubble::MessageData &data) const;
    void beginReplyToItem(QListWidgetItem *item);
    void clearPendingReply();
    void scrollToMessage(int messageId);
    void highlightMessageItem(QListWidgetItem *item);
    void scrollToBottom();
    void topBarCreate();
    void animateItemRemoval(QListWidgetItem *item);
    void onUnpinClicked();
    QPixmap getRoundedPixmap(const QPixmap &src, int size);
    QPixmap getColoredIcon(const QString &path, const QColor &color, int size);

};

#endif // CHATWIDGET_H
