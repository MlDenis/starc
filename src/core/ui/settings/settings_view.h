#pragma once

#include <ui/widgets/stack_widget/stack_widget.h>

class QAbstractItemModel;
class HierarchicalModel;


namespace Ui {

enum class ApplicationTheme;

/**
 * @brief Представление настроек
 */
class SettingsView : public StackWidget
{
    Q_OBJECT

public:
    explicit SettingsView(QWidget* _parent = nullptr);
    ~SettingsView() override;

    /**
     * @brief Показать основную страницу
     */
    void showDefaultPage();

    /**
     * @brief По возможности сфокусировать на экране заданный виджет
     */
    void showApplication();
    void showApplicationUserInterface();
    void showApplicationSaveAndBackups();
    void showApplicationTextEditing();
    void showComponents();
    void showComponentsSimpleText();
    void showComponentsScreenplay();
    void showComponentsComicBook();
    void showShortcuts();

    //
    // Задание параметров приложения
    //
    void setApplicationLanguage(int _language);
    void setApplicationScaleFactor(qreal _scaleFactor);
    void setApplicationUseAutoSave(bool _use);
    void setApplicationSaveBackups(bool _save);
    void setApplicationBackupsFolder(const QString& _path);
    void setApplicationShowDocumentsPages(bool _show);
    void setApplicationUseTypewriterSound(bool _use);
    void setApplicationUseSpellChecker(bool _use);
    void setApplicationSpellCheckerLanguage(const QString& _languageCode);
    void setApplicationHighlightCurrentLine(bool _highlight);
    void setApplicationFocusCurrentParagraph(bool _focus);
    void setApplicationUseTypewriterScrolling(bool _use);

    //
    // Задание параметров редактора текста
    //
    void setSimpleTextEditorDefaultTemplate(const QString& _templateId);
    //
    void setSimpleTextNavigatorShowSceneText(bool _show, int _lines);

    //
    // Задание параметров редактора сценария
    //
    void setScreenplayEditorDefaultTemplate(const QString& _templateId);
    void setScreenplayEditorShowSceneNumber(bool _show, bool _atLeft, bool _atRight);
    void setScreenplayEditorShowDialogueNumber(bool _show);
    void setScreenplayEditorContinueDialogue(bool _continue);
    void setScreenplayEditorUseCharactersFromText(bool _use);
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
    // Задание параметров редактора комикса
    //
    void setComicBookEditorDefaultTemplate(const QString& _templateId);
    //
    void setComicBookNavigatorShowSceneText(bool _show, int _lines);

    //
    // Задание параметров горячих клавиш
    //
    void setShortcutsForScreenplayModel(HierarchicalModel* _model);

signals:
    /**
     * @brief Пользователь нажал кнопку выборя языка
     */
    void applicationLanguagePressed();

    //
    // Уведомление об изменении параметров приложения
    //
    void applicationThemePressed(Ui::ApplicationTheme _theme);
    void applicationScaleFactorChanged(qreal _scaleFactor);
    void applicationUseAutoSaveChanged(bool _use);
    void applicationSaveBackupsChanged(bool _save);
    void applicationBackupsFolderChanged(const QString& _path);
    void applicationShowDocumentsPagesChanged(bool _show);
    void applicationUseTypewriterSoundChanged(bool _use);
    void applicationUseSpellCheckerChanged(bool _use);
    void applicationSpellCheckerLanguageChanged(const QString& _languageCode);
    void applicationHighlightCurentLineChanged(bool _highlight);
    void applicationFocusCurrentParagraphChanged(bool _focus);
    void applicationUseTypewriterScrollingChanged(bool _use);

    //
    // Уведомление об изменении параметров редактора текста
    //
    void simpleTextEditorDefaultTemplateChanged(const QString& _templateId);
    //
    void simpleTextNavigatorShowSceneTextChanged(bool _show, int _lines);

    //
    // Уведомление об изменении параметров редактора сценария
    //
    void screenplayEditorDefaultTemplateChanged(const QString& _templateId);
    void screenplayEditorShowSceneNumberChanged(bool _show, bool _atLeft, bool _atRight);
    void screenplayEditorShowDialogueNumberChanged(bool _show);
    void screenplayEditorContinueDialogueChanged(bool _continue);
    void screenplayEditorUseCharactersFromTextChanged(bool _use);
    //
    void screenplayNavigatorShowSceneNumberChanged(bool _show);
    void screenplayNavigatorShowSceneTextChanged(bool _show, int _lines);
    //
    void screenplayDurationTypeChanged(int _type);
    void screenplayDurationByPageDurationChanged(int _duration);
    void screenplayDurationByCharactersCharactersChanged(int _characters);
    void screenplayDurationByCharactersIncludeSpacesChanged(bool _include);
    void screenplayDurationByCharactersDurationChanged(int _duration);

    //
    // Уведомление об изменении параметров редактора комикса
    //
    void comicBookEditorDefaultTemplateChanged(const QString& _templateId);
    //
    void comicBookNavigatorShowSceneTextChanged(bool _show, int _lines);

    //
    // Редактирование шаблонов
    //
    void editCurrentScreenplayEditorTemplateRequested(const QString& _templateId);
    void duplicateCurrentScreenplayEditorTemplateRequested(const QString& _templateId);
    void removeCurrentScreenplayEditorTemplateRequested(const QString& _templateId);

    //
    // Параметры горячих клавиш
    //
    void shortcutsForScreenplayEditorChanged(const QString& _blockType, const QString& _shortcut,
                                             const QString& _jumpByTab, const QString& _jumpByEnter,
                                             const QString& _changeByTab,
                                             const QString& _changeByEnter);

protected:
    /**
     * @brief Переопределяем для обновления размеров таблиц
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
