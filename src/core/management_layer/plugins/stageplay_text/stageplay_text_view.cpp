#include "stageplay_text_view.h"

#include "text/stageplay_text_edit.h"
#include "text/stageplay_text_edit_shortcuts_manager.h"
#include "text/stageplay_text_edit_toolbar.h"
#include "text/stageplay_text_fast_format_widget.h"
#include "text/stageplay_text_search_manager.h"

#include <business_layer/document/text/text_block_data.h>
#include <business_layer/document/text/text_cursor.h>
#include <business_layer/model/stageplay/stageplay_information_model.h>
#include <business_layer/model/stageplay/text/stageplay_text_model.h>
#include <business_layer/templates/stageplay_template.h>
#include <business_layer/templates/templates_facade.h>
#include <data_layer/storage/settings_storage.h>
#include <data_layer/storage/storage_facade.h>
#include <ui/design_system/design_system.h>
#include <ui/modules/bookmarks/bookmarks_model.h>
#include <ui/modules/bookmarks/bookmarks_view.h>
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
const int kBookmarksTabIndex = 2;

const QString kSettingsKey = "stageplay-text";
const QString kScaleFactorKey = kSettingsKey + "/scale-factor";
const QString kSidebarStateKey = kSettingsKey + "/sidebar-state";
const QString kIsFastFormatPanelVisibleKey = kSettingsKey + "/is-fast-format-panel-visible";
const QString kIsCommentsModeEnabledKey = kSettingsKey + "/is-comments-mode-enabled";
const QString kIsBookmarksListVisibleKey = kSettingsKey + "/is-bookmarks-list-visible";
const QString kSidebarPanelIndexKey = kSettingsKey + "/sidebar-panel-index";
} // namespace

class StageplayTextView::Implementation
{
public:
    explicit Implementation(QWidget* _parent);

    /**
     * @brief Переконфигурировать представление
     */
    void reconfigureTemplate(bool _withModelReinitialization = true);
    void reconfigureBlockNumbersVisibility();

    /**
     * @brief Обновить переводы дополнительных действий
     */
    void updateOptionsTranslations();

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


    //
    // Модели
    //
    QPointer<BusinessLayer::StageplayTextModel> model;
    BusinessLayer::CommentsModel* commentsModel = nullptr;
    BusinessLayer::BookmarksModel* bookmarksModel = nullptr;

    //
    // Редактор текста
    //
    StageplayTextEdit* stageplayText = nullptr;
    StageplayTextEditShortcutsManager shortcutsManager;
    ScalableWrapper* scalableWrapper = nullptr;

    //
    // Панели инструментов
    //
    StageplayTextEditToolbar* toolbar = nullptr;
    BusinessLayer::StageplayTextSearchManager* searchManager = nullptr;
    FloatingToolbarAnimator* toolbarAnimation = nullptr;
    BusinessLayer::TextParagraphType currentParagraphType
        = BusinessLayer::TextParagraphType::Undefined;
    QStandardItemModel* paragraphTypesModel = nullptr;
    //
    CommentsToolbar* commentsToolbar = nullptr;

    //
    // Сайдбар
    //
    Shadow* sidebarShadow = nullptr;
    //
    Widget* sidebarWidget = nullptr;
    TabBar* sidebarTabs = nullptr;
    StackWidget* sidebarContent = nullptr;
    StageplayTextFastFormatWidget* fastFormatWidget = nullptr;
    CommentsView* commentsView = nullptr;
    BookmarksView* bookmarksView = nullptr;
    //
    Splitter* splitter = nullptr;

    //
    // Действия опций редактора
    //
    QAction* showBookmarksAction = nullptr;
};

StageplayTextView::Implementation::Implementation(QWidget* _parent)
    : commentsModel(new BusinessLayer::CommentsModel(_parent))
    , bookmarksModel(new BusinessLayer::BookmarksModel(_parent))
    , stageplayText(new StageplayTextEdit(_parent))
    , shortcutsManager(stageplayText)
    , scalableWrapper(new ScalableWrapper(stageplayText, _parent))
    , toolbar(new StageplayTextEditToolbar(scalableWrapper))
    , searchManager(new BusinessLayer::StageplayTextSearchManager(scalableWrapper, stageplayText))
    , toolbarAnimation(new FloatingToolbarAnimator(_parent))
    , paragraphTypesModel(new QStandardItemModel(toolbar))
    , commentsToolbar(new CommentsToolbar(_parent))
    , sidebarShadow(new Shadow(Qt::RightEdge, scalableWrapper))
    , sidebarWidget(new Widget(_parent))
    , sidebarTabs(new TabBar(_parent))
    , sidebarContent(new StackWidget(_parent))
    , fastFormatWidget(new StageplayTextFastFormatWidget(_parent))
    , commentsView(new CommentsView(_parent))
    , bookmarksView(new BookmarksView(_parent))
    , splitter(new Splitter(_parent))
    //
    , showBookmarksAction(new QAction(_parent))

