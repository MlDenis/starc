#include "text_model_group_item.h"

#include "text_model_splitter_item.h"
#include "text_model_text_item.h"
#include "text_model_xml.h"
#include "text_model_xml_writer.h"

#include <business_layer/templates/screenplay_template.h>
#include <utils/helpers/text_helper.h>

#include <QLocale>
#include <QUuid>
#include <QVariant>
#include <QXmlStreamReader>

#include <optional>


namespace BusinessLayer {

class TextModelGroupItem::Implementation
{
public:
    /**
     * @brief Тип группы
     */
    TextGroupType groupType = TextGroupType::Undefined;

    /**
     * @brief Идентификатор группы
     */
    QUuid uuid;

    /**
     * @brief Уровень группы (чем больше число, тем ниже уровень)
     */
    int level = 0;

    /**
     * @brief Пропущена ли группа
     */
    bool isOmited = false;

    /**
     * @brief Номер группы
     */
    std::optional<Number> number;

    /**
     * @brief Цвет группы
     */
    QColor color;

    //    /**
    //     * @brief Тэги группы
    //     */
    //    QVector<Tag> tags;

    //    /**
    //     * @brief Сюжетные линии группы
    //     */
    //    QVector<StoryLine> storyLines;

    /**
     * @brief Штамп на группе
     */
    QString stamp;

    //
    // Ридонли свойства, которые формируются по ходу работы со сценарием
    //

    /**
     * @brief Заголовок группы
     */
    QString heading;

    /**
     * @brief Текст группы
     */
    QString text;

    /**
     * @brief Количество заметок по тексту
     */
    int inlineNotesSize = 0;

