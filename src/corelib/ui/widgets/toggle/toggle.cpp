#include "toggle.h"

#include <ui/design_system/design_system.h>
#include <ui/widgets/animations/click_animation.h>
#include <utils/helpers/color_helper.h>
#include <utils/helpers/image_helper.h>

#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>


class Toggle::Implementation
{
public:
    explicit Implementation(Toggle* _q);

    /**
     * @brief Размер контента
     */
    QSizeF contentSize() const;

    /**
     * @brief Анимировать переключение
     */
    void animateToggle();

    /**
     * @brief Прекратить аниацию, если виджет невидим
     */
    void finishAnimationIfInvisible();


    Toggle* q = nullptr;
    bool isChecked = true;
    QVariantAnimation tumblerAnimation;

    /**
     * @brief  Декорации переключателя при клике
     */
    ClickAnimation decorationAnimation;
};

Toggle::Implementation::Implementation(Toggle* _q)
    : q(_q)
{
    tumblerAnimation.setDuration(160);
    tumblerAnimation.setEasingCurve(QEasingCurve::OutQuad);
}

QSizeF Toggle::Implementation::contentSize() const
{
    return QSizeF(q->contentsMargins().left() + Ui::DesignSystem::layout().px12()
                      + Ui::DesignSystem::toggle().tumblerOverflow()
                      + Ui::DesignSystem::toggle().trackSize().width()
                      + Ui::DesignSystem::toggle().tumblerOverflow()
                      + Ui::DesignSystem::layout().px12() + q->contentsMargins().right(),
                  q->contentsMargins().top() + Ui::DesignSystem::layout().px12()
                      + Ui::DesignSystem::toggle().tumblerSize().height()
                      + Ui::DesignSystem::layout().px12() + q->contentsMargins().bottom());
}

void Toggle::Implementation::animateToggle()
{
    //
    // Определим крайние положения переключателя
    //

    const QRectF toggleLeftRect({ q->contentsMargins().left() + Ui::DesignSystem::layout().px12(),
                                  q->contentsMargins().top() + Ui::DesignSystem::layout().px12() },
                                Ui::DesignSystem::toggle().tumblerSize());
    const QRectF toggleRightRect({ q->contentsMargins().left() + Ui::DesignSystem::layout().px12()
                                       + Ui::DesignSystem::toggle().tumblerOverflow() * 2
                                       + Ui::DesignSystem::toggle().trackSize().width()
                                       - Ui::DesignSystem::toggle().tumblerSize().width(),
                                   toggleLeftRect.top() },
                                 toggleLeftRect.size());
    if ((isChecked && q->isLeftToRight()) || (!isChecked && q->isRightToLeft())) {
        tumblerAnimation.setStartValue(toggleLeftRect);
        tumblerAnimation.setEndValue(toggleRightRect);
    } else {
        tumblerAnimation.setStartValue(toggleRightRect);
        tumblerAnimation.setEndValue(toggleLeftRect);
    }
    tumblerAnimation.start();

    //
    // Если переключатель скрыт, то выполнять всю анимацию ни к чему
    //
    if (!q->isVisible()) {
        tumblerAnimation.setCurrentTime(tumblerAnimation.duration());
    }
}


// ****


Toggle::Toggle(QWidget* _parent)
    : Widget(_parent)
    , d(new Implementation(this))
{
    setAttribute(Qt::WA_Hover);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    d->animateToggle();

    connect(&d->tumblerAnimation, &QVariantAnimation::valueChanged, this,
            qOverload<>(&Toggle::update));
    connect(&d->decorationAnimation, &ClickAnimation::valueChanged, this,
            qOverload<>(&Toggle::update));
}

bool Toggle::isChecked() const
{
    return d->isChecked;
}

Toggle::~Toggle() = default;

void Toggle::setChecked(bool _checked)
{
    if (d->isChecked == _checked) {
        return;
    }

    d->isChecked = _checked;
    d->animateToggle();
    emit checkedChanged(d->isChecked);
}

void Toggle::toggle()
{
    setChecked(!isChecked());
}

QSize Toggle::minimumSizeHint() const
{
    return d->contentSize().toSize();
}

bool Toggle::event(QEvent* _event)
{
    if (_event->type() == QEvent::ContentsRectChange
        || _event->type() == QEvent::LayoutDirectionChange) {
        d->animateToggle();
    }

    return Widget::event(_event);
}

