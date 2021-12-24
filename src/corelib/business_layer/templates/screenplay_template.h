#pragma once

#include <QHash>
#include <QPageSize>
#include <QTextFormat>

#include <corelib_global.h>

class QTextBlock;
class QXmlStreamAttributes;


namespace BusinessLayer {

class SimpleTextTemplate;

/**
 * @brief Типы параграфов в сценарии
 */
enum class ScreenplayParagraphType {
    Undefined,
    UnformattedText,
    SceneHeading,
    SceneCharacters,
    Action,
    Character,
    Parenthetical,
    Dialogue,
    Lyrics,
    Transition,
    Shot,
    InlineNote,
    FolderHeader,
    FolderFooter,
    //
    SceneHeadingShadow, //!< Время и место, для вспомогательных разрывов
    //
    PageSplitter, //!< Разделитель страницы (для блоков внутри которых находятся таблицы)
};

/**
 * @brief Определим метод для возможности использовать типы в виде ключей в словарях
 */
CORE_LIBRARY_EXPORT inline uint qHash(ScreenplayParagraphType _type)
{
    return ::qHash(static_cast<int>(_type));
}

/**
 * @brief Получить текстовое представление типа блока
 */
CORE_LIBRARY_EXPORT QString toString(ScreenplayParagraphType _type);
CORE_LIBRARY_EXPORT QString toDisplayString(ScreenplayParagraphType _type);

/**
 * @brief Получить тип блока из текстового представления
 */
CORE_LIBRARY_EXPORT ScreenplayParagraphType screenplayParagraphTypeFromString(const QString& _text);
CORE_LIBRARY_EXPORT ScreenplayParagraphType
screenplayParagraphTypeFromDisplayString(const QString& _text);


/**
 * @brief Класс стиля блока сценария
 */
class CORE_LIBRARY_EXPORT ScreenplayBlockStyle
{
public:
    /**
     * @brief Дополнительные свойства стилей текстовых блоков
     */
    enum Property {
        PropertyType = QTextFormat::UserProperty + 100, //!< Тип блока
        PropertyHeaderType, //!< Тип блока заголовка
        PropertyPrefix, //!< Префикс блока
        PropertyPostfix, //!< Постфикс блока
        PropertyIsFirstUppercase, //!< Необходимо ли первый символ поднимать в верхний регистр
        PropertyIsCanModify, //!< Редактируемый ли блок
        //
        // Свойства редакторских заметок
        //
        PropertyIsReviewMark, //!< Формат является редакторской правкой
        PropertyIsDone, //!< Правка помечена как выполненная
        PropertyComments, //!< Список комментариев к правке
        PropertyCommentsAuthors, //!< Список авторов комментариев
        PropertyCommentsDates, //!< Список дат комментариев
        PropertyCommentsIsEdited, //!< Список признаков изменений комментариев
        //
        // Свойства корректирующих текст блоков
        //
        PropertyIsCorrection, //!< Не разрывающий текст блок (пустые блоки в конце страницы, блоки с
                              //!< текстом ПРОД, или именем персонажа)
        PropertyIsCorrectionContinued, //!< Блок с текстом ПРОД., вставляемый на обрыве реплики
        PropertyIsCorrectionCharacter, //!< Блок с именем персонажа, вставляемый на новой странице
        PropertyIsBreakCorrectionStart, //!< Разрывающий текст блок в начале разрыва
        PropertyIsBreakCorrectionEnd, //!< Разрывающий текст блок в конце разрыва
        PropertyIsCharacterContinued, //!< Имя персонажа для которого необходимо отображать
                                      //!< допольнительный текст ПРОД., не пишем в xml
    };

    /**
     * @brief Виды межстрочных интервалов
     */
    enum class LineSpacingType {
        SingleLineSpacing,
        OneAndHalfLineSpacing,
        DoubleLineSpacing,
        FixedLineSpacing
    };

    /**
     * @brief Получить тип блока
     */
    static ScreenplayParagraphType forBlock(const QTextBlock& _block);

public:
    ScreenplayBlockStyle() = default;

    /**
     * @brief Тип блока
     */
    ScreenplayParagraphType type() const;
    void setType(ScreenplayParagraphType _type);

    /**
     * @brief Активен ли стиль блока
     */
    bool isActive() const;
    void setActive(bool _isActive);

    /**
     * @brief Располагается ли блок с начала страницы
     */
    bool isStartFromNewPage() const;
    void setStartFromNewPage(bool _startFromNewPage);

    /**
     * @brief Получить шрифт блока
     */
    QFont font() const;
    void setFont(const QFont& _font);

    /**
     * @brief Выравнивание блока
     */
    Qt::Alignment align() const;
    void setAlign(Qt::Alignment _align);

    /**
     * @brief Межстрочный интервал
     */
    LineSpacingType lineSpacingType() const;
    void setLineSpacingType(LineSpacingType _type);

    /**
     * @brief Значение межстрочного интервала для FixedLineSpacing, мм
     */
    qreal lineSpacingValue() const;
    void setLineSpacingValue(qreal _value);

    /**
     * @brief Отступ сверху, линий
     */
    int linesBefore() const;
    void setLinesBefore(int _linesBefore);

    /**
     * @brief Отступы вокруг блока, мм
     */
    QMarginsF margins() const;
    void setMargins(const QMarginsF& _margins);

    /**
     * @brief Отступы вокруг блока в режиме разделения на колонки, мм
     */
    QMarginsF marginsOnHalfPage() const;
    void setMarginsOnHalfPage(const QMarginsF& _margins);

