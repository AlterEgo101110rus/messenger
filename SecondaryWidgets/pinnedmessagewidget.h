#ifndef PINNEDMESSAGEWIDGET_H
#define PINNEDMESSAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPropertyAnimation>

class PinnedMessageWidget : public QWidget {
    Q_OBJECT
public:
    explicit PinnedMessageWidget(const QString &text, QWidget *parent = nullptr);
    void updateText(const QString &text);

    void animateShow(int topBarHeight);
    void animateHide(int topBarHeight);

    void setPinIcon(QPixmap pix);
    void setCloseBtnIcon(QIcon icon);

signals:
    void clicked();         // Клик по тексту (для скролла)
    void unpinRequested();  // Клик по крестику
    void animationFinished();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QLabel *m_pinIcon;
    QLabel *m_textLabel;
    QPushButton *m_closeBtn;
};

#endif // PINNEDMESSAGEWIDGET_H
