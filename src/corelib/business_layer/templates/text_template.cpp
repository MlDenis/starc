#include "text_template.h"

#include <ui/widgets/text_edit/page/page_text_edit.h>
#include <utils/helpers/measurement_helper.h>
#include <utils/helpers/string_helper.h>
#include <utils/helpers/text_helper.h>

#include <QCoreApplication>
#include <QFile>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QUuid>
#include <QXmlStreamAttributes>


namespace BusinessLayer {

namespace {

const QHash<TextFolderType, QString> kAbstractParagraphFolderToString = {
    { TextFolderType::Undefined, QLatin1String("undefined") },
    { TextFolderType::Act, QLatin1String("act") },
    { TextFolderType::Sequence, QLatin1String("sequence") },
};

const QHash<TextGroupType, QString> kAbstractParagraphGroupToString = {
    { TextGroupType::Undefined, QLatin1String("undefined") },
    { TextGroupType::Scene, QLatin1String("scene") },
    { TextGroupType::Beat, QLatin1String("beat") },
    { TextGroupType::Page, QLatin1String("page") },
    { TextGroupType::Panel, QLatin1String("panel") },
    { TextGroupType::Chapter, QLatin1String("chapter") },
};

const QHash<TextParagraphType, QString> kAbstractParagraphTypeToString = {
    { TextParagraphType::Undefined, QLatin1String("undefined") },
    { TextParagraphType::UnformattedText, QLatin1String("unformatted_text") },
    { TextParagraphType::InlineNote, QLatin1String("inline_note") },
    { TextParagraphType::ActHeader, QLatin1String("act_header") },
    { TextParagraphType::ActFooter, QLatin1String("act_footer") },
    { TextParagraphType::SequenceHeader, QLatin1String("sequence_heading") },
    { TextParagraphType::SequenceFooter, QLatin1String("sequence_footer") },
    { TextParagraphType::PageSplitter, QLatin1String("page_splitter") },
    //
    { TextParagraphType::SceneHeading, QLatin1String("scene_heading") },
    { TextParagraphType::Character, QLatin1String("character") },
    { TextParagraphType::Dialogue, QLatin1String("dialogue") },
    //
    { TextParagraphType::SceneCharacters, QLatin1String("scene_characters") },
    { TextParagraphType::Action, QLatin1String("action") },
    { TextParagraphType::Parenthetical, QLatin1String("parenthetical") },
    { TextParagraphType::Lyrics, QLatin1String("lyrics") },
    { TextParagraphType::Transition, QLatin1String("transition") },
    { TextParagraphType::Shot, QLatin1String("shot") },
    //
    { TextParagraphType::Sound, QLatin1String("sound") },
    { TextParagraphType::Music, QLatin1String("music") },
    { TextParagraphType::Cue, QLatin1String("cue") },
    //
    { TextParagraphType::Page, QLatin1String("page_heading") },
    { TextParagraphType::Panel, QLatin1String("panel_heading") },
    { TextParagraphType::Description, QLatin1String("description") },
    //
    { TextParagraphType::Heading1, QLatin1String("heading_1") },
    { TextParagraphType::Heading2, QLatin1String("heading_2") },
    { TextParagraphType::Heading3, QLatin1String("heading_3") },
    { TextParagraphType::Heading4, QLatin1String("heading_4") },
    { TextParagraphType::Heading5, QLatin1String("heading_5") },
    { TextParagraphType::Heading6, QLatin1String("heading_6") },
    { TextParagraphType::Text, QLatin1String("text") },
};

const QHash<TextBlockStyle::LineSpacingType, QString> kLineSpacingToString = {
    { TextBlockStyle::LineSpacingType::SingleLineSpacing, "single" },
    { TextBlockStyle::LineSpacingType::OneAndHalfLineSpacing, "oneandhalf" },
    { TextBlockStyle::LineSpacingType::DoubleLineSpacing, "double" },
    { TextBlockStyle::LineSpacingType::FixedLineSpacing, "fixed" },
};

QString toString(TextBlockStyle::LineSpacingType _type)
{
    return kLineSpacingToString.value(_type);
}

TextBlockStyle::LineSpacingType lineSpacingFromString(const QString& _lineSpacing)
{
    return kLineSpacingToString.key(_lineSpacing);
}

} // namespace


QString toString(TextFolderType _type)
{
    return kAbstractParagraphFolderToString.value(_type);
}

TextFolderType textFolderTypeFromString(const QString& _text)
{
    return kAbstractParagraphFolderToString.key(_text, TextFolderType::Undefined);
}

QString toString(TextGroupType _type)
{
    return kAbstractParagraphGroupToString.value(_type);
}

TextGroupType textGroupTypeFromString(const QString& _text)
{
    return kAbstractParagraphGroupToString.key(_text, TextGroupType::Undefined);
}

QString toString(TextParagraphType _type)
{
    return kAbstractParagraphTypeToString.value(_type);
}

QString toDisplayString(TextParagraphType _type)
{
    switch (_type) {
    default:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Undefined");
    case TextParagraphType::UnformattedText:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Unformatted text");
    case TextParagraphType::InlineNote:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Inline note");
    case TextParagraphType::ActHeader:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Act");
    case TextParagraphType::ActFooter:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Act footer");
    case TextParagraphType::SequenceHeader:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Sequence");
    case TextParagraphType::SequenceFooter:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Sequence footer");
    //
    case TextParagraphType::SceneHeading:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Scene heading");
    case TextParagraphType::Character:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Character");
    case TextParagraphType::Dialogue:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Dialogue");
    //
    case TextParagraphType::SceneCharacters:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Scene characters");
    case TextParagraphType::Action:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Action");
    case TextParagraphType::Parenthetical:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Parenthetical");
    case TextParagraphType::Lyrics:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Lyrics");
    case TextParagraphType::Transition:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Transition");
    case TextParagraphType::Shot:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Shot");
    //
    case TextParagraphType::Sound:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Sound");
    case TextParagraphType::Music:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Music");
    case TextParagraphType::Cue:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Cue");
    //
    case TextParagraphType::Page:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Page");
    case TextParagraphType::Panel:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Panel");
    case TextParagraphType::Description:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Description");
    //
    case TextParagraphType::Heading1:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Heading 1");
    case TextParagraphType::Heading2:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Heading 2");
    case TextParagraphType::Heading3:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Heading 3");
    case TextParagraphType::Heading4:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Heading 4");
    case TextParagraphType::Heading5:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Heading 5");
    case TextParagraphType::Heading6:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Heading 6");
    case TextParagraphType::Text:
        return QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Text");
    }
}

TextParagraphType textParagraphTypeFromString(const QString& _text)
{
    return kAbstractParagraphTypeToString.key(_text, TextParagraphType::Undefined);
}

TextParagraphType textParagraphTypeFromDisplayString(const QString& _text)
{
    //
    // Ищем среди всех строк, т.к. карты нет, а перевод может измениться в любой момент выполнения
    //
    for (auto iter = kAbstractParagraphTypeToString.begin();
         iter != kAbstractParagraphTypeToString.end(); ++iter) {
        if (toDisplayString(iter.key()) == _text) {
            return iter.key();
        }
    }

    return TextParagraphType::Undefined;
}


// ****


TextParagraphType TextBlockStyle::forBlock(const QTextBlock& _block)
{
    TextParagraphType blockType = TextParagraphType::Undefined;
    if (_block.blockFormat().hasProperty(TextBlockStyle::PropertyType)) {
        blockType = static_cast<TextParagraphType>(
            _block.blockFormat().intProperty(TextBlockStyle::PropertyType));
    }
    return blockType;
}

TextParagraphType TextBlockStyle::type() const
{
    return m_type;
}

void TextBlockStyle::setType(TextParagraphType _type)
{
    if (m_type == _type) {
        return;
    }

    m_type = _type;
    m_blockFormat.setProperty(TextBlockStyle::PropertyType, static_cast<int>(_type));
    m_blockFormatOnHalfPage.setProperty(TextBlockStyle::PropertyType, static_cast<int>(_type));
}

bool TextBlockStyle::isActive() const
{
    return m_isActive;
}

void TextBlockStyle::setActive(bool _active)
{
    m_isActive = _active;
}

bool TextBlockStyle::isStartFromNewPage() const
{
    return m_isStartFromNewPage;
}

void TextBlockStyle::setStartFromNewPage(bool _startFromNewPage)
{
    if (m_isStartFromNewPage == _startFromNewPage) {
        return;
    }

    m_isStartFromNewPage = _startFromNewPage;
    m_blockFormat.setPageBreakPolicy(m_isStartFromNewPage ? QTextFormat::PageBreak_AlwaysBefore
                                                          : QTextFormat::PageBreak_Auto);
    m_blockFormatOnHalfPage.setPageBreakPolicy(m_blockFormat.pageBreakPolicy());
}

QFont TextBlockStyle::font() const
{
    return m_font;
}

void TextBlockStyle::setFont(const QFont& _font)
{
    if (m_font == _font) {
        return;
    }

    m_font = _font;
    m_charFormat.setFont(m_font);
    updateLineHeight();
}

Qt::Alignment TextBlockStyle::align() const
{
    return m_align;
}

void TextBlockStyle::setAlign(Qt::Alignment _align)
{
    if (m_align == _align) {
        return;
    }

    m_align = _align;
    m_blockFormat.setAlignment(m_align);
    m_blockFormatOnHalfPage.setAlignment(m_blockFormat.alignment());
}

TextBlockStyle::LineSpacingType TextBlockStyle::lineSpacingType() const
{
    return m_lineSpacing.type;
}

void TextBlockStyle::setLineSpacingType(TextBlockStyle::LineSpacingType _type)
{
    if (m_lineSpacing.type == _type) {
        return;
    }

    m_lineSpacing.type = _type;
    updateLineHeight();
}

qreal TextBlockStyle::lineSpacingValue() const
{
    return m_lineSpacing.value;
}

void TextBlockStyle::setLineSpacingValue(qreal _value)
{
    if (m_lineSpacing.value == _value) {
        return;
    }

    m_lineSpacing.value = _value;
    updateLineHeight();
}

int TextBlockStyle::linesBefore() const
{
    return m_linesBefore;
}

void TextBlockStyle::setLinesBefore(int _linesBefore)
{
    if (m_linesBefore == _linesBefore) {
        return;
    }

    m_linesBefore = _linesBefore;
    updateTopMargin();
}

QMarginsF TextBlockStyle::margins() const
{
    return m_margins;
}

void TextBlockStyle::setMargins(const QMarginsF& _margins)
{
    if (m_margins.left() != _margins.left()) {
        m_margins.setLeft(_margins.left());
        m_blockFormat.setLeftMargin(MeasurementHelper::mmToPx(m_margins.left()));
    }

    if (m_margins.top() != _margins.top()) {
        m_margins.setTop(_margins.top());
        updateTopMargin();
    }

    if (m_margins.right() != _margins.right()) {
        m_margins.setRight(_margins.right());
        m_blockFormat.setRightMargin(MeasurementHelper::mmToPx(m_margins.right()));
    }

    if (m_margins.bottom() != _margins.bottom()) {
        m_margins.setBottom(_margins.bottom());
        updateBottomMargin();
    }
}

QMarginsF TextBlockStyle::marginsOnHalfPage() const
{
    return m_marginsOnHalfPage;
}

void TextBlockStyle::setMarginsOnHalfPage(const QMarginsF& _margins)
{
    if (m_marginsOnHalfPage.left() != _margins.left()) {
        m_marginsOnHalfPage.setLeft(_margins.left());
        m_blockFormatOnHalfPage.setLeftMargin(
            MeasurementHelper::mmToPx(m_marginsOnHalfPage.left()));
    }

    if (m_marginsOnHalfPage.right() != _margins.right()) {
        m_marginsOnHalfPage.setRight(_margins.right());
        m_blockFormatOnHalfPage.setRightMargin(
            MeasurementHelper::mmToPx(m_marginsOnHalfPage.right()));
    }
}

void TextBlockStyle::setPageSplitterWidth(qreal _width)
{
    m_charFormat.setProperty(QTextFormat::TableCellLeftPadding, _width);
}

int TextBlockStyle::linesAfter() const
{
    return m_linesAfter;
}

void TextBlockStyle::setLinesAfter(int _linesAfter)
{
    if (m_linesAfter == _linesAfter) {
        return;
    }

    m_linesAfter = _linesAfter;
    updateBottomMargin();
}

QTextBlockFormat TextBlockStyle::blockFormat(bool _onHalfPage) const
{
    return _onHalfPage ? m_blockFormatOnHalfPage : m_blockFormat;
}

void TextBlockStyle::setBackgroundColor(const QColor& _color)
{
    m_blockFormat.setBackground(_color);
    m_blockFormatOnHalfPage.setBackground(m_blockFormat.background());
}

QTextCharFormat TextBlockStyle::charFormat() const
{
    return m_charFormat;
}

void TextBlockStyle::setTextColor(const QColor& _color)
{
    m_charFormat.setForeground(_color);
}

bool TextBlockStyle::isCanModify() const
{
    return m_charFormat.boolProperty(TextBlockStyle::PropertyIsCanModify);
}

QString TextBlockStyle::prefix() const
{
    return m_charFormat.stringProperty(TextBlockStyle::PropertyPrefix);
}

QString TextBlockStyle::postfix() const
{
    return m_charFormat.stringProperty(TextBlockStyle::PropertyPostfix);
}

TextBlockStyle::TextBlockStyle(const QXmlStreamAttributes& _blockAttributes)
{
    //
    // Считываем параметры
    //
    // ... тип блока и его основные параметры в стиле
    //
    m_type = textParagraphTypeFromString(_blockAttributes.value("id").toString());
    m_isActive = _blockAttributes.value("active").toString() == "true";
    m_isStartFromNewPage = _blockAttributes.value("starts_from_new_page").toString() == "true";
    //
    // ... настройки шрифта
    //
    m_font.setFamily(_blockAttributes.value("font_family").toString());
    m_font.setPixelSize(MeasurementHelper::ptToPx(_blockAttributes.value("font_size").toDouble()));
    //
    // ... начертание
    //
    m_font.setBold(_blockAttributes.value("bold").toString() == "true");
    m_font.setItalic(_blockAttributes.value("italic").toString() == "true");
    m_font.setUnderline(_blockAttributes.value("underline").toString() == "true");
    m_font.setCapitalization(_blockAttributes.value("uppercase").toString() == "true"
                                 ? QFont::AllUppercase
                                 : QFont::MixedCase);
    //
    // ... расположение блока
    //
    m_align = alignmentFromString(_blockAttributes.value("alignment").toString());
    m_lineSpacing.type = lineSpacingFromString(_blockAttributes.value("line_spacing").toString());
    m_lineSpacing.value = _blockAttributes.value("line_spacing_value").toDouble();
    m_linesBefore = _blockAttributes.value("lines_before").toInt();
    m_margins = marginsFromString(_blockAttributes.value("margins").toString());
    m_marginsOnHalfPage
        = marginsFromString(_blockAttributes.value("margins_on_half_page").toString());
    m_linesAfter = _blockAttributes.value("lines_after").toInt();

    //
    // Настроим форматы
    //
    // ... блока
    //
    m_blockFormat.setAlignment(m_align);
    m_blockFormat.setLeftMargin(MeasurementHelper::mmToPx(m_margins.left()));
    m_blockFormat.setRightMargin(MeasurementHelper::mmToPx(m_margins.right()));
    m_blockFormat.setPageBreakPolicy(m_isStartFromNewPage ? QTextFormat::PageBreak_AlwaysBefore
                                                          : QTextFormat::PageBreak_Auto);
    m_blockFormatOnHalfPage.setAlignment(m_align);
    m_blockFormatOnHalfPage.setLeftMargin(MeasurementHelper::mmToPx(m_marginsOnHalfPage.left()));
    m_blockFormatOnHalfPage.setRightMargin(MeasurementHelper::mmToPx(m_marginsOnHalfPage.right()));
    m_blockFormatOnHalfPage.setPageBreakPolicy(m_blockFormat.pageBreakPolicy());
    updateLineHeight();
    //
    // ... текста
    //
    m_charFormat.setFont(m_font);

    //
    // Запомним в стиле его настройки
    //
    m_blockFormat.setProperty(TextBlockStyle::PropertyType, static_cast<int>(m_type));
    m_blockFormatOnHalfPage.setProperty(TextBlockStyle::PropertyType, static_cast<int>(m_type));
    m_charFormat.setProperty(TextBlockStyle::PropertyIsFirstUppercase, true);
    m_charFormat.setProperty(TextBlockStyle::PropertyIsCanModify, true);

    //
    // Настроим остальные характеристики
    //
    switch (m_type) {
    case TextParagraphType::Parenthetical: {
        m_charFormat.setProperty(TextBlockStyle::PropertyIsFirstUppercase, false);
        //
        // Стандартное обрамление
        //
        m_charFormat.setProperty(TextBlockStyle::PropertyPrefix, "(");
        m_charFormat.setProperty(TextBlockStyle::PropertyPostfix, ")");
        break;
    }

    case TextParagraphType::PageSplitter: {
        //
        // Запрещаем редактирование данного блока и отображение в нём курсора
        //
        m_charFormat.setProperty(TextBlockStyle::PropertyIsCanModify, false);
        m_blockFormat.setProperty(PageTextEdit::PropertyDontShowCursor, true);
        break;
    }

    default: {
        break;
    }
    }
}

void TextBlockStyle::updateLineHeight()
{
    qreal lineHeight = TextHelper::fineLineSpacing(m_font);
    switch (m_lineSpacing.type) {
    case LineSpacingType::FixedLineSpacing: {
        lineHeight = MeasurementHelper::mmToPx(m_lineSpacing.value);
        break;
    }

    case LineSpacingType::DoubleLineSpacing: {
        lineHeight *= 2.0;
        break;
    }

    case LineSpacingType::OneAndHalfLineSpacing: {
        lineHeight *= 1.5;
        break;
    }

    case LineSpacingType::SingleLineSpacing:
    default: {
        break;
    }
    }
    m_blockFormat.setLineHeight(lineHeight, QTextBlockFormat::FixedHeight);
    m_blockFormatOnHalfPage.setLineHeight(lineHeight, QTextBlockFormat::FixedHeight);

    updateTopMargin();
    updateBottomMargin();
}

void TextBlockStyle::updateTopMargin()
{
    m_blockFormat.setTopMargin(m_blockFormat.lineHeight() * m_linesBefore
                               + MeasurementHelper::mmToPx(m_margins.top()));
    m_blockFormatOnHalfPage.setTopMargin(m_blockFormat.topMargin());
}

void TextBlockStyle::updateBottomMargin()
{
    m_blockFormat.setBottomMargin(m_blockFormat.lineHeight() * m_linesAfter
                                  + MeasurementHelper::mmToPx(m_margins.bottom()));
    m_blockFormatOnHalfPage.setBottomMargin(m_blockFormat.bottomMargin());
}


// ****


class TextTemplate::Implementation
{
public:
    Implementation();

