#include "screenplay_text_view.h"

#include "text/screenplay_text_edit.h"
#include "text/screenplay_text_edit_shortcuts_manager.h"
#include "text/screenplay_text_edit_toolbar.h"
#include "text/screenplay_text_fast_format_widget.h"
#include "text/screenplay_text_scrollbar_manager.h"
#include "text/screenplay_text_search_manager.h"

#include <business_layer/document/text/text_block_data.h>
#include <business_layer/document/text/text_cursor.h>
#include <business_layer/model/screenplay/screenplay_information_model.h>
#include <business_layer/model/screenplay/text/screenplay_text_model.h>
#include <business_layer/templates/screenplay_template.h>
#include <business_layer/templates/templates_facade.h>
#include <data_layer/storage/settings_storage.h>
#include <data_layer/storage/storage_facade.h>
#include <ui/design_system/design_system.h>
#include <ui/modules/comments/comments_model.h>
#include <ui/modules/comments/comments_toolbar.h>
#include <ui/modules/comments/comments_view.h>
#include <ui/widgets/floating_tool_bar/floating_toolbar_animator.h>
#include <ui/widgets/scroll_bar/scroll_bar.h>
#include <ui/widgets/shadow/shadow.h>
#include <ui/widgets/splitter/splitter.h>
#include <ui/widgets/stack_widget/stack_widget.h>
#include <ui/widgets/tab_bar/tab_bar.h>
#include <ui/widgets/text_edit/completer/completer.h>
#include <ui/widgets/text_edit/scalable_wrapper/scalable_wrapper.h>
#include <utils/helpers/color_helper.h>
#include <utils/helpers/measurement_helper.h>
#include <utils/helpers/ui_helper.h>

#include <QAction>
#include <QPointer>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>

namespace Ui {

namespace {
const int kTypeDataRole = Qt::UserRole + 100;

const int kFastFormatTabIndex = 0;
const int kCommentsTabIndex = 1;

const QString kSettingsKey = "screenplay-text";
const QString kScaleFactorKey = kSettingsKey + "/scale-factor";
const QString kSidebarStateKey = kSettingsKey + "/sidebar-state";
const QString kIsFastFormatPanelVisibleKey = kSettingsKey + "/is-fast-format-panel-visible";
const QString kIsCommentsModeEnabledKey = kSettingsKey + "/is-comments-mode-enabled";
const QString kSidebarPanelIndexKey = kSettingsKey + "/sidebar-panel-index";
} // namespace

class ScreenplayTextView::Implementation
{
public:
    explicit Implementation(QWidget* _parent);

    /**
     * @brief Переконфигурировать представление
     */
    void reconfigureTemplate(bool _withModelReinitialization = true);
    void reconfigureSceneNumbersVisibility();
    void reconfigureDialoguesNumbersVisibility();

    /**
     * @brief Обновить настройки UI панели инструментов
     */
    void updateToolBarUi();

    /**
     * @brief Обновить текущий отображаемый тип абзаца в панели инструментов
     */
    void updateToolBarCurrentParagraphTypeName();

    /**
     * @brief Обновить компоновку страницы
     */
    void updateTextEditPageMargins();

    /**
     * @brief Обновить видимость и положение панели инструментов рецензирования
     */
    void updateCommentsToolBar();

    /**
     * @brief Обновить видимость боковой панели (показана, если показана хотя бы одна из вложенных
     * панелей)
     */
    void updateSideBarVisibility(QWidget* _container);

    /**
     * @brief Добавить редакторскую заметку для текущего выделения
     */
    void addReviewMark(const QColor& _textColor, const QColor& _backgroundColor,
                       const QString& _comment);


    QPointer<BusinessLayer::ScreenplayTextModel> model;
    BusinessLayer::CommentsModel* commentsModel = nullptr;

    ScreenplayTextEdit* screenplayText = nullptr;
    ScreenplayTextEditShortcutsManager shortcutsManager;
    ScalableWrapper* scalableWrapper = nullptr;
    ScreenplayTextScrollBarManager* screenplayTextScrollbarManager = nullptr;

