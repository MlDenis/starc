#include "audioplay_text_structure_manager.h"

#include "business_layer/audioplay_text_structure_model.h"
#include "ui/audioplay_text_structure_view.h"

#include <business_layer/model/audioplay/audioplay_information_model.h>
#include <business_layer/model/audioplay/text/audioplay_text_model.h>
#include <business_layer/model/audioplay/text/audioplay_text_model_folder_item.h>
#include <business_layer/model/audioplay/text/audioplay_text_model_scene_item.h>
#include <data_layer/storage/settings_storage.h>
#include <data_layer/storage/storage_facade.h>
#include <ui/design_system/design_system.h>
#include <ui/widgets/color_picker/color_picker.h>
#include <ui/widgets/context_menu/context_menu.h>

#include <QAction>
#include <QTimer>
#include <QWidgetAction>

#include <optional>


namespace ManagementLayer {

class AudioplayTextStructureManager::Implementation
{
public:
    explicit Implementation();

    /**
     * @brief Создать представление
     */
    Ui::AudioplayTextStructureView* createView();

    /**
     * @brief Настроить контекстное меню
     */
    void updateContextMenu(const QModelIndexList& _indexes);


    /**
     * @brief Текущая модель сценария
     */
    QPointer<BusinessLayer::AudioplayTextModel> model;

    /**
     * @brief Индекс модели, который необходимо выделить
     * @note Используется в случаях, когда в навигаторе установлена не та модель, что отображается
     *       в редакторе, когда будет установлена нужная модель, в навигаторе будет выделен элемент
     *       с данным индексом
     */
    QModelIndex modelIndexToSelect;

    /**
     * @brief Модель отображения структуры сценария
     */
    BusinessLayer::AudioplayTextStructureModel* structureModel = nullptr;

    /**
     * @brief Предаставление для основного окна
     */
    Ui::AudioplayTextStructureView* view = nullptr;

    /**
     * @brief Контекстное меню для элементов навигатора
     */
    ContextMenu* contextMenu = nullptr;

