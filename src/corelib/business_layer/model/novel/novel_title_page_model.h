#pragma once

#include "../simple_text/simple_text_model.h"


namespace BusinessLayer {

class NovelInformationModel;

/**
 * @brief Модель титульной страницы
 */
class CORE_LIBRARY_EXPORT NovelTitlePageModel : public SimpleTextModel
{
    Q_OBJECT

public:
    explicit NovelTitlePageModel(QObject* _parent = nullptr);
    ~NovelTitlePageModel() override;

    /**
     * @brief Название документа
     */
    QString documentName() const override;

    /**
     * @brief Игнорируем установку названия документа
     */
    void setDocumentName(const QString& _name) override;

    /**
     * @brief Задать модель информации
     */
    void setInformationModel(NovelInformationModel* _model);
    NovelInformationModel* informationModel() const;

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace BusinessLayer