    ScreenplayTextEditToolbar* toolbar = nullptr;
    BusinessLayer::ScreenplayTextSearchManager* searchManager = nullptr;
    FloatingToolbarAnimator* toolbarAnimation = nullptr;
    BusinessLayer::TextParagraphType currentParagraphType
        = BusinessLayer::TextParagraphType::Undefined;
    QStandardItemModel* paragraphTypesModel = nullptr;

    CommentsToolbar* commentsToolbar = nullptr;

    Shadow* sidebarShadow = nullptr;

    bool isSidebarShownFirstTime = true;
    Widget* sidebarWidget = nullptr;
    TabBar* sidebarTabs = nullptr;
    StackWidget* sidebarContent = nullptr;
    ScreenplayTextFastFormatWidget* fastFormatWidget = nullptr;
    CommentsView* commentsView = nullptr;

    Splitter* splitter = nullptr;
};

ScreenplayTextView::Implementation::Implementation(QWidget* _parent)
    : commentsModel(new BusinessLayer::CommentsModel(_parent))
    , screenplayText(new ScreenplayTextEdit(_parent))
    , shortcutsManager(screenplayText)
    , scalableWrapper(new ScalableWrapper(screenplayText, _parent))
    , screenplayTextScrollbarManager(new ScreenplayTextScrollBarManager(scalableWrapper))
    , toolbar(new ScreenplayTextEditToolbar(scalableWrapper))
    , searchManager(new BusinessLayer::ScreenplayTextSearchManager(scalableWrapper, screenplayText))
    , toolbarAnimation(new FloatingToolbarAnimator(_parent))
    , paragraphTypesModel(new QStandardItemModel(toolbar))
    , commentsToolbar(new CommentsToolbar(_parent))
    , sidebarShadow(new Shadow(Qt::RightEdge, scalableWrapper))
    , sidebarWidget(new Widget(_parent))
    , sidebarTabs(new TabBar(_parent))
    , sidebarContent(new StackWidget(_parent))
    , fastFormatWidget(new ScreenplayTextFastFormatWidget(_parent))
    , commentsView(new CommentsView(_parent))
    , splitter(new Splitter(_parent))

{
    toolbar->setParagraphTypesModel(paragraphTypesModel);

    commentsToolbar->hide();

    screenplayText->setVerticalScrollBar(new ScrollBar);
    screenplayText->setHorizontalScrollBar(new ScrollBar);
    shortcutsManager.setShortcutsContext(scalableWrapper);
    //
    // Вертикальный скрол настраивается менеджером screenplayTextScrollbarManager
    //
    scalableWrapper->setHorizontalScrollBar(new ScrollBar);
    scalableWrapper->initScrollBarsSyncing();
    screenplayTextScrollbarManager->initScrollBarsSyncing();

    screenplayText->setUsePageMode(true);

    sidebarWidget->hide();
    sidebarTabs->setFixed(true);
    sidebarTabs->addTab({}); // fastformat
    sidebarTabs->setTabVisible(kFastFormatTabIndex, false);
    sidebarTabs->addTab({}); // comments
    sidebarTabs->setTabVisible(kCommentsTabIndex, false);
    sidebarContent->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sidebarContent->setAnimationType(StackWidget::AnimationType::Slide);
    sidebarContent->addWidget(fastFormatWidget);
    sidebarContent->addWidget(commentsView);
    fastFormatWidget->hide();
    fastFormatWidget->setParagraphTypesModel(paragraphTypesModel);
    commentsView->setModel(commentsModel);
    commentsView->hide();
}

void ScreenplayTextView::Implementation::reconfigureTemplate(bool _withModelReinitialization)
{
    paragraphTypesModel->clear();

    using namespace BusinessLayer;
    const auto& usedTemplate = BusinessLayer::TemplatesFacade::screenplayTemplate(
        model && model->informationModel() ? model->informationModel()->templateId() : "");
    const QVector<TextParagraphType> types = {
        TextParagraphType::SceneHeading,    TextParagraphType::SceneCharacters,
        TextParagraphType::Action,          TextParagraphType::Character,
        TextParagraphType::Parenthetical,   TextParagraphType::Dialogue,
        TextParagraphType::Lyrics,          TextParagraphType::Shot,
        TextParagraphType::Transition,      TextParagraphType::InlineNote,
        TextParagraphType::UnformattedText, TextParagraphType::SequenceHeading,
    };
    for (const auto type : types) {
        if (!usedTemplate.paragraphStyle(type).isActive()) {
            continue;
        }

        auto typeItem = new QStandardItem(toDisplayString(type));
        typeItem->setData(shortcutsManager.shortcut(type), Qt::WhatsThisRole);
        typeItem->setData(static_cast<int>(type), kTypeDataRole);
        paragraphTypesModel->appendRow(typeItem);
    }

    shortcutsManager.reconfigure();

    if (_withModelReinitialization) {
        screenplayText->reinit();
    }
}

void ScreenplayTextView::Implementation::reconfigureSceneNumbersVisibility()
{
    if (model && model->informationModel()) {
        screenplayText->setShowSceneNumber(model->informationModel()->showSceneNumbers(),
                                           model->informationModel()->showSceneNumbersOnLeft(),
                                           model->informationModel()->showSceneNumbersOnRight());
    } else {
        screenplayText->setShowSceneNumber(
            settingsValue(DataStorageLayer::kComponentsScreenplayEditorShowSceneNumbersKey)
                .toBool(),
            settingsValue(DataStorageLayer::kComponentsScreenplayEditorShowSceneNumbersOnLeftKey)
                .toBool(),
            settingsValue(DataStorageLayer::kComponentsScreenplayEditorShowSceneNumbersOnRightKey)
                .toBool());
    }
}

void ScreenplayTextView::Implementation::reconfigureDialoguesNumbersVisibility()
{
    screenplayText->setShowDialogueNumber(
        model && model->informationModel()
            ? model->informationModel()->showDialoguesNumbers()
            : settingsValue(DataStorageLayer::kComponentsScreenplayEditorShowDialogueNumbersKey)
                  .toBool());
}

void ScreenplayTextView::Implementation::updateToolBarUi()
{
    toolbar->move(
        QPointF(Ui::DesignSystem::layout().px24(), Ui::DesignSystem::layout().px24()).toPoint());
    toolbar->setBackgroundColor(Ui::DesignSystem::color().background());
    toolbar->setTextColor(Ui::DesignSystem::color().onBackground());
    toolbar->raise();

    searchManager->toolbar()->move(
        QPointF(Ui::DesignSystem::layout().px24(), Ui::DesignSystem::layout().px24()).toPoint());
    searchManager->toolbar()->setBackgroundColor(Ui::DesignSystem::color().background());
    searchManager->toolbar()->setTextColor(Ui::DesignSystem::color().onBackground());
    searchManager->toolbar()->raise();

    toolbarAnimation->setBackgroundColor(Ui::DesignSystem::color().background());
    toolbarAnimation->setTextColor(Ui::DesignSystem::color().onBackground());

    commentsToolbar->setBackgroundColor(Ui::DesignSystem::color().background());
    commentsToolbar->setTextColor(Ui::DesignSystem::color().onBackground());
    commentsToolbar->raise();
    updateCommentsToolBar();
}

void ScreenplayTextView::Implementation::updateToolBarCurrentParagraphTypeName()
{
    auto paragraphType = screenplayText->currentParagraphType();
    if (currentParagraphType == paragraphType) {
        return;
    }

    currentParagraphType = paragraphType;

    if (paragraphType == BusinessLayer::TextParagraphType::SequenceFooter) {
        paragraphType = BusinessLayer::TextParagraphType::SequenceHeading;
        toolbar->setParagraphTypesEnabled(false);
        fastFormatWidget->setEnabled(false);
    } else {
        toolbar->setParagraphTypesEnabled(true);
        fastFormatWidget->setEnabled(true);
    }

    for (int itemRow = 0; itemRow < paragraphTypesModel->rowCount(); ++itemRow) {
        const auto item = paragraphTypesModel->item(itemRow);
        const auto itemType
            = static_cast<BusinessLayer::TextParagraphType>(item->data(kTypeDataRole).toInt());
        if (itemType == paragraphType) {
            toolbar->setCurrentParagraphType(paragraphTypesModel->index(itemRow, 0));
            fastFormatWidget->setCurrentParagraphType(paragraphTypesModel->index(itemRow, 0));
            return;
        }
    }
}

void ScreenplayTextView::Implementation::updateTextEditPageMargins()
{
    if (screenplayText->usePageMode()) {
        return;
    }

    const QMarginsF pageMargins
        = QMarginsF{ 15, 20 / scalableWrapper->zoomRange(),
                     12 / scalableWrapper->zoomRange()
                         + MeasurementHelper::pxToMm(scalableWrapper->verticalScrollBar()->width()),
                     5 };
    screenplayText->setPageMarginsMm(pageMargins);
}

void ScreenplayTextView::Implementation::updateCommentsToolBar()
{
    if (!toolbar->isCommentsModeEnabled() || !screenplayText->textCursor().hasSelection()) {
        commentsToolbar->hideToolbar();
        return;
    }

    //
    // Определяем точку на границе страницы, либо если страница не влезает в экран, то с боку экрана
    //
    const int x = (screenplayText->width() - screenplayText->viewport()->width()) / 2
        + screenplayText->viewport()->width() - commentsToolbar->width();
    const qreal textRight = scalableWrapper->mapFromEditor(QPoint(x, 0)).x();
    const auto cursorRect = screenplayText->cursorRect();
    const auto globalCursorCenter = screenplayText->mapToGlobal(cursorRect.center());
    const auto localCursorCenter
        = commentsToolbar->parentWidget()->mapFromGlobal(globalCursorCenter);
    //
    // И смещаем панель рецензирования к этой точке
    //
    commentsToolbar->moveToolbar(QPoint(std::min(scalableWrapper->width() - commentsToolbar->width()
                                                     - Ui::DesignSystem::layout().px24(),
                                                 textRight),
                                        localCursorCenter.y() - (commentsToolbar->height() / 3)));

    //
    // Если панель ещё не была показана, отобразим её
    //
    commentsToolbar->showToolbar();
}

void ScreenplayTextView::Implementation::updateSideBarVisibility(QWidget* _container)
{
    const bool isSidebarShouldBeVisible
        = toolbar->isFastFormatPanelVisible() || toolbar->isCommentsModeEnabled();
    if (sidebarWidget->isVisible() == isSidebarShouldBeVisible) {
        return;
    }

    sidebarShadow->setVisible(isSidebarShouldBeVisible);
    sidebarWidget->setVisible(isSidebarShouldBeVisible);

    if (isSidebarShownFirstTime && isSidebarShouldBeVisible) {
        isSidebarShownFirstTime = false;
        const auto sideBarWidth = sidebarContent->sizeHint().width();
        splitter->setSizes({ _container->width() - sideBarWidth, sideBarWidth });
    }
}

void ScreenplayTextView::Implementation::addReviewMark(const QColor& _textColor,
                                                       const QColor& _backgroundColor,
                                                       const QString& _comment)
{
    //
    // Добавим заметку
    //
    const auto textColor
        = _textColor.isValid() ? _textColor : ColorHelper::contrasted(_backgroundColor);
    screenplayText->addReviewMark(textColor, _backgroundColor, _comment);

    //
    // Снимем выделение, чтобы пользователь получил обратную связь от приложения, что выделение
    // добавлено
    //
    BusinessLayer::TextCursor cursor(screenplayText->textCursor());
    const auto selectionInterval = cursor.selectionInterval();
    //
    // ... делаем танец с бубном, чтобы получить сигнал об обновлении позиции курсора
    //     и выделить новую заметку в общем списке
    //
    cursor.setPosition(selectionInterval.to);
    screenplayText->setTextCursorReimpl(cursor);
    cursor.setPosition(selectionInterval.from);
    screenplayText->setTextCursorReimpl(cursor);

    //
    // Фокусируем редактор сценария, чтобы пользователь мог продолжать работать с ним
    //
    scalableWrapper->setFocus();
}


// ****


ScreenplayTextView::ScreenplayTextView(QWidget* _parent)
    : Widget(_parent)
    , d(new Implementation(this))
{
    setFocusProxy(d->scalableWrapper);
    d->scalableWrapper->installEventFilter(this);

    QVBoxLayout* sidebarLayout = new QVBoxLayout(d->sidebarWidget);
    sidebarLayout->setContentsMargins({});
    sidebarLayout->setSpacing(0);
    sidebarLayout->addWidget(d->sidebarTabs);
    sidebarLayout->addWidget(d->sidebarContent);

    d->splitter->setWidgets(d->scalableWrapper, d->sidebarWidget);
    d->splitter->setSizes({ 1, 0 });

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(d->splitter);

    connect(d->toolbar, &ScreenplayTextEditToolbar::undoPressed, d->screenplayText,
            &ScreenplayTextEdit::undo);
    connect(d->toolbar, &ScreenplayTextEditToolbar::redoPressed, d->screenplayText,
            &ScreenplayTextEdit::redo);
    connect(d->toolbar, &ScreenplayTextEditToolbar::paragraphTypeChanged, this,
            [this](const QModelIndex& _index) {
                const auto type = static_cast<BusinessLayer::TextParagraphType>(
                    _index.data(kTypeDataRole).toInt());
                d->screenplayText->setCurrentParagraphType(type);
                d->scalableWrapper->setFocus();
            });
    connect(d->toolbar, &ScreenplayTextEditToolbar::fastFormatPanelVisibleChanged, this,
            [this](bool _visible) {
                d->sidebarTabs->setTabVisible(kFastFormatTabIndex, _visible);
                d->fastFormatWidget->setVisible(_visible);
                if (_visible) {
                    d->sidebarTabs->setCurrentTab(kFastFormatTabIndex);
                    d->sidebarContent->setCurrentWidget(d->fastFormatWidget);
                }
                d->updateSideBarVisibility(this);
            });
    connect(d->toolbar, &ScreenplayTextEditToolbar::commentsModeEnabledChanged, this,
            [this](bool _enabled) {
                d->sidebarTabs->setTabVisible(kCommentsTabIndex, _enabled);
                d->commentsView->setVisible(_enabled);
                if (_enabled) {
                    d->sidebarTabs->setCurrentTab(kCommentsTabIndex);
                    d->sidebarContent->setCurrentWidget(d->commentsView);
                    d->updateCommentsToolBar();
                }
                d->updateSideBarVisibility(this);
            });
    connect(d->toolbar, &ScreenplayTextEditToolbar::searchPressed, this, [this] {
        d->toolbarAnimation->switchToolbars(d->toolbar->searchIcon(),
                                            d->toolbar->searchIconPosition(), d->toolbar,
                                            d->searchManager->toolbar());
    });
    //
    connect(d->searchManager, &BusinessLayer::ScreenplayTextSearchManager::hideToolbarRequested,
            this, [this] { d->toolbarAnimation->switchToolbarsBack(); });
    //
    connect(d->commentsToolbar, &CommentsToolbar::textColorChangeRequested, this,
            [this](const QColor& _color) { d->addReviewMark(_color, {}, {}); });
    connect(d->commentsToolbar, &CommentsToolbar::textBackgoundColorChangeRequested, this,
            [this](const QColor& _color) { d->addReviewMark({}, _color, {}); });
    connect(d->commentsToolbar, &CommentsToolbar::commentAddRequested, this,
            [this](const QColor& _color) {
                d->sidebarTabs->setCurrentTab(kCommentsTabIndex);
                d->commentsView->showAddCommentView(_color);
            });
    connect(d->commentsView, &CommentsView::addReviewMarkRequested, this,
            [this](const QColor& _color, const QString& _comment) {
                d->addReviewMark({}, _color, _comment);
            });
    connect(d->commentsView, &CommentsView::changeReviewMarkRequested, this,
            [this](const QModelIndex& _index, const QString& _comment) {
                QSignalBlocker blocker(d->commentsView);
                d->commentsModel->setComment(_index, _comment);
            });
    connect(d->commentsView, &CommentsView::addReviewMarkReplyRequested, this,
            [this](const QModelIndex& _index, const QString& _reply) {
                QSignalBlocker blocker(d->commentsView);
                d->commentsModel->addReply(_index, _reply);
            });
    connect(d->commentsView, &CommentsView::commentSelected, this,
            [this](const QModelIndex& _index) {
                const auto positionHint = d->commentsModel->mapToModel(_index);
                const auto position = d->screenplayText->positionForModelIndex(positionHint.index)
                    + positionHint.blockPosition;
                auto cursor = d->screenplayText->textCursor();
                cursor.setPosition(position);
                d->screenplayText->ensureCursorVisible(cursor);
                d->scalableWrapper->setFocus();
            });
    connect(d->commentsView, &CommentsView::markAsDoneRequested, this,
            [this](const QModelIndexList& _indexes) {
                QSignalBlocker blocker(d->commentsView);
                d->commentsModel->markAsDone(_indexes);
            });
    connect(d->commentsView, &CommentsView::markAsUndoneRequested, this,
            [this](const QModelIndexList& _indexes) {
                QSignalBlocker blocker(d->commentsView);
                d->commentsModel->markAsUndone(_indexes);
            });
    connect(d->commentsView, &CommentsView::removeRequested, this,
            [this](const QModelIndexList& _indexes) {
                QSignalBlocker blocker(d->commentsView);
                d->commentsModel->remove(_indexes);
            });
    //
    connect(d->sidebarTabs, &TabBar::currentIndexChanged, this, [this](int _currentIndex) {
        if (_currentIndex == kFastFormatTabIndex) {
            d->sidebarContent->setCurrentWidget(d->fastFormatWidget);
        } else {
            d->sidebarContent->setCurrentWidget(d->commentsView);
        }
    });
    //
    connect(d->fastFormatWidget, &ScreenplayTextFastFormatWidget::paragraphTypeChanged, this,
            [this](const QModelIndex& _index) {
                const auto type = static_cast<BusinessLayer::TextParagraphType>(
                    _index.data(kTypeDataRole).toInt());
                d->screenplayText->setCurrentParagraphType(type);
                d->scalableWrapper->setFocus();
            });
    //
    connect(d->scalableWrapper->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [this] { d->updateCommentsToolBar(); });
    connect(d->scalableWrapper->horizontalScrollBar(), &QScrollBar::valueChanged, this,
            [this] { d->updateCommentsToolBar(); });
    connect(
        d->scalableWrapper, &ScalableWrapper::zoomRangeChanged, this,
        [this] {
            d->updateTextEditPageMargins();
            d->updateCommentsToolBar();
        },
        Qt::QueuedConnection);
    //
    auto handleCursorPositionChanged = [this] {
        //
        // Обновим состояние панелей форматов
        //
        d->updateToolBarCurrentParagraphTypeName();
        //
        // Уведомим навигатор клиентов, о смене текущего элемента
        //
        const auto screenplayModelIndex = d->screenplayText->currentModelIndex();
        emit currentModelIndexChanged(screenplayModelIndex);
        //
        // Если необходимо выберем соответствующий комментарий
        //
        const auto positionInBlock = d->screenplayText->textCursor().positionInBlock();
        const auto commentModelIndex
            = d->commentsModel->mapFromModel(screenplayModelIndex, positionInBlock);
        d->commentsView->setCurrentIndex(commentModelIndex);
    };
    connect(d->screenplayText, &ScreenplayTextEdit::paragraphTypeChanged, this,
            handleCursorPositionChanged);
    connect(d->screenplayText, &ScreenplayTextEdit::cursorPositionChanged, this,
            handleCursorPositionChanged);
    connect(d->screenplayText, &ScreenplayTextEdit::selectionChanged, this,
            [this] { d->updateCommentsToolBar(); });

    updateTranslations();
    designSystemChangeEvent(nullptr);

    reconfigure({});
}

ScreenplayTextView::~ScreenplayTextView() = default;

QWidget* ScreenplayTextView::asQWidget()
{
    return this;
}

void ScreenplayTextView::toggleFullScreen(bool _isFullScreen)
{
    d->toolbar->setVisible(!_isFullScreen);
    d->screenplayTextScrollbarManager->setScrollBarVisible(!_isFullScreen);
}

void ScreenplayTextView::reconfigure(const QStringList& _changedSettingsKeys)
{
    UiHelper::initSpellingFor(d->screenplayText);

    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(
            DataStorageLayer::kComponentsScreenplayEditorDefaultTemplateKey)) {
        d->reconfigureTemplate();
    }

    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(
            DataStorageLayer::kComponentsScreenplayEditorShowSceneNumbersKey)) {
        d->reconfigureSceneNumbersVisibility();
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(
            DataStorageLayer::kComponentsScreenplayEditorShowDialogueNumbersKey)) {
        d->reconfigureDialoguesNumbersVisibility();
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(
            DataStorageLayer::kComponentsScreenplayEditorContinueDialogueKey)) {
        d->screenplayText->setCorrectionOptions(
            settingsValue(DataStorageLayer::kComponentsScreenplayEditorContinueDialogueKey)
                .toBool(),
            true);
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(
            DataStorageLayer::kComponentsScreenplayEditorShortcutsKey)) {
        d->shortcutsManager.reconfigure();
    }

    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(DataStorageLayer::kApplicationShowDocumentsPagesKey)) {
        const auto usePageMode
            = settingsValue(DataStorageLayer::kApplicationShowDocumentsPagesKey).toBool();
        d->screenplayText->setUsePageMode(usePageMode);
        if (usePageMode) {
            d->screenplayText->reinit();
        } else {
            d->updateTextEditPageMargins();
        }
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(DataStorageLayer::kApplicationHighlightCurrentLineKey)) {
        d->screenplayText->setHighlightCurrentLine(
            settingsValue(DataStorageLayer::kApplicationHighlightCurrentLineKey).toBool());
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(DataStorageLayer::kApplicationFocusCurrentParagraphKey)) {
        d->screenplayText->setFocusCurrentParagraph(
            settingsValue(DataStorageLayer::kApplicationFocusCurrentParagraphKey).toBool());
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(DataStorageLayer::kApplicationUseTypewriterScrollingKey)) {
        d->screenplayText->setUseTypewriterScrolling(
            settingsValue(DataStorageLayer::kApplicationUseTypewriterScrollingKey).toBool());
    }
}

