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
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // Функция для добавления сообщения (можно перегрузить для "моих" и "чужих")
    void addMessage(const QString &text, bool isIncoming = false);
    void showContextMenu(const QPoint &pos);
    void animateTyping(); // Слот для таймера

private:
    QListWidget *listWidget;
    PinnedMessageWidget *pinnedWidget;
    QSvgRenderer pattern;
    QBrush m_backgroundPattern; // Храним готовую кисть с узором
    QTextEdit *messageEdit;
    QWidget *inputBar;
    QWidget *topBar;
    QLabel *statusLabel; // Переносим сюда
    QPushButton *sendButton;

    // логика для аватаров
    bool m_lastWasIncoming = false;
    QWidget* m_lastMessageContainer = nullptr;

    QTimer *m_typingTimer;
    int m_typingStep = 0;
    ChatStatus m_currentStatus = Online;

    void updateBackgroundPattern();
    void scrollToBottom();
    void topBarCreate();
    void animateItemRemoval(QListWidgetItem *item);
    void onUnpinClicked();
    QPixmap getRoundedPixmap(const QPixmap &src, int size);
    QPixmap getColoredIcon(const QString &path, const QColor &color, int size);

};

#endif // CHATWIDGET_H