    /**
     * @brief Сформировать шаблоны компаньоны
     */
    void buildTitlePageTemplate();
    void buildSynopsisTemplate();


    /**
     * @brief Идентификатор
     */
    QString id;

    /**
     * @brief Является ли шаблон умолчальным
     */
    bool isDefault = false;

    /**
     * @brief Название
     */
    QString name;

    /**
     * @brief Описание
     */
    QString description;

    /**
     * @brief Формат страницы
     */
    QPageSize::PageSizeId pageSizeId;

    /**
     * @brief Поля страницы в миллиметрах
     */
    QMarginsF pageMargins;

    /**
     * @brief Расположение нумерации
     */
    Qt::Alignment pageNumbersAlignment;

    /**
     * @brief Процент от ширины страницы, которые занимает левая часть разделения
     */
    int leftHalfOfPageWidthPercents = 50;

    /**
     * @brief Шаблон-компаньён, используемый для титульной страницы
     */
    QScopedPointer<TextTemplate> titlePageTemplate;

    /**
     * @brief Xml титульной страницы
     */
    QString titlePage;

    /**
     * @brief Шаблон-компаньён, используемый для синопсиса
     */
    QScopedPointer<TextTemplate> synopsisTemplate;

    /**
     * @brief Стили блоков текста
     */
    QHash<TextParagraphType, TextBlockStyle> paragraphsStyles;
};

TextTemplate::Implementation::Implementation()
    : id(QUuid::createUuid().toString())
{
}

void TextTemplate::Implementation::buildTitlePageTemplate()
{
    if (titlePageTemplate.isNull()) {
        titlePageTemplate.reset(new TextTemplate);
    }

    titlePageTemplate->setPageSizeId(pageSizeId);
    titlePageTemplate->setPageMargins(pageMargins);
    titlePageTemplate->setPageNumbersAlignment(pageNumbersAlignment);

    TextBlockStyle defaultBlockStyle;
    defaultBlockStyle.setActive(true);
    defaultBlockStyle.setStartFromNewPage(false);
    TextBlockStyle textBlockStyle;
    if (paragraphsStyles.contains(TextParagraphType::Action)) {
        textBlockStyle = paragraphsStyles.value(TextParagraphType::Action);
    } else if (paragraphsStyles.contains(TextParagraphType::Description)) {
        textBlockStyle = paragraphsStyles.value(TextParagraphType::Description);
    } else {
        textBlockStyle = paragraphsStyles.value(TextParagraphType::Text);
    }
    defaultBlockStyle.setFont(textBlockStyle.font());
    defaultBlockStyle.setAlign(textBlockStyle.align());
    //
    for (auto type : {
             TextParagraphType::Heading1,
             TextParagraphType::Heading2,
             TextParagraphType::Heading3,
             TextParagraphType::Heading4,
             TextParagraphType::Heading5,
             TextParagraphType::Heading6,
             TextParagraphType::Text,
             TextParagraphType::InlineNote,
         }) {
        auto blockStyle = defaultBlockStyle;
        blockStyle.setType(type);
        titlePageTemplate->setParagraphStyle(blockStyle);
    }
}

void TextTemplate::Implementation::buildSynopsisTemplate()
{
    if (synopsisTemplate.isNull()) {
        synopsisTemplate.reset(new TextTemplate);
    }

    synopsisTemplate->setPageSizeId(pageSizeId);
    synopsisTemplate->setPageMargins(pageMargins);
    synopsisTemplate->setPageNumbersAlignment(pageNumbersAlignment);

    TextBlockStyle defaultBlockStyle;
    defaultBlockStyle.setActive(true);
    defaultBlockStyle.setStartFromNewPage(false);
    TextBlockStyle textBlockStyle;
    if (paragraphsStyles.contains(TextParagraphType::Action)) {
        textBlockStyle = paragraphsStyles.value(TextParagraphType::Action);
    } else if (paragraphsStyles.contains(TextParagraphType::Description)) {
        textBlockStyle = paragraphsStyles.value(TextParagraphType::Description);
    } else {
        textBlockStyle = paragraphsStyles.value(TextParagraphType::Text);
    }
    defaultBlockStyle.setFont(textBlockStyle.font());
    defaultBlockStyle.setAlign(textBlockStyle.align());
    //
    for (auto type : {
             TextParagraphType::Heading1,
             TextParagraphType::Heading2,
             TextParagraphType::Heading3,
             TextParagraphType::Heading4,
             TextParagraphType::Heading5,
             TextParagraphType::Heading6,
             TextParagraphType::Text,
             TextParagraphType::InlineNote,
         }) {
        auto blockStyle = defaultBlockStyle;
        blockStyle.setType(type);
        synopsisTemplate->setParagraphStyle(blockStyle);
    }
}

// **

TextTemplate::TextTemplate()
    : d(new Implementation)
{
}

TextTemplate::TextTemplate(const QString& _fromFile)
    : d(new Implementation)
{
    load(_fromFile);
}

TextTemplate::TextTemplate(const TextTemplate& _other)
    : d(new Implementation)
{
    d->id = _other.d->id;
    d->isDefault = _other.d->isDefault;
    d->name = _other.d->name;
    d->description = _other.d->description;
    d->pageSizeId = _other.d->pageSizeId;
    d->pageMargins = _other.d->pageMargins;
    d->pageNumbersAlignment = _other.d->pageNumbersAlignment;
    d->leftHalfOfPageWidthPercents = _other.d->leftHalfOfPageWidthPercents;
    d->titlePage = _other.d->titlePage;
    d->paragraphsStyles = _other.d->paragraphsStyles;
}

TextTemplate& TextTemplate::operator=(const TextTemplate& _other)
{
    if (this != &_other) {
        d->id = _other.d->id;
        d->isDefault = _other.d->isDefault;
        d->name = _other.d->name;
        d->description = _other.d->description;
        d->pageSizeId = _other.d->pageSizeId;
        d->pageMargins = _other.d->pageMargins;
        d->pageNumbersAlignment = _other.d->pageNumbersAlignment;
        d->leftHalfOfPageWidthPercents = _other.d->leftHalfOfPageWidthPercents;
        d->titlePage = _other.d->titlePage;
        d->paragraphsStyles = _other.d->paragraphsStyles;
    }
    return *this;
}

TextTemplate::~TextTemplate() = default;

void TextTemplate::load(const QString& _fromFile)
{
    QFile templateFile(_fromFile);
    if (!templateFile.open(QIODevice::ReadOnly)) {
        return;
    }
    const QString templateXml = templateFile.readAll();
    QXmlStreamReader reader(templateXml);

    //
    // Считываем данные в соответствии с заданным форматом
    //
    if (!reader.readNextStartElement() || reader.name() != QLatin1String("style")) {
        return;
    }

    //
    // Считываем атрибуты шаблона
    //
    QXmlStreamAttributes templateAttributes = reader.attributes();
    if (templateAttributes.hasAttribute("id")) {
        d->id = templateAttributes.value("id").toString();
    }
    d->isDefault = templateAttributes.value("default").toString() == "true";
    d->name = templateAttributes.value("name").toString();
    d->description = templateAttributes.value("description").toString();
    d->pageSizeId = pageSizeIdFromString(templateAttributes.value("page_format").toString());
    d->pageMargins = marginsFromString(templateAttributes.value("page_margins").toString());
    d->pageNumbersAlignment
        = alignmentFromString(templateAttributes.value("page_numbers_alignment").toString());
    d->leftHalfOfPageWidthPercents = templateAttributes.value("left_half_of_page_width").toInt();

    //
    // Считываем титульную страницу
    //
    reader.readNextStartElement();
    Q_ASSERT(reader.name() == QLatin1String("titlepage"));
    const auto titlePageXmlFrom = reader.characterOffset();
    reader.readNextStartElement();
    if (!reader.isEndElement()) {
        reader.skipCurrentElement();
        const auto titlePageXmlEnd = reader.characterOffset();
        d->titlePage
            = templateXml.mid(titlePageXmlFrom, titlePageXmlEnd - titlePageXmlFrom).simplified();
        reader.readNext();
    }
    reader.readNext();

    //
    // Считываем настройки оформления блоков текста
    //
    reader.readNextStartElement();
    Q_ASSERT(reader.name() == QLatin1String("blocks"));
    while (reader.readNextStartElement() && reader.name() == QLatin1String("block")) {
        TextBlockStyle blockStyle(reader.attributes());
        blockStyle.setPageSplitterWidth(pageSplitterWidth());
        d->paragraphsStyles.insert(blockStyle.type(), blockStyle);

        //
        // Если ещё не находимся в конце элемента, то остальное пропускаем
        //
        if (!reader.isEndElement()) {
            reader.skipCurrentElement();
        }
    }

    //
    // Копируем стили для теневых стилей из заданных в шаблоне стилей
    //
    {
        TextBlockStyle sceneHeadingShadowStyle
            = d->paragraphsStyles.value(TextParagraphType::SceneHeading);
        sceneHeadingShadowStyle.setType(TextParagraphType::SceneHeadingShadow);
        setParagraphStyle(sceneHeadingShadowStyle);
        //
        TextBlockStyle panelShadowStyle = d->paragraphsStyles.value(TextParagraphType::Panel);
        panelShadowStyle.setType(TextParagraphType::PanelShadow);
        setParagraphStyle(panelShadowStyle);
    }

    //
    // Шаблоны компаньоны будут сформированы по мере необходимости
    //
}

void TextTemplate::setIsNew()
{
    d->isDefault = false;
    d->name = QCoreApplication::translate("BusinessLayer::AbstractTemplate", "Copy of ") + name();
    d->id = QUuid::createUuid().toString();
    d->description.clear();
}

void TextTemplate::saveToFile(const QString& _filePath) const
{
    QFile templateFile(_filePath);
    if (!templateFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    QXmlStreamWriter writer(&templateFile);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("style");
    writer.writeAttribute("id", d->id);
    writer.writeAttribute("name", d->name);
    writer.writeAttribute("description", d->description);
    writer.writeAttribute("page_format", toString(d->pageSizeId));
    writer.writeAttribute("page_margins", ::toString(d->pageMargins));
    writer.writeAttribute("page_numbers_alignment", ::toString(d->pageNumbersAlignment));
    writer.writeAttribute("left_half_of_page_width", ::toString(d->leftHalfOfPageWidthPercents));
    writer.writeStartElement("titlepage");
    writer.writeCharacters(""); // это нужно, чтобы корректно записался открывающий тэг титула
    writer.device()->write(d->titlePage.toUtf8());
    writer.writeEndElement(); // titlepage
    writer.writeStartElement("blocks");
    for (const auto& blockStyle : std::as_const(d->paragraphsStyles)) {
        if (toString(blockStyle.type()).isEmpty()) {
            continue;
        }

        writer.writeStartElement("block");
        writer.writeAttribute("id", toString(blockStyle.type()));
        writer.writeAttribute("active", ::toString(blockStyle.isActive()));
        writer.writeAttribute("starts_from_new_page", ::toString(blockStyle.isStartFromNewPage()));
        writer.writeAttribute("font_family", blockStyle.font().family());
        writer.writeAttribute("font_size",
                              ::toString(MeasurementHelper::pxToPt(blockStyle.font().pixelSize())));
        writer.writeAttribute("bold", ::toString(blockStyle.font().bold()));
        writer.writeAttribute("italic", ::toString(blockStyle.font().italic()));
        writer.writeAttribute("underline", ::toString(blockStyle.font().underline()));
        writer.writeAttribute(
            "uppercase", ::toString(blockStyle.font().capitalization() == QFont::AllUppercase));
        writer.writeAttribute("alignment", toString(blockStyle.align()));
        writer.writeAttribute("line_spacing", toString(blockStyle.lineSpacingType()));
        writer.writeAttribute("line_spacing_value", ::toString(blockStyle.lineSpacingValue()));
        writer.writeAttribute("lines_before", ::toString(blockStyle.linesBefore()));
        writer.writeAttribute("margins", ::toString(blockStyle.margins()));
        writer.writeAttribute("margins_on_half_page", ::toString(blockStyle.marginsOnHalfPage()));
        writer.writeAttribute("lines_after", ::toString(blockStyle.linesAfter()));
        writer.writeEndElement(); // block
    }
    writer.writeEndElement(); // blocks
    writer.writeEndElement(); // style
    writer.writeEndDocument();

    templateFile.close();
}

QString TextTemplate::id() const
{
    return d->id;
}

bool TextTemplate::isDefault() const
{
    return d->isDefault;
}

QString TextTemplate::name() const
{
    return d->name;
}

void TextTemplate::setName(const QString& _name)
{
    d->name = _name;
}

QString TextTemplate::description() const
{
    return d->description;
}

void TextTemplate::setDescription(const QString& _description)
{
    d->description = _description;
}

QPageSize::PageSizeId TextTemplate::pageSizeId() const
{
    return d->pageSizeId;
}

void TextTemplate::setPageSizeId(QPageSize::PageSizeId _pageSizeId)
{
    d->pageSizeId = _pageSizeId;
}

QMarginsF TextTemplate::pageMargins() const
{
    return d->pageMargins;
}

void TextTemplate::setPageMargins(const QMarginsF& _pageMargins)
{
    d->pageMargins = _pageMargins;
}

Qt::Alignment TextTemplate::pageNumbersAlignment() const
{
    return d->pageNumbersAlignment;
}

void TextTemplate::setPageNumbersAlignment(Qt::Alignment _alignment)
{
    d->pageNumbersAlignment = _alignment;
}

int TextTemplate::leftHalfOfPageWidthPercents() const
{
    return d->leftHalfOfPageWidthPercents;
}

void TextTemplate::setLeftHalfOfPageWidthPercents(int _width)
{
    d->leftHalfOfPageWidthPercents = _width;
}

qreal TextTemplate::pageSplitterWidth() const
{
    //
    // TODO: вынести в параметры шаблона
    //
    return MeasurementHelper::mmToPx(5);
}

const TextTemplate& TextTemplate::titlePageTemplate() const
{
    if (d->titlePageTemplate.isNull()) {
        d->buildTitlePageTemplate();
    }

    return *d->titlePageTemplate;
}

const QString& TextTemplate::titlePage() const
{
    return d->titlePage;
}

void TextTemplate::setTitlePage(const QString& _titlePage)
{
    d->titlePage = _titlePage;
}

const TextTemplate& TextTemplate::synopsisTemplate() const
{
    if (d->synopsisTemplate.isNull()) {
        d->buildSynopsisTemplate();
    }

    return *d->synopsisTemplate;
}

TextBlockStyle TextTemplate::paragraphStyle(TextParagraphType _forType) const
{
    return d->paragraphsStyles.value(_forType);
}

TextBlockStyle TextTemplate::paragraphStyle(const QTextBlock& _forBlock) const
{
    return paragraphStyle(TextBlockStyle::forBlock(_forBlock));
}

void TextTemplate::setParagraphStyle(const TextBlockStyle& _style)
{
    d->paragraphsStyles.insert(_style.type(), _style);

    //
    // Если сменился стиль на основе которого стоится шаблон компаньон - пересроим их
    //
    if (_style.type() == TextParagraphType::Action
        || _style.type() == TextParagraphType::Description
        || _style.type() == TextParagraphType::Text) {
        d->buildTitlePageTemplate();
        d->buildSynopsisTemplate();
    }
}

} // namespace BusinessLayer
