#pragma once

#include "../manager_plugin_global.h"

#include <interfaces/management_layer/i_document_manager.h>

#include <QObject>


namespace ManagementLayer {

/**
 * @brief Менеджер информации о комиксе
 */
class MANAGER_PLUGIN_EXPORT ComicBookInformationManager : public QObject, public IDocumentManager
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "app.starc.ManagementLayer.IDocumentManager")
    Q_INTERFACES(ManagementLayer::IDocumentManager)

public:
    explicit ComicBookInformationManager(QObject* _parent = nullptr);
    ~ComicBookInformationManager() override;

    /**
     * @brief Реализуем интерфейс менеджера документа
     */
    /** @{ */
    void setModel(BusinessLayer::AbstractModel* _model) override;
    QWidget* view() override;
    QWidget* createView() override;
    /** @} */

private:
    class Implementation;
    QScopedPointer<Implementation> d;
};

} // namespace ManagementLayer
