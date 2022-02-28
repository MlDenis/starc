#pragma once

#include <business_layer/model/abstract_model.h>


namespace BusinessLayer {

class CharactersModel;
class ComicBookDictionariesModel;
class ComicBookInformationModel;
class ComicBookTextModelItem;
class TextModel;

/**
 * @brief Модель текста комикса
 */
class CORE_LIBRARY_EXPORT ComicBookTextModel : public AbstractModel
{
    Q_OBJECT

public:
    explicit ComicBookTextModel(QObject* _parent = nullptr);
    ~ComicBookTextModel() override;

    /**
     * @brief Добавить элемент в конец
     */
    void appendItem(ComicBookTextModelItem* _item, ComicBookTextModelItem* _parentItem = nullptr);

    /**
     * @brief Добавить элемент в начало
     */
    void prependItem(ComicBookTextModelItem* _item, ComicBookTextModelItem* _parentItem = nullptr);

    /**
     * @brief Вставить элемент после заданного
     */
    void insertItem(ComicBookTextModelItem* _item, ComicBookTextModelItem* _afterSiblingItem);
    void insertItems(const QVector<ComicBookTextModelItem*>& _items,
                     ComicBookTextModelItem* _afterSiblingItem);

    /**
     * @brief Извлечь заданный элемент без удаления
     */
    void takeItem(ComicBookTextModelItem* _item, ComicBookTextModelItem* _parentItem = nullptr);

    /**
     * @brief Удалить заданный элемент
     */
    void removeItem(ComicBookTextModelItem* _item);
    void removeItems(ComicBookTextModelItem* _fromItem, ComicBookTextModelItem* _toItem);

    /**
     * @brief Обновить заданный элемент
     */
    void updateItem(ComicBookTextModelItem* _item);

    /**
     * @brief Реализация древовидной модели
     */
    /** @{ */
    QModelIndex index(int _row, int _column, const QModelIndex& _parent = {}) const override;
    QModelIndex parent(const QModelIndex& _child) const override;
    int columnCount(const QModelIndex& _parent = {}) const override;
    int rowCount(const QModelIndex& _parent = {}) const override;
    Qt::ItemFlags flags(const QModelIndex& _index) const override;
    QVariant data(const QModelIndex& _index, int _role) const override;
    //! Реализация перетаскивания элементов
    bool canDropMimeData(const QMimeData* _data, Qt::DropAction _action, int _row, int _column,
                         const QModelIndex& _parent = {}) const override;
    bool dropMimeData(const QMimeData* _data, Qt::DropAction _action, int _row, int _column,
                      const QModelIndex& _parent = {}) override;
    QMimeData* mimeData(const QModelIndexList& _indexes) const override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    /** @} */

    /**
     * @brief Сформировать mime-данные комикса в заданном диапазоне
     */
    QString mimeFromSelection(const QModelIndex& _from, int _fromPosition, const QModelIndex& _to,
                              int _toPosition, bool _clearUuid) const;

    /**
     * @brief Вставить контент из mime-данных с комиксом в заданной позиции
     */
    void insertFromMime(const QModelIndex& _index, int _positionInBlock, const QString& _mimeData);

    /**
     * @brief Получить элемент находящийся в заданном индексе
     */
    ComicBookTextModelItem* itemForIndex(const QModelIndex& _index) const;

    /**
     * @brief Получить индекс заданного элемента
     */
    QModelIndex indexForItem(ComicBookTextModelItem* _item) const;

    /**
     * @brief Задать модель информации о комиксе
     */
    void setInformationModel(ComicBookInformationModel* _model);
    ComicBookInformationModel* informationModel() const;

    /**
     * @brief Задать модель титульной страницы
     */
    void setTitlePageModel(TextModel* _model);
    TextModel* titlePageModel() const;

    /**
     * @brief Задать модель справочников комикса
     */
    void setDictionariesModel(ComicBookDictionariesModel* _model);
    ComicBookDictionariesModel* dictionariesModel() const;

    /**
     * @brief Задать модель персонажей проекта
     */
    void setCharactersModel(CharactersModel* _model);
    CharactersModel* charactersModel() const;

    /**
     * @brief Обновить имя персонажа
     */
    void updateCharacterName(const QString& _oldName, const QString& _newName);

    /**
     * @brief Найти всех персонажей сценария
     */
    QSet<QString> findCharactersFromText() const;

protected:
    /**
     * @brief Реализация модели для работы с документами
     */
    /** @{ */
    void initDocument() override;
    void clearDocument() override;
    QByteArray toXml() const override;
    void applyPatch(const QByteArray& _patch) override;
    /** @} */

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace BusinessLayer