{
    toolbar->setParagraphTypesModel(paragraphTypesModel);

    commentsToolbar->hide();

    stageplayText->setVerticalScrollBar(new ScrollBar);
    stageplayText->setHorizontalScrollBar(new ScrollBar);
    shortcutsManager.setShortcutsContext(scalableWrapper);
    scalableWrapper->setHorizontalScrollBar(new ScrollBar);
    scalableWrapper->setVerticalScrollBar(new ScrollBar);
    scalableWrapper->initScrollBarsSyncing();

    stageplayText->setUsePageMode(true);

    sidebarWidget->hide();
    sidebarTabs->setFixed(false);
    sidebarTabs->addTab({}); // fastformat
    sidebarTabs->setTabVisible(kFastFormatTabIndex, false);
    sidebarTabs->addTab({}); // comments
    sidebarTabs->setTabVisible(kCommentsTabIndex, false);
    sidebarTabs->addTab({}); // bookmarks
    sidebarTabs->setTabVisible(kBookmarksTabIndex, false);
    sidebarContent->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sidebarContent->setAnimationType(StackWidget::AnimationType::Slide);
    sidebarContent->addWidget(fastFormatWidget);
    sidebarContent->addWidget(commentsView);
    sidebarContent->addWidget(bookmarksView);
    fastFormatWidget->hide();
    fastFormatWidget->setParagraphTypesModel(paragraphTypesModel);
    commentsView->setModel(commentsModel);
    commentsView->hide();
    bookmarksView->setModel(bookmarksModel);
    bookmarksView->hide();

    showBookmarksAction->setCheckable(true);
    showBookmarksAction->setIconText(u8"\U000F0E16");
}

void StageplayTextView::Implementation::reconfigureTemplate(bool _withModelReinitialization)
{
    paragraphTypesModel->clear();

    using namespace BusinessLayer;
    const auto& usedTemplate = BusinessLayer::TemplatesFacade::stageplayTemplate(
        model && model->informationModel() ? model->informationModel()->templateId() : "");
    const QVector<TextParagraphType> types = {
        TextParagraphType::SceneHeading,    TextParagraphType::Character,
        TextParagraphType::Parenthetical,   TextParagraphType::Dialogue,
        TextParagraphType::Action,          TextParagraphType::InlineNote,
        TextParagraphType::UnformattedText,
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
        stageplayText->reinit();
    }
}

void StageplayTextView::Implementation::reconfigureBlockNumbersVisibility()
{
    if (model && model->informationModel()) {
        stageplayText->setShowBlockNumbers(model->informationModel()->showBlockNumbers(),
                                           model->informationModel()->continueBlockNumbers());
    } else {
        stageplayText->setShowBlockNumbers(
            settingsValue(DataStorageLayer::kComponentsStageplayEditorShowBlockNumbersKey).toBool(),
            settingsValue(DataStorageLayer::kComponentsStageplayEditorContinueBlockNumbersKey)
                .toBool());
    }
}

void StageplayTextView::Implementation::updateOptionsTranslations()
{
    showBookmarksAction->setText(showBookmarksAction->isChecked() ? tr("Hide bookmarks list")
                                                                  : tr("Show bookmarks list"));
}

void StageplayTextView::Implementation::updateToolBarUi()
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