void ScreenplayTextView::loadViewSettings()
{
    using namespace DataStorageLayer;

    const auto scaleFactor = settingsValue(kScaleFactorKey, 1.0).toReal();
    d->scalableWrapper->setZoomRange(scaleFactor);

    const auto isCommentsModeEnabled = settingsValue(kIsCommentsModeEnabledKey, false).toBool();
    d->toolbar->setCommentsModeEnabled(isCommentsModeEnabled);
    const auto isFastFormatPanelVisible
        = settingsValue(kIsFastFormatPanelVisibleKey, false).toBool();
    d->toolbar->setFastFormatPanelVisible(isFastFormatPanelVisible);
    const auto sidebarPanelIndex = settingsValue(kSidebarPanelIndexKey, 0).toInt();
    d->sidebarTabs->setCurrentTab(sidebarPanelIndex);

    const auto sidebarState = settingsValue(kSidebarStateKey);
    if (sidebarState.isValid()) {
        d->isSidebarShownFirstTime = false;
        d->splitter->restoreState(sidebarState.toByteArray());
    }
}

void ScreenplayTextView::saveViewSettings()
{
    setSettingsValue(kScaleFactorKey, d->scalableWrapper->zoomRange());

    setSettingsValue(kIsFastFormatPanelVisibleKey, d->toolbar->isFastFormatPanelVisible());
    setSettingsValue(kIsCommentsModeEnabledKey, d->toolbar->isCommentsModeEnabled());
    setSettingsValue(kSidebarPanelIndexKey, d->sidebarTabs->currentTab());

    setSettingsValue(kSidebarStateKey, d->splitter->saveState());
}

