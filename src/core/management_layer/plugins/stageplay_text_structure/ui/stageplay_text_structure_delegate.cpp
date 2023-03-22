#include "stageplay_text_structure_delegate.h"

#include <business_layer/model/stageplay/text/stageplay_text_model_folder_item.h>
#include <business_layer/model/stageplay/text/stageplay_text_model_scene_item.h>
#include <business_layer/stageplay_text_structure_model.h>
#include <ui/design_system/design_system.h>
#include <utils/helpers/color_helper.h>
#include <utils/helpers/text_helper.h>
#include <utils/helpers/time_helper.h>

#include <QAbstractItemView>
#include <QPainter>


namespace Ui {

class StageplayTextStructureDelegate::Implementation
{
public:
    /**
     * @brief Нарисовать цвет элемента
     */
    void paintItemColor(QPainter* _painter, const QStyleOptionViewItem& _option,
                        const QVariant& _color, const QModelIndex& _index) const;

    /**
     * @brief Нарисовать элемент
     */
    void paintFolder(QPainter* _painter, const QStyleOptionViewItem& _option,
                     const QModelIndex& _index) const;
    void paintScene(QPainter* _painter, const QStyleOptionViewItem& _option,
                    const QModelIndex& _index) const;
    void paintText(QPainter* _painter, const QStyleOptionViewItem& _option,
                   const QModelIndex& _index) const;

    /**
     * @brief Идеальный размер для элемент
     */
    QSize folderSizeHint(const QStyleOptionViewItem& _option, const QModelIndex& _index) const;
    QSize sceneSizeHint(const QStyleOptionViewItem& _option, const QModelIndex& _index) const;
    QSize textSizeHint(const QStyleOptionViewItem& _option, const QModelIndex& _index) const;


