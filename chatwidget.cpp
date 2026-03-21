#include "chatwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QTime>
#include <QPixmap>
#include <QStyle>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QMenu>
#include <QIcon>
#include <QAction>
#include <QWidgetAction>
#include <QClipboard>
#include <QScrollBar>
#include <QListWidgetItem>
#include <QDialog>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
{

    /*pattern.load(QString(":/new/background/pattern.svg"));
    updateBackgroundPattern();*/

    // --- 1. ГЛАВНЫЙ СЛОЙ ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 2. СПИСОК СООБЩЕНИЙ ---
    // Создаем список
    listWidget = new QListWidget(this);
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    listWidget->setFocusPolicy(Qt::NoFocus);
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    listWidget->setWordWrap(true); // Разрешаем перенос текста внутри ячейки

    listWidget->setStyleSheet(
        "QListWidget {"
        "background: transparent;"
        "border: none;"
        "}"
        "QListWidget::item {"
        "background: transparent;"
        "}"
        );

    // Включаем попиксельную прокрутку вместо пошаговой по итемам
    listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // Настройка скорости и плавности для колесика мыши
    listWidget->verticalScrollBar()->setSingleStep(10); // Размер одного "шага" в пикселях

    m_topSpacerItem = new QListWidgetItem();
    m_topSpacerItem->setFlags(Qt::NoItemFlags);
    m_topSpacerItem->setData(Qt::UserRole, "spacer");
    listWidget->addItem(m_topSpacerItem);

    m_bottomSpacerItem = new QListWidgetItem();
    m_bottomSpacerItem->setFlags(Qt::NoItemFlags);
    m_bottomSpacerItem->setData(Qt::UserRole, "spacer");
    listWidget->addItem(m_bottomSpacerItem);

    listWidget->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;" // Прозрачный фон
        "    width: 8px;"               // Узкий
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(0, 0, 0, 0.2);" // Полупрозрачный ползунок
        "    border-radius: 4px;"             // Скругленный
        "    min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(0, 0, 0, 0.3);" // Темнеет при наведении
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }" // Убираем стрелочки
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        );

    // Настройка поведения ячеек: они должны быть фиксированной высоты, равной высоте содержимого + отступы
    listWidget->setMinimumWidth(250); // Минимальная ширина всего чата

    mainLayout->addWidget(listWidget); // Добавляем список в верхнюю часть

    // --- 3. ПАНЕЛЬ ВВОДА (Input Bar) ---
    // Создаем панель ввода БЕЗ добавления в Layout
    inputBar = new QWidget(this);
    inputBar->setObjectName("floatingInputBar");
    inputBar->setAttribute(Qt::WA_TranslucentBackground, true);
    // Делаем саму полоску прозрачной
    //inputBar->setStyleSheet("background: transparent;");
    /*inputBar->setStyleSheet(
        "QWidget#floatingInputBar {"
        "  background: rgba(255, 255, 255, 0.2);" // Очень слабая подложка
        "}"
        );*/

    inputBlurOverlay = new QWidget(this);
    inputBlurOverlay->setObjectName("inputBlurOverlay");
    inputBlurOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    inputBlurOverlay->setStyleSheet(
        "QWidget#inputBlurOverlay {"
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                            stop:0 rgba(70, 75, 160, 145),"
        "                            stop:1 rgba(35, 35, 55, 190));"
        "  border: 1px solid rgba(255, 255, 255, 45);"
        "  border-top: 1px solid rgba(255, 255, 255, 80);"
        "  border-radius: 24px;"
        "}"
        );
    inputBlurOverlay->lower();

    QVBoxLayout *inputBarLayout = new QVBoxLayout(inputBar);
    inputBarLayout->setContentsMargins(20, 8, 20, 16);
    inputBarLayout->setSpacing(22);

    replyPreviewBar = new QWidget(inputBar);
    replyPreviewBar->setFixedHeight(48);
    replyPreviewBar->hide();
    replyPreviewBar->setStyleSheet(
        "background: rgba(255, 255, 255, 0.88);"
        "border-radius: 16px;"
        );

    QHBoxLayout *replyPreviewLayout = new QHBoxLayout(replyPreviewBar);
    replyPreviewLayout->setContentsMargins(12, 7, 8, 7);
    replyPreviewLayout->setSpacing(10);

    QWidget *replyAccent = new QWidget(replyPreviewBar);
    replyAccent->setFixedSize(3, 30);
    replyAccent->setStyleSheet("background: #4e54c8; border-radius: 1px;");
    replyPreviewLayout->addWidget(replyAccent, 0, Qt::AlignVCenter);

    QWidget *replyTextWidget = new QWidget(replyPreviewBar);
    replyTextWidget->setAttribute(Qt::WA_TranslucentBackground);
    QVBoxLayout *replyTextLayout = new QVBoxLayout(replyTextWidget);
    replyTextLayout->setContentsMargins(0, 0, 0, 0);
    replyTextLayout->setSpacing(0);

    QLabel *replyTitleLabel = new QLabel("Ответ на сообщение", replyTextWidget);
    replyTitleLabel->setFont(QFont("Roboto", 10, QFont::Medium));
    replyTitleLabel->setStyleSheet("color: rgba(0,0,0,0.48); background: transparent;");
    replyTextLayout->addWidget(replyTitleLabel);

    replyPreviewButton = new QPushButton(replyTextWidget);
    replyPreviewButton->setFlat(true);
    replyPreviewButton->setFont(QFont("Roboto", 11, QFont::Normal));
    replyPreviewButton->setCursor(Qt::PointingHandCursor);
    replyPreviewButton->setStyleSheet(
        "QPushButton {"
        "  text-align: left;"
        "  border: none;"
        "  background: transparent;"
        "  color: #4e54c8;"
        "  padding: 0;"
        "}"
        "QPushButton:hover { color: #2e05a8; }"
        );
    replyTextLayout->addWidget(replyPreviewButton);
    replyPreviewLayout->addWidget(replyTextWidget, 1);

    replyCancelButton = new QPushButton(replyPreviewBar);
    replyCancelButton->setFixedSize(28, 28);
    replyCancelButton->setCursor(Qt::PointingHandCursor);
    replyCancelButton->setIcon(QIcon(getColoredIcon(Icons::Cross, QColor("#888"), 24)));
    replyCancelButton->setIconSize(QSize(24, 24));
    replyCancelButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; border-radius: 14px; }"
        "QPushButton:hover { background: rgba(0,0,0,0.05); }"
        );
    replyPreviewLayout->addWidget(replyCancelButton, 0, Qt::AlignVCenter);

    QWidget *inputRow = new QWidget(inputBar);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputRow);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(8);

    // --- КНОПКА СКРЕПКИ ---
    QPushButton *attachBtn = new QPushButton();
    int btnSize = 36;   // Общий размер кнопки
    int iconPx = 28;    // Желаемый размер самой иконки (скрепки)

    attachBtn->setFixedSize(btnSize, btnSize);
    attachBtn->setCursor(Qt::PointingHandCursor);

    // 1. Генерируем картинку нужного размера
    QPixmap pix = getColoredIcon(Icons::Attach, QColor("#777"), iconPx);
    attachBtn->setIcon(QIcon(pix));

    // 2. ВАЖНО: Принудительно задаем размер отображения иконки
    attachBtn->setIconSize(QSize(iconPx, iconPx));

    attachBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 18px;" // Скругление (половина от btnSize)
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(0, 0, 0, 0.05);" // Легкое затемнение при наведении
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(0, 0, 0, 0.1);"  // Чуть сильнее при нажатии
        "}"
        );
    //attachBtn->setStyleSheet("background: transparent; border: none;");

    // Скругленное поле (как в предыдущем шаге)
    QWidget *textContainer = new QWidget();
    textContainer->setObjectName("textContainer");
    textContainer->setStyleSheet(
        "QWidget#textContainer {"
        "  background: rgba(255, 255, 255, 0.95);" // Почти непрозрачный белый
        "  border-radius: 22px;"
        "  border: 1px solid rgba(0,0,0,0.1);"
        "}"
        );

    // --- ОБНОВЛЕННЫЙ TEXT CONTAINER (Liquid Glass) ---
    /*textContainer->setStyleSheet(
        "QWidget#textContainer {"
        "  background: rgba(255, 255, 255, 0.5);" // Полупрозрачный белый
        "  border-radius: 22px;"
        "  border: 1px solid rgba(255, 255, 255, 0.3);" // Светлая кайма
        "}"
        );*/

    QHBoxLayout *textLayout = new QHBoxLayout(textContainer);
    //textLayout->setContentsMargins(10, 0, 15, 0);

    // --- 3. ПОЛЕ ВВОДА (теперь QTextEdit) ---
    messageEdit = new QTextEdit();
    messageEdit->setPlaceholderText("Сообщение...");
    messageEdit->setFrameStyle(QFrame::NoFrame);
    messageEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Скрываем скролл
    messageEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messageEdit->installEventFilter(this);
    messageEdit->document()->setDocumentMargin(4);
    messageEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 2. ВАЖНО: Чтобы текст был визуально по центру в начале,
    // задаем начальную высоту чуть больше и добавляем padding
    messageEdit->setFixedHeight(35);
    messageEdit->setStyleSheet(
        "QTextEdit {"
        "  background: transparent;"
        "  border: none;"
        "  padding-top: 4px;" // Смещает текст чуть ниже к центру
        "  font-size: 16px;"
        "}"
        );

    textLayout->addWidget(messageEdit);
    textLayout->insertWidget(0, attachBtn, 0, Qt::AlignBottom);
    textLayout->setContentsMargins(10, 5, 10, 8); // Нижний отступ (8) выровняет скрепку по линии текста

    // Кнопка отправки
    sendButton = new QPushButton();
    sendButton->setFixedSize(44, 44);
    sendButton->setCursor(Qt::PointingHandCursor); // Курсор-ручка при наведении

    // Установка иконки из ресурсов
    QIcon sendIcon(Icons::Send); // Укажите ваш путь к SVG
    sendButton->setIcon(sendIcon);
    sendButton->setIconSize(QSize(24, 24));

    // Продвинутый стиль с эффектами
    sendButton->setStyleSheet(
        "QPushButton {"
        "  background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, "
        "                                    stop:0 #858cf2, stop:1 #636ae8);"
        "  border-radius: 22px;"
        "  padding-right: 2px;"
        "  padding-top: 1px;"
        "  color: white;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "  background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, "
        "                                    stop:0 #949bfb, stop:1 #7279f5);"
        "}"
        "QPushButton:pressed {"
        "  background-color: #646be6;"
        "  padding-top: 4px;"
        "  padding-right: 2px;"
        "}"
        );

    // connect(sendButton, &QPushButton::clicked, this, )

    inputLayout->addWidget(textContainer);
    inputLayout->addWidget(sendButton, 0, Qt::AlignBottom);

    inputBarLayout->addWidget(replyPreviewBar);
    inputBarLayout->addWidget(inputRow);

    // Загрузка паттерна и остальное...
    pattern.load(QString(Backgrounds::PatternDefault));
    updateBackgroundPattern();

    // TOPBAR CREATING
    topBarCreate();

    pinnedWidget = new PinnedMessageWidget("Закрепленное сообщение", this);
    // Настраиваем иконки через наш метод
    pinnedWidget->setPinIcon(getColoredIcon(Icons::Pin, QColor("#4e54c8"), 20));
    pinnedWidget->setCloseBtnIcon(QIcon(getColoredIcon(Icons::Cross, QColor("#888"), 24)));

    // Соединяем сигналы
    connect(pinnedWidget, &PinnedMessageWidget::unpinRequested, this, &ChatWidget::onUnpinClicked);
    connect(pinnedWidget, &PinnedMessageWidget::clicked, this, [this](){
        // Логика скролла (пока просто в начало для теста)
        listWidget->scrollToTop();
    });
    connect(pinnedWidget, &PinnedMessageWidget::animationFinished, this, [this](){
        // Принудительно вызываем resize, чтобы список "подтянулся" вверх
        QResizeEvent *re = new QResizeEvent(size(), size());
        this->resizeEvent(re);
    });
    connect(replyPreviewButton, &QPushButton::clicked, this, [this]() {
        if (m_pendingReplyMessageId != -1) {
            scrollToMessage(m_pendingReplyMessageId);
        }
    });
    connect(replyCancelButton, &QPushButton::clicked, this, &ChatWidget::clearPendingReply);

    // Убедитесь, что TopBar выше закрепа
    topBar->raise();

    updateListContentInsets();

    this->ensurePolished();

    // --- Добавляем тестовые сообщения (чтобы проверить видимость) ---
    MessageBubble::MessageData outgoingReply;
    outgoingReply.text = "Да, вижу. Отвечаю прямо из нового bubble с reply-блоком.";
    outgoingReply.type = MessageBubble::Reply;
    outgoingReply.replyPreview = "Бот MAXим: Привет! Покажешь новые типы сообщений?";
    outgoingReply.replyClickable = true;
    outgoingReply.attachmentType = MessageBubble::PhotoAttachment;
    outgoingReply.attachmentTitle = "Фотография";
    outgoingReply.attachmentSubtitle = "Нажмите, чтобы открыть вложение";

    MessageBubble::MessageData incomingForward;
    incomingForward.text = "Вот пример пересланного сообщения с вложением под заголовком.";
    incomingForward.type = MessageBubble::Forward;
    incomingForward.forwardSource = "Канал MAX Design";
    incomingForward.forwardSourceClickable = true;
    incomingForward.attachmentType = MessageBubble::FileAttachment;
    incomingForward.attachmentTitle = "Презентация.pdf";
    incomingForward.attachmentSubtitle = "2,4 МБ • 12 слайдов";

    MessageBubble::MessageData plainWithAttachment;
    plainWithAttachment.text = "А это обычное сообщение, где вложение находится прямо над текстом.";
    plainWithAttachment.attachmentType = MessageBubble::PollAttachment;
    plainWithAttachment.attachmentTitle = "Опрос команды";
    plainWithAttachment.attachmentSubtitle = "4 варианта ответа";

    addMessage(outgoingReply, false);
    addMessage(incomingForward, true);
    addMessage(plainWithAttachment, false);

    // Сразу прокручиваем вниз, чтобы их увидели
    scrollToBottom();

    connect(messageEdit, &QTextEdit::textChanged, this, [this]() {
        // Считаем высоту текста
        int docHeight = messageEdit->document()->size().height();
        int newEditHeight = qBound(35, docHeight + 8, 150);

        if (messageEdit->height() != newEditHeight) {
            messageEdit->setFixedHeight(newEditHeight);

            // Вычисляем новую высоту всей панели
            int newBarHeight = newEditHeight + (70 - 35) + replyPreviewHeight();
            inputBar->setFixedHeight(newBarHeight);

            // ВАЖНО: Сразу пересчитываем позицию, чтобы не ждать resizeEvent
            inputBar->setGeometry(0, height() - newBarHeight, width(), newBarHeight);
            if (inputBlurOverlay) {
                inputBlurOverlay->setGeometry(0, height() - newBarHeight - 18, width(), newBarHeight + 36);
                inputBlurOverlay->raise();
                inputBar->raise();
            }

            updateListContentInsets(newBarHeight + 5);
        }
    });

    // 2. Основная логика отправки в лямбда-выражении
    connect(sendButton, &QPushButton::clicked, this, [this]() {

        // Используем toPlainText() вместо text()
        QString text = messageEdit->toPlainText().trimmed();

        if (!text.isEmpty()) {
            MessageBubble::MessageData outgoingData;
            outgoingData.text = text;

            if (m_pendingReplyMessageId != -1 && m_messageItems.contains(m_pendingReplyMessageId)) {
                QListWidgetItem *targetItem = m_messageItems.value(m_pendingReplyMessageId);
                outgoingData.type = MessageBubble::Reply;
                outgoingData.replyToMessageId = m_pendingReplyMessageId;
                outgoingData.replyClickable = true;
                outgoingData.replyPreview = targetItem->data(Qt::UserRole + 2).toString();
            }

            addMessage(outgoingData, false);
            messageEdit->clear();
            messageEdit->setFixedHeight(35);
            clearPendingReply();
            inputBar->setFixedHeight(70 + replyPreviewHeight());
            inputBar->move(0, height() - inputBar->height());
            if (inputBlurOverlay) {
                inputBlurOverlay->setGeometry(0, height() - 88, width(), 106);
                inputBlurOverlay->raise();
                inputBar->raise();
            }
            scrollToBottom();

            // Имитация бота
            QTimer::singleShot(800, this, [this]() {
                setChatStatus(Typing);
            });

            QTimer::singleShot(3500, this, [this]() {
                setChatStatus(Online);
                addMessage("Привет! Я получил твой текст. Архитектура работает!", true);
            });
        }
    });

    connect(listWidget, &QListWidget::customContextMenuRequested, this, &ChatWidget::showContextMenu);

    // В конструкторе инициализируем таймер
    m_typingTimer = new QTimer(this);
    m_typingTimer->setInterval(400);
    connect(m_typingTimer, &QTimer::timeout, this, &ChatWidget::animateTyping);
}

void ChatWidget::addMessage(const QString &text, bool isIncoming)
{
    MessageBubble::MessageData data;
    data.text = text;
    addMessage(data, isIncoming);
}

void ChatWidget::addMessage(const MessageBubble::MessageData &data, bool isIncoming)
{
    // 1. Если автор тот же, убираем хвостик у предыдущего пузыря
    if (m_lastMessageContainer && isIncoming == m_lastWasIncoming) {
        // Приводим указатель к нашему классу, чтобы вызвать setTail
        if (MessageBubble* prevBubble = qobject_cast<MessageBubble*>(m_lastMessageContainer)) {
            prevBubble->setTail(false);
        }
    }

    // 2. Создаем элемент списка и контейнер
    QListWidgetItem *item = new QListWidgetItem();
    item->setData(Qt::UserRole, "message");
    int messageId = m_nextMessageId++;
    item->setData(Qt::UserRole + 1, messageId);
    item->setData(Qt::UserRole + 2, buildMessagePreview(data));

    QWidget *msgContainer = new QWidget();
    QHBoxLayout *msgLayout = new QHBoxLayout(msgContainer);
    msgLayout->setSpacing(0);

    // 3. Создаем наш кастомный пузырь
    MessageBubble *bubble = new MessageBubble(data, isIncoming);
    if (data.type == MessageBubble::Reply && data.replyToMessageId != -1) {
        connect(bubble, &MessageBubble::replyClicked, this, [this, data]() {
            scrollToMessage(data.replyToMessageId);
        });
    }

    QWidget *prevContainer;
    QHBoxLayout *prevLayout;

    if (m_lastMessageContainer) {
        qDebug() << "m_lastMessageContainer";
        prevContainer = m_lastMessageContainer->parentWidget();
        prevLayout = qobject_cast<QHBoxLayout *>(prevContainer ? prevContainer->layout() : nullptr);
    }

    // 4. Размещаем пузырь в зависимости от того, входящее оно или исходящее
    if (isIncoming) {
        msgLayout->addWidget(bubble);
        msgLayout->addStretch();
        // Оставляем место справа, чтобы пузырь не растягивался на всю ширину
        if (m_lastMessageContainer) {
            if (isIncoming == m_lastWasIncoming) {
                msgLayout->setContentsMargins(10, 1, 80, 1);
            } else {
                int left, top, right, bottom;
                prevLayout->getContentsMargins(&left, &top, &right, &bottom);
                prevLayout->setContentsMargins(left, top, right, 6);
                msgLayout->setContentsMargins(10, 6, 80, 1);
            }
        } else {
            msgLayout->setContentsMargins(10, 1, 80, 1);
        }
    } else {
        msgLayout->addStretch();
        msgLayout->addWidget(bubble);
        // Оставляем место слева
        if (m_lastMessageContainer) {
            if (isIncoming == m_lastWasIncoming) {
                msgLayout->setContentsMargins(80, 1, 10, 1);
            } else {
                int left, top, right, bottom;
                prevLayout->getContentsMargins(&left, &top, &right, &bottom);
                prevLayout->setContentsMargins(left, top, right, 6);
                msgLayout->setContentsMargins(80, 6, 10, 1);
            }
        } else {
            msgLayout->setContentsMargins(80, 1, 10, 1);
        }
    }

    // Запоминаем текущее сообщение как "последнее"
    m_lastMessageContainer = bubble;
    m_lastWasIncoming = isIncoming;

    // 5. Передаем управление списку
    item->setSizeHint(msgContainer->sizeHint());
    int insertRow = m_bottomSpacerItem ? listWidget->row(m_bottomSpacerItem) : listWidget->count();
    listWidget->insertItem(insertRow, item);
    listWidget->setItemWidget(item, msgContainer);
    m_messageItems.insert(messageId, item);

    // --- АНИМАЦИЯ ПРОЗРАЧНОСТИ ---
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(msgContainer);
    msgContainer->setGraphicsEffect(opacityEffect);

    QPropertyAnimation *opacityAnim = new QPropertyAnimation(opacityEffect, "opacity");
    opacityAnim->setDuration(300);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
    opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);

    // 6. Если у предыдущего итема изменился размер (из-за скрытия хвоста), обновляем его
    if (insertRow > 1) {
        QListWidgetItem *prevItem = listWidget->item(insertRow - 1);
        if (prevItem && prevItem->data(Qt::UserRole).toString() == "message") {
            prevItem->setSizeHint(listWidget->itemWidget(prevItem)->sizeHint());
        }
    }

    scrollToBottom();
}

void ChatWidget::showContextMenu(const QPoint &pos) {
    QListWidgetItem *item = listWidget->itemAt(pos);
    if (!item) return;
    if (item->data(Qt::UserRole).toString() != "message") return;

    QMenu menu(this);
    // ПРИНУДИТЕЛЬНО ставим наш стиль для этого меню
    menu.setStyle(new MenuIconStyle);

    int iconSize = 24;

    // В QSS обязательно увеличиваем отступ слева, чтобы текст не наезжал
    menu.setStyleSheet(
        "QMenu {"
        "  background-color: white;"
        "  border: 1px solid rgba(0, 0, 0, 0.15);"
        "  border-radius: 10px;"
        "  padding: 8px 0px;"
        "}"
        "QMenu::item {"
        "  /* Отступ должен быть больше размера иконки (36 + запас) */"
        "  padding: 4px 20px 4px 24px;"
        "  background-color: transparent;"
        "  font-size: 16px;"
        "}"
        "QMenu::icon {"
        "  margin-left: 24px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #f5f5f5;"
        "  color: #000;"
        "}"
        "QMenu::item:selected[isDanger='true'] {"
        "  background-color: #d32f2f;"
        "  color: #d32f2f;"
        "}"
        "QMenu::separator { height: 1px; background: rgba(0, 0, 0, 0.08); margin: 4px 12px; }"
        );

    // 2. НАСТРОЙКА ОКНА (Убираем системные рамки для корректного скругления)
    menu.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    menu.setAttribute(Qt::WA_TranslucentBackground);

    // 3. ДОБАВЛЕНИЕ ДЕЙСТВИЙ (Используем нашу функцию перекраски)
    QAction *reply = menu.addAction(getColoredIcon(Icons::Reply, QColor("#000000"), iconSize), "Ответить");
    QAction *copy = menu.addAction(getColoredIcon(Icons::Copy, QColor("#000000"), iconSize), "Копировать");
    QAction *pin = menu.addAction(getColoredIcon(Icons::Pin, QColor("#000000"), iconSize), "Закрепить");
    QAction *forward = menu.addAction(getColoredIcon(Icons::Forward, QColor("#000000"), iconSize), "Переслать");

    QWidgetAction *remove = new QWidgetAction(&menu);
    QWidget *removeWidget = new QWidget(&menu);
    removeWidget->setStyleSheet(
        "QWidget { background-color: transparent; }"
        "QWidget:hover { background-color: #fce6e6; }"
        );

    QHBoxLayout *removeLayout = new QHBoxLayout(removeWidget);
    removeLayout->setContentsMargins(12, 4, 4, 4);
    removeLayout->setSpacing(16);

    QLabel *text = new QLabel("Удалить", removeWidget);
    text->setStyleSheet("color: #d32f2f;");

    QLabel *icon = new QLabel(removeWidget);
    icon->setPixmap(getColoredIcon(Icons::Delete, QColor("#d32f2f"), iconSize));
    icon->setFixedSize(24, 24);
    icon->setScaledContents(true);

    removeLayout->addWidget(icon);
    removeLayout->addWidget(text);
    removeLayout->addStretch();

    remove->setDefaultWidget(removeWidget);

    menu.addAction(remove);
    //menu.addSeparator();

    // 4. ОТОБРАЖЕНИЕ
    QAction *selectedAction = menu.exec(listWidget->mapToGlobal(pos));

    // 5. ЛОГИКА
    if (selectedAction == reply) {
        beginReplyToItem(item);
    }
    else if (selectedAction == copy) {
        QWidget *container = listWidget->itemWidget(item);
        if (container) {
            // 1. Сначала ищем QTextEdit (наш новый формат сообщений)
            QTextEdit *edit = container->findChild<QTextEdit*>("msgTextEdit");
            if (edit) {
                QApplication::clipboard()->setText(edit->toPlainText());
            }
            else {
                // 2. Если не нашли (например, сообщение старого формата), ищем QLabel
                // Но чтобы не скопировать время, ищем по имени объекта (если задавали)
                // или просто берем первый QLabel, который НЕ является временем.
                QList<QLabel*> labels = container->findChildren<QLabel*>();
                for (QLabel* lbl : labels) {
                    // Время у нас обычно очень короткое и серое,
                    // но надежнее проверить, что это не m_timeLabel
                    if (lbl->text().length() > 5 || !lbl->text().contains(":")) {
                        QApplication::clipboard()->setText(lbl->text());
                        break;
                    }
                }
            }
        }
    }
    else if (selectedAction == remove) {
        animateItemRemoval(item);
    }
}

int ChatWidget::replyPreviewHeight() const {
    return (replyPreviewBar && replyPreviewBar->isVisible()) ? 68 : 0;
}

QString ChatWidget::buildMessagePreview(const MessageBubble::MessageData &data) const {
    QString preview = data.text.simplified();
    if (preview.isEmpty()) {
        preview = data.attachmentTitle;
    }

    if (preview.length() > 72) {
        preview = preview.left(69) + "...";
    }

    return preview;
}

void ChatWidget::beginReplyToItem(QListWidgetItem *item) {
    if (!item) {
        return;
    }

    m_pendingReplyMessageId = item->data(Qt::UserRole + 1).toInt();
    if (replyPreviewButton) {
        replyPreviewButton->setText(item->data(Qt::UserRole + 2).toString());
    }

    if (replyPreviewBar && !replyPreviewBar->isVisible()) {
        replyPreviewBar->show();
    }

    int newBarHeight = messageEdit->height() + (70 - 35) + replyPreviewHeight();
    inputBar->setFixedHeight(newBarHeight);
    inputBar->setGeometry(0, height() - newBarHeight, width(), newBarHeight);

    if (inputBlurOverlay) {
        inputBlurOverlay->setGeometry(0, height() - newBarHeight - 18, width(), newBarHeight + 36);
        inputBlurOverlay->raise();
        inputBar->raise();
    }

    updateListContentInsets(newBarHeight + 5);
    messageEdit->setFocus();
}

void ChatWidget::clearPendingReply() {
    m_pendingReplyMessageId = -1;
    if (replyPreviewBar) {
        replyPreviewBar->hide();
    }

    int newBarHeight = messageEdit->height() + (70 - 35);
    inputBar->setFixedHeight(newBarHeight);
    inputBar->setGeometry(0, height() - newBarHeight, width(), newBarHeight);

    if (inputBlurOverlay) {
        inputBlurOverlay->setGeometry(0, height() - newBarHeight - 18, width(), newBarHeight + 36);
        inputBlurOverlay->raise();
        inputBar->raise();
    }

    updateListContentInsets(newBarHeight + 5);
}

void ChatWidget::scrollToMessage(int messageId) {
    if (!m_messageItems.contains(messageId)) {
        return;
    }

    QListWidgetItem *item = m_messageItems.value(messageId);
    if (!item) {
        return;
    }

    listWidget->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    highlightMessageItem(item);
}

void ChatWidget::highlightMessageItem(QListWidgetItem *item) {
    QWidget *container = listWidget->itemWidget(item);
    if (!container) {
        return;
    }

    if (container->property("replyRocking").toBool()) {
        return;
    }

    QPoint basePos = container->pos();
    container->setProperty("replyRocking", true);
    auto *rockAnim = new QPropertyAnimation(container, "pos", this);
    rockAnim->setDuration(520);
    rockAnim->setEasingCurve(QEasingCurve::InOutSine);
    rockAnim->setKeyValueAt(0.0, basePos);
    rockAnim->setKeyValueAt(0.18, basePos + QPoint(10, 0));
    rockAnim->setKeyValueAt(0.36, basePos + QPoint(-8, 0));
    rockAnim->setKeyValueAt(0.56, basePos + QPoint(6, 0));
    rockAnim->setKeyValueAt(0.76, basePos + QPoint(-4, 0));
    rockAnim->setKeyValueAt(1.0, basePos);
    connect(rockAnim, &QPropertyAnimation::finished, this, [container, basePos, rockAnim]() {
        container->move(basePos);
        container->setProperty("replyRocking", false);
        rockAnim->deleteLater();
    });
    rockAnim->start();
}

// --- ГЛАВНЫЙ МЕТОД УПРАВЛЕНИЯ СТАТУСОМ ---
void ChatWidget::setChatStatus(ChatStatus status) {
    m_currentStatus = status;

    if (status == Typing) {
        if (!m_typingTimer->isActive()) {
            m_typingStep = 0;
            m_typingTimer->start();
        }
    } else {
        m_typingTimer->stop();
        if (status == Online) {
            statusLabel->setText("в сети");
            statusLabel->setFont(QFont("Roboto", 11, QFont::Normal));
            statusLabel->setStyleSheet("color: #4e54c8;");
        } else {
            statusLabel->setText("был(а) недавно");
            statusLabel->setFont(QFont("Roboto", 11, QFont::Normal));
            statusLabel->setStyleSheet("color: #888;");
        }
    }
}

// Слот самой анимации
void ChatWidget::animateTyping() {
    m_typingStep = (m_typingStep + 1) % 4;
    QString dots = QString(".").repeated(m_typingStep);

    // Используем фиксированную ширину или пробелы, чтобы текст не "прыгал" влево-вправо
    statusLabel->setText(QString("печатает%1").arg(dots));
    statusLabel->setFont(QFont("Roboto", 11, QFont::Normal));
    statusLabel->setStyleSheet("color: #4e54c8;");
}

void ChatWidget::scrollToBottom() {
    if (m_bottomSpacerItem) {
        int topInset = m_topSpacerItem ? m_topSpacerItem->sizeHint().height() : 0;
        int bottomInset = m_bottomSpacerItem->sizeHint().height();
        int messageContentHeight = 0;

        for (int i = 0; i < listWidget->count(); ++i) {
            QListWidgetItem *item = listWidget->item(i);
            if (!item || item->data(Qt::UserRole).toString() != "message") {
                continue;
            }

            messageContentHeight += item->sizeHint().height();
        }

        int availableMessageArea = listWidget->viewport()->height() - topInset - bottomInset;
        if (messageContentHeight <= availableMessageArea) {
            listWidget->verticalScrollBar()->setValue(0);
            return;
        }

        listWidget->scrollToItem(m_bottomSpacerItem, QAbstractItemView::PositionAtBottom);
        return;
    }

    listWidget->scrollToBottom();
}

void ChatWidget::updateFirstMessageOffset() {
    if (!listWidget) {
        return;
    }

    int topInset = m_topSpacerItem ? m_topSpacerItem->sizeHint().height() : 0;
    int bottomInset = m_bottomSpacerItem ? m_bottomSpacerItem->sizeHint().height() : 0;
    int messageContentHeight = 0;

    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (!item || item->data(Qt::UserRole).toString() != "message") {
            continue;
        }

        messageContentHeight += item->sizeHint().height();
    }

    bool pinReserved = pinnedWidget && (pinnedWidget->isVisible() || !m_pinnedWidgetAnimated);
    bool shouldLowerFirstMessage = pinReserved
                                   && messageContentHeight <= (listWidget->viewport()->height() - topInset - bottomInset);

    bool firstMessageHandled = false;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (!item || item->data(Qt::UserRole).toString() != "message") {
            continue;
        }

        QWidget *container = listWidget->itemWidget(item);
        if (!container) {
            continue;
        }

        if (auto *layout = qobject_cast<QHBoxLayout *>(container->layout())) {
            QMargins margins = layout->contentsMargins();
            margins.setTop(!firstMessageHandled && shouldLowerFirstMessage ? 57 : 2);
            layout->setContentsMargins(margins);
            item->setSizeHint(container->sizeHint());
        }

        firstMessageHandled = true;
    }
}

void ChatWidget::updateListContentInsets(int bottomInsetOverride) {
    int topInset = 65;
    if (pinnedWidget && (pinnedWidget->isVisible() || !m_pinnedWidgetAnimated)) {
        topInset += 55;
    }

    int bottomInset = bottomInsetOverride >= 0 ? bottomInsetOverride : inputBar->height() + 5;
    bottomInset += 24;
    if (bottomInset < 75) {
        bottomInset = 75;
    }

    if (m_topSpacerItem) {
        m_topSpacerItem->setSizeHint(QSize(0, topInset));
    }

    if (m_bottomSpacerItem) {
        m_bottomSpacerItem->setSizeHint(QSize(0, bottomInset));
    }

    if (listWidget) {
        listWidget->doItemsLayout();
        listWidget->update();
        listWidget->viewport()->update();
    }

    updateFirstMessageOffset();
}

void ChatWidget::topBarCreate()
{
    // --- ВЕРХНЯЯ ПАНЕЛЬ (Top Bar) ---
    topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(60);
    // Белый фон с легким разделителем снизу
    topBar->setStyleSheet("QWidget#topBar { background: rgba(255, 255, 255, 0.9); border-bottom: 1px solid #eee; }");

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(15, 0, 15, 0);
    topLayout->setSpacing(12);

    // 1. Аватарка собеседника (используем нашу функцию getRoundedPixmap)
    QLabel *topAvatar = new QLabel();
    topAvatar->setFixedSize(40, 40);
    QPixmap avatarPix(Avatars::Bot);
    topAvatar->setPixmap(getRoundedPixmap(avatarPix, 40));
    topLayout->addWidget(topAvatar);

    // 2. Блок Имя + Статус (вертикальный)
    QVBoxLayout *nameStatusLayout = new QVBoxLayout();
    nameStatusLayout->setSpacing(2);
    nameStatusLayout->setContentsMargins(0, 10, 0, 10);

    QLabel *nameLabel = new QLabel("Бот-помощник");
    nameLabel->setFont(QFont("Roboto", 12, QFont::Medium));
    nameLabel->setStyleSheet("color: #222; border:none; background:transparent;");

    statusLabel = new QLabel("был(а) недавно");
    statusLabel->setFont(QFont("Roboto", 10, QFont::Normal));
    statusLabel->setStyleSheet("color: #888; border:none; background:transparent;");

    nameStatusLayout->addWidget(nameLabel);
    nameStatusLayout->addWidget(statusLabel);
    topLayout->addLayout(nameStatusLayout);

    topLayout->addStretch(); // Расталкиваем элементы в разные стороны

    // 3. Кнопки действий (Поиск и Настройки)
    auto createToolButton = [this](const QString &iconPath) {
        QPushButton *btn = new QPushButton();
        btn->setFixedSize(40, 40);
        btn->setCursor(Qt::PointingHandCursor);
        // Используем перекраску в серый #777
        btn->setIcon(QIcon(getColoredIcon(iconPath, QColor("#707579"), 200)));
        btn->setStyleSheet("QPushButton { border: none; background: transparent; border-radius: 18px; }"
                           "QPushButton:hover { background-color: rgba(0,0,0,0.05); }");
        return btn;
    };

    QPushButton *searchBtn = createToolButton(Icons::Search);
    QPushButton *menuBtn = createToolButton(Icons::More);

    topLayout->addWidget(searchBtn);
    topLayout->addWidget(menuBtn);
}

