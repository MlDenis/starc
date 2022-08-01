#pragma once

#include <QObject>

namespace Domain {
enum class SubscriptionType;
}

namespace ManagementLayer {

class Project;

/**
 * @brief Менеджер экрана со списком проектов
 */
class ProjectsManager : public QObject
{
    Q_OBJECT

public:
    ProjectsManager(QObject* _parent, QWidget* _parentWidget);
    ~ProjectsManager() override;

    QWidget* toolBar() const;
    QWidget* navigator() const;
    QWidget* view() const;

    /**
     * @brief Показать/скрыть наличие непрочитанных комментариев
     */
    void setHasUnreadNotifications(bool _hasUnreadNotifications);

    /**
     * @brief Загрузить список проектов
     */
    void loadProjects();

    /**
     * @brief Сохранить список проектов
     */
    void saveProjects();

    /**
     * @brief Обновить дату последнего изменения проекта
     */
    void saveChanges();

    /**
     * @brief Можно ли создавать проекты в облаке
     */
    void setProjectsInCloudCanBeCreated(bool _authorized,
                                        Domain::SubscriptionType _subscritionType);

    /**
     * @brief Создать проект
     */
    void createProject();

    /**
     * @brief Выбрать проект для открытия
     */
    void openProject();

    /**
     * @brief Установить текущий проект
     */
    void setCurrentProject(const QString& _path);

    /**
     * @brief Установить текущий проект
     * @note Для случаев, когда в программе открывается проект другой программы, @b _path указвыает
     *       путь к исходному файлу, а @b _realPath путь к временному файлу проекта
     */
    void setCurrentProject(const QString& _path, const QString& _realPath);

    /**
     * @brief Установить название текущего проекта
     */
    void setCurrentProjectName(const QString& _name);

    /**
     * @brief Установить короткое описание текущего проекта
     */
    void setCurrentProjectLogline(const QString& _logline);

    /**
     * @brief Установить обложку текущего проекта
     */
    void setCurrentProjectCover(const QPixmap& _cover);

    /**
     * @brief Запомнить, что пользователь больше никогда нехочет видеть вопроса о переключении
     * формата проекта
     */
    void setCurrentProjectNeverAskAboutSwitch();

    /**
     * @brief Закрыть текущий проект
     */
    void closeCurrentProject();

    /**
     * @brief Скрыть проект
     */
    void hideProject(const QString& _path);

    /**
     * @brief Получить текущий проект
     */
    const Project& currentProject() const;

signals:
    /**
     * @brief Запрос на отображение меню
     */
    void menuRequested();

    /**
     * @brief Пользователь хочет создать проект
     */
    void createProjectRequested();

    /**
     * @brief Пользователь хочет создать локальный проект
     */
    void createLocalProjectRequested(const QString& _projectName, const QString& _projectFilePath,
                                     const QString& _importFilePath);

    /**
     * @brief Пользователь хочет создать проект в облаке
     */
    void createCloudProjectRequested(const QString& _projectName, const QString& _importFilePath);

    /**
     * @brief Пользователь хочет открыть проект
     */
    void openProjectRequested();

    /**
     * @brief Пользователь хочет открыть проект по заданному пути
     */
    void openChoosedProjectRequested(const QString& _path);

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace ManagementLayer
