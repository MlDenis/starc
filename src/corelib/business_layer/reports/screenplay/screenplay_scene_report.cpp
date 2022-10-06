#include "screenplay_scene_report.h"

#include <business_layer/document/screenplay/text/screenplay_text_document.h>
#include <business_layer/model/screenplay/screenplay_information_model.h>
#include <business_layer/model/screenplay/text/screenplay_text_block_parser.h>
#include <business_layer/model/screenplay/text/screenplay_text_model.h>
#include <business_layer/model/screenplay/text/screenplay_text_model_scene_item.h>
#include <business_layer/model/screenplay/text/screenplay_text_model_text_item.h>
#include <business_layer/templates/screenplay_template.h>
#include <business_layer/templates/templates_facade.h>
#include <ui/design_system/design_system.h>
#include <ui/widgets/text_edit/page/page_text_edit.h>
#include <utils/helpers/text_helper.h>
#include <utils/helpers/time_helper.h>

#include <QCoreApplication>
#include <QRegularExpression>
#include <QStandardItemModel>

#include <set>

namespace BusinessLayer {

class ScreenplaySceneReport::Implementation
{
public:
    QScopedPointer<QStandardItemModel> sceneModel;
};


// ****


ScreenplaySceneReport::ScreenplaySceneReport()
    : d(new Implementation)
{
}

ScreenplaySceneReport::~ScreenplaySceneReport() = default;

void ScreenplaySceneReport::build(QAbstractItemModel* _model)
{
    if (_model == nullptr) {
        return;
    }

    auto screenplayModel = qobject_cast<ScreenplayTextModel*>(_model);
    if (screenplayModel == nullptr) {
        return;
    }

    //
    // Подготовим необходимые структуры для сбора статистики
    //
    struct CharacterData {
        QString name;
        bool isFirstAppearance = false;
        int totalDialogues = 0;
    };
    const int invalidPage = 0;
    struct SceneData {
        QString name;
        int page = invalidPage;
        QString number;
        std::chrono::milliseconds duration;
        QVector<CharacterData> characters;

        CharacterData& character(const QString& _name)
        {
            for (int index = 0; index < characters.size(); ++index) {
                if (characters[index].name == _name) {
                    return characters[index];
                }
            }

            characters.append({ _name });
            return characters.last();
        }
    };
    QVector<SceneData> scenes;
    SceneData lastScene;
    QSet<QString> characters;

    //
    // Сформируем регулярное выражение для выуживания молчаливых персонажей
    //
    QString rxPattern;
    auto charactersModel = screenplayModel->charactersModel();
    for (int index = 0; index < charactersModel->rowCount(); ++index) {
        auto characterName = charactersModel->index(index, 0).data().toString();
        if (!rxPattern.isEmpty()) {
            rxPattern.append("|");
        }
        rxPattern.append(characterName);
    }
    if (!rxPattern.isEmpty()) {
        rxPattern.prepend("(^|\\W)(");
        rxPattern.append(")($|\\W)");
    }
    const QRegularExpression rxCharacterFinder(
        rxPattern,
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);

    //
    // Подготовим текстовый документ, для определения страниц сцен
    //
    const auto& screenplayTemplate
        = TemplatesFacade::screenplayTemplate(screenplayModel->informationModel()->templateId());
    PageTextEdit screenplayTextEdit;
    screenplayTextEdit.setUsePageMode(true);
    screenplayTextEdit.setPageSpacing(0);
    screenplayTextEdit.setPageFormat(screenplayTemplate.pageSizeId());
    screenplayTextEdit.setPageMarginsMm(screenplayTemplate.pageMargins());
    ScreenplayTextDocument screenplayDocument;
    screenplayTextEdit.setDocument(&screenplayDocument);
    const bool kCanChangeModel = false;
    screenplayDocument.setModel(screenplayModel, kCanChangeModel);
    QTextCursor screenplayCursor(&screenplayDocument);
    auto textItemPage = [&screenplayTextEdit, &screenplayDocument,
                         &screenplayCursor](TextModelTextItem* _item) {
        screenplayCursor.setPosition(
            screenplayDocument.itemPosition(screenplayDocument.model()->indexForItem(_item), true));
        return screenplayTextEdit.cursorPage(screenplayCursor);
    };

    //
    // Собираем статистику
    //
    std::function<void(const TextModelItem*)> includeInReport;
    includeInReport = [&includeInReport, &scenes, &lastScene, &characters, &rxCharacterFinder,
                       textItemPage, invalidPage](const TextModelItem* _item) {
        for (int childIndex = 0; childIndex < _item->childCount(); ++childIndex) {
            auto childItem = _item->childAt(childIndex);
            switch (childItem->type()) {
            case TextModelItemType::Folder:
            case TextModelItemType::Group: {
                includeInReport(childItem);
                break;
            }

            case TextModelItemType::Text: {
                auto textItem = static_cast<ScreenplayTextModelTextItem*>(childItem);
                //
                // ... стата по объектам
                //
                switch (textItem->paragraphType()) {
                case TextParagraphType::SceneHeading: {
                    //
                    // Началась новая сцена
                    //
                    if (lastScene.page != invalidPage) {
                        scenes.append(lastScene);
                        lastScene = SceneData();
                    }

                    const auto sceneItem
                        = static_cast<ScreenplayTextModelSceneItem*>(textItem->parent());
                    lastScene.name = sceneItem->heading();
                    lastScene.number = sceneItem->number()->text;
                    lastScene.duration = sceneItem->duration();
                    lastScene.page = textItemPage(textItem);
                    break;
                }

                case TextParagraphType::SceneCharacters: {
                    const auto sceneCharacters
                        = ScreenplaySceneCharactersParser::characters(textItem->text());
                    for (const auto& character : sceneCharacters) {
                        auto& characterData = lastScene.character(character);
                        if (!characters.contains(character)) {
                            characters.insert(character);
                            characterData.isFirstAppearance = true;
                        }
                    }
                    break;
                }

                case TextParagraphType::Character: {
                    const auto character = ScreenplayCharacterParser::name(textItem->text());
                    auto& characterData = lastScene.character(character);
                    ++characterData.totalDialogues;
                    if (!characters.contains(character)) {
                        characters.insert(character);
                        characterData.isFirstAppearance = true;
                    }
                    break;
                }

                case TextParagraphType::Action: {
                    if (rxCharacterFinder.pattern().isEmpty()) {
                        break;
                    }

                    auto match = rxCharacterFinder.match(textItem->text());
                    while (match.hasMatch()) {
                        const QString character = TextHelper::smartToUpper(match.captured(2));
                        auto& characterData = lastScene.character(character);
                        if (!characters.contains(character)) {
                            characters.insert(character);
                            characterData.isFirstAppearance = true;
                        }

                        //
                        // Ищем дальше
                        //
                        match = rxCharacterFinder.match(textItem->text(), match.capturedEnd());
                    }
                    break;
                }

                default:
                    break;
                }

                break;
            }

            default:
                break;
            }
        }
    };
    includeInReport(screenplayModel->itemForIndex({}));
    if (lastScene.page != invalidPage) {
        scenes.append(lastScene);
    }

    //
    // Формируем отчёт
    //
    auto createModelItem = [](const QString& _text) {
        auto item = new QStandardItem(_text);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        return item;
    };
    //
    // ... наполняем таблицу
    //
    if (d->sceneModel.isNull()) {
        d->sceneModel.reset(new QStandardItemModel);
    } else {
        d->sceneModel->clear();
    }
    for (const auto& scene : scenes) {
        auto sceneItem = createModelItem(scene.name);
        for (const auto& character : scene.characters) {
            auto characterItem = createModelItem(
                QString("%1 (%2)").arg(character.name, QString::number(character.totalDialogues)));
            if (character.isFirstAppearance) {
                characterItem->setData(u8"\U000F09DE", Qt::DecorationRole);
                characterItem->setData(Ui::DesignSystem::color().secondary(),
                                       Qt::DecorationPropertyRole);
            }
            sceneItem->appendRow({ characterItem, createModelItem({}), createModelItem({}),
                                   createModelItem({}), createModelItem({}) });
        }

        d->sceneModel->appendRow({
            sceneItem,
            createModelItem(scene.number),
            createModelItem(QString::number(scene.page)),
            createModelItem(QString::number(scene.characters.size())),
            createModelItem(TimeHelper::toString(scene.duration)),
        });
    }
    //
    d->sceneModel->setHeaderData(
        0, Qt::Horizontal,
        QCoreApplication::translate("BusinessLayer::ScreenplaySceneReport", "Scene/characters"),
        Qt::DisplayRole);
    d->sceneModel->setHeaderData(
        1, Qt::Horizontal,
        QCoreApplication::translate("BusinessLayer::ScreenplaySceneReport", "Number"),
        Qt::DisplayRole);
    d->sceneModel->setHeaderData(
        2, Qt::Horizontal,
        QCoreApplication::translate("BusinessLayer::ScreenplaySceneReport", "Page"),
        Qt::DisplayRole);
    d->sceneModel->setHeaderData(
        3, Qt::Horizontal,
        QCoreApplication::translate("BusinessLayer::ScreenplaySceneReport", "Characters"),
        Qt::DisplayRole);
    d->sceneModel->setHeaderData(
        4, Qt::Horizontal,
        QCoreApplication::translate("BusinessLayer::ScreenplaySceneReport", "Duration"),
        Qt::DisplayRole);
}

QAbstractItemModel* ScreenplaySceneReport::sceneModel() const
{
    return d->sceneModel.data();
}

} // namespace BusinessLayer
