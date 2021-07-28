#include "icon_button.h"

#include <ui/design_system/design_system.h>

#include <QPaintEvent>
#include <QPainter>
#include <QVariantAnimation>


class IconButton::Implementation
{
public:
    Implementation();

    /**
     * @brief Анимировать клик
     */
    void animateClick();


    bool isChackable = false;
    bool isChecked = false;
    QString icon;

    /**
     * @brief  Декорации переключателя при клике
     */
    QVariantAnimation decorationRadiusAnimation;
    QVariantAnimation decorationOpacityAnimation;
};

IconButton::Implementation::Implementation()
{
    decorationRadiusAnimation.setEasingCurve(QEasingCurve::OutQuad);
    decorationRadiusAnimation.setDuration(160);

    decorationOpacityAnimation.setEasingCurve(QEasingCurve::InQuad);
    decorationOpacityAnimation.setStartValue(0.5);
    decorationOpacityAnimation.setEndValue(0.0);
    decorationOpacityAnimation.setDuration(160);
}

void IconButton::Implementation::animateClick()
{
    decorationOpacityAnimation.setCurrentTime(0);
    decorationRadiusAnimation.start();
    decorationOpacityAnimation.start();
}


// ****

IconButton::IconButton(QWidget* _parent)
    : Widget(_parent)
    , d(new Implementation)
{
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(&d->decorationRadiusAnimation, &QVariantAnimation::valueChanged, this,
            [this] { update(); });
    connect(&d->decorationOpacityAnimation, &QVariantAnimation::valueChanged, this,
            [this] { update(); });

    designSystemChangeEvent(nullptr);
}

IconButton::~IconButton() = default;

void IconButton::setCheckable(bool _checkable)
{
    d->isChackable = _checkable;
}

bool IconButton::isChecked() const
{
    return d->isChecked;
}

void IconButton::setChecked(bool _checked)
{
    if (!d->isChackable) {
        return;
    }

    if (d->isChecked == _checked) {
        return;
    }

    d->isChecked = _checked;
    emit checkedChanged(d->isChecked);
    update();
}

void IconButton::setIcon(const QString& _icon)
{
    if (d->icon == _icon) {
        return;
    }

    d->icon = _icon;
    update();
}

QSize IconButton::sizeHint() const
{
    return QRectF({}, Ui::DesignSystem::toggleButton().size())
        .marginsAdded(contentsMargins())
        .size()
        .toSize();
}

void IconButton::paintEvent(QPaintEvent* _event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //
    // Заливаем фон
    //
    painter.fillRect(_event->rect(), backgroundColor());
    painter.translate(contentsRect().topLeft());

    //
    // Рисуем декорацию кнопки
    //
    const QRectF iconRect(QPointF(Ui::DesignSystem::toggleButton().margins().left(),
                                  Ui::DesignSystem::toggleButton().margins().top()),
                          Ui::DesignSystem::toggleButton().iconSize());
    if (d->decorationRadiusAnimation.state() == QVariantAnimation::Running
        || d->decorationOpacityAnimation.state() == QVariantAnimation::Running) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(Ui::DesignSystem::color().secondary());
        painter.setOpacity(d->decorationOpacityAnimation.currentValue().toReal());
        painter.drawEllipse(iconRect.center(), d->decorationRadiusAnimation.currentValue().toReal(),
                            d->decorationRadiusAnimation.currentValue().toReal());
        painter.setOpacity(1.0);
    }

    //
    // Рисуем иконку
    //
    painter.setFont(Ui::DesignSystem::font().iconsMid());
    painter.setPen(d->isChecked ? Ui::DesignSystem::color().secondary() : textColor());
    painter.drawText(iconRect, Qt::AlignCenter, d->icon);
}

void IconButton::mouseReleaseEvent(QMouseEvent* _event)
{
    Q_UNUSED(_event);

    if (!rect().contains(_event->pos())) {
        return;
    }

    setChecked(!d->isChecked);
    d->animateClick();

    emit clicked();
}

void IconButton::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    Q_UNUSED(_event);

    d->decorationRadiusAnimation.setStartValue(Ui::DesignSystem::toggleButton().iconSize().height()
                                               / 2.0);
    d->decorationRadiusAnimation.setEndValue(Ui::DesignSystem::toggleButton().size().height()
                                             / 2.5);

    updateGeometry();
    update();
}
