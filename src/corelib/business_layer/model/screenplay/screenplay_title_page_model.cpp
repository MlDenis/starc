#include "screenplay_title_page_model.h"

#include <business_layer/model/text/text_model_text_item.h>
#include <business_layer/templates/simple_text_template.h>
#include <domain/document_object.h>


namespace BusinessLayer {

class ScreenplayTitlePageModel::Implementation
{
public:
    /**
     * @brief Модель информации о проекте
     */
    ScreenplayInformationModel* informationModel = nullptr;
};

ScreenplayTitlePageModel::ScreenplayTitlePageModel(QObject* _parent)
    : TextModel(_parent)
    , d(new Implementation)
{
}

ScreenplayTitlePageModel::~ScreenplayTitlePageModel() = default;

void ScreenplayTitlePageModel::setDocumentName(const QString& _name)
{
    Q_UNUSED(_name);
}

void ScreenplayTitlePageModel::setInformationModel(ScreenplayInformationModel* _model)
{
    d->informationModel = _model;
}

ScreenplayInformationModel* ScreenplayTitlePageModel::informationModel() const
{
    return d->informationModel;
}

void ScreenplayTitlePageModel::initDocument()
{
    //
    // Если документ пустой, создаём первоначальную структуру
    //
    if (document()->content().isEmpty()) {
        //
        // FIXME: подгрузка структуры из шаблона сценария
        //
        auto textItem = new TextModelTextItem;
        textItem->setParagraphType(TextParagraphType::Text);
        appendItem(textItem);
    }
    //
    // А если данные есть, то загрузим их из документа
    //
    else {
        TextModel::initDocument();
    }
}

} // namespace BusinessLayer
