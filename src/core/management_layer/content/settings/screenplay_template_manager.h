#pragma once

#include <QObject>


namespace ManagementLayer {

/**
 * @brief Менеджер экрана параметров шаблона сценария
 */
class ScreenplayTemplateManager : public QObject
{
    Q_OBJECT

public:
    explicit ScreenplayTemplateManager(QObject* _parent, QWidget* _parentWidget);
    ~ScreenplayTemplateManager() override;

    QWidget* toolBar() const;
    QWidget* navigator() const;
    QWidget* view() const;

signals:
    /**
     * @brief Запрос на закрытие редактора шаблона
     */
    void closeRequested();

    /**
     * @brief Запрос на отображение заданного представления
     */
    void showViewRequested(QWidget* _view);

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace ManagementLayer