void ScreenplayTextView::setModel(BusinessLayer::ScreenplayTextModel* _model)
{
    if (d->model && d->model->informationModel()) {
        disconnect(d->model->informationModel());
    }

    d->model = _model;

    //
    // Отслеживаем изменения некоторых параметров
    //
    if (d->model && d->model->informationModel()) {
        const bool reinitModel = true;
        d->reconfigureTemplate(!reinitModel);
        d->reconfigureSceneNumbersVisibility();
        d->reconfigureDialoguesNumbersVisibility();

        connect(d->model->informationModel(),
                &BusinessLayer::ScreenplayInformationModel::templateIdChanged, this,
                [this] { d->reconfigureTemplate(); });
        connect(d->model->informationModel(),
                &BusinessLayer::ScreenplayInformationModel::showSceneNumbersChanged, this,
                [this] { d->reconfigureSceneNumbersVisibility(); });
        connect(d->model->informationModel(),
                &BusinessLayer::ScreenplayInformationModel::showSceneNumbersOnLeftChanged, this,
                [this] { d->reconfigureSceneNumbersVisibility(); });
        connect(d->model->informationModel(),
                &BusinessLayer::ScreenplayInformationModel::showSceneNumbersOnRightChanged, this,
                [this] { d->reconfigureSceneNumbersVisibility(); });
        connect(d->model->informationModel(),
                &BusinessLayer::ScreenplayInformationModel::showDialoguesNumbersChanged, this,
                [this] { d->reconfigureDialoguesNumbersVisibility(); });
    }

    d->screenplayText->initWithModel(d->model);
    d->screenplayTextScrollbarManager->setModel(d->model);
    d->commentsModel->setTextModel(d->model);

    d->updateToolBarCurrentParagraphTypeName();
}

QModelIndex ScreenplayTextView::currentModelIndex() const
{
    return d->screenplayText->currentModelIndex();
}

void ScreenplayTextView::setCurrentModelIndex(const QModelIndex& _index)
{
    d->screenplayText->setCurrentModelIndex(_index);
}

int ScreenplayTextView::cursorPosition() const
{
    return d->screenplayText->textCursor().position();
}

void ScreenplayTextView::setCursorPosition(int _position)
{
    auto cursor = d->screenplayText->textCursor();
    cursor.setPosition(_position);
    d->screenplayText->ensureCursorVisible(cursor, false);
}

bool ScreenplayTextView::eventFilter(QObject* _target, QEvent* _event)
{
    if (_target == d->scalableWrapper) {
        if (_event->type() == QEvent::Resize) {
            QTimer::singleShot(0, this, [this] { d->updateCommentsToolBar(); });
        } else if (_event->type() == QEvent::KeyPress && d->searchManager->toolbar()->isVisible()
                   && d->scalableWrapper->hasFocus()) {
            auto keyEvent = static_cast<QKeyEvent*>(_event);
            if (keyEvent->key() == Qt::Key_Escape) {
                d->toolbarAnimation->switchToolbarsBack();
            }
        }
    }

    return Widget::eventFilter(_target, _event);
}