    /**
     * @brief Все созданные представления
     */
    QVector<Ui::AudioplayTextStructureView*> allViews;
};

AudioplayTextStructureManager::Implementation::Implementation()
{
    view = createView();
    contextMenu = new ContextMenu(view);
}

Ui::AudioplayTextStructureView* AudioplayTextStructureManager::Implementation::createView()
{
    allViews.append(new Ui::AudioplayTextStructureView);
    return allViews.last();
}

void AudioplayTextStructureManager::Implementation::updateContextMenu(
    const QModelIndexList& _indexes)
{
    if (_indexes.isEmpty()) {
        return;
    }

    //
    // Настроим внешний вид меню
    //
    contextMenu->setBackgroundColor(Ui::DesignSystem::color().background());
    contextMenu->setTextColor(Ui::DesignSystem::color().onBackground());

    //
    // Настроим действия меню
    //
    QVector<QAction*> actions;
    //
    // ... для одного элемента
    //
    if (_indexes.size() == 1) {
        const auto itemIndex = structureModel->mapToSource(_indexes.constFirst());
        std::optional<QColor> itemColor;
        const auto item = model->itemForIndex(itemIndex);
        if (item->type() == BusinessLayer::TextModelItemType::Folder) {
            const auto folderItem = static_cast<BusinessLayer::AudioplayTextModelFolderItem*>(item);
            itemColor = folderItem->color();
        } else if (item->type() == BusinessLayer::TextModelItemType::Group) {
            const auto sceneItem = static_cast<BusinessLayer::AudioplayTextModelSceneItem*>(item);
            itemColor = sceneItem->color();
        }
        if (itemColor.has_value()) {
            auto colorMenu = new QAction;
            colorMenu->setText(tr("Color"));
            actions.append(colorMenu);

            auto colorAction = new QWidgetAction(colorMenu);
            auto colorPicker = new ColorPicker;
            colorAction->setDefaultWidget(colorPicker);
            colorPicker->setColorCanBeDeselected(true);
            colorPicker->setSelectedColor(itemColor.value());
            colorPicker->setBackgroundColor(Ui::DesignSystem::color().background());
            colorPicker->setTextColor(Ui::DesignSystem::color().onBackground());
            connect(colorPicker, &ColorPicker::selectedColorChanged, view,
                    [this, itemColor, item](const QColor& _color) {
                        if (item->type() == BusinessLayer::TextModelItemType::Folder) {
                            const auto folderItem
                                = static_cast<BusinessLayer::AudioplayTextModelFolderItem*>(item);
                            folderItem->setColor(_color);
                        } else if (item->type() == BusinessLayer::TextModelItemType::Group) {
                            const auto sceneItem
                                = static_cast<BusinessLayer::AudioplayTextModelSceneItem*>(item);
                            sceneItem->setColor(_color);
                        }

                        model->updateItem(item);
                        contextMenu->hideContextMenu();
                    });
        }
    }
    //
    // ... для нескольких
    //
    else {
        //
        // Цвета показываем только для папок и сцен
        //
    }

    contextMenu->setActions(actions);
}


// ****


AudioplayTextStructureManager::AudioplayTextStructureManager(QObject* _parent)
    : QObject(_parent)
    , d(new Implementation)
{
    connect(d->view, &Ui::AudioplayTextStructureView::currentModelIndexChanged, this,
            [this](const QModelIndex& _index) {
                emit currentModelIndexChanged(d->structureModel->mapToSource(_index));
            });
    connect(d->view, &Ui::AudioplayTextStructureView::customContextMenuRequested, this,
            [this](const QPoint& _pos) {
                if (d->view->selectedIndexes().isEmpty()) {
                    return;
                }

                d->updateContextMenu(d->view->selectedIndexes());
                d->contextMenu->showContextMenu(d->view->mapToGlobal(_pos));
            });
}

AudioplayTextStructureManager::~AudioplayTextStructureManager() = default;

QObject* AudioplayTextStructureManager::asQObject()
{
    return this;
}

void AudioplayTextStructureManager::setModel(BusinessLayer::AbstractModel* _model)
{
    //
    // Разрываем соединения со старой моделью
    //
    if (d->model != nullptr) {
        d->view->disconnect(d->model);
    }

    //
    // Определяем новую модель
    //
    d->model = qobject_cast<BusinessLayer::AudioplayTextModel*>(_model);

    //
    // Создаём прокси модель, если ещё не была создана и настриваем её
    //
    if (d->structureModel == nullptr) {
        d->structureModel = new BusinessLayer::AudioplayTextStructureModel(d->view);
        d->view->setModel(d->structureModel);
    }

    //
    // Помещаем модель с данными в прокси
    //
    d->structureModel->setSourceModel(d->model);

    //
    // Настраиваем соединения с новой моделью
    //
    if (d->model != nullptr) {
        d->view->setTitle(
            QString("%1 | %2").arg(tr("Audioplay"), d->model->informationModel()->name()));
        connect(d->model->informationModel(),
                &BusinessLayer::AudioplayInformationModel::nameChanged, d->view,
                &Ui::AudioplayTextStructureView::setTitle);
    }

    //
    // Если элемент к выделению уже задан, выберем его в структуре
    //
    if (d->modelIndexToSelect.isValid()) {
        setCurrentModelIndex(d->modelIndexToSelect);
    }

    //
    // Переконфигурируемся
    //
    reconfigure({});
}

Ui::IDocumentView* AudioplayTextStructureManager::view()
{
    return d->view;
}

Ui::IDocumentView* AudioplayTextStructureManager::createView()
{
    return d->createView();
}

void AudioplayTextStructureManager::reconfigure(const QStringList& _changedSettingsKeys)
{
    Q_UNUSED(_changedSettingsKeys);

    d->view->reconfigure();
}

void AudioplayTextStructureManager::bind(IDocumentManager* _manager)
{
    Q_ASSERT(_manager);

    connect(_manager->asQObject(), SIGNAL(currentModelIndexChanged(QModelIndex)), this,
            SLOT(setCurrentModelIndex(QModelIndex)), Qt::UniqueConnection);
}

void AudioplayTextStructureManager::setCurrentModelIndex(const QModelIndex& _index)
{
    if (!_index.isValid()) {
        return;
    }

    if (d->model != _index.model()) {
        d->modelIndexToSelect = _index;
        return;
    }

    QSignalBlocker signalBlocker(this);

    //
    // Из редактора сценария мы получаем индексы текстовых элементов, они хранятся внутри
    // папок, сцен или битов, которые как раз и отображаются в навигаторе
    //
    auto indexForSelect = d->structureModel->mapFromSource(_index.parent());
    //
    // ... когда быти скрыты в навигаторе, берём папку или сцену, в которой они находятся
    //
    if (!indexForSelect.isValid()) {
        indexForSelect = d->structureModel->mapFromSource(_index.parent().parent());
    }
    d->view->setCurrentModelIndex(_index.parent(), indexForSelect);
    d->modelIndexToSelect = {};
}

} // namespace ManagementLayer