void ChatWidget::updateBackgroundPattern() {
    // Желаемый размер одного элемента (например, 150px)
    int targetSize = 1000;

    // Получаем пропорции SVG
    QSize svgSize = pattern.defaultSize();
    QSize scaledSize = svgSize;
    scaledSize.scale(targetSize, targetSize, Qt::KeepAspectRatio);

    // Создаем QImage с учетом Device Pixel Ratio (для 4K/Retina мониторов)
    qreal dpr = devicePixelRatioF();
    QImage image(scaledSize * dpr, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(dpr); // Важно для четкости на высоких разрешениях
    image.fill(Qt::transparent);

    QPainter imgPainter(&image);
    // Включаем все флаги качества
    imgPainter.setRenderHint(QPainter::Antialiasing);
    imgPainter.setRenderHint(QPainter::SmoothPixmapTransform);
    imgPainter.setRenderHint(QPainter::TextAntialiasing);

    // Рендерим SVG в чистый размер без полей
    pattern.render(&imgPainter, QRect(0, 0, scaledSize.width(), scaledSize.height()));
    imgPainter.end();

    // Создаем кисть из картинки
    m_backgroundPattern = QBrush(image);
}

QPixmap ChatWidget::getRoundedPixmap(const QPixmap &src, int size) {
    // Учитываем коэффициент пикселей (DPR) для четкости
    qreal dpr = qApp->devicePixelRatio();
    int targetPx = size * dpr;

    // Создаем прозрачное полотно
    QImage result(targetPx, targetPx, QImage::Format_ARGB32_Premultiplied);
    result.setDevicePixelRatio(dpr);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Создаем круглую маску
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);

    // Рисуем картинку, вписывая её в круг
    painter.drawPixmap(0, 0, size, size, src);
    painter.end();

    return QPixmap::fromImage(result);
}

QPixmap ChatWidget::getColoredIcon(const QString &path, const QColor &color, int size) {
    qreal dpr = devicePixelRatioF();

    // Создаем холст
    QPixmap pixmap(size * dpr, size * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QSvgRenderer renderer(path);

    // --- РЕШЕНИЕ: Добавляем отступ в 1-2 пикселя ---
    // Вместо того чтобы рисовать во весь rect(0,0,size,size),
    // рисуем чуть меньше и по центру
    int margin = 2;
    QRectF renderRect(margin, margin, size - 2 * margin, size - 2 * margin);

    renderer.render(&painter, renderRect);

    // Перекраска
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return pixmap;
}

void ChatWidget::animateItemRemoval(QListWidgetItem *item) {
    QWidget *container = listWidget->itemWidget(item);
    if (!container) return;

    // 1. Анимация прозрачности (исчезновение)
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(container);
    container->setGraphicsEffect(opacityEffect);

    QPropertyAnimation *opacityAnim = new QPropertyAnimation(opacityEffect, "opacity");
    opacityAnim->setDuration(300);
    opacityAnim->setStartValue(1.0);
    opacityAnim->setEndValue(0.0);
    opacityAnim->setEasingCurve(QEasingCurve::InCubic);

    // 2. Анимация "схлопывания" высоты
    // Мы анимируем maximumHeight, чтобы заставить Layout сжиматься
    QPropertyAnimation *heightAnim = new QPropertyAnimation(container, "maximumHeight");
    heightAnim->setDuration(350);
    heightAnim->setStartValue(container->height());
    heightAnim->setEndValue(0);
    heightAnim->setEasingCurve(QEasingCurve::InOutQuart);

    // Группируем их, чтобы они шли параллельно
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(opacityAnim);
    group->addAnimation(heightAnim);

    // 3. ВАЖНО: После завершения анимации удаляем итем из списка
    connect(group, &QParallelAnimationGroup::finished, this, [this, item, group]() {
        int row = listWidget->row(item);
        if (row != -1) {
            int removedMessageId = item->data(Qt::UserRole + 1).toInt();
            m_messageItems.remove(removedMessageId);
            if (m_pendingReplyMessageId == removedMessageId) {
                clearPendingReply();
            }

            // 1. Физически удаляем сообщение
            delete listWidget->takeItem(row);

            // 2. Ищем новое "последнее" сообщение, чтобы вернуть ему хвост
            if (listWidget->count() > 0) {
                // Берем самый нижний элемент
                QListWidgetItem* lastItem = listWidget->item(listWidget->count() - 1);
                QWidget* container = listWidget->itemWidget(lastItem);

                // Ищем в нем наш пузырь (он первый в layout контейнера)
                MessageBubble* lastBubble = container->findChild<MessageBubble*>();

                if (lastBubble) {
                    // Возвращаем хвост последнему выжившему
                    lastBubble->setTail(true);
                    // Обновляем глобальный указатель, чтобы следующее сообщение знало, у кого забирать хвост
                    m_lastMessageContainer = lastBubble;
                    // m_lastWasIncoming уже содержит актуальное значение, так как мы не меняли автора
                }
            } else {
                m_lastMessageContainer = nullptr;
            }
        }
        group->deleteLater();
    });

    group->start();
}

void ChatWidget::onUnpinClicked() {
    QDialog dialog(this);
    dialog.setModal(true);
    dialog.setAttribute(Qt::WA_StyledBackground, true);
    dialog.setAttribute(Qt::WA_TranslucentBackground, true);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    auto *outerLayout = new QVBoxLayout(&dialog);
    outerLayout->setContentsMargins(24, 24, 24, 24);

    auto *card = new QWidget(&dialog);
    card->setObjectName("unpinDialogCard");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 24, 24, 20);
    cardLayout->setSpacing(12);

    auto *titleLabel = new QLabel("Открепить сообщение", card);
    titleLabel->setObjectName("unpinDialogTitle");
    titleLabel->setWordWrap(true);
    QFont titleFont("Roboto", 20, QFont::DemiBold);
    titleFont.setStyleStrategy(QFont::PreferAntialias);
    titleLabel->setFont(titleFont);

    auto *questionLabel = new QLabel("Хотите открепить сообщение?", card);
    questionLabel->setObjectName("unpinDialogQuestion");
    questionLabel->setWordWrap(true);
    QFont questionFont("Roboto", 14, QFont::Normal);
    questionFont.setStyleStrategy(QFont::PreferAntialias);
    questionLabel->setFont(questionFont);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 8, 0, 0);
    buttonLayout->setSpacing(12);

    QPushButton *cancel = new QPushButton("Отмена", card);
    QPushButton *yes = new QPushButton("Открепить", card);
    yes->setObjectName("unpinConfirmButton");

    QFont buttonFont("Roboto", 15, QFont::Medium);
    buttonFont.setStyleStrategy(QFont::PreferAntialias);
    cancel->setFont(buttonFont);
    yes->setFont(buttonFont);

    buttonLayout->addWidget(cancel);
    buttonLayout->addWidget(yes);

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(questionLabel);
    cardLayout->addLayout(buttonLayout);
    outerLayout->addWidget(card);

    dialog.setStyleSheet(
        "QDialog { background: transparent; }"
        "QWidget#unpinDialogCard {"
        "  background-color: white;"
        "  border: 1px solid rgba(78, 84, 200, 0.10);"
        "  border-radius: 24px;"
        "}"
        "QLabel#unpinDialogTitle {"
        "  color: #1f2233;"
        "  background: transparent;"
        "}"
        "QLabel#unpinDialogQuestion {"
        "  color: #5f6475;"
        "  background: transparent;"
        "}"
        "QPushButton {"
        "  background: white;"
        "  border: none;"
        "  border-radius: 14px;"
        "  color: #4e54c8;"
        "  padding: 12px 18px;"
        "  min-width: 132px;"
        "}"
        "QPushButton:hover {"
        "  background: #f6f7ff;"
        "}"
        "QPushButton:pressed {"
        "  background: #eef0ff;"
        "}"
        "QPushButton#unpinConfirmButton {"
        "  color: #e14b5a;"
        "}"
        "QPushButton#unpinConfirmButton:hover {"
        "  background: #fff1f3;"
        "}"
        "QPushButton#unpinConfirmButton:pressed {"
        "  background: #ffe4e8;"
        "}"
        );

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(yes, &QPushButton::clicked, &dialog, &QDialog::accept);

    for (QPushButton *button : {cancel, yes}) {
        auto *shadow = new QGraphicsDropShadowEffect(button);
        shadow->setBlurRadius(0);
        shadow->setOffset(0, 0);
        shadow->setColor(Qt::transparent);
        button->setGraphicsEffect(shadow);
        button->setCursor(Qt::PointingHandCursor);
        button->setProperty("dialogHoverShadow", true);
        button->installEventFilter(this);
    }

    if (dialog.exec() == QDialog::Accepted) {
        pinnedWidget->animateHide(60); // 60 - высота вашего TopBar
    }
}

void ChatWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    // 1. Градиент
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor("#4e54c8"));
    gradient.setColorAt(1, QColor("#8f94fb"));
    painter.fillRect(rect(), gradient);

    // 2. Накладываем паттерн (уже готовый и четкий)
    painter.setOpacity(0.25);
    painter.setBrush(m_backgroundPattern);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    // 3. Вызов базы (если нужно)
    // QWidget::paintEvent(event);
}

void ChatWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);

    if (m_initialLayoutApplied) {
        return;
    }

    m_initialLayoutApplied = true;
    QTimer::singleShot(0, this, [this]() {
        QResizeEvent initialResize(size(), size());
        resizeEvent(&initialResize);
        scrollToBottom();
    });
}

void ChatWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateBackgroundPattern();

    // 1. КОНСТАНТЫ И ПАРАМЕТРЫ
    int topHeight = 60;
    int sidePadding = 20;
    int currentInputHeight = inputBar->height();
    if (currentInputHeight < 70) currentInputHeight = 70; // Страховка

    // 2. ВЕРХНЯЯ ПАНЕЛЬ (Top Bar)
    topBar->setGeometry(0, 0, width(), topHeight);

    // 3. ЗАКРЕПЛЕННОЕ СООБЩЕНИЕ (Pinned Message)
    // Размещаем его строго между топ-баром и списком
    if (pinnedWidget && !m_pinnedWidgetAnimated && width() > (sidePadding * 2)) {
        m_pinnedWidgetAnimated = true;
        pinnedWidget->setGeometry(sidePadding, topHeight + 5, width() - (sidePadding * 2), 50);
        pinnedWidget->animateShow(topHeight);
        pinnedWidget->raise();

    } else if (pinnedWidget && pinnedWidget->isVisible()) {
        int pWidth = width() - (sidePadding * 2);
        // Позиция: X=20, Y=65 (сразу под топ-баром), ширина = окно - 40
        pinnedWidget->setGeometry(sidePadding, topHeight + 5, pWidth, 50);
        pinnedWidget->raise();
    }

    // 4. НИЖНЯЯ ПАНЕЛЬ (Input Bar)
    // Всегда прижата к самому низу окна
    inputBar->setGeometry(0, height() - currentInputHeight, width(), currentInputHeight);
    if (inputBlurOverlay) {
        inputBlurOverlay->setGeometry(0, height() - currentInputHeight - 18, width(), currentInputHeight + 36);
        inputBlurOverlay->raise();
        inputBar->raise();
    }

    // 5. СПИСОК СООБЩЕНИЙ (List Widget)
    // Занимает все окно, но контент (сообщения) ограничен падингами
    listWidget->setGeometry(0, 0, width(), height());
    updateListContentInsets(currentInputHeight + 5);
}


bool ChatWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == messageEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                sendButton->click(); // Имитируем нажатие "Отправить"
                return true; // Событие обработано
            }
        }
    }

    if (obj->property("dialogHoverShadow").toBool()) {
        if (auto *button = qobject_cast<QPushButton *>(obj)) {
            if (auto *shadow = qobject_cast<QGraphicsDropShadowEffect *>(button->graphicsEffect())) {
                if (event->type() == QEvent::Enter) {
                    QColor shadowColor = button->objectName() == "unpinConfirmButton"
                                             ? QColor(225, 75, 90, 70)
                                             : QColor(78, 84, 200, 55);
                    shadow->setBlurRadius(24);
                    shadow->setOffset(0, 8);
                    shadow->setColor(shadowColor);
                } else if (event->type() == QEvent::Leave) {
                    shadow->setBlurRadius(0);
                    shadow->setOffset(0, 0);
                    shadow->setColor(Qt::transparent);
                }
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}