void ScreenplayTextView::resizeEvent(QResizeEvent* _event)
{
    Widget::resizeEvent(_event);

    const auto toolbarPosition
        = QPointF(Ui::DesignSystem::layout().px24(), Ui::DesignSystem::layout().px24()).toPoint();
    d->toolbar->move(toolbarPosition);
    d->searchManager->toolbar()->move(toolbarPosition);
    d->updateCommentsToolBar();
}

void ScreenplayTextView::updateTranslations()
{
    d->sidebarTabs->setTabName(kFastFormatTabIndex, tr("Formatting"));
    d->sidebarTabs->setTabName(kCommentsTabIndex, tr("Comments"));
}

void ScreenplayTextView::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    Widget::designSystemChangeEvent(_event);

    setBackgroundColor(Ui::DesignSystem::color().surface());

    d->updateToolBarUi();

    d->screenplayText->setPageSpacing(Ui::DesignSystem::layout().px24());
    QPalette palette;
    palette.setColor(QPalette::Window, Ui::DesignSystem::color().surface());
    palette.setColor(QPalette::Base, Ui::DesignSystem::color().textEditor());
    palette.setColor(QPalette::Text, Ui::DesignSystem::color().onTextEditor());
    palette.setColor(QPalette::Highlight, Ui::DesignSystem::color().secondary());
    palette.setColor(QPalette::HighlightedText, Ui::DesignSystem::color().onSecondary());
    d->scalableWrapper->setPalette(palette);
    d->screenplayText->setPalette(palette);
    palette.setColor(QPalette::Base, Qt::transparent);
    d->screenplayText->viewport()->setPalette(palette);
    d->screenplayText->completer()->setTextColor(Ui::DesignSystem::color().onBackground());
    d->screenplayText->completer()->setBackgroundColor(Ui::DesignSystem::color().background());

    d->splitter->setBackgroundColor(Ui::DesignSystem::color().background());

    d->sidebarTabs->setTextColor(Ui::DesignSystem::color().onPrimary());
    d->sidebarTabs->setBackgroundColor(Ui::DesignSystem::color().primary());
    d->sidebarContent->setBackgroundColor(Ui::DesignSystem::color().primary());
}

} // namespace Ui
