#pragma once

#include <ui/design_system/design_system.h>

#include <QLocale>
#include <QObject>

namespace Ui {
enum class ApplicationTheme;
class ThemeSetupView;
} // namespace Ui


namespace ManagementLayer {

/**
 * @brief Менеджер экрана настроек
 */
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    SettingsManager(QObject* _parent, QWidget* _parentWidget);
    ~SettingsManager() override;

    QWidget* toolBar() const;
    QWidget* navigator() const;
    QWidget* view() const;

    /**
     * @brief Задать виджет настройки темы
     */
    void setThemeSetupView(Ui::ThemeSetupView* _view);

    /**
     * @brief Обновить используемый коэффициент масштабирования в представлении
     */
    void updateScaleFactor();

signals:
    /**
     * @brief Пользователь хочет закрыть настройки
     */
    void closeSettingsRequested();

    /**
     * @brief Изменились параметры приложения
     */
    void applicationLanguageChanged(QLocale::Language _language);
    void applicationUseSpellCheckerChanged(bool _use);
    void applicationSpellCheckerLanguageChanged(const QString& _languageCode);
    void applicationThemeChanged(Ui::ApplicationTheme _theme);
    void applicationCustomThemeColorsChanged(const Ui::DesignSystem::Color& _color);
    void applicationScaleFactorChanged(qreal _scaleFactor);
    void applicationUseAutoSaveChanged(bool _use);
    void applicationSaveBackupsChanged(bool _save);
    void applicationBackupsFolderChanged(const QString& _path);

    /**
     * @brief Изменились параметры компонентов
     */
    void simpleTextEditorChanged(const QStringList& _changedSettingsKeys);
    void simpleTextNavigatorChanged();
    void screenplayEditorChanged(const QStringList& _changedSettingsKeys);
    void screenplayNavigatorChanged();
    void screenplayDurationChanged();
    void comicBookEditorChanged(const QStringList& _changedSettingsKeys);
    void comicBookNavigatorChanged();

protected:
    /**
     * @brief Реализуем фильтр на событие смены языка и дизайн системы, чтобы обновить значения в
     * представлении
     */
    bool eventFilter(QObject* _watched, QEvent* _event) override;

private:
    //
    // Сохранение параметров приложения
    //
    void setApplicationLanguage(int _language);
    void setApplicationTheme(Ui::ApplicationTheme _theme);
    void setApplicationCustomThemeColors(const Ui::DesignSystem::Color& _color);
    void setApplicationScaleFactor(qreal _scaleFactor);
    void setApplicationUseAutoSave(bool _use);
    void setApplicationSaveBackups(bool _save);
    void setApplicationBackupsFolder(const QString& _path);
    void setApplicationShowDocumentsPages(bool _show);
    void setApplicationUseTypeWriterSound(bool _use);
    void setApplicationUseSpellChecker(bool _use);
    void setApplicationSpellCheckerLanguage(const QString& _languageCode);
    void loadSpellingDictionary(const QString& _languageCode);
    void loadSpellingDictionaryAffFile(const QString& _languageCode);
    void loadSpellingDictionaryDicFile(const QString& _languageCode);
    void setApplicationHighlightCurrentLine(bool _highlight);
    void setApplicationFocusCurrentParagraph(bool _focus);
    void setApplicationUseTypewriterScrolling(bool _use);

    //
    // Сохранение параметров редактора сценария
    //
    void setSimpleTextEditorDefaultTemplate(const QString& _templateId);
    //
    void setSimpleTextNavigatorShowSceneText(bool _show, int _lines);

    //
    // Сохранение параметров редактора сценария
    //
    void setScreenplayEditorDefaultTemplate(const QString& _templateId);
    void setScreenplayEditorShowSceneNumber(bool _show, bool _atLeft, bool _atRight);
    void setScreenplayEditorShowDialogueNumber(bool _show);
    void setScreenplayEditorContinueDialogue(bool _continue);
    //
    void setScreenplayNavigatorShowSceneNumber(bool _show);
    void setScreenplayNavigatorShowSceneText(bool _show, int _lines);
    //
    void setScreenplayDurationType(int _type);
    void setScreenplayDurationByPageDuration(int _duration);
    void setScreenplayDurationByCharactersCharacters(int _characters);
    void setScreenplayDurationByCharactersIncludeSpaces(bool _include);
    void setScreenplayDurationByCharactersDuration(int _duration);

    //
    // Сохранение параметров редактора комикса
    //
    void setComicBookEditorDefaultTemplate(const QString& _templateId);
    //
    void setComicBookNavigatorShowSceneText(bool _show, int _lines);

    //
    // Сохранение параметров горячих клавиш
    //
    void setShortcutsForScreenplayEdit(const QString& _blockType, const QString& _shortcut,
                                       const QString& _jumpByTab, const QString& _jumpByEnter,
                                       const QString& _changeByTab, const QString& _changeByEnter);

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace ManagementLayer