void StageplayTextView::Implementation::updateToolBarCurrentParagraphTypeName()
{
    auto paragraphType = stageplayText->currentParagraphType();
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

void StageplayTextView::Implementation::updateTextEditPageMargins()
{
    if (stageplayText->usePageMode()) {
        return;
    }

    const QMarginsF pageMargins
        = QMarginsF{ 15, 20 / scalableWrapper->zoomRange(),
                     12 / scalableWrapper->zoomRange()
                         + MeasurementHelper::pxToMm(scalableWrapper->verticalScrollBar()->width()),
                     5 };
    stageplayText->setPageMarginsMm(pageMargins);
}

void StageplayTextView::Implementation::updateCommentsToolBar()
{
    if (!toolbar->isCommentsModeEnabled() || !stageplayText->textCursor().hasSelection()) {
        commentsToolbar->hideToolbar();
        return;
    }

    //
    // Определяем точку на границе страницы, либо если страница не влезает в экран, то с боку экрана
    //
    const int x = (stageplayText->width() - stageplayText->viewport()->width()) / 2
        + stageplayText->viewport()->width() - commentsToolbar->width();
    const qreal textRight = scalableWrapper->mapFromEditor(QPoint(x, 0)).x();
    const auto cursorRect = stageplayText->cursorRect();
    const auto globalCursorCenter = stageplayText->mapToGlobal(cursorRect.center());
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

void StageplayTextView::Implementation::updateSideBarVisibility(QWidget* _container)
{
    const bool isSidebarShouldBeVisible = toolbar->isFastFormatPanelVisible()
        || toolbar->isCommentsModeEnabled() || showBookmarksAction->isChecked();
    if (sidebarWidget->isVisible() == isSidebarShouldBeVisible) {
        return;
    }

    sidebarShadow->setVisible(isSidebarShouldBeVisible);
    sidebarWidget->setVisible(isSidebarShouldBeVisible);

    if (isSidebarShouldBeVisible && splitter->sizes().constLast() == 0) {
        const auto sideBarWidth = sidebarContent->sizeHint().width();
        splitter->setSizes({ _container->width() - sideBarWidth, sideBarWidth });
    }
}

void StageplayTextView::Implementation::addReviewMark(const QColor& _textColor,
                                                      const QColor& _backgroundColor,
                                                      const QString& _comment)
{
    //
    // Добавим заметку
    //
    const auto textColor
        = _textColor.isValid() ? _textColor : ColorHelper::contrasted(_backgroundColor);
    stageplayText->addReviewMark(textColor, _backgroundColor, _comment);

    //
    // Снимем выделение, чтобы пользователь получил обратную связь от приложения, что выделение
    // добавлено
    //
    BusinessLayer::TextCursor cursor(stageplayText->textCursor());
    const auto selectionInterval = cursor.selectionInterval();
    //
    // ... делаем танец с бубном, чтобы получить сигнал об обновлении позиции курсора
    //     и выделить новую заметку в общем списке
    //
    cursor.setPosition(selectionInterval.to);
    stageplayText->setTextCursorReimpl(cursor);
    cursor.setPosition(selectionInterval.from);
    stageplayText->setTextCursorReimpl(cursor);

    //
    // Фокусируем редактор сценария, чтобы пользователь мог продолжать работать с ним
    //
    scalableWrapper->setFocus();
}


// ****


StageplayTextView::StageplayTextView(QWidget* _parent)
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

    connect(d->toolbar, &StageplayTextEditToolbar::undoPressed, d->stageplayText,
            &StageplayTextEdit::undo);
    connect(d->toolbar, &StageplayTextEditToolbar::redoPressed, d->stageplayText,
            &StageplayTextEdit::redo);
    connect(d->toolbar, &StageplayTextEditToolbar::paragraphTypeChanged, this,
            [this](const QModelIndex& _index) {
                const auto type = static_cast<BusinessLayer::TextParagraphType>(
                    _index.data(kTypeDataRole).toInt());
                d->stageplayText->setCurrentParagraphType(type);
                d->scalableWrapper->setFocus();
            });
    connect(d->toolbar, &StageplayTextEditToolbar::fastFormatPanelVisibleChanged, this,
            [this](bool _visible) {
                d->sidebarTabs->setTabVisible(kFastFormatTabIndex, _visible);
                d->fastFormatWidget->setVisible(_visible);
                if (_visible) {
                    d->sidebarTabs->setCurrentTab(kFastFormatTabIndex);
                    d->sidebarContent->setCurrentWidget(d->fastFormatWidget);
                }
                d->updateSideBarVisibility(this);
            });
    connect(d->toolbar, &StageplayTextEditToolbar::commentsModeEnabledChanged, this,
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
    connect(d->toolbar, &StageplayTextEditToolbar::searchPressed, this, [this] {
        d->toolbarAnimation->switchToolbars(d->toolbar->searchIcon(),
                                            d->toolbar->searchIconPosition(), d->toolbar,
                                            d->searchManager->toolbar());
    });
    //
    connect(d->searchManager, &BusinessLayer::StageplayTextSearchManager::hideToolbarRequested,
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
                const auto position = d->stageplayText->positionForModelIndex(positionHint.index)
                    + positionHint.blockPosition;
                auto cursor = d->stageplayText->textCursor();
                cursor.setPosition(position);
                d->stageplayText->ensureCursorVisible(cursor);
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
    connect(d->bookmarksView, &BookmarksView::addBookmarkRequested, this,
            &StageplayTextView::createBookmarkRequested);
    connect(d->bookmarksView, &BookmarksView::changeBookmarkRequested, this,
            [this](const QModelIndex& _index, const QString& _text, const QColor& _color) {
                emit changeBookmarkRequested(d->bookmarksModel->mapToModel(_index), _text, _color);
            });
    connect(d->bookmarksView, &BookmarksView::bookmarkSelected, this,
            [this](const QModelIndex& _index) {
                const auto index = d->bookmarksModel->mapToModel(_index);
                const auto position = d->stageplayText->positionForModelIndex(index);
                auto cursor = d->stageplayText->textCursor();
                cursor.setPosition(position);
                d->stageplayText->ensureCursorVisible(cursor);
                d->scalableWrapper->setFocus();
            });
    connect(d->bookmarksView, &BookmarksView::removeRequested, this,
            [this](const QModelIndexList& _indexes) {
                QSignalBlocker blocker(d->commentsView);
                d->bookmarksModel->remove(_indexes);
            });
    //
    connect(d->sidebarTabs, &TabBar::currentIndexChanged, this, [this](int _currentIndex) {
        switch (_currentIndex) {
        case kFastFormatTabIndex: {
            d->sidebarContent->setCurrentWidget(d->fastFormatWidget);
            break;
        }
        case kCommentsTabIndex: {
            d->sidebarContent->setCurrentWidget(d->commentsView);
            break;
        }

        case kBookmarksTabIndex: {
            d->sidebarContent->setCurrentWidget(d->bookmarksView);
            break;
        }
        }
    });
    //
    connect(d->fastFormatWidget, &StageplayTextFastFormatWidget::paragraphTypeChanged, this,
            [this](const QModelIndex& _index) {
                const auto type = static_cast<BusinessLayer::TextParagraphType>(
                    _index.data(kTypeDataRole).toInt());
                d->stageplayText->setCurrentParagraphType(type);
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
        const auto stageplayModelIndex = d->stageplayText->currentModelIndex();
        emit currentModelIndexChanged(stageplayModelIndex);
        //
        // Если необходимо выберем соответствующий комментарий
        //
        const auto positionInBlock = d->stageplayText->textCursor().positionInBlock();
        const auto commentModelIndex
            = d->commentsModel->mapFromModel(stageplayModelIndex, positionInBlock);
        d->commentsView->setCurrentIndex(commentModelIndex);
        //
        // Выберем закладку, если курсор в блоке с закладкой
        //
        const auto bookmarkModelIndex = d->bookmarksModel->mapFromModel(stageplayModelIndex);
        d->bookmarksView->setCurrentIndex(bookmarkModelIndex);
    };
    connect(d->stageplayText, &StageplayTextEdit::paragraphTypeChanged, this,
            handleCursorPositionChanged);
    connect(d->stageplayText, &StageplayTextEdit::cursorPositionChanged, this,
            handleCursorPositionChanged);
    connect(d->stageplayText, &StageplayTextEdit::selectionChanged, this,
            [this] { d->updateCommentsToolBar(); });
    connect(d->stageplayText, &StageplayTextEdit::addBookmarkRequested, this, [this] {
        //
        // Если список закладок показан, добавляем новую через него
        //
        if (d->showBookmarksAction->isChecked()) {
            d->bookmarksView->showAddBookmarkView({});
        }
        //
        // В противном случае, через диалог
        //
        else {
            emit addBookmarkRequested();
        }
    });
    connect(d->stageplayText, &StageplayTextEdit::editBookmarkRequested, this, [this] {
        //
        // Если список закладок показан, редактируем через него
        //
        if (d->showBookmarksAction->isChecked()) {
            d->bookmarksView->showAddBookmarkView(
                d->bookmarksModel->mapFromModel(currentModelIndex()));
        }
        //
        // В противном случае, через диалог
        //
        else {
            emit addBookmarkRequested();
        }
    });
    connect(d->stageplayText, &StageplayTextEdit::removeBookmarkRequested, this,
            &StageplayTextView::removeBookmarkRequested);
    connect(d->stageplayText, &StageplayTextEdit::showBookmarksRequested, d->showBookmarksAction,
            &QAction::toggle);
    //
    connect(d->showBookmarksAction, &QAction::toggled, this, [this](bool _checked) {
        d->updateOptionsTranslations();
        d->sidebarTabs->setTabVisible(kBookmarksTabIndex, _checked);
        d->bookmarksView->setVisible(_checked);
        if (_checked) {
            d->sidebarTabs->setCurrentTab(kBookmarksTabIndex);
            d->sidebarContent->setCurrentWidget(d->bookmarksView);
        }
        d->updateSideBarVisibility(this);
    });

    updateTranslations();
    designSystemChangeEvent(nullptr);

    reconfigure({});
}

StageplayTextView::~StageplayTextView() = default;

QWidget* StageplayTextView::asQWidget()
{
    return this;
}

void StageplayTextView::toggleFullScreen(bool _isFullScreen)
{
    d->toolbar->setVisible(!_isFullScreen);
}

QVector<QAction*> StageplayTextView::options() const
{
    return {
        d->showBookmarksAction,
    };
}

void StageplayTextView::reconfigure(const QStringList& _changedSettingsKeys)
{
    UiHelper::initSpellingFor(d->stageplayText);

    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(
            DataStorageLayer::kComponentsStageplayEditorDefaultTemplateKey)) {
        d->reconfigureTemplate();
    }

    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(
            DataStorageLayer::kComponentsStageplayEditorShowBlockNumbersKey)
        || _changedSettingsKeys.contains(
            DataStorageLayer::kComponentsStageplayEditorContinueBlockNumbersKey)) {
        d->reconfigureBlockNumbersVisibility();
    }
    if (_changedSettingsKeys.isEmpty()) {
        d->stageplayText->setCorrectionOptions(true);
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(
            DataStorageLayer::kComponentsStageplayEditorShortcutsKey)) {
        d->shortcutsManager.reconfigure();
    }

    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(DataStorageLayer::kApplicationShowDocumentsPagesKey)) {
        const auto usePageMode
            = settingsValue(DataStorageLayer::kApplicationShowDocumentsPagesKey).toBool();
        d->stageplayText->setUsePageMode(usePageMode);
        if (usePageMode) {
            d->stageplayText->reinit();
        } else {
            d->updateTextEditPageMargins();
        }
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(DataStorageLayer::kApplicationHighlightCurrentLineKey)) {
        d->stageplayText->setHighlightCurrentLine(
            settingsValue(DataStorageLayer::kApplicationHighlightCurrentLineKey).toBool());
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(DataStorageLayer::kApplicationFocusCurrentParagraphKey)) {
        d->stageplayText->setFocusCurrentParagraph(
            settingsValue(DataStorageLayer::kApplicationFocusCurrentParagraphKey).toBool());
    }
    if (_changedSettingsKeys.isEmpty()
        || _changedSettingsKeys.contains(DataStorageLayer::kApplicationUseTypewriterScrollingKey)) {
        d->stageplayText->setUseTypewriterScrolling(
            settingsValue(DataStorageLayer::kApplicationUseTypewriterScrollingKey).toBool());
    }
}

void StageplayTextView::loadViewSettings()
{
    using namespace DataStorageLayer;

    const auto scaleFactor = settingsValue(kScaleFactorKey, 1.0).toReal();
    d->scalableWrapper->setZoomRange(scaleFactor);

    const auto isBookmarksListVisible = settingsValue(kIsBookmarksListVisibleKey, false).toBool();
    d->showBookmarksAction->setChecked(isBookmarksListVisible);
    const auto isCommentsModeEnabled = settingsValue(kIsCommentsModeEnabledKey, false).toBool();
    d->toolbar->setCommentsModeEnabled(isCommentsModeEnabled);
    const auto isFastFormatPanelVisible
        = settingsValue(kIsFastFormatPanelVisibleKey, false).toBool();
    d->toolbar->setFastFormatPanelVisible(isFastFormatPanelVisible);
    const auto sidebarPanelIndex = settingsValue(kSidebarPanelIndexKey, 0).toInt();
    d->sidebarTabs->setCurrentTab(sidebarPanelIndex);

    const auto sidebarState = settingsValue(kSidebarStateKey);
    if (sidebarState.isValid()) {
        d->splitter->restoreState(sidebarState.toByteArray());
    }
}

void StageplayTextView::saveViewSettings()
{
    setSettingsValue(kScaleFactorKey, d->scalableWrapper->zoomRange());

    setSettingsValue(kIsFastFormatPanelVisibleKey, d->toolbar->isFastFormatPanelVisible());
    setSettingsValue(kIsCommentsModeEnabledKey, d->toolbar->isCommentsModeEnabled());
    setSettingsValue(kIsBookmarksListVisibleKey, d->showBookmarksAction->isChecked());
    setSettingsValue(kSidebarPanelIndexKey, d->sidebarTabs->currentTab());

    setSettingsValue(kSidebarStateKey, d->splitter->saveState());
}

void StageplayTextView::setModel(BusinessLayer::StageplayTextModel* _model)
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
        d->reconfigureBlockNumbersVisibility();

        connect(d->model->informationModel(),
                &BusinessLayer::StageplayInformationModel::templateIdChanged, this,
                [this] { d->reconfigureTemplate(); });
        connect(d->model->informationModel(),
                &BusinessLayer::StageplayInformationModel::showBlockNumbersChanged, this,
                [this] { d->reconfigureBlockNumbersVisibility(); });
        connect(d->model->informationModel(),
                &BusinessLayer::StageplayInformationModel::continueBlockNumbersChanged, this,
                [this] { d->reconfigureBlockNumbersVisibility(); });
    }

    d->stageplayText->initWithModel(d->model);
    d->commentsModel->setTextModel(d->model);
    d->bookmarksModel->setTextModel(d->model);

    d->updateToolBarCurrentParagraphTypeName();
}

QModelIndex StageplayTextView::currentModelIndex() const
{
    return d->stageplayText->currentModelIndex();
}

void StageplayTextView::setCurrentModelIndex(const QModelIndex& _index)
{
    d->stageplayText->setCurrentModelIndex(_index);
}

int StageplayTextView::cursorPosition() const
{
    return d->stageplayText->textCursor().position();
}

void StageplayTextView::setCursorPosition(int _position)
{
    auto cursor = d->stageplayText->textCursor();
    cursor.setPosition(_position);
    d->stageplayText->ensureCursorVisible(cursor, false);
}

int StageplayTextView::verticalScroll() const
{
    return d->stageplayText->verticalScrollBar()->value();
}

void StageplayTextView::setverticalScroll(int _value)
{
    d->stageplayText->verticalScrollBar()->setValue(_value);
}

bool StageplayTextView::eventFilter(QObject* _target, QEvent* _event)
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

void StageplayTextView::resizeEvent(QResizeEvent* _event)
{
    Widget::resizeEvent(_event);

    const auto toolbarPosition
        = QPointF(Ui::DesignSystem::layout().px24(), Ui::DesignSystem::layout().px24()).toPoint();
    d->toolbar->move(toolbarPosition);
    d->searchManager->toolbar()->move(toolbarPosition);
    d->updateCommentsToolBar();
}

void StageplayTextView::updateTranslations()
{
    d->sidebarTabs->setTabName(kFastFormatTabIndex, tr("Formatting"));
    d->sidebarTabs->setTabName(kCommentsTabIndex, tr("Comments"));
    d->sidebarTabs->setTabName(kBookmarksTabIndex, tr("Bookmarks"));

    d->updateOptionsTranslations();
}

void StageplayTextView::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    Widget::designSystemChangeEvent(_event);

    setBackgroundColor(Ui::DesignSystem::color().surface());

    d->updateToolBarUi();

    d->stageplayText->setPageSpacing(Ui::DesignSystem::layout().px24());
    QPalette palette;
    palette.setColor(QPalette::Window, Ui::DesignSystem::color().surface());
    palette.setColor(QPalette::Base, Ui::DesignSystem::color().textEditor());
    palette.setColor(QPalette::Text, Ui::DesignSystem::color().onTextEditor());
    palette.setColor(QPalette::Highlight, Ui::DesignSystem::color().secondary());
    palette.setColor(QPalette::HighlightedText, Ui::DesignSystem::color().onSecondary());
    d->scalableWrapper->setPalette(palette);
    d->stageplayText->setPalette(palette);
    palette.setColor(QPalette::Base, Qt::transparent);
    d->stageplayText->viewport()->setPalette(palette);
    d->stageplayText->completer()->setTextColor(Ui::DesignSystem::color().onBackground());
    d->stageplayText->completer()->setBackgroundColor(Ui::DesignSystem::color().background());

    d->splitter->setBackgroundColor(Ui::DesignSystem::color().background());

    d->sidebarTabs->setTextColor(Ui::DesignSystem::color().onPrimary());
    d->sidebarTabs->setBackgroundColor(Ui::DesignSystem::color().primary());
    d->sidebarContent->setBackgroundColor(Ui::DesignSystem::color().primary());
}

} // namespace Ui