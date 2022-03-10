#pragma once

#include "abstract_text_model_item.h"

#include <QString>

#include <chrono>

class QColor;
class QUuid;
class QXmlStreamReader;


namespace BusinessLayer {

enum class TextGroupType;

/**
 * @brief Класс элементов групп модели текста
 */
class CORE_LIBRARY_EXPORT AbstractTextModelGroupItem : public AbstractTextModelItem
{
public:
    /**
     * @brief Номер группы
     */
    struct Number {
        int value = 0;
        QString text;

        bool operator==(const Number& _other) const;
    };

    /**
     * @brief Роли данных из модели
     */
    enum DataRole {
        SceneNumberRole = Qt::UserRole + 1,
        SceneHeadingRole,
        SceneTextRole,
        SceneColorRole,
        SceneInlineNotesSizeRole,
        SceneReviewMarksSizeRole,
    };

public:
    explicit AbstractTextModelGroupItem(const AbstractTextModel* _model);
    AbstractTextModelGroupItem(const AbstractTextModel* _model, QXmlStreamReader& _contentReader);
    ~AbstractTextModelGroupItem() override;

    /**
     * @brief Тип группы
     */
    const TextGroupType& groupType() const;
    void setGroupType(TextGroupType _type);

    /**
     * @brief Идентификатор группы
     */
    QUuid uuid() const;

    /**
     * @brief Уровень вложенности группы
     */
    int level() const;
    void setLevel(int _level);

    /**
     * @brief Номер группы
     */
    Number number() const;
    bool setNumber(int _number, const QString& _prefix);

    /**
     * @brief Цвет группы
     */
    QColor color() const;
    void setColor(const QColor& _color);

    /**
     * @brief Заголовок группы
     */
    QString heading() const;

    /**
     * @brief Определяем интерфейс получения данных группы
     */
    QVariant data(int _role) const override;

    /**
     * @brief Определяем интерфейс для получения XML блока
     */
    QByteArray toXml() const override;
    QByteArray toXml(AbstractTextModelItem* _from, int _fromPosition, AbstractTextModelItem* _to,
                     int _toPosition, bool _clearUuid) const;
    QByteArray xmlHeader(bool _clearUuid = false) const;

    /**
     * @brief Скопировать контент с заданного элемента
     */
    void copyFrom(AbstractTextModelItem* _item) override;

    /**
     * @brief Проверить равен ли текущий элемент заданному
     */
    bool isEqual(AbstractTextModelItem* _item) const override;

protected:
    /**
     * @brief Обновляем текст группы при изменении кого-то из детей
     */
    void handleChange() override;

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace BusinessLayer
