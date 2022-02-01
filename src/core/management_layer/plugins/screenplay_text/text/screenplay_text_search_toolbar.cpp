#include "screenplay_text_search_toolbar.h"

#include <ui/design_system/design_system.h>
#include <ui/widgets/button/button.h>
#include <ui/widgets/card/card_popup.h>
#include <ui/widgets/text_field/text_field.h>

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QStringListModel>


namespace Ui {

class ScreenplayTextSearchToolbar::Implementation
{
public:
    explicit Implementation(QWidget* _parent);

    /**
     * @brief Показать попап
     */
    void showPopup(ScreenplayTextSearchToolbar* _parent);


    QAction* closeAction = nullptr;

    QAction* searchTextAction = nullptr;
    TextField* searchText = nullptr;
    QString lastSearchText; // чтобы защититься от флуда при активации окна
    QAction* goToNextAction = nullptr;
    QAction* goToPreviousAction = nullptr;
    QAction* matchCaseAction = nullptr;
    QAction* searchInAction = nullptr;

    CardPopup* popup = nullptr;

    QAction* replaceTextAction = nullptr;
    TextField* replaceText = nullptr;
    QAction* replaceAction = nullptr;
    Button* replace = nullptr;
    QAction* replaceAllAction = nullptr;
    Button* replaceAll = nullptr;
};

ScreenplayTextSearchToolbar::Implementation::Implementation(QWidget* _parent)
    : closeAction(new QAction)
    , searchTextAction(new QAction)
    , searchText(new TextField(_parent))
    , goToNextAction(new QAction)
    , goToPreviousAction(new QAction)
    , matchCaseAction(new QAction)
    , searchInAction(new QAction)
    , popup(new CardPopup(_parent))
    , replaceTextAction(new QAction)
    , replaceText(new TextField(_parent))
    , replaceAction(new QAction)
    , replace(new Button(_parent))
    , replaceAllAction(new QAction)
    , replaceAll(new Button(_parent))
{
    searchText->setSpellCheckPolicy(SpellCheckPolicy::Manual);
    searchText->setUnderlineDecorationVisible(false);

    replaceText->setSpellCheckPolicy(SpellCheckPolicy::Manual);
    replaceText->setUnderlineDecorationVisible(false);

    replace->setFocusPolicy(Qt::NoFocus);
    replaceAll->setFocusPolicy(Qt::NoFocus);
}

void ScreenplayTextSearchToolbar::Implementation::showPopup(ScreenplayTextSearchToolbar* _parent)
{
    const auto width = Ui::DesignSystem::floatingToolBar().spacing() * 2
        + _parent->actionCustomWidth(searchInAction);

    const auto left = QPoint(
        Ui::DesignSystem::floatingToolBar().shadowMargins().left()
            + Ui::DesignSystem::floatingToolBar().margins().left() + searchText->width()
            + Ui::DesignSystem::floatingToolBar().spacing()
            + (Ui::DesignSystem::floatingToolBar().iconSize().width()
               + Ui::DesignSystem::floatingToolBar().spacing())
                * 4
            - Ui::DesignSystem::floatingToolBar().spacing()
            - Ui::DesignSystem::card().shadowMargins().left(),
        _parent->rect().bottom() - Ui::DesignSystem::floatingToolBar().shadowMargins().bottom());
    const auto position = _parent->mapToGlobal(left)
        + QPointF(Ui::DesignSystem::textField().margins().left(),
                  -Ui::DesignSystem::textField().margins().bottom());

    popup->showPopup(position.toPoint(), width, popup->contentModel()->rowCount());
}


// ****


ScreenplayTextSearchToolbar::ScreenplayTextSearchToolbar(QWidget* _parent)
    : FloatingToolBar(_parent)
    , d(new Implementation(this))
{
    _parent->installEventFilter(this);
    d->searchText->installEventFilter(this);
    d->replaceText->installEventFilter(this);
    setFocusProxy(d->searchText);

    d->closeAction->setIconText(u8"\U000f004d");
    d->closeAction->setShortcut(QKeySequence::Find);
    addAction(d->closeAction);
    connect(d->closeAction, &QAction::triggered, this, [this] {
        if (d->searchText->hasFocus()) {
            emit closePressed();
        } else {
            d->searchText->setFocus();
            d->searchText->selectAll();
        }
    });
    //
    addAction(d->searchTextAction);
    connect(d->searchText, &TextField::textChanged, this, [this] {
        if (d->lastSearchText == d->searchText->text()) {
            return;
        }

        d->lastSearchText = d->searchText->text();
        emit findTextRequested();
    });
    //
    d->goToPreviousAction->setIconText(u8"\U000f0143");
    d->goToPreviousAction->setShortcut(QKeySequence::FindPrevious);
    addAction(d->goToPreviousAction);
    connect(d->goToPreviousAction, &QAction::triggered, this,
            &ScreenplayTextSearchToolbar::findPreviousRequested);
    d->goToNextAction->setIconText(u8"\U000f0140");
    d->goToNextAction->setShortcut(QKeySequence::FindNext);
    addAction(d->goToNextAction);
    connect(d->goToNextAction, &QAction::triggered, this,
            &ScreenplayTextSearchToolbar::findNextRequested);
    d->matchCaseAction->setIconText(u8"\U000f0b34");
    d->matchCaseAction->setCheckable(true);
    addAction(d->matchCaseAction);
    connect(d->matchCaseAction, &QAction::toggled, this, [this] {
        d->matchCaseAction->setToolTip(d->matchCaseAction->isChecked()
                                           ? tr("Search without case sensitive")
                                           : tr("Search with case sensitive"));
    });
    connect(d->matchCaseAction, &QAction::toggled, this,
            &ScreenplayTextSearchToolbar::findTextRequested);
    //
    d->searchInAction->setText(tr("In the whole text"));
    d->searchInAction->setIconText(u8"\U000f035d");
    auto _model = new QStringListModel(d->popup);
    d->popup->setContentModel(_model);
    connect(_model, &QAbstractItemModel::rowsInserted, this,
            [this] { designSystemChangeEvent(nullptr); });
    connect(d->popup, &CardPopup::currentIndexChanged, this, [this](const QModelIndex& _index) {
        d->searchInAction->setText(_index.data().toString());
        update();

        emit findTextRequested();
    });
    connect(d->popup, &Card::disappeared, this, [this] {
        d->searchInAction->setIconText(u8"\U000f035d");
        update();
    });
    //
    addAction(d->searchInAction);
    connect(d->searchInAction, &QAction::triggered, this, [this] {
        d->searchInAction->setIconText(u8"\U000f0360");
        d->showPopup(this);
    });

    addAction(d->replaceTextAction);
    addAction(d->replaceAction);
    connect(d->replace, &Button::clicked, this, &ScreenplayTextSearchToolbar::replaceOnePressed);
    addAction(d->replaceAllAction);
    connect(d->replaceAll, &Button::clicked, this, &ScreenplayTextSearchToolbar::replaceAllPressed);

    updateTranslations();
    designSystemChangeEvent(nullptr);
}

ScreenplayTextSearchToolbar::~ScreenplayTextSearchToolbar() = default;

void ScreenplayTextSearchToolbar::refocus()
{
    d->searchText->setFocus();
}

QString ScreenplayTextSearchToolbar::searchText() const
{
    return d->lastSearchText;
}

bool ScreenplayTextSearchToolbar::isCaseSensitive() const
{
    return d->matchCaseAction->isChecked();
}

int ScreenplayTextSearchToolbar::searchInType() const
{
    return d->popup->currentIndex().row();
}

QString ScreenplayTextSearchToolbar::replaceText() const
{
    return d->replaceText->text();
}

bool ScreenplayTextSearchToolbar::eventFilter(QObject* _watched, QEvent* _event)
{
    switch (_event->type()) {
    case QEvent::Resize: {
        if (_watched == parent()) {
            designSystemChangeEvent(nullptr);
        }
        break;
    }

    case QEvent::KeyPress: {
        if (_watched == d->searchText) {
            const auto keyEvent = static_cast<QKeyEvent*>(_event);
            if ((keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
                && !d->searchText->text().isEmpty()) {
                emit findTextRequested();
            }
        }
        break;
    }

    case QEvent::KeyRelease: {
        if (_watched == d->searchText) {
            const auto keyEvent = static_cast<QKeyEvent*>(_event);
            if (keyEvent->key() == Qt::Key_Escape) {
                emit focusTextRequested();
            }
        }
        break;
    }

    default:
        break;
    }

    return FloatingToolBar::eventFilter(_watched, _event);
}

void ScreenplayTextSearchToolbar::processBackgroundColorChange()
{
    d->searchText->setBackgroundColor(backgroundColor());
    d->replaceText->setBackgroundColor(backgroundColor());
    d->replace->setBackgroundColor(Ui::DesignSystem::color().secondary());
    d->replaceAll->setBackgroundColor(Ui::DesignSystem::color().secondary());
}

void ScreenplayTextSearchToolbar::processTextColorChange()
{
    d->searchText->setTextColor(textColor());
    d->replaceText->setTextColor(textColor());
    d->replace->setTextColor(Ui::DesignSystem::color().secondary());
    d->replaceAll->setTextColor(Ui::DesignSystem::color().secondary());
}

void ScreenplayTextSearchToolbar::updateTranslations()
{
    d->closeAction->setToolTip(
        tr("Exit from search")
        + QString(" (%1)").arg(
            QKeySequence(QKeySequence::Find).toString(QKeySequence::NativeText)));
    d->searchText->setLabel(tr("Search"));
    d->searchText->setPlaceholderText(tr("Enter search phrase here"));
    if (auto model = qobject_cast<QStringListModel*>(d->popup->contentModel())) {
        model->setStringList({ tr("In the whole text"), tr("In scene heading"), tr("In action"),
                               tr("In character"), tr("In dialogue") });
        d->popup->setCurrentIndex(model->index(0, 0));
    }
    d->goToNextAction->setToolTip(
        tr("Go to the next search result")
        + QString(" (%1)").arg(
            QKeySequence(QKeySequence::FindNext).toString(QKeySequence::NativeText)));
    d->goToPreviousAction->setToolTip(
        tr("Go to the previous search result")
        + QString(" (%1)").arg(
            QKeySequence(QKeySequence::FindPrevious).toString(QKeySequence::NativeText)));
    d->matchCaseAction->setToolTip(d->matchCaseAction->isChecked()
                                       ? tr("Search without case sensitive")
                                       : tr("Search with case sensitive"));

    d->replaceText->setLabel(tr("Replace with"));
    d->replaceText->setPlaceholderText(tr("Enter phrase to replace"));
    d->replace->setText(tr("Replace"));
    d->replaceAll->setText(tr("All"));
}

void ScreenplayTextSearchToolbar::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    FloatingToolBar::designSystemChangeEvent(_event);

    //
    // Рассчитываем размер полей поиска и замены
    //
    const auto searchInActionWidth = Ui::DesignSystem::treeOneLineItem().margins().left()
        + d->popup->sizeHintForColumn(0) + Ui::DesignSystem::treeOneLineItem().margins().right();
    d->replace->resize(d->replace->sizeHint());
    const auto replaceActionWidth
        = d->replace->sizeHint().width() - Ui::DesignSystem::floatingToolBar().spacing();
    d->replaceAll->resize(d->replaceAll->sizeHint());
    const auto replaceAllActionWidth
        = d->replaceAll->sizeHint().width() - Ui::DesignSystem::floatingToolBar().spacing();
    auto textFieldWidth = parentWidget()->width() * 0.8;
    textFieldWidth -= (Ui::DesignSystem::floatingToolBar().iconSize().width()
                       + Ui::DesignSystem::floatingToolBar().spacing())
            * 4 // 4 обычных кнопки
        + searchInActionWidth // выпадающий список мест поиска
        + replaceActionWidth // кнопка единичной замены
        + replaceAllActionWidth; // кнопка полной замены
    textFieldWidth /= 2; // делим поровну
    if (textFieldWidth < 0) {
        return;
    }

    //
    // Расставляем элементы в панели
    //
    setActionCustomWidth(d->searchTextAction, textFieldWidth);
    d->searchText->setFixedWidth(textFieldWidth);
    const auto searchLeft = Ui::DesignSystem::floatingToolBar().shadowMargins().left()
        + Ui::DesignSystem::floatingToolBar().iconSize().width()
        + Ui::DesignSystem::floatingToolBar().spacing();
    d->searchText->move(searchLeft, Ui::DesignSystem::floatingToolBar().shadowMargins().top());
    //
    setActionCustomWidth(d->searchInAction, static_cast<int>(searchInActionWidth));
    //
    d->popup->setBackgroundColor(Ui::DesignSystem::color().primary());


    const auto replaceLeft = searchLeft + d->searchText->width()
        + Ui::DesignSystem::floatingToolBar().spacing()
        + (Ui::DesignSystem::floatingToolBar().iconSize().width()
           + Ui::DesignSystem::floatingToolBar().spacing())
            * 3
        + actionCustomWidth(d->searchInAction) + Ui::DesignSystem::floatingToolBar().spacing();

    setActionCustomWidth(d->replaceTextAction, textFieldWidth);
    d->replaceText->setFixedWidth(textFieldWidth);
    d->replaceText->move(replaceLeft, Ui::DesignSystem::floatingToolBar().shadowMargins().top());
    //
    setActionCustomWidth(d->replaceAction, replaceActionWidth);
    d->replace->move(d->replaceText->geometry().right()
                         + Ui::DesignSystem::floatingToolBar().spacing(),
                     Ui::DesignSystem::floatingToolBar().shadowMargins().top()
                         + Ui::DesignSystem::layout().px8());
    setActionCustomWidth(d->replaceAllAction, replaceAllActionWidth);
    d->replaceAll->move(d->replace->geometry().right(), d->replace->geometry().top());

    resize(sizeHint());
}

} // namespace Ui
