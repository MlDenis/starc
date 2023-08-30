#pragma once

#include <ui/widgets/dialog/abstract_dialog.h>


namespace BusinessLayer {
class AbstractModel;
struct CharactersExportOptions;
} // namespace BusinessLayer

namespace Ui {

/**
 * @brief Диалог настройки параметров экспорта
 */
class CharactersExportDialog : public AbstractDialog
{
    Q_OBJECT

public:
    explicit CharactersExportDialog(QWidget* _parent = nullptr);
    ~CharactersExportDialog() override;

    /**
     * @brief Задать модель персонажей для отображения списка
     */
    void setModel(BusinessLayer::AbstractModel* _model) const;

    /**
     * @brief Получить опции экспорта
     */
    BusinessLayer::CharactersExportOptions exportOptions() const;

    /**
     * @brief Нужно ли открыть экспортированный документ после экспорта
     */
    bool openDocumentAfterExport() const;

signals:
    /**
     * @brief Пользователь хочет экспортировать сценарий с заданными параметрами
     */
    void exportRequested();

    /**
     * @brief Пользователь передумал импортировать данные
     */
    void canceled();

protected:
    /**
     * @brief Определим виджет, который необходимо сфокусировать после отображения диалога
     */
    QWidget* focusedWidgetAfterShow() const override;

    /**
     * @brief Опеределим последний фокусируемый виджет в диалоге
     */
    QWidget* lastFocusableWidget() const override;

    /**
     * @brief Обновить переводы
     */
    void updateTranslations() override;

    /**
     * @brief Обновляем UI при изменении дизайн системы
     */
    void designSystemChangeEvent(DesignSystemChangeEvent* _event) override;

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace Ui
