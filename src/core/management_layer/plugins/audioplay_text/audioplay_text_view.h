#pragma once

#include <interfaces/ui/i_document_view.h>
#include <ui/widgets/widget/widget.h>


namespace BusinessLayer {
class AudioplayTextModel;
}

namespace Ui {

/**
 * @brief Представление редактора документа аудиопостановки
 */
class AudioplayTextView : public Widget, public IDocumentView
{
    Q_OBJECT

public:
    explicit AudioplayTextView(QWidget* _parent = nullptr);
    ~AudioplayTextView() override;

    /**
     * @brief Реализация интерфейса IDocumentView
     */
    /** @{ */
    QWidget* asQWidget() override;
    void toggleFullScreen(bool _isFullScreen) override;
    QVector<QAction*> options() const override;
    /** @{ */

    /**
     * @brief Настроить редактор в соответствии с параметрами заданными в настройках
     */
    void reconfigure(const QStringList& _changedSettingsKeys);

    /**
     * @brief Работа с параметрами отображения представления
     */
    void loadViewSettings();
    void saveViewSettings();

    /**
     * @brief Установить модель
     */
    void setModel(BusinessLayer::AudioplayTextModel* _model);

    /**
     * @brief Получить индекс элемента модели в текущей позиции курсора
     */
    QModelIndex currentModelIndex() const;

    /**
     * @brief Поставить курсор в позицию элемента с заданным индексом модели
     */
    void setCurrentModelIndex(const QModelIndex& _index);

    /**
     * @brief Позиция курсора
     */
    int cursorPosition() const;
    void setCursorPosition(int _position);

signals:
    /**
     * @brief Изменился индекс текущего элемента модели в текстовом документе (перестился курсор)
     */
    void currentModelIndexChanged(const QModelIndex& _index);

    /**
     * @brief Нажатия пользователя в контекстном меню для активации действия
     */
    void addBookmarkRequested();
    void editBookmarkRequested();

    /**
     * @brief Уведомления, что пользователь осуществил действие в панели редактирования закладки
     */
    void createBookmarkRequested(const QString& _text, const QColor& _color);
    void changeBookmarkRequested(const QModelIndex& _index, const QString& _text,
                                 const QColor& _color);

    /**
     * @brief Пользователь хочет удалить закладку
     */
    void removeBookmarkRequested();

protected:
    /**
     * @brief Фильтруем события для корректировки панелей
     */
    bool eventFilter(QObject* _target, QEvent* _event) override;

    /**
     * @brief Переопределяем для корректировки положения тулбара действий над проектами
     */
    void resizeEvent(QResizeEvent* _event) override;

    /**
     * @brief Обновить переводы
     */
    void updateTranslations() override;

    /**
     * @brief Обновляем виджет при изменении дизайн системы
     */
    void designSystemChangeEvent(DesignSystemChangeEvent* _event) override;

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace Ui