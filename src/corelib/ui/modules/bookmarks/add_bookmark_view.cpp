#include "add_bookmark_view.h"

#include <ui/design_system/design_system.h>
#include <ui/widgets/button/button.h>
#include <ui/widgets/color_picker/color_picker_popup.h>
#include <ui/widgets/scroll_bar/scroll_bar.h>
#include <ui/widgets/text_field/text_field.h>
#include <utils/helpers/ui_helper.h>

#include <QKeyEvent>
#include <QScrollArea>
#include <QVBoxLayout>


namespace Ui {

class AddBookmarkView::Implementation
{
public:
    explicit Implementation(QWidget* _parent);

    QScrollArea* content = nullptr;
    TextField* bookmarkName = nullptr;
    ColorPickerPopup* bookmarkColorPopup = nullptr;
    TextField* bookmarkText = nullptr;
    QHBoxLayout* buttonsLayout = nullptr;
    Button* cancelButton = nullptr;
    Button* saveButton = nullptr;
};

AddBookmarkView::Implementation::Implementation(QWidget* _parent)
    : content(new QScrollArea(_parent))
    , bookmarkName(new TextField(_parent))
    , bookmarkColorPopup(new ColorPickerPopup(_parent))
    , bookmarkText(new TextField(_parent))
    , buttonsLayout(new QHBoxLayout)
    , cancelButton(new Button(_parent))
    , saveButton(new Button(_parent))
{
    QPalette palette;
    palette.setColor(QPalette::Base, Qt::transparent);
    palette.setColor(QPalette::Window, Qt::transparent);
    content->setPalette(palette);
    content->setFrameShape(QFrame::NoFrame);
    content->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    content->setVerticalScrollBar(new ScrollBar);

    UiHelper::initSpellingFor(bookmarkName);
    bookmarkName->setEnterMakesNewLine(true);
    bookmarkName->setTrailingIcon(u8"\U000F0765");
    bookmarkColorPopup->setColorCanBeDeselected(false);
    bookmarkColorPopup->setSelectedColor(Qt::red);
    UiHelper::initSpellingFor(bookmarkText);
    bookmarkText->setEnterMakesNewLine(true);

    buttonsLayout->setContentsMargins({});
    buttonsLayout->setSpacing(0);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(saveButton);
}


// ****


AddBookmarkView::AddBookmarkView(QWidget* _parent)
    : Widget(_parent)
    , d(new Implementation(this))
{
    setFocusProxy(d->bookmarkText);

    d->bookmarkText->installEventFilter(this);

    QWidget* contentWidget = new QWidget;
    d->content->setWidgetResizable(true);
    d->content->setWidget(contentWidget);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins({});
    contentLayout->setSpacing(0);
    contentLayout->addWidget(d->bookmarkName);
    contentLayout->addWidget(d->bookmarkText);
    contentLayout->addLayout(d->buttonsLayout);
    contentLayout->addStretch();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(d->content);


    connect(d->bookmarkName, &TextField::trailingIconPressed, this, [this] {
        d->bookmarkColorPopup->showPopup(d->bookmarkText, Qt::AlignBottom | Qt::AlignRight);
    });
    connect(d->bookmarkColorPopup, &ColorPickerPopup::selectedColorChanged, this,
            [this](const QColor& _color) { d->bookmarkName->setTrailingIconColor(_color); });
    connect(d->bookmarkText, &TextField::cursorPositionChanged, this, [this] {
        if (!d->bookmarkText->hasFocus()) {
            return;
        }

        d->content->ensureVisible(
            0, d->bookmarkText->pos().y() + d->bookmarkText->cursorRect().bottom());
    });
    connect(d->saveButton, &Button::clicked, this, &AddBookmarkView::savePressed);
    connect(d->cancelButton, &Button::clicked, this, &AddBookmarkView::cancelPressed);


    updateTranslations();
    designSystemChangeEvent(nullptr);
}

AddBookmarkView::~AddBookmarkView() = default;

QString AddBookmarkView::name() const
{
    return d->bookmarkName->text();
}

void AddBookmarkView::setName(const QString& _name)
{
    d->bookmarkName->setText(_name);
}

QColor AddBookmarkView::color() const
{
    return d->bookmarkColorPopup->selectedColor();
}

void AddBookmarkView::setColor(const QColor& _color)
{
    if (d->bookmarkColorPopup->selectedColor() == _color) {
        return;
    }

    d->bookmarkColorPopup->setSelectedColor(_color);
}

QString AddBookmarkView::text() const
{
    return d->bookmarkText->text();
}

void AddBookmarkView::setText(const QString& _text)
{
    d->bookmarkText->setText(_text);
}

bool AddBookmarkView::eventFilter(QObject* _watched, QEvent* _event)
{
    if ((_watched == d->bookmarkName || _watched == d->bookmarkText)
        && _event->type() == QEvent::KeyPress) {
        const auto keyEvent = static_cast<QKeyEvent*>(_event);
        if (keyEvent->key() == Qt::Key_Escape) {
            emit cancelPressed();
        } else if (!keyEvent->modifiers().testFlag(Qt::ShiftModifier)
                   && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) {
            emit savePressed();
            return true;
        }
    }

    return Widget::eventFilter(_watched, _event);
}

void AddBookmarkView::updateTranslations()
{
    d->bookmarkName->setLabel(tr("Bookmark name"));
    d->bookmarkName->setTrailingIconToolTip(tr("Select bookmark color"));
    d->bookmarkText->setLabel(tr("Description"));
    d->cancelButton->setText(tr("Cancel"));
    d->saveButton->setText(tr("Save"));
}

void AddBookmarkView::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    Widget::designSystemChangeEvent(_event);

    setBackgroundColor(Ui::DesignSystem::color().primary());

    d->content->widget()->layout()->setContentsMargins(0, Ui::DesignSystem::layout().px24(), 0, 0);
    d->content->widget()->layout()->setSpacing(Ui::DesignSystem::layout().px12());

    for (auto textField : {
             d->bookmarkName,
             d->bookmarkText,
         }) {
        textField->setBackgroundColor(Ui::DesignSystem::color().onPrimary());
        textField->setTextColor(Ui::DesignSystem::color().onPrimary());
    }

    d->bookmarkColorPopup->setBackgroundColor(Ui::DesignSystem::color().background());
    d->bookmarkColorPopup->setTextColor(Ui::DesignSystem::color().onBackground());

    d->buttonsLayout->setContentsMargins(
        0, 0, Ui::DesignSystem::layout().px12() + Ui::DesignSystem::layout().px2(), 0);
    d->cancelButton->setBackgroundColor(Ui::DesignSystem::color().secondary());
    d->cancelButton->setTextColor(Ui::DesignSystem::color().secondary());
    d->saveButton->setBackgroundColor(Ui::DesignSystem::color().secondary());
    d->saveButton->setTextColor(Ui::DesignSystem::color().secondary());
}

} // namespace Ui
