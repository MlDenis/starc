#include "model_helper.h"

#include <business_layer/model/audioplay/audioplay_information_model.h>
#include <business_layer/model/audioplay/audioplay_title_page_model.h>
#include <business_layer/model/comic_book/comic_book_information_model.h>
#include <business_layer/model/comic_book/comic_book_title_page_model.h>
#include <business_layer/model/novel/novel_information_model.h>
#include <business_layer/model/novel/novel_title_page_model.h>
#include <business_layer/model/screenplay/screenplay_information_model.h>
#include <business_layer/model/screenplay/screenplay_title_page_model.h>
#include <business_layer/model/stageplay/stageplay_information_model.h>
#include <business_layer/model/stageplay/stageplay_title_page_model.h>
#include <business_layer/model/text/text_model_text_item.h>
#include <business_layer/model/text/text_model_xml.h>
#include <business_layer/templates/audioplay_template.h>
#include <business_layer/templates/comic_book_template.h>
#include <business_layer/templates/novel_template.h>
#include <business_layer/templates/screenplay_template.h>
#include <business_layer/templates/stageplay_template.h>
#include <business_layer/templates/templates_facade.h>
#include <domain/document_object.h>

#include <QDomDocument>

using namespace BusinessLayer;


int ModelHelper::recursiveRowCount(QAbstractItemModel* _model)
{
    int count = 0;
    std::function<void(const QModelIndex&)> recursiveCount;
    recursiveCount = [_model, &count, &recursiveCount](const QModelIndex& _parent) {
        for (int row = 0; row < _model->rowCount(_parent); ++row) {
            const auto index = _model->index(row, 0, _parent);
            ++count;

            recursiveCount(index);
        }
    };
    recursiveCount({});
    return count;
}

void ModelHelper::initTitlePageModel(BusinessLayer::SimpleTextModel* _model)
{
    if (_model == nullptr || _model->rowCount() != 1) {
        return;
    }

    const auto item = _model->itemForIndex(_model->index(0, 0));
    if (item->type() != BusinessLayer::TextModelItemType::Text) {
        return;
    }

    const auto textItem = static_cast<BusinessLayer::TextModelTextItem*>(item);
    if (textItem->text().isEmpty()) {
        resetTitlePageModel(_model);
    }
}

void ModelHelper::resetTitlePageModel(BusinessLayer::SimpleTextModel* _model)
{
    if (!_model) {
        return;
    }

    QString titlePage;
    if (auto model = qobject_cast<BusinessLayer::ScreenplayTitlePageModel*>(_model)) {
        titlePage = TemplatesFacade::screenplayTemplate(model->informationModel()->templateId())
                        .titlePage();
    } else if (auto model = qobject_cast<BusinessLayer::ComicBookTitlePageModel*>(_model)) {
        titlePage = TemplatesFacade::comicBookTemplate(model->informationModel()->templateId())
                        .titlePage();
    } else if (auto model = qobject_cast<BusinessLayer::AudioplayTitlePageModel*>(_model)) {
        titlePage = TemplatesFacade::audioplayTemplate(model->informationModel()->templateId())
                        .titlePage();
    } else if (auto model = qobject_cast<BusinessLayer::StageplayTitlePageModel*>(_model)) {
        titlePage = TemplatesFacade::stageplayTemplate(model->informationModel()->templateId())
                        .titlePage();
    } else if (auto model = qobject_cast<BusinessLayer::NovelTitlePageModel*>(_model)) {
        titlePage
            = TemplatesFacade::novelTemplate(model->informationModel()->templateId()).titlePage();
    }

    const auto oldContent = _model->document()->content();

    //
    // Сначла загрузим в документ содержимое из шаблона, а потом перезапишем его правильным xml,
    // чтобы не формировался бесполезный патч о добавлении служебных заголовков и замены пробелов на
    // переносы строк
    //
    _model->setDocumentContent(titlePage.toUtf8());
    _model->reassignContent();
    //
    // ... а затем восстанавливаем исходный контент, чтобы сформировать правильный патч
    //
    _model->document()->setContent(oldContent);
}

QPair<bool, bool> ModelHelper::isMimeHasJustOneBlock(const QString& _mime)
{
    auto isMimeContainsJustOneBlock = false;
    auto isMimeContainsFolderOrSequence = false;

    QDomDocument mimeDocument;
    mimeDocument.setContent(_mime);
    const auto document = mimeDocument.firstChildElement(xml::kDocumentTag);

    //
    // Всего один текстовый блок
    //
    if (document.childNodes().size() == 1
        && textFolderTypeFromString(document.firstChild().nodeName()) == TextFolderType::Undefined
        && textGroupTypeFromString(document.firstChild().nodeName()) == TextGroupType::Undefined) {
        isMimeContainsJustOneBlock = true;
    }

    //
    // или группа с одним заголовком
    //
    else if (document.childNodes().size() == 1
             && (textFolderTypeFromString(document.firstChild().nodeName())
                     != TextFolderType::Undefined
                 || textGroupTypeFromString(document.firstChild().nodeName())
                     != TextGroupType::Undefined)
             && document.firstChild().firstChildElement(xml::kContentTag).childNodes().size()
                 == 1) {
        isMimeContainsJustOneBlock = true;
        isMimeContainsFolderOrSequence = true;
    }
    return { isMimeContainsJustOneBlock, isMimeContainsFolderOrSequence };
}
