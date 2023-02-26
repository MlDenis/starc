#include "novel_outline_structure_view.h"

#include "novel_outline_structure_delegate.h"

#include <data_layer/storage/settings_storage.h>
#include <data_layer/storage/storage_facade.h>
#include <interfaces/management_layer/i_document_manager.h>
#include <ui/design_system/design_system.h>
#include <ui/widgets/label/label.h>
#include <ui/widgets/scroll_bar/scroll_bar.h>
#include <ui/widgets/shadow/shadow.h>
#include <ui/widgets/tree/tree.h>

#include <QStringListModel>
#include <QVBoxLayout>


namespace Ui {

class NovelOutlineStructureView::Implementation
{
public:
    explicit Implementation(QWidget* _parent);


    IconsMidLabel* backIcon = nullptr;
    Subtitle2Label* backText = nullptr;
    Tree* content = nullptr;
    NovelOutlineStructureDelegate* contentDelegate = nullptr;
};

NovelOutlineStructureView::Implementation::Implementation(QWidget* _parent)
    : backIcon(new IconsMidLabel(_parent))
    , backText(new Subtitle2Label(_parent))
    , content(new Tree(_parent))
    , contentDelegate(new NovelOutlineStructureDelegate(content))
{
    backIcon->setIcon(u8"\U000F0141");

    content->setContextMenuPolicy(Qt::CustomContextMenu);
    content->setDragDropEnabled(true);
    content->setSelectionMode(QAbstractItemView::ExtendedSelection);
    content->setItemDelegate(contentDelegate);

    new Shadow(Qt::TopEdge, content);
}


// ****


NovelOutlineStructureView::NovelOutlineStructureView(QWidget* _parent)
    : AbstractNavigator(_parent)
    , d(new Implementation(this))
{
    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->setContentsMargins({});
    topLayout->setSpacing(0);
    topLayout->addWidget(d->backIcon);
    topLayout->addWidget(d->backText, 1);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addLayout(topLayout);
    layout->addWidget(d->content);
    setLayout(layout);

    connect(d->backIcon, &AbstractLabel::clicked, this, &NovelOutlineStructureView::backPressed);
    connect(d->backText, &AbstractLabel::clicked, this, &NovelOutlineStructureView::backPressed);
    connect(d->content, &Tree::clicked, this, &NovelOutlineStructureView::currentModelIndexChanged);
    connect(d->content, &Tree::doubleClicked, this,
            &NovelOutlineStructureView::currentModelIndexChanged);
    connect(d->content, &Tree::customContextMenuRequested, this, [this](const QPoint& _pos) {
        emit customContextMenuRequested(d->content->mapToParent(_pos));
    });

    reconfigure();
}

NovelOutlineStructureView::~NovelOutlineStructureView() = default;

QWidget* NovelOutlineStructureView::asQWidget()
{
    return this;
}

void NovelOutlineStructureView::setEditingMode(ManagementLayer::DocumentEditingMode _mode)
{
    const auto readOnly = _mode != ManagementLayer::DocumentEditingMode::Edit;
    d->content->setContextMenuPolicy(readOnly ? Qt::NoContextMenu : Qt::CustomContextMenu);
    d->content->setDragDropEnabled(!readOnly);
}

void NovelOutlineStructureView::setCurrentModelIndex(const QModelIndex& _index)
{
    d->content->setCurrentIndex(_index);
}

void NovelOutlineStructureView::reconfigure()
{
    const bool showSceneNumber
        = settingsValue(DataStorageLayer::kComponentsScreenplayNavigatorShowSceneNumberKey)
              .toBool();
    d->contentDelegate->showSceneNumber(showSceneNumber);

    //    const bool showSceneText
    //        =
    //        settingsValue(DataStorageLayer::kComponentsScreenplayNavigatorShowSceneTextKey).toBool();
    //    if (showSceneText == false) {
    //        d->contentDelegate->setTextLinesSize(0);
    //    } else {
    //        const int sceneTextLines
    //            = settingsValue(DataStorageLayer::kComponentsScreenplayNavigatorSceneTextLinesKey)
    //                  .toInt();
    //        d->contentDelegate->setTextLinesSize(sceneTextLines);
    //    }

    d->content->setItemDelegate(nullptr);
    d->content->setItemDelegate(d->contentDelegate);
}

void NovelOutlineStructureView::setTitle(const QString& _title)
{
    d->backText->setText(_title);
}

void NovelOutlineStructureView::setModel(QAbstractItemModel* _model)
{
    d->content->setModel(_model);
}

QModelIndexList NovelOutlineStructureView::selectedIndexes() const
{
    return d->content->selectedIndexes();
}

void NovelOutlineStructureView::updateTranslations()
{
}

void NovelOutlineStructureView::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    Widget::designSystemChangeEvent(_event);

    setBackgroundColor(DesignSystem::color().primary());
    auto backTextColor = DesignSystem::color().onPrimary();
    backTextColor.setAlphaF(Ui::DesignSystem::inactiveTextOpacity());
    for (auto widget : QVector<Widget*>{ d->backIcon, d->backText }) {
        widget->setBackgroundColor(DesignSystem::color().primary());
        widget->setTextColor(backTextColor);
    }
    d->content->setBackgroundColor(DesignSystem::color().primary());
    d->content->setTextColor(DesignSystem::color().onPrimary());

    d->backIcon->setContentsMargins(QMarginsF(isLeftToRight() ? Ui::DesignSystem::layout().px12()
                                                              : Ui::DesignSystem::layout().px4(),
                                              Ui::DesignSystem::layout().px8(),
                                              isLeftToRight() ? Ui::DesignSystem::layout().px4()
                                                              : Ui::DesignSystem::layout().px12(),
                                              Ui::DesignSystem::layout().px8())
                                        .toMargins());
    d->backText->setContentsMargins(
        QMarginsF(isLeftToRight() ? 0.0 : Ui::DesignSystem::layout().px16(),
                  Ui::DesignSystem::layout().px12(),
                  isLeftToRight() ? Ui::DesignSystem::layout().px16() : 0.0,
                  Ui::DesignSystem::layout().px12())
            .toMargins());
}

} // namespace Ui