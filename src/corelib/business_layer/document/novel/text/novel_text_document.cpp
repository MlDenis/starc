#include "novel_text_document.h"

#include "novel_text_corrector.h"

#include <business_layer/document/text/text_block_data.h>
#include <business_layer/document/text/text_cursor.h>
#include <business_layer/model/novel/text/novel_text_model.h>
#include <business_layer/model/novel/text/novel_text_model_scene_item.h>
#include <business_layer/templates/novel_template.h>


namespace BusinessLayer {

class NovelTextDocument::Implementation
{
public:
    explicit Implementation(NovelTextDocument* _q);


    /**
     * @brief Владелец
     */
    NovelTextDocument* q = nullptr;

    /**
     * @brief Отображать ли элементы поэпизодника (true) или текста сценария (false)
     */
    bool isTreatmentDocument = false;

    /**
     * @brief Отображать ли биты (только для режима сценария, когда isTreatmentDocument == false)
     */
    bool isBeatsVisible = false;
};

NovelTextDocument::Implementation::Implementation(NovelTextDocument* _q)
    : q(_q)
{
}


// ****


NovelTextDocument::NovelTextDocument(QObject* _parent)
    : TextDocument(_parent)
    , d(new Implementation(this))
{
    setCorrector(new NovelTextCorrector(this));

    connect(this, &NovelTextDocument::contentsChanged, this, [this] {
        if (!canChangeModel()) {
            return;
        }

        auto novelModel = qobject_cast<NovelTextModel*>(model());
        if (novelModel == nullptr) {
            return;
        }

        if (d->isTreatmentDocument) {
            novelModel->setTreatmentPageCount(pageCount());

            //
            // Если включён режим отображения поэпизодника, то обработаем кейс, когда в документе
            // есть только один блок и он невидим в поэпизоднике
            //
            if (blockCount() == 1
                && !visibleBlocksTypes().contains(TextBlockStyle::forBlock(begin()))) {
                setParagraphType(TextParagraphType::SceneHeading, TextCursor(this));
            }
        } else {
            novelModel->setScriptPageCount(pageCount());
        }
    });
}

NovelTextDocument::~NovelTextDocument() = default;

bool NovelTextDocument::isTreatmentDocument() const
{
    return d->isTreatmentDocument;
}

void NovelTextDocument::setTreatmentDocument(bool _treatment)
{
    if (d->isTreatmentDocument == _treatment) {
        return;
    }

    d->isTreatmentDocument = _treatment;
}

bool NovelTextDocument::isBeatsVisible() const
{
    return d->isBeatsVisible;
}

void NovelTextDocument::setBeatsVisible(bool _visible)
{
    if (d->isBeatsVisible == _visible) {
        return;
    }

    d->isBeatsVisible = _visible;

    emit contentsChange(0, 0, 0);
    emit contentsChanged();
}

QSet<TextParagraphType> NovelTextDocument::visibleBlocksTypes() const
{
    if (d->isTreatmentDocument) {
        return {
            TextParagraphType::SceneHeading,      TextParagraphType::SceneHeadingShadowTreatment,
            TextParagraphType::SceneCharacters,   TextParagraphType::BeatHeading,
            TextParagraphType::BeatHeadingShadow, TextParagraphType::ActHeading,
            TextParagraphType::ActFooter,         TextParagraphType::SequenceHeading,
            TextParagraphType::SequenceFooter,
        };
    }

    if (d->isBeatsVisible) {
        return {
            TextParagraphType::SceneHeading,
            TextParagraphType::SceneHeadingShadow,
            TextParagraphType::SceneCharacters,
            TextParagraphType::BeatHeading,
            TextParagraphType::BeatHeadingShadow,
            TextParagraphType::Action,
            TextParagraphType::Character,
            TextParagraphType::Parenthetical,
            TextParagraphType::Dialogue,
            TextParagraphType::Lyrics,
            TextParagraphType::Shot,
            TextParagraphType::Transition,
            TextParagraphType::InlineNote,
            TextParagraphType::UnformattedText,
            TextParagraphType::ActHeading,
            TextParagraphType::ActFooter,
            TextParagraphType::SequenceHeading,
            TextParagraphType::SequenceFooter,
            TextParagraphType::PageSplitter,
        };
    }

    return {
        TextParagraphType::SceneHeading,
        TextParagraphType::SceneHeadingShadow,
        TextParagraphType::SceneCharacters,
        TextParagraphType::Action,
        TextParagraphType::Character,
        TextParagraphType::Parenthetical,
        TextParagraphType::Dialogue,
        TextParagraphType::Lyrics,
        TextParagraphType::Shot,
        TextParagraphType::Transition,
        TextParagraphType::InlineNote,
        TextParagraphType::UnformattedText,
        TextParagraphType::ActHeading,
        TextParagraphType::ActFooter,
        TextParagraphType::SequenceHeading,
        TextParagraphType::SequenceFooter,
        TextParagraphType::PageSplitter,
    };
}

void NovelTextDocument::setCorrectionOptions(bool _needToCorrectCharactersNames,
                                             bool _needToCorrectPageBreaks)
{
    QStringList correctionOptions;
    if (_needToCorrectCharactersNames) {
        correctionOptions.append("correct-characters-names");
    }
    if (_needToCorrectPageBreaks) {
        correctionOptions.append("correct-page-breaks");
    }
    TextDocument::setCorrectionOptions(correctionOptions);
}

QString NovelTextDocument::sceneNumber(const QTextBlock& _forBlock) const
{
    if (_forBlock.userData() == nullptr) {
        return {};
    }

    const auto blockData = static_cast<TextBlockData*>(_forBlock.userData());
    if (blockData == nullptr) {
        return {};
    }

    const auto itemParent = blockData->item()->parent();
    if (itemParent == nullptr || itemParent->type() != TextModelItemType::Group) {
        return {};
    }

    const auto sceneItem = static_cast<const NovelTextModelSceneItem*>(itemParent);
    return sceneItem->number().value_or(TextModelGroupItem::Number()).text;
}

QString NovelTextDocument::dialogueNumber(const QTextBlock& _forBlock) const
{
    if (_forBlock.userData() == nullptr) {
        return {};
    }

    const auto blockData = static_cast<TextBlockData*>(_forBlock.userData());
    if (blockData == nullptr) {
        return {};
    }

    const auto item = blockData->item();
    if (item == nullptr || item->type() != TextModelItemType::Text) {
        return {};
    }

    const auto textItem = static_cast<const TextModelTextItem*>(item);
    return textItem->number().value_or(TextModelTextItem::Number()).text;
}

} // namespace BusinessLayer