    bool showSceneNumber = true;
    int textLines = 2;
};

void StageplayTextStructureDelegate::Implementation::paintItemColor(
    QPainter* _painter, const QStyleOptionViewItem& _option, const QVariant& _color,
    const QModelIndex& _index) const
{
    //
    // Собираем цвета до самого верха
    //
    QVector<QColor> colors;
    auto addColor = [&colors](const QVariant& _color) {
        if (_color.isNull() || !_color.canConvert<QColor>()) {
            return;
        }

        const QColor color = _color.value<QColor>();
        if (!color.isValid()) {
            return;
        }

        colors.prepend(color);
    };
    addColor(_color);
    //
    auto index = _index;
    while (index.parent().isValid()) {
        index = index.parent();

        const auto typeValue = index.data(Qt::UserRole);
        if (!typeValue.isValid()) {
            continue;
        }

        using namespace BusinessLayer;
        const auto type = static_cast<TextModelItemType>(typeValue.toInt());
        switch (type) {
        case TextModelItemType::Folder: {
            addColor(index.data(TextModelFolderItem::FolderColorRole));
            break;
        }
        case TextModelItemType::Group: {
            addColor(index.data(TextModelGroupItem::GroupColorRole));
            break;
        }
        default: {
            break;
        }
        }
    }

    //
    // Рисуем цвета
    //
    auto fullIndicatorWidth = [_index] {
        int level = 0;
        auto index = _index;
        while (index.isValid()) {
            ++level;
            index = index.parent();
        }
        return level * DesignSystem::tree().indicatorWidth();
    };
    const auto backgroundRect = _option.rect;
    auto lastX = QLocale().textDirection() == Qt::LeftToRight
        ? 0.0
        : (backgroundRect.right() + fullIndicatorWidth() - DesignSystem::layout().px(5));
    for (const auto& color : colors) {
        const QRectF colorRect(lastX, backgroundRect.top(), DesignSystem::layout().px(5),
                               backgroundRect.height());
        _painter->fillRect(colorRect, color);

        lastX += (QLocale().textDirection() == Qt::LeftToRight ? 1 : -1)
            * DesignSystem::layout().px(7);
    }
}

void StageplayTextStructureDelegate::Implementation::paintFolder(
    QPainter* _painter, const QStyleOptionViewItem& _option, const QModelIndex& _index) const
{
    using namespace BusinessLayer;

    auto backgroundColor = _option.palette.color(QPalette::Base);
    auto textColor = _option.palette.color(QPalette::Text);
    const auto isLeftToRight = QLocale().textDirection() == Qt::LeftToRight;

    //
    // Рисуем
    //

    //
    // ... фон
    //
    const QRectF backgroundRect = _option.rect;
    if (_option.state.testFlag(QStyle::State_Selected)) {
        //
        // ... для выделенных элементов
        //
        backgroundColor = _option.palette.color(QPalette::Highlight);
        textColor = _option.palette.color(QPalette::HighlightedText);
    } else if (_option.state.testFlag(QStyle::State_MouseOver)) {
        //
        // ... для элементов на которые наведена мышь
        //
        backgroundColor = _option.palette.color(QPalette::AlternateBase);
    } else {
        //
        // ... для остальных элементов
        //
        textColor.setAlphaF(DesignSystem::inactiveTextOpacity());
    }
    _painter->fillRect(backgroundRect, backgroundColor);

    //
    // ... цвет папки
    //
    paintItemColor(_painter, _option, _index.data(StageplayTextModelFolderItem::FolderColorRole),
                   _index);

    //
    // ... иконка
    //
    _painter->setPen(textColor);
    QRectF iconRect;
    if (_index.data(Qt::DecorationRole).isValid()) {
        if (isLeftToRight) {
            iconRect = QRectF(QPointF(std::max(backgroundRect.left(),
                                               DesignSystem::treeOneLineItem().margins().left()),
                                      backgroundRect.top()),
                              QSizeF(DesignSystem::treeOneLineItem().iconSize().width(),
                                     DesignSystem::treeOneLineItem().height()));
        } else {
            iconRect = QRectF(
                QPointF(backgroundRect.right() - DesignSystem::treeOneLineItem().iconSize().width(),
                        backgroundRect.top()),
                QSizeF(DesignSystem::treeOneLineItem().iconSize().width(),
                       DesignSystem::treeOneLineItem().height()));
        }
        _painter->setFont(DesignSystem::font().iconsMid());
        _painter->drawText(iconRect, Qt::AlignLeft | Qt::AlignVCenter,
                           _index.data(Qt::DecorationRole).toString());
    }

    //
    // ... название папки
    //
    _painter->setFont(DesignSystem::font().subtitle2());
    _painter->setPen(textColor);
    qreal headingLeft = 0.0;
    qreal headingWidth = 0.0;
    if (isLeftToRight) {
        headingLeft = iconRect.right() + DesignSystem::treeOneLineItem().spacing();
        headingWidth = backgroundRect.right() - DesignSystem::treeOneLineItem().margins().right()
            - headingLeft - DesignSystem::treeOneLineItem().spacing();
    } else {
        headingLeft = backgroundRect.left() + DesignSystem::treeOneLineItem().margins().left();
        headingWidth = iconRect.left() - headingLeft - DesignSystem::treeOneLineItem().spacing();
    }
    const QRectF headingRect(
        QPointF(headingLeft,
                backgroundRect.top() + DesignSystem::treeOneLineItem().margins().top()),
        QSizeF(headingWidth, DesignSystem::treeOneLineItem().contentHeight()));
    const auto headingName = _painter->fontMetrics().elidedText(
        _index.data(StageplayTextModelFolderItem::FolderHeadingRole).toString(), Qt::ElideRight,
        static_cast<int>(headingRect.width()));
    _painter->drawText(headingRect, Qt::AlignLeft | Qt::AlignVCenter, headingName);
}

void StageplayTextStructureDelegate::Implementation::paintScene(QPainter* _painter,
                                                                const QStyleOptionViewItem& _option,
                                                                const QModelIndex& _index) const
{
    using namespace BusinessLayer;

    auto backgroundColor = _option.palette.color(QPalette::Base);
    auto textColor = _option.palette.color(QPalette::Text);
    const auto isLeftToRight = QLocale().textDirection() == Qt::LeftToRight;

    //
    // Рисуем
    //

    //
    // ... фон
    //
    const QRectF backgroundRect = _option.rect;
    if (_option.state.testFlag(QStyle::State_Selected)) {
        //
        // ... для выделенных элементов
        //
        backgroundColor = _option.palette.color(QPalette::Highlight);
        textColor = _option.palette.color(QPalette::HighlightedText);
    } else if (_option.state.testFlag(QStyle::State_MouseOver)) {
        //
        // ... для элементов на которые наведена мышь
        //
        backgroundColor = _option.palette.color(QPalette::AlternateBase);
    } else {
        //
        // ... для остальных элементов
        //
        textColor.setAlphaF(DesignSystem::inactiveTextOpacity());
    }
    _painter->fillRect(backgroundRect, backgroundColor);

    //
    // ... цвет сцены
    //
    paintItemColor(_painter, _option, _index.data(TextModelGroupItem::GroupColorRole), _index);

    //
    // ... иконка
    //
    _painter->setPen(textColor);
    QRectF iconRect;
    if (_index.data(Qt::DecorationRole).isValid()) {
        if (isLeftToRight) {
            iconRect = QRectF(QPointF(std::max(backgroundRect.left(),
                                               DesignSystem::treeOneLineItem().margins().left()),
                                      backgroundRect.top()),
                              QSizeF(DesignSystem::treeOneLineItem().iconSize().width(),
                                     DesignSystem::treeOneLineItem().height()));
        } else {
            iconRect = QRectF(
                QPointF(backgroundRect.right() - DesignSystem::treeOneLineItem().iconSize().width(),
                        backgroundRect.top()),
                QSizeF(DesignSystem::treeOneLineItem().iconSize().width(),
                       DesignSystem::treeOneLineItem().height()));
        }
        _painter->setFont(DesignSystem::font().iconsMid());
        _painter->drawText(iconRect, Qt::AlignLeft | Qt::AlignVCenter,
                           _index.data(Qt::DecorationRole).toString());
    }

    //
    // ... заголовок сцены
    //
    _painter->setFont(DesignSystem::font().subtitle2());
    qreal headingLeft = 0.0;
    qreal headingWidth = 0.0;
    if (isLeftToRight) {
        headingLeft = iconRect.right() + DesignSystem::treeOneLineItem().spacing();
        headingWidth = backgroundRect.right() - DesignSystem::treeOneLineItem().margins().right()
            - headingLeft - DesignSystem::treeOneLineItem().spacing();
    } else {
        headingLeft = backgroundRect.left() + DesignSystem::treeOneLineItem().margins().left();
        headingWidth = iconRect.left() - headingLeft - DesignSystem::treeOneLineItem().spacing();
    }
    const QRectF headingRect(
        QPointF(headingLeft,
                backgroundRect.top() + DesignSystem::treeOneLineItem().margins().top()),
        QSizeF(headingWidth, DesignSystem::treeOneLineItem().contentHeight()));
    auto headingText = _index.data(TextModelGroupItem::GroupHeadingRole).toString();
    if (showSceneNumber) {
        headingText.prepend(_index.data(TextModelGroupItem::GroupNumberRole).toString() + " ");
    }
    headingText = _painter->fontMetrics().elidedText(headingText, Qt::ElideRight,
                                                     static_cast<int>(headingRect.width()));
    _painter->drawText(headingRect, Qt::AlignLeft | Qt::AlignVCenter, headingText);

    //
    // ... текст сцены
    //
    auto sceneText = _index.data(TextModelGroupItem::GroupTextRole).toString();
    if (sceneText.isEmpty()) {
        return;
    }
    QRectF textRect;
    if (textLines > 0) {
        _painter->setFont(DesignSystem::font().body2());
        if (isLeftToRight) {
            const qreal textLeft = iconRect.left();
            const qreal textWidth = backgroundRect.right() - textLeft
                - DesignSystem::treeOneLineItem().margins().right();
            textRect = QRectF(
                QPointF(textLeft, headingRect.bottom() + DesignSystem::compactLayout().px8()),
                QSizeF(textWidth, TextHelper::fineLineSpacing(_painter->font()) * textLines));
        } else {
            const qreal textLeft = headingRect.left();
            const qreal textWidth = iconRect.right() - textLeft;
            textRect = QRectF(
                QPointF(textLeft, headingRect.bottom() + DesignSystem::compactLayout().px8()),
                QSizeF(textWidth, TextHelper::fineLineSpacing(_painter->font()) * textLines));
        }
        sceneText = TextHelper::elidedText(sceneText, DesignSystem::font().body2(), textRect);
        _painter->drawText(textRect, Qt::TextWordWrap, sceneText);
    }

    //
    // ... иконки заметок
    //
    const auto inlineNotesSize = _index.data(TextModelGroupItem::GroupInlineNotesSizeRole).toInt();
    const qreal notesIconWidth = DesignSystem::layout().px24();
    const qreal notesLeft = isLeftToRight ? iconRect.left() : (iconRect.right() - notesIconWidth);
    const qreal notesTop = (textRect.isValid() ? textRect : headingRect).bottom()
        + DesignSystem::treeOneLineItem().spacing();
    const qreal notesHeight = TextHelper::fineLineSpacing(DesignSystem::font().caption());
    QRectF inlineNotesRect;
    if (inlineNotesSize > 0) {
        _painter->setFont(DesignSystem::font().iconsSmall());
        const QRectF inlineNotesIconRect(QPointF(notesLeft, notesTop),
                                         QSizeF(notesIconWidth, notesHeight));
        _painter->drawText(inlineNotesIconRect, Qt::AlignLeft | Qt::AlignVCenter, u8"\U000F09A8");
        //
        _painter->setFont(DesignSystem::font().caption());
        const auto inlineNotesSizeText = QString::number(inlineNotesSize);
        const auto inlineNotesSizeTextWidth
            = TextHelper::fineTextWidthF(inlineNotesSizeText, _painter->font());
        inlineNotesRect
            = { QPointF(isLeftToRight ? (inlineNotesIconRect.right() + DesignSystem::layout().px2())
                                      : (inlineNotesIconRect.left() - DesignSystem::layout().px2()
                                         - inlineNotesSizeTextWidth),
                        inlineNotesIconRect.top()),
                QSizeF(inlineNotesSizeTextWidth, notesHeight) };
        _painter->drawText(inlineNotesRect, Qt::AlignLeft | Qt::AlignVCenter, inlineNotesSizeText);
    }
    //
    const auto reviewMarksSize = _index.data(TextModelGroupItem::GroupReviewMarksSizeRole).toInt();
    if (reviewMarksSize > 0) {
        _painter->setFont(DesignSystem::font().iconsSmall());
        const QRectF reviewMarksIconRect(
            QPointF(inlineNotesRect.isValid() ? (
                        isLeftToRight
                            ? (inlineNotesRect.right() + DesignSystem::treeOneLineItem().spacing())
                            : (inlineNotesRect.left() - DesignSystem::treeOneLineItem().spacing()
                               - notesIconWidth))
                                              : notesLeft,
                    notesTop),
            QSizeF(notesIconWidth, notesHeight));
        _painter->drawText(reviewMarksIconRect, Qt::AlignLeft | Qt::AlignVCenter, u8"\U000F0E31");
        //
        _painter->setFont(DesignSystem::font().caption());
        const auto reviewMarksSizeText = QString::number(reviewMarksSize);
        const auto reviewMarksSizeTextWidth
            = TextHelper::fineTextWidthF(reviewMarksSizeText, _painter->font());
        const QRectF reviewMarksRect(
            QPointF(isLeftToRight ? (reviewMarksIconRect.right() + DesignSystem::layout().px2())
                                  : (reviewMarksIconRect.left() - DesignSystem::layout().px2()
                                     - reviewMarksSizeTextWidth),
                    reviewMarksIconRect.top()),
            QSizeF(reviewMarksSizeTextWidth, notesHeight));
        _painter->drawText(reviewMarksRect, Qt::AlignLeft | Qt::AlignVCenter, reviewMarksSizeText);
    }
}

void StageplayTextStructureDelegate::Implementation::paintText(QPainter* _painter,
                                                               const QStyleOptionViewItem& _option,
                                                               const QModelIndex& _index) const
{
    using namespace BusinessLayer;

    auto backgroundColor = _option.palette.color(QPalette::Base);
    auto textColor = _option.palette.color(QPalette::Text);

    //
    // Рисуем
    //

    //
    // ... фон
    //
    const QRectF backgroundRect = _option.rect;
    if (_option.state.testFlag(QStyle::State_Selected)) {
        //
        // ... для выделенных элементов
        //
        backgroundColor = _option.palette.color(QPalette::Highlight);
        textColor = _option.palette.color(QPalette::HighlightedText);
    } else if (_option.state.testFlag(QStyle::State_MouseOver)) {
        //
        // ... для элементов на которые наведена мышь
        //
        backgroundColor = _option.palette.color(QPalette::AlternateBase);
    } else {
        //
        // ... для остальных элементов
        //
        textColor.setAlphaF(DesignSystem::inactiveTextOpacity());
    }
    _painter->fillRect(backgroundRect, backgroundColor);

    //
    // ... иконка
    //
    _painter->setPen(textColor);
    QRectF iconRect;
    if (_index.data(Qt::DecorationRole).isValid()) {
        iconRect = QRectF(QPointF(std::max(backgroundRect.left(),
                                           DesignSystem::treeOneLineItem().margins().left()),
                                  backgroundRect.top()),
                          QSizeF(DesignSystem::treeOneLineItem().iconSize().width(),
                                 DesignSystem::treeOneLineItem().height()));
        _painter->setFont(DesignSystem::font().iconsMid());
        _painter->drawText(iconRect, Qt::AlignLeft | Qt::AlignVCenter,
                           _index.data(Qt::DecorationRole).toString());
    }

    //
    // ... текст элемента
    //
    _painter->setFont(DesignSystem::font().subtitle2());
    _painter->setPen(textColor);
    const qreal folderNameLeft = iconRect.right() + DesignSystem::treeOneLineItem().spacing();
    const qreal folderNameWidth
        = backgroundRect.right() - folderNameLeft - DesignSystem::treeOneLineItem().spacing();
    const QRectF folderNameRect(
        QPointF(folderNameLeft,
                backgroundRect.top() + DesignSystem::treeOneLineItem().margins().top()),
        QSizeF(folderNameWidth, DesignSystem::treeOneLineItem().contentHeight()));
    const auto folderName = _painter->fontMetrics().elidedText(
        _index.data(Qt::DisplayRole).toString(), Qt::ElideRight,
        static_cast<int>(folderNameRect.width()));
    _painter->drawText(folderNameRect, Qt::AlignLeft | Qt::AlignVCenter, folderName);
}

QSize StageplayTextStructureDelegate::Implementation::folderSizeHint(
    const QStyleOptionViewItem& _option, const QModelIndex& _index) const
{
    Q_UNUSED(_index)

    using namespace BusinessLayer;

    //
    // Ширина
    //
    int width = _option.widget->width();
    if (const QAbstractItemView* view = qobject_cast<const QAbstractItemView*>(_option.widget)) {
        width = view->viewport()->width();
    }
    width -= DesignSystem::layout().px8() + DesignSystem::layout().px16()
        + DesignSystem::layout().px16();

    //
    // Считаем высоту
    //
    int height = DesignSystem::treeOneLineItem().height();

    return { width, height };
}

QSize StageplayTextStructureDelegate::Implementation::sceneSizeHint(
    const QStyleOptionViewItem& _option, const QModelIndex& _index) const
{
    using namespace BusinessLayer;

    //
    // Ширина
    //
    int width = _option.widget->width();
    if (const QAbstractItemView* view = qobject_cast<const QAbstractItemView*>(_option.widget)) {
        width = view->viewport()->width();
    }
    width -= DesignSystem::layout().px8() + DesignSystem::layout().px16()
        + DesignSystem::layout().px16();

    //
    // Считаем высоту
    //
    int height = DesignSystem::treeOneLineItem().height();
    if (textLines > 0) {
        height += DesignSystem::compactLayout().px8()
            + TextHelper::fineLineSpacing(DesignSystem::font().body2()) * textLines;
    }
    const bool haveNotesLine = (_index.data(TextModelGroupItem::GroupInlineNotesSizeRole).toInt()
                                + _index.data(TextModelGroupItem::GroupReviewMarksSizeRole).toInt())
        > 0;
    if (haveNotesLine) {
        height += DesignSystem::treeOneLineItem().spacing()
            + TextHelper::fineLineSpacing(DesignSystem::font().caption());
    }
    return { width, height };
}

QSize StageplayTextStructureDelegate::Implementation::textSizeHint(
    const QStyleOptionViewItem& _option, const QModelIndex& _index) const
{
    return folderSizeHint(_option, _index);
}


// ****


StageplayTextStructureDelegate::StageplayTextStructureDelegate(QObject* _parent)
    : QStyledItemDelegate(_parent)
    , d(new Implementation)
{
}

StageplayTextStructureDelegate::~StageplayTextStructureDelegate() = default;

void StageplayTextStructureDelegate::showSceneNumber(bool _show)
{
    d->showSceneNumber = _show;
}

void StageplayTextStructureDelegate::setTextLinesSize(int _size)
{
    d->textLines = _size;
}

void StageplayTextStructureDelegate::paint(QPainter* _painter, const QStyleOptionViewItem& _option,
                                           const QModelIndex& _index) const
{
    //
    // Получим настройки стиля
    //
    QStyleOptionViewItem opt = _option;
    initStyleOption(&opt, _index);

    //
    // Рисуем ручками
    //
    _painter->setRenderHint(QPainter::Antialiasing, true);

    //
    // Рисуем в зависимости от типа элемента
    //

    const auto typeValue = _index.data(Qt::UserRole);
    if (!typeValue.isValid()) {
        return;
    }

    using namespace BusinessLayer;
    const auto type = static_cast<TextModelItemType>(typeValue.toInt());
    switch (type) {
    case TextModelItemType::Folder: {
        d->paintFolder(_painter, opt, _index);
        break;
    }

    case TextModelItemType::Group: {
        d->paintScene(_painter, opt, _index);
        break;
    }

    case TextModelItemType::Text: {
        d->paintText(_painter, opt, _index);
        break;
    }

    default:
        break;
    }
}

QSize StageplayTextStructureDelegate::sizeHint(const QStyleOptionViewItem& _option,
                                               const QModelIndex& _index) const
{
    if (_option.widget == nullptr) {
        return QStyledItemDelegate::sizeHint(_option, _index);
    }

    const auto typeValue = _index.data(Qt::UserRole);
    if (!typeValue.isValid()) {
        return QStyledItemDelegate::sizeHint(_option, _index);
    }

    using namespace BusinessLayer;
    const auto type = static_cast<TextModelItemType>(typeValue.toInt());
    switch (type) {
    case TextModelItemType::Folder: {
        return d->folderSizeHint(_option, _index);
    }

    case TextModelItemType::Group: {
        return d->sceneSizeHint(_option, _index);
    }

    case TextModelItemType::Text: {
        return d->textSizeHint(_option, _index);
    }

    default: {
        return {};
    }
    }
}

} // namespace Ui