    /**
     * @brief Настроить стиль в соответствии с шириной разделителя страницы
     */
    void setPageSplitterWidth(qreal _width);

    /**
     * @brief Отступ снизу, линий
     */
    int linesAfter() const;
    void setLinesAfter(int _linesAfter);


    /**
     * @brief Настройки стиля отображения блока
     */
    QTextBlockFormat blockFormat(bool _onHalfPage = false) const;
    void setBackgroundColor(const QColor& _color);

    /**
     * @brief Настройки шрифта блока
     */
    QTextCharFormat charFormat() const;
    void setTextColor(const QColor& _color);


    /**
     * @brief Разрешено изменять текст блока
     */
    bool isCanModify() const;

    /**
     * @brief Префикс стиля
     */
    QString prefix() const;

    /**
     * @brief Постфикс стиля
     */
    QString postfix() const;

private:
    /**
     * @brief Инициилизация возможна только в классе стиля сценария
     */
    explicit ScreenplayBlockStyle(const QXmlStreamAttributes& _blockAttributes);
    friend class ScreenplayTemplate;

    /**
     * @brief Обновить межстрочный интервал блока
     */
    void updateLineHeight();

    /**
     * @brief Обновить верхний отступ
     */
    void updateTopMargin();

    /**
     * @brief Обновить нижний отступ
     */
    void updateBottomMargin();

private:
    /**
     * @brief Тип блока
     */
    ScreenplayParagraphType m_type = ScreenplayParagraphType::Undefined;

    /**
     * @brief Активен ли блок
     */
    bool m_isActive = false;

    /**
     * @brief Начинается ли блок с начала страницы
     */
    bool m_isStartFromNewPage = false;

    /**
     * @brief Шрифт блока
     */
    QFont m_font = QFont("Courier Prime", 12);

    /**
     * @brief Выравнивание блока
     */
    Qt::Alignment m_align = Qt::AlignLeft;

    /**
     * @brief Межстрочный интервал
     */
    struct {
        LineSpacingType type = LineSpacingType::SingleLineSpacing;
        qreal value = 0.0;
    } m_lineSpacing;

    /**
     * @brief Отступ сверху, линий
     */
    int m_linesBefore = 0;

    /**
     * @brief Отступы вокруг блока, мм
     */
    QMarginsF m_margins;

    /**
     * @brief Отступы вокруг блока в режиме разделения на колонки, мм
     */
    QMarginsF m_marginsOnHalfPage;

    /**
     * @brief Отступ снизу, линий
     */
    int m_linesAfter = 0;

    /**
     * @brief Формат блока
     */
    QTextBlockFormat m_blockFormat;
    QTextBlockFormat m_blockFormatOnHalfPage;

    /**
     * @brief Формат текста
     */
    QTextCharFormat m_charFormat;
};


/**
 * @brief Класс шаблона сценария
 */
class CORE_LIBRARY_EXPORT ScreenplayTemplate
{
public:
    ScreenplayTemplate();
    ScreenplayTemplate(const ScreenplayTemplate& _other);
    ScreenplayTemplate& operator=(const ScreenplayTemplate& _other);
    ~ScreenplayTemplate();

    /**
     * @brief Назначить шаблон новым
     */
    void setIsNew();

    /**
     * @brief Сохранить шаблон в файл
     */
    void saveToFile(const QString& _filePath) const;

    /**
     * @brief Идентификатор шаблона
     */
    QString id() const;

    /**
     * @brief Является ли шаблон умолчальным
     */
    bool isDefault() const;

    /**
     * @brief Название
     */
    QString name() const;
    void setName(const QString& _name);

    /**
     * @brief Описание
     */
    QString description() const;
    void setDescription(const QString& _description);

    /**
     * @brief Размер страницы
     */
    QPageSize::PageSizeId pageSizeId() const;
    void setPageSizeId(QPageSize::PageSizeId _pageSizeId);

    /**
     * @brief Отступы страницы в миллиметрах
     */
    QMarginsF pageMargins() const;
    void setPageMargins(const QMarginsF& _pageMargins);

    /**
     * @brief Расположение нумерации
     */
    Qt::Alignment pageNumbersAlignment() const;
    void setPageNumbersAlignment(Qt::Alignment _alignment);

    /**
     * @brief Процент ширины страницы для левой части разделителя
     */
    int leftHalfOfPageWidthPercents() const;
    void setLeftHalfOfPageWidthPercents(int _width);

    /**
     * @brief Ширина разделителя колонок
     */
    qreal pageSplitterWidth() const;

    /**
     * @brief Шаблон оформления титульной страницы
     */
    const SimpleTextTemplate& titlePageTemplate() const;

    /**
     * @brief Стандартный текст титульной страницы
     */
    const QString& titlePage() const;

    /**
     * @brief Получить стиль блока
     */
    ScreenplayBlockStyle paragraphStyle(ScreenplayParagraphType _forType) const;
    ScreenplayBlockStyle paragraphStyle(const QTextBlock& _forBlock) const;
    void setParagraphStyle(const ScreenplayBlockStyle& _style);

private:
    explicit ScreenplayTemplate(const QString& _fromFile);
    friend class ScreenplayTemplateFacade;
    friend class TemplatesFacade;

    /**
     * @brief Загрузить шаблон из файла
     */
    void load(const QString& _fromFile);

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

/**
 * @brief Определим метод для возможности использовать типы в виде ключей в словарях
 */
CORE_LIBRARY_EXPORT inline uint qHash(ScreenplayBlockStyle::LineSpacingType _type)
{
    return ::qHash(static_cast<int>(_type));
}

} // namespace BusinessLayer
