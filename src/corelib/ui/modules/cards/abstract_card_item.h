#pragma once

#include <QGraphicsRectItem>


namespace Ui {

/**
 * @brief Абстврактный класс карточки
 */
class AbstractCardItem : public QGraphicsRectItem
{
public:
    explicit AbstractCardItem(QGraphicsItem* _parent = nullptr);
    ~AbstractCardItem() override;

    /**
     * @brief Индекс элемента модели
     */
    QModelIndex modelItemIndex() const;
    void setModelItemIndex(const QModelIndex& _index);

    /**
     * @brief Высота заголовка
     */
    virtual qreal headerHeight() const;

    /**
     * @brief Является ли карточка контейнером
     */
    virtual bool isContainer() const;

    /**
     * @brief Является ли карточка контейнером заданной (рекурсивно)
     */
    bool isContainerOf(AbstractCardItem* _card) const;

    /**
     * @brief Состояние карточки относительно перемещаемой в данный момент
     */
    enum class InsertionState {
        Empty,
        InsertBefore,
        InsertAfter,
        InsertInside,
    };
    InsertionState insertionState() const;
    void setInsertionState(InsertionState _state);

    /**
     * @brief Открыта ли карточка
     */
    virtual bool isOpened() const;

    /**
     * @brief Может ли карточка быть вложена в контейнер
     */
    virtual bool canBeEmbedded() const;

    /**
     * @brief Контейнер, в который карточка вложена
     */
    AbstractCardItem* container() const;
    void setContainer(AbstractCardItem* _container);

    /**
     * @brief Управление вложенными карточками
     */
    void embedCard(AbstractCardItem* _child);
    void unembedCard(AbstractCardItem* _child);
    QList<AbstractCardItem*> embeddedCards() const;
    int childCount() const;

    /**
     * @brief Обновить положения вложенных карточек
     */
    void updateEmbeddedCardsPositions();

    /**
     * @brief Подходит ли карточка под условия фильтра
     */
    virtual bool isFilterAccepted(const QString& _text, bool _caseSensitive, int _filterType) const;

protected:
    /**
     * @brief Инициилизировать карточку после установки элемента модели в неё
     */
    virtual void init();

    /**
     * @brief При захвате карточки обновляем значения положений вложенных карточек
     */
    void mousePressEvent(QGraphicsSceneMouseEvent* _event) override;

    /**
     * @brief Переопределяем для реализации эффекта вложения карточки в контейнер
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* _event) override;

    /**
     * @brief При именении положения, корректируем положение вложенных элементов
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace Ui