    /**
     * @brief Количество редакторских заметок
     */
    int reviewMarksSize = 0;
};


// ****


bool TextModelGroupItem::Number::operator==(const TextModelGroupItem::Number& _other) const
{
    return text == _other.text;
}

TextModelGroupItem::TextModelGroupItem(const TextModel* _model)
    : TextModelItem(TextModelItemType::Group, _model)
    , d(new Implementation)
{
    d->uuid = QUuid::createUuid();
}

TextModelGroupItem::TextModelGroupItem(const TextModel* _model, QXmlStreamReader& _contentReader)
    : TextModelItem(TextModelItemType::Group, _model)
    , d(new Implementation)
{
    d->groupType = textGroupTypeFromString(_contentReader.name().toString());
    Q_ASSERT(d->groupType != TextGroupType::Undefined);

    const auto attributes = _contentReader.attributes();
    if (attributes.hasAttribute(xml::kUuidAttribute)) {
        d->uuid = QUuid::fromString(attributes.value(xml::kUuidAttribute).toString());
    }

    //
    // TODO: plots
    //
    d->isOmited = attributes.hasAttribute(xml::kOmitedAttribute);
    xml::readNextElement(_contentReader);

    auto currentTag = _contentReader.name();
    if (currentTag == xml::kNumberTag) {
        //        d->number = {
        //        _contentReader.attributes().value(xml::kNumberValueAttribute).toString() };
        xml::readNextElement(_contentReader); // end
        currentTag = xml::readNextElement(_contentReader); // next
    }

    if (currentTag == xml::kColorTag) {
        d->color = xml::readContent(_contentReader).toString();
        xml::readNextElement(_contentReader); // end
        currentTag = xml::readNextElement(_contentReader); // next
    }

    if (currentTag == xml::kStampTag) {
        d->stamp = TextHelper::fromHtmlEscaped(xml::readContent(_contentReader).toString());
        xml::readNextElement(_contentReader); // end
        currentTag = xml::readNextElement(_contentReader); // next
    }

    if (currentTag == xml::kContentTag) {
        xml::readNextElement(_contentReader); // next item
        do {
            currentTag = _contentReader.name();

            //
            // Проглатываем закрывающий контентный тэг
            //
            if (currentTag == xml::kContentTag && _contentReader.isEndElement()) {
                xml::readNextElement(_contentReader);
                continue;
            }
            //
            // Если дошли до конца группы, выходим из обработки
            //
            else if (textGroupTypeFromString(currentTag.toString()) != TextGroupType::Undefined
                     && _contentReader.isEndElement()) {
                xml::readNextElement(_contentReader);
                break;
            }
            //
            // Считываем вложенный контент
            //
            else if (textGroupTypeFromString(currentTag.toString()) != TextGroupType::Undefined) {
                appendItem(new TextModelGroupItem(model(), _contentReader));
            } else if (currentTag == xml::kSplitterTag) {
                appendItem(new TextModelSplitterItem(model(), _contentReader));
            } else {
                appendItem(new TextModelTextItem(model(), _contentReader));
            }
        } while (!_contentReader.atEnd());
    }

    //
    // Соберём заголовок, текст группы и прочие параметры
    //
    handleChange();
}

TextModelGroupItem::~TextModelGroupItem() = default;

const TextGroupType& TextModelGroupItem::groupType() const
{
    return d->groupType;
}

void TextModelGroupItem::setGroupType(TextGroupType _type)
{
    if (d->groupType == _type) {
        return;
    }

    d->groupType = _type;
    setChanged(true);
}

QUuid TextModelGroupItem::uuid() const
{
    return d->uuid;
}

int TextModelGroupItem::level() const
{
    return d->level;
}

void TextModelGroupItem::setLevel(int _level)
{
    if (d->level == _level) {
        return;
    }

    d->level = _level;
    setChanged(true);
}

TextModelGroupItem::Number TextModelGroupItem::number() const
{
    if (!d->number.has_value()) {
        return {};
    }

    return *d->number;
}

bool TextModelGroupItem::setNumber(int _number, const QString& _prefix)
{
    if (childCount() == 0) {
        return false;
    }

    bool hasContent = false;
    for (int childIndex = 0; childIndex < childCount(); ++childIndex) {
        const auto child = childAt(childIndex);
        if (child->type() != TextModelItemType::Text) {
            continue;
        }

        const auto textItemChild = static_cast<const TextModelTextItem*>(child);
        if (!textItemChild->isCorrection()) {
            hasContent = true;
            break;
        }
    }
    if (!hasContent) {
        return false;
    }

    const auto newNumberText
        = QString(QLocale().textDirection() == Qt::LeftToRight ? "%1%2." : ".%2%1")
              .arg(_prefix, QString::number(_number));
    if (d->number.has_value() && d->number->text == newNumberText) {
        return true;
    }

    d->number = { _number, newNumberText };
    //
    // Т.к. пока мы не сохраняем номера, в указании, что произошли изменения нет смысла
    //
    //    setChanged(true);

    return true;
}

QColor TextModelGroupItem::color() const
{
    return d->color;
}

void TextModelGroupItem::setColor(const QColor& _color)
{
    if (d->color == _color) {
        return;
    }

    d->color = _color;
    setChanged(true);
}

QString TextModelGroupItem::heading() const
{
    return d->heading;
}

QVariant TextModelGroupItem::data(int _role) const
{
    switch (_role) {
    case Qt::DecorationRole: {
        return u8"\U000f021a";
    }

    case SceneNumberRole: {
        if (d->number.has_value()) {
            return d->number->text;
        }
        return {};
    }

    case SceneColorRole: {
        return d->color;
    }

    case SceneHeadingRole: {
        return d->heading;
    }

    case SceneTextRole: {
        return d->text;
    }

    case SceneInlineNotesSizeRole: {
        return d->inlineNotesSize;
    }

    case SceneReviewMarksSizeRole: {
        return d->reviewMarksSize;
    }

    default: {
        return TextModelItem::data(_role);
    }
    }
}

QByteArray TextModelGroupItem::toXml() const
{
    return toXml(nullptr, 0, nullptr, 0, false);
}

QByteArray TextModelGroupItem::toXml(TextModelItem* _from, int _fromPosition,
                                     TextModelItem* _to, int _toPosition,
                                     bool _clearUuid) const
{
    xml::TextModelXmlWriter xml;
    xml += xmlHeader(_clearUuid);
    for (int childIndex = 0; childIndex < childCount(); ++childIndex) {
        auto child = childAt(childIndex);

        //
        // Нетекстовые блоки, просто добавляем к общему xml
        //
        if (child->type() != TextModelItemType::Text) {
            xml += child;
            continue;
        }

        //
        // Текстовые блоки, в зависимости от необходимости вставить блок целиком, или его часть
        //
        auto textItem = static_cast<TextModelTextItem*>(child);
        if (textItem == _to) {
            if (textItem == _from) {
                xml += { textItem, _fromPosition, _toPosition - _fromPosition };
            } else {
                xml += { textItem, 0, _toPosition };
            }
            break;
        }
        //
        else if (textItem == _from) {
            xml += { textItem, _fromPosition, textItem->text().length() - _fromPosition };
        } else {
            xml += textItem;
        }
    }
    xml += QString("</%1>\n").arg(xml::kContentTag).toUtf8();
    xml += QString("</%1>\n").arg(toString(d->groupType)).toUtf8();

    return xml.data();
}

QByteArray TextModelGroupItem::xmlHeader(bool _clearUuid) const
{
    QByteArray xml;
    //
    // TODO: plots
    //
    xml += QString("<%1 %2=\"%3\" %4=\"%5\" %6>\n")
               .arg(toString(d->groupType), xml::kUuidAttribute,
                    _clearUuid ? QUuid::createUuid().toString() : d->uuid.toString(),
                    xml::kPlotsAttribute, QString(),
                    (d->isOmited ? QString("%1=\"true\"").arg(xml::kOmitedAttribute) : ""))
               .toUtf8();
    //
    // TODO: Номера будем сохранять только когда они кастомные или фиксированные
    //
    //    if (d->number.has_value()) {
    //        xml += QString("<%1 %2=\"%3\"/>\n")
    //               .arg(xml::kNumberTag, xml::kNumberValueAttribute, d->number->value).toUtf8();
    //    }
    if (d->color.isValid()) {
        xml += QString("<%1><![CDATA[%2]]></%1>\n").arg(xml::kColorTag, d->color.name()).toUtf8();
    }
    if (!d->stamp.isEmpty()) {
        xml += QString("<%1><![CDATA[%2]]></%1>\n")
                   .arg(xml::kStampTag, TextHelper::toHtmlEscaped(d->stamp))
                   .toUtf8();
    }
    xml += QString("<%1>\n").arg(xml::kContentTag).toUtf8();

    return xml;
}

void TextModelGroupItem::copyFrom(TextModelItem* _item)
{
    if (_item->type() != TextModelItemType::Group) {
        Q_ASSERT(false);
        return;
    }

    auto sceneItem = static_cast<TextModelGroupItem*>(_item);
    d->uuid = sceneItem->d->uuid;
    d->isOmited = sceneItem->d->isOmited;
    d->number = sceneItem->d->number;
    d->color = sceneItem->d->color;
    d->stamp = sceneItem->d->stamp;
}

bool TextModelGroupItem::isEqual(TextModelItem* _item) const
{
    if (_item == nullptr || type() != _item->type()) {
        return false;
    }

    const auto sceneItem = static_cast<TextModelGroupItem*>(_item);
    return d->uuid == sceneItem->d->uuid
        && d->isOmited == sceneItem->d->isOmited
        //
        // TODO: тут нужно сравнивать, только когда номера зафиксированы
        //
        //            && d->number == sceneItem->d->number
        && d->color == sceneItem->d->color && d->stamp == sceneItem->d->stamp;
}

void TextModelGroupItem::handleChange()
{
    //
    // TODO: Возможно это нужно переопределить для дочерних элементов
    //
    d->heading.clear();
    d->text.clear();
    d->inlineNotesSize = 0;
    d->reviewMarksSize = 0;

    for (int childIndex = 0; childIndex < childCount(); ++childIndex) {
        auto child = childAt(childIndex);
        if (child->type() != TextModelItemType::Text) {
            continue;
        }

        auto childTextItem = static_cast<TextModelTextItem*>(child);

        //
        // Собираем текст
        //
        switch (childTextItem->paragraphType()) {
        case TextParagraphType::SceneHeading:
        case TextParagraphType::Beat:
        case TextParagraphType::Page:
        case TextParagraphType::Panel:
        case TextParagraphType::Heading1:
        case TextParagraphType::Heading2:
        case TextParagraphType::Heading3:
        case TextParagraphType::Heading4:
        case TextParagraphType::Heading5:
        case TextParagraphType::Heading6: {
            d->heading = TextHelper::smartToUpper(childTextItem->text());
            break;
        }

        case TextParagraphType::InlineNote: {
            ++d->inlineNotesSize;
            break;
        }

        default: {
            d->text.append(childTextItem->text() + " ");
            d->reviewMarksSize += std::count_if(
                childTextItem->reviewMarks().begin(), childTextItem->reviewMarks().end(),
                [](const TextModelTextItem::ReviewMark& _reviewMark) {
                    return !_reviewMark.isDone;
                });
            break;
        }
        }
    }
}

} // namespace BusinessLayer
