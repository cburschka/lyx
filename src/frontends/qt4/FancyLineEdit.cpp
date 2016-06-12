/**
 * \file fancylineedit.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Nokia Corporation (qt-info@nokia.com)
 *
 * Full author contact details are available in file CREDITS.
 *
 */

// Code taken from the Qt Creator project and customized a little

#include <config.h>

#include "FancyLineEdit.h"

#if QT_VERSION >= 0x040600

#include <QEvent>
#include <QDebug>
#include <QString>
#include <QPropertyAnimation>
#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QLabel>
#include <QAbstractButton>
#include <QPainter>
#include <QStyle>
#include <QPaintEvent>

enum { margin = 6 };

#define ICONBUTTON_HEIGHT 18
#define FADE_TIME 160


namespace lyx {
namespace frontend {

// --------- FancyLineEditPrivate
class FancyLineEditPrivate : public QObject {
public:
    explicit FancyLineEditPrivate(FancyLineEdit *parent);

    virtual bool eventFilter(QObject *obj, QEvent *event);

    FancyLineEdit  *m_lineEdit;
    QPixmap m_pixmap[2];
    QMenu *m_menu[2];
    bool m_menuTabFocusTrigger[2];
    IconButton *m_iconbutton[2];
    bool m_iconEnabled[2];
};


FancyLineEditPrivate::FancyLineEditPrivate(FancyLineEdit *parent) :
    QObject(parent),
    m_lineEdit(parent)
{
    for (int i = 0; i < 2; ++i) {
        m_menu[i] = 0;
        m_menuTabFocusTrigger[i] = false;
        m_iconbutton[i] = new IconButton(parent);
        m_iconbutton[i]->installEventFilter(this);
        m_iconbutton[i]->hide();
        m_iconbutton[i]->setAutoHide(false);
        m_iconEnabled[i] = false;
    }
}

bool FancyLineEditPrivate::eventFilter(QObject *obj, QEvent *event)
{
    int buttonIndex = -1;
    for (int i = 0; i < 2; ++i) {
        if (obj == m_iconbutton[i]) {
            buttonIndex = i;
            break;
        }
    }
    if (buttonIndex == -1)
        return QObject::eventFilter(obj, event);
    switch (event->type()) {
    case QEvent::FocusIn:
        if (m_menuTabFocusTrigger[buttonIndex] && m_menu[buttonIndex]) {
            m_lineEdit->setFocus();
            m_menu[buttonIndex]->exec(m_iconbutton[buttonIndex]->mapToGlobal(
                    m_iconbutton[buttonIndex]->rect().center()));
            return true;
        }
    default:
        break;
    }
    return QObject::eventFilter(obj, event);
}


// --------- FancyLineEdit
FancyLineEdit::FancyLineEdit(QWidget *parent) :
    QLineEdit(parent),
    m_d(new FancyLineEditPrivate(this))
{
    ensurePolished();
    updateMargins();

    connect(this, SIGNAL(textChanged(QString)), this, SLOT(checkButtons(QString)));
    connect(m_d->m_iconbutton[Left], SIGNAL(clicked()), this, SLOT(iconClicked()));
    connect(m_d->m_iconbutton[Right], SIGNAL(clicked()), this, SLOT(iconClicked()));
}

void FancyLineEdit::checkButtons(const QString &text)
{
    if (m_oldText.isEmpty() || text.isEmpty()) {
        for (int i = 0; i < 2; ++i) {
            if (m_d->m_iconbutton[i]->hasAutoHide())
                m_d->m_iconbutton[i]->animateShow(!text.isEmpty());
        }
        m_oldText = text;
    }
}

FancyLineEdit::~FancyLineEdit()
{
}

void FancyLineEdit::setButtonVisible(Side side, bool visible)
{
    m_d->m_iconbutton[side]->setVisible(visible);
    m_d->m_iconEnabled[side] = visible;
    updateMargins();
}

bool FancyLineEdit::isButtonVisible(Side side) const
{
    return m_d->m_iconEnabled[side];
}

void FancyLineEdit::iconClicked()
{
    IconButton *button = qobject_cast<IconButton *>(sender());
    int index = -1;
    for (int i = 0; i < 2; ++i)
        if (m_d->m_iconbutton[i] == button)
            index = i;
    if (index == -1)
        return;
    if (m_d->m_menu[index]) {
        m_d->m_menu[index]->exec(QCursor::pos());
    } else {
		buttonClicked((Side)index);
        if (index == Left)
			leftButtonClicked();
        else if (index == Right)
			rightButtonClicked();
    }
}

void FancyLineEdit::updateMargins()
{
    bool leftToRight = (layoutDirection() == Qt::LeftToRight);
    Side realLeft = (leftToRight ? Left : Right);
    Side realRight = (leftToRight ? Right : Left);

    int leftMargin = m_d->m_iconbutton[realLeft]->pixmap().width() + 8;
    int rightMargin = m_d->m_iconbutton[realRight]->pixmap().width() + 8;
    // Note KDE does not reserve space for the highlight color
    if (style()->inherits("OxygenStyle")) {
        leftMargin = qMax(24, leftMargin);
        rightMargin = qMax(24, rightMargin);
    }

    QMargins margins((m_d->m_iconEnabled[realLeft] ? leftMargin : 0), 0,
                     (m_d->m_iconEnabled[realRight] ? rightMargin : 0), 0);

    setTextMargins(margins);
}

void FancyLineEdit::updateButtonPositions()
{
    QRect contentRect = rect();
    for (int i = 0; i < 2; ++i) {
        Side iconpos = (Side)i;
        if (layoutDirection() == Qt::RightToLeft)
            iconpos = (iconpos == Left ? Right : Left);

        if (iconpos == FancyLineEdit::Right) {
            const int iconoffset = textMargins().right() + 4;
            m_d->m_iconbutton[i]->setGeometry(contentRect.adjusted(width() - iconoffset, 0, 0, 0));
        } else {
            const int iconoffset = textMargins().left() + 4;
            m_d->m_iconbutton[i]->setGeometry(contentRect.adjusted(0, 0, -width() + iconoffset, 0));
        }
    }
}

void FancyLineEdit::resizeEvent(QResizeEvent *)
{
    updateButtonPositions();
}

void FancyLineEdit::setButtonPixmap(Side side, const QPixmap &buttonPixmap)
{
    m_d->m_iconbutton[side]->setPixmap(buttonPixmap);
    updateMargins();
    updateButtonPositions();
    update();
}

QPixmap FancyLineEdit::buttonPixmap(Side side) const
{
    return m_d->m_pixmap[side];
}

void FancyLineEdit::setButtonMenu(Side side, QMenu *buttonMenu)
{
     m_d->m_menu[side] = buttonMenu;
     m_d->m_iconbutton[side]->setIconOpacity(1.0);
 }

QMenu *FancyLineEdit::buttonMenu(Side side) const
{
    return  m_d->m_menu[side];
}

bool FancyLineEdit::hasMenuTabFocusTrigger(Side side) const
{
    return m_d->m_menuTabFocusTrigger[side];
}

void FancyLineEdit::setMenuTabFocusTrigger(Side side, bool v)
{
    if (m_d->m_menuTabFocusTrigger[side] == v)
        return;

    m_d->m_menuTabFocusTrigger[side] = v;
    m_d->m_iconbutton[side]->setFocusPolicy(v ? Qt::TabFocus : Qt::NoFocus);
}

bool FancyLineEdit::hasAutoHideButton(Side side) const
{
    return m_d->m_iconbutton[side]->hasAutoHide();
}

void FancyLineEdit::setAutoHideButton(Side side, bool h)
{
    m_d->m_iconbutton[side]->setAutoHide(h);
    if (h)
        m_d->m_iconbutton[side]->setIconOpacity(text().isEmpty() ?  0.0 : 1.0);
    else
        m_d->m_iconbutton[side]->setIconOpacity(1.0);
}

void FancyLineEdit::setButtonToolTip(Side side, const QString &tip)
{
    m_d->m_iconbutton[side]->setToolTip(tip);
}

void FancyLineEdit::setButtonFocusPolicy(Side side, Qt::FocusPolicy policy)
{
    m_d->m_iconbutton[side]->setFocusPolicy(policy);
}

// IconButton - helper class to represent a clickable icon

IconButton::IconButton(QWidget *parent)
    : QAbstractButton(parent), m_iconOpacity(0.0), m_autoHide(false)
{
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::NoFocus);
}

void IconButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect pixmapRect = QRect(0, 0, m_pixmap.width(), m_pixmap.height());
    pixmapRect.moveCenter(rect().center());

    if (m_autoHide)
        painter.setOpacity(m_iconOpacity);

    painter.drawPixmap(pixmapRect, m_pixmap);
}

void IconButton::animateShow(bool visible)
{
    if (visible) {
        QPropertyAnimation *animation = new QPropertyAnimation(this, "iconOpacity");
        animation->setDuration(FADE_TIME);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        QPropertyAnimation *animation = new QPropertyAnimation(this, "iconOpacity");
        animation->setDuration(FADE_TIME);
        animation->setEndValue(0.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

}

}

#endif // QT_VERSION >= 0x040600

#include "moc_FancyLineEdit.cpp"