void Toggle::paintEvent(QPaintEvent* _event)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(_event->rect(), backgroundColor());

    QColor tumblerColor;
    QColor trackColor;

    if (isEnabled()) {
        tumblerColor = d->isChecked
            ? Ui::DesignSystem::color().accent()
            : (ColorHelper::isColorLight(backgroundColor()) ? backgroundColor() : textColor());
        trackColor
            = d->isChecked ? tumblerColor.lighter(140) : ColorHelper::nearby(tumblerColor, 160);
    } else {
        tumblerColor = d->isChecked
            ? Ui::DesignSystem::color().accent().darker(70)
            : (ColorHelper::isColorLight(backgroundColor()) ? backgroundColor()
                                                            : textColor().darker(130));
        trackColor
            = d->isChecked ? tumblerColor.lighter(110) : ColorHelper::nearby(tumblerColor, 110);
    }

    //
    // Трэк
    //
    painter.setBrush(trackColor);
    const QRectF trackRect({ contentsMargins().left() + Ui::DesignSystem::layout().px12()
                                 + Ui::DesignSystem::toggle().tumblerOverflow(),
                             contentsMargins().top() + Ui::DesignSystem::layout().px12()
                                 + Ui::DesignSystem::toggle().tumblerOverflow() },
                           Ui::DesignSystem::toggle().trackSize());

    painter.drawRoundedRect(trackRect, trackRect.height() / 2.0, trackRect.height() / 2.0);

    //
    // Переключатель
    //
    painter.setBrush(isEnabled() ? tumblerColor : tumblerColor.lighter(150));
    const QRectF toggleRect = d->tumblerAnimation.currentValue().toRectF();
    const qreal borderRadius = toggleRect.height() / 2.0;
    const qreal disabledOpacity = isEnabled() ? 1.0 : 0.3;
    painter.setOpacity(disabledOpacity);
    //
    // ... подготовим тень
    //
    static QPixmap backgroundImage;
    if (backgroundImage.size() != toggleRect.size().toSize()) {
        backgroundImage = QPixmap(toggleRect.size().toSize());
        backgroundImage.fill(Qt::transparent);
        QPainter backgroundImagePainter(&backgroundImage);
        backgroundImagePainter.setPen(Qt::NoPen);
        backgroundImagePainter.setBrush(isEnabled()
                                            ? Ui::DesignSystem::color().textEditor()
                                            : Ui::DesignSystem::color().textEditor().lighter(150));
        backgroundImagePainter.setOpacity(disabledOpacity);
        backgroundImagePainter.drawRoundedRect(QRect({ 0, 0 }, backgroundImage.size()),
                                               borderRadius, borderRadius);
    }
    //
    // ... рисуем тень
    //
    const qreal shadowHeight = Ui::DesignSystem::card().minimumShadowBlurRadius();
    const bool useCache = true;
    const QPixmap shadow = ImageHelper::dropShadow(
        backgroundImage, Ui::DesignSystem::card().shadowMargins(), shadowHeight,
        isEnabled() ? Ui::DesignSystem::color().shadow()
                    : Ui::DesignSystem::color().shadow().lighter(150),
        useCache);
    painter.setOpacity(1.0);
    painter.drawPixmap(toggleRect.topLeft()
                           - QPointF{ Ui::DesignSystem::card().shadowMargins().left(),
                                      Ui::DesignSystem::card().shadowMargins().top() },
                       shadow);
    //
    // ... рисуем декорацию
    //
    if (isEnabled() && (underMouse() || hasFocus())) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(isChecked() ? Ui::DesignSystem::color().accent() : textColor());
        painter.setOpacity(hasFocus() ? Ui::DesignSystem::focusBackgroundOpacity()
                                      : Ui::DesignSystem::hoverBackgroundOpacity());
        const auto radius = d->decorationAnimation.maximumRadius();
        painter.drawEllipse(toggleRect.center(), radius, radius);
        painter.setOpacity(1.0);
    }
    if (isEnabled() && d->decorationAnimation.state() == ClickAnimation::Running) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(isChecked() ? Ui::DesignSystem::color().accent() : textColor());
        painter.setOpacity(d->decorationAnimation.opacity());
        const auto radius = d->decorationAnimation.radius();
        painter.drawEllipse(toggleRect.center(), radius, radius);
        painter.setOpacity(1.0);
    }
    painter.setBrush(tumblerColor);
    //
    // ... рисуем сам переключатель
    //
    painter.drawRoundedRect(toggleRect, toggleRect.width() / 2.0, toggleRect.width() / 2.0);
}

void Toggle::mousePressEvent(QMouseEvent* _event)
{
    Q_UNUSED(_event);
    d->decorationAnimation.start();
}

void Toggle::mouseReleaseEvent(QMouseEvent* _event)
{
    if (!rect().contains(_event->pos())) {
        return;
    }

    setChecked(!d->isChecked);
}

void Toggle::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    Q_UNUSED(_event)

    d->decorationAnimation.setRadiusInterval(Ui::DesignSystem::toggle().tumblerSize().height()
                                                 / 2.0,
                                             Ui::DesignSystem::toggle().tumblerSize().height());

    updateGeometry();
    update();
}
