#include "export_manager.h"

#include <business_layer/export/audioplay/audioplay_docx_exporter.h>
#include <business_layer/export/audioplay/audioplay_export_options.h>
#include <business_layer/export/audioplay/audioplay_pdf_exporter.h>
#include <business_layer/export/comic_book/comic_book_docx_exporter.h>
#include <business_layer/export/comic_book/comic_book_export_options.h>
#include <business_layer/export/comic_book/comic_book_pdf_exporter.h>
#include <business_layer/export/export_options.h>
#include <business_layer/export/screenplay/screenplay_docx_exporter.h>
#include <business_layer/export/screenplay/screenplay_export_options.h>
#include <business_layer/export/screenplay/screenplay_fdx_exporter.h>
#include <business_layer/export/screenplay/screenplay_fountain_exporter.h>
#include <business_layer/export/screenplay/screenplay_pdf_exporter.h>
#include <business_layer/model/audioplay/audioplay_information_model.h>
#include <business_layer/model/audioplay/text/audioplay_text_model.h>
#include <business_layer/model/comic_book/comic_book_information_model.h>
#include <business_layer/model/comic_book/text/comic_book_text_model.h>
#include <business_layer/model/screenplay/screenplay_information_model.h>
#include <business_layer/model/screenplay/text/screenplay_text_model.h>
#include <data_layer/storage/settings_storage.h>
#include <data_layer/storage/storage_facade.h>
#include <domain/document_object.h>
#include <ui/export/audioplay_export_dialog.h>
#include <ui/export/comic_book_export_dialog.h>
#include <ui/export/screenplay_export_dialog.h>
#include <ui/widgets/dialog/standard_dialog.h>
#include <utils/helpers/dialog_helper.h>
#include <utils/helpers/extension_helper.h>

#include <QDesktopServices>
#include <QFileDialog>

namespace ManagementLayer {

class ExportManager::Implementation
{
public:
    explicit Implementation(ExportManager* _parent, QWidget* _topLevelWidget);

    /**
     * @brief Экспортировать документ
     */
    void exportScreenplay(BusinessLayer::AbstractModel* _model);
    void exportComicBook(BusinessLayer::AbstractModel* _model);
    void exportAudioplay(BusinessLayer::AbstractModel* _model);

    //
    // Данные
    //

    ExportManager* q = nullptr;

    QWidget* topLevelWidget = nullptr;

    Ui::ScreenplayExportDialog* screenplayExportDialog = nullptr;
    Ui::ComicBookExportDialog* comicBookExportDialog = nullptr;
    Ui::AudioplayExportDialog* audioplayExportDialog = nullptr;
};

ExportManager::Implementation::Implementation(ExportManager* _parent, QWidget* _topLevelWidget)
    : q(_parent)
    , topLevelWidget(_topLevelWidget)
{
}

void ExportManager::Implementation::exportScreenplay(BusinessLayer::AbstractModel* _model)
{
    using namespace BusinessLayer;

    if (screenplayExportDialog == nullptr) {
        screenplayExportDialog = new Ui::ScreenplayExportDialog(topLevelWidget);
        connect(
            screenplayExportDialog, &Ui::ScreenplayExportDialog::exportRequested,
            screenplayExportDialog, [this, _model] {
                auto exportOptions = screenplayExportDialog->exportOptions();

                //
                // Предоставим пользователю возможность выбрать файл, куда он будет экспортировать
                //
                const auto projectExportFolder
                    = settingsValue(DataStorageLayer::kProjectExportFolderKey).toString();
                QString exportFilter;
                QString exportExtension;
                switch (exportOptions.fileFormat) {
                default:
                case ScreenplayExportFileFormat::Pdf: {
                    exportFilter = DialogHelper::pdfFilter();
                    exportExtension = ExtensionHelper::pdf();
                    break;
                }
                case ScreenplayExportFileFormat::Docx: {
                    exportFilter = DialogHelper::msWordFilter();
                    exportExtension = ExtensionHelper::msOfficeOpenXml();
                    break;
                }
                case ScreenplayExportFileFormat::Fdx: {
                    exportFilter = DialogHelper::finalDraftFilter();
                    exportExtension = ExtensionHelper::finalDraft();
                    break;
                }
                case ScreenplayExportFileFormat::Fountain: {
                    exportFilter = DialogHelper::fountainFilter();
                    exportExtension = ExtensionHelper::fountain();
                    break;
                }
                }
                const auto screenplayTextModel
                    = qobject_cast<BusinessLayer::ScreenplayTextModel*>(_model);
                const auto projectExportFile
                    = QString("%1/%2.%3")
                          .arg(projectExportFolder, screenplayTextModel->informationModel()->name(),
                               exportExtension);
                auto exportFilePath
                    = QFileDialog::getSaveFileName(topLevelWidget, tr("Choose the file to export"),
                                                   projectExportFile, exportFilter);
                if (exportFilePath.isEmpty()) {
                    return;
                }

                //
                // Если файл был выбран
                //
                exportOptions.filePath = exportFilePath;
                //
                // ... проверяем возможность записи в файл
                //
                QFile file(exportFilePath);
                const bool canWrite = file.open(QIODevice::WriteOnly);
                file.close();
                if (!canWrite) {
                    //
                    // ... предупреждаем
                    //
                    QString errorMessage;
                    const QFileInfo fileInfo(exportFilePath);
                    if (fileInfo.exists()) {
                        errorMessage = tr("Can't write to file. Looks like it's opened by another "
                                          "application. Please close it and retry the export.");
                    } else {
                        errorMessage = tr("Can't write to file. Check permissions to write in the "
                                          "chosen folder or choose another folder.");
                    }
                    StandardDialog::information(topLevelWidget, tr("Export error"), errorMessage);
                    return;
                }

                //
                // ... донастроим параметры экспорта
                //
                const auto screenplayInformation = screenplayTextModel->informationModel();
                exportOptions.templateId = screenplayInformation->templateId();
                exportOptions.showScenesNumbers = screenplayInformation->showSceneNumbers();
                exportOptions.showScenesNumbersOnLeft
                    = screenplayInformation->showSceneNumbersOnLeft();
                exportOptions.showScenesNumbersOnRight
                    = screenplayInformation->showSceneNumbersOnRight();
                exportOptions.showDialoguesNumbers = screenplayInformation->showDialoguesNumbers();
                exportOptions.header = screenplayInformation->header();
                exportOptions.printHeaderOnTitlePage
                    = screenplayInformation->printHeaderOnTitlePage();
                exportOptions.footer = screenplayInformation->footer();
                exportOptions.printFooterOnTitlePage
                    = screenplayInformation->printFooterOnTitlePage();
                //
                // ... обновим папку, куда в следующий раз он предположительно опять будет
                //     экспортировать
                //
                setSettingsValue(DataStorageLayer::kProjectExportFolderKey,
                                 QFileInfo(exportFilePath).dir().absolutePath());
                //
                // ... и экспортируем документ
                //
                QScopedPointer<BusinessLayer::ScreenplayAbstractExporter> exporter;
                switch (exportOptions.fileFormat) {
                default:
                case ScreenplayExportFileFormat::Pdf: {
                    exporter.reset(new BusinessLayer::ScreenplayPdfExporter);
                    break;
                }
                case ScreenplayExportFileFormat::Docx: {
                    exporter.reset(new BusinessLayer::ScreenplayDocxExporter);
                    break;
                }
                case ScreenplayExportFileFormat::Fdx: {
                    exporter.reset(new BusinessLayer::ScreenplayFdxExporter);
                    break;
                }
                case ScreenplayExportFileFormat::Fountain: {
                    exporter.reset(new BusinessLayer::ScreenplayFountainExporter);
                    break;
                }
                }
                if (exporter.isNull()) {
                    return;
                }
                exporter->exportTo(screenplayTextModel, exportOptions);

                //
                // Если необходимо, откроем экспортированный документ
                //
                if (screenplayExportDialog->openDocumentAfterExport()) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(exportOptions.filePath));
                }
                //
                // ... и закрываем диалог экспорта
                //
                screenplayExportDialog->hideDialog();
            });
        connect(screenplayExportDialog, &Ui::ScreenplayExportDialog::canceled,
                screenplayExportDialog, &Ui::ScreenplayExportDialog::hideDialog);
        connect(screenplayExportDialog, &Ui::ScreenplayExportDialog::disappeared,
                screenplayExportDialog, [this] {
                    screenplayExportDialog->deleteLater();
                    screenplayExportDialog = nullptr;
                });
    }

    screenplayExportDialog->showDialog();
}

void ExportManager::Implementation::exportComicBook(BusinessLayer::AbstractModel* _model)
{
    using namespace BusinessLayer;

    if (comicBookExportDialog == nullptr) {
        comicBookExportDialog = new Ui::ComicBookExportDialog(topLevelWidget);
        connect(
            comicBookExportDialog, &Ui::ComicBookExportDialog::exportRequested,
            comicBookExportDialog, [this, _model] {
                auto exportOptions = comicBookExportDialog->exportOptions();

                //
                // Предоставим пользователю возможность выбрать файл, куда он будет
                // экспортировать
                //
                const auto projectExportFolder
                    = settingsValue(DataStorageLayer::kProjectExportFolderKey).toString();
                QString exportFilter;
                QString exportExtension;
                switch (exportOptions.fileFormat) {
                default:
                case ExportFileFormat::Pdf: {
                    exportFilter = DialogHelper::pdfFilter();
                    exportExtension = ExtensionHelper::pdf();
                    break;
                }
                case ExportFileFormat::Docx: {
                    exportFilter = DialogHelper::msWordFilter();
                    exportExtension = ExtensionHelper::msOfficeOpenXml();
                    break;
                }
                }
                const auto comicBookTextModel
                    = qobject_cast<BusinessLayer::ComicBookTextModel*>(_model);
                const auto projectExportFile
                    = QString("%1/%2.%3")
                          .arg(projectExportFolder, comicBookTextModel->informationModel()->name(),
                               exportExtension);
                auto exportFilePath
                    = QFileDialog::getSaveFileName(topLevelWidget, tr("Choose the file to export"),
                                                   projectExportFile, exportFilter);
                if (exportFilePath.isEmpty()) {
                    return;
                }

                //
                // Если файл был выбран
                //
                exportOptions.filePath = exportFilePath;
                //
                // ... проверяем возможность записи в файл
                //
                QFile file(exportFilePath);
                const bool canWrite = file.open(QIODevice::WriteOnly);
                file.close();
                if (!canWrite) {
                    //
                    // ... предупреждаем
                    //
                    QString errorMessage;
                    const QFileInfo fileInfo(exportFilePath);
                    if (fileInfo.exists()) {
                        errorMessage = tr("Can't write to file. Looks like it's opened by another "
                                          "application. Please close it and retry the export.");
                    } else {
                        errorMessage = tr("Can't write to file. Check permissions to write in the "
                                          "chosen folder or choose another folder.");
                    }
                    StandardDialog::information(topLevelWidget, tr("Export error"), errorMessage);
                    return;
                }
                //
                // ... донастроим параметры экспорта
                //
                const auto comicBookInformation = comicBookTextModel->informationModel();
                exportOptions.templateId = comicBookInformation->templateId();
                exportOptions.header = comicBookInformation->header();
                exportOptions.footer = comicBookInformation->footer();
                //
                // ... обновим папку, куда в следующий раз он предположительно опять будет
                //     экспортировать
                //
                setSettingsValue(DataStorageLayer::kProjectExportFolderKey,
                                 QFileInfo(exportFilePath).dir().absolutePath());
                //
                // ... и экспортируем документ
                //
                QScopedPointer<BusinessLayer::ComicBookExporter> exporter;
                switch (exportOptions.fileFormat) {
                default:
                case ExportFileFormat::Pdf: {
                    exporter.reset(new BusinessLayer::ComicBookPdfExporter);
                    break;
                }
                case ExportFileFormat::Docx: {
                    exporter.reset(new BusinessLayer::ComicBookDocxExporter);
                    break;
                }
                }
                if (exporter.isNull()) {
                    return;
                }
                exporter->exportTo(comicBookTextModel, exportOptions);

                //
                // Если необходимо, откроем экспортированный документ
                //
                if (comicBookExportDialog->openDocumentAfterExport()) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(exportOptions.filePath));
                }
                //
                // ... и закрываем диалог экспорта
                //
                comicBookExportDialog->hideDialog();
            });
        connect(comicBookExportDialog, &Ui::ComicBookExportDialog::canceled, comicBookExportDialog,
                &Ui::ComicBookExportDialog::hideDialog);
        connect(comicBookExportDialog, &Ui::ComicBookExportDialog::disappeared,
                comicBookExportDialog, [this] {
                    comicBookExportDialog->deleteLater();
                    comicBookExportDialog = nullptr;
                });
    }

    comicBookExportDialog->showDialog();
}

void ExportManager::Implementation::exportAudioplay(BusinessLayer::AbstractModel* _model)
{
    using namespace BusinessLayer;

    if (audioplayExportDialog == nullptr) {
        audioplayExportDialog = new Ui::AudioplayExportDialog(topLevelWidget);
        connect(
            audioplayExportDialog, &Ui::AudioplayExportDialog::exportRequested,
            audioplayExportDialog, [this, _model] {
                auto exportOptions = audioplayExportDialog->exportOptions();

                //
                // Предоставим пользователю возможность выбрать файл, куда он будет экспортировать
                //
                const auto projectExportFolder
                    = settingsValue(DataStorageLayer::kProjectExportFolderKey).toString();
                QString exportFilter;
                QString exportExtension;
                switch (exportOptions.fileFormat) {
                default:
                case ExportFileFormat::Pdf: {
                    exportFilter = DialogHelper::pdfFilter();
                    exportExtension = ExtensionHelper::pdf();
                    break;
                }
                case ExportFileFormat::Docx: {
                    exportFilter = DialogHelper::msWordFilter();
                    exportExtension = ExtensionHelper::msOfficeOpenXml();
                    break;
                }
                }
                const auto audioplayTextModel
                    = qobject_cast<BusinessLayer::AudioplayTextModel*>(_model);
                const auto projectExportFile
                    = QString("%1/%2.%3")
                          .arg(projectExportFolder, audioplayTextModel->informationModel()->name(),
                               exportExtension);
                auto exportFilePath
                    = QFileDialog::getSaveFileName(topLevelWidget, tr("Choose the file to export"),
                                                   projectExportFile, exportFilter);
                if (exportFilePath.isEmpty()) {
                    return;
                }

                //
                // Если файл был выбран
                //
                exportOptions.filePath = exportFilePath;
                //
                // ... проверяем возможность записи в файл
                //
                QFile file(exportFilePath);
                const bool canWrite = file.open(QIODevice::WriteOnly);
                file.close();
                if (!canWrite) {
                    //
                    // ... предупреждаем
                    //
                    QString errorMessage;
                    const QFileInfo fileInfo(exportFilePath);
                    if (fileInfo.exists()) {
                        errorMessage = tr("Can't write to file. Looks like it's opened by another "
                                          "application. Please close it and retry the export.");
                    } else {
                        errorMessage = tr("Can't write to file. Check permissions to write in the "
                                          "chosen folder or choose another folder.");
                    }
                    StandardDialog::information(topLevelWidget, tr("Export error"), errorMessage);
                    return;
                }

                //
                // ... донастроим параметры экспорта
                //
                const auto audioplayInformation = audioplayTextModel->informationModel();
                exportOptions.templateId = audioplayInformation->templateId();
                exportOptions.showBlockNumbers = audioplayInformation->showBlockNumbers();
                exportOptions.header = audioplayInformation->header();
                exportOptions.printHeaderOnTitlePage
                    = audioplayInformation->printHeaderOnTitlePage();
                exportOptions.footer = audioplayInformation->footer();
                exportOptions.printFooterOnTitlePage
                    = audioplayInformation->printFooterOnTitlePage();
                //
                // ... обновим папку, куда в следующий раз он предположительно опять будет
                //     экспортировать
                //
                setSettingsValue(DataStorageLayer::kProjectExportFolderKey,
                                 QFileInfo(exportFilePath).dir().absolutePath());
                //
                // ... и экспортируем документ
                //
                QScopedPointer<BusinessLayer::AbstractExporter> exporter;
                switch (exportOptions.fileFormat) {
                default:
                case ExportFileFormat::Pdf: {
                    exporter.reset(new BusinessLayer::AudioplayPdfExporter);
                    break;
                }
                case ExportFileFormat::Docx: {
                    exporter.reset(new BusinessLayer::AudioplayDocxExporter);
                    break;
                }
                }
                if (exporter.isNull()) {
                    return;
                }
                exporter->exportTo(audioplayTextModel, exportOptions);

                //
                // Если необходимо, откроем экспортированный документ
                //
                if (audioplayExportDialog->openDocumentAfterExport()) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(exportOptions.filePath));
                }
                //
                // ... и закрываем диалог экспорта
                //
                audioplayExportDialog->hideDialog();
            });
        connect(audioplayExportDialog, &Ui::AudioplayExportDialog::canceled, audioplayExportDialog,
                &Ui::AudioplayExportDialog::hideDialog);
        connect(audioplayExportDialog, &Ui::AudioplayExportDialog::disappeared,
                audioplayExportDialog, [this] {
                    audioplayExportDialog->deleteLater();
                    audioplayExportDialog = nullptr;
                });
    }

    audioplayExportDialog->showDialog();
}


// ****


ExportManager::ExportManager(QObject* _parent, QWidget* _parentWidget)
    : QObject(_parent)
    , d(new Implementation(this, _parentWidget))
{
}

ExportManager::~ExportManager() = default;

bool ExportManager::canExportDocument(BusinessLayer::AbstractModel* _model) const
{
    if (_model == nullptr || _model->document() == nullptr) {
        return false;
    }

    switch (_model->document()->type()) {
    case Domain::DocumentObjectType::ScreenplayText:
    case Domain::DocumentObjectType::ComicBookText:
    case Domain::DocumentObjectType::AudioplayText:
        return true;

    default:
        return false;
    }
}

void ExportManager::exportDocument(BusinessLayer::AbstractModel* _model)
{
    if (!canExportDocument(_model)) {
        return;
    }

    switch (_model->document()->type()) {
    case Domain::DocumentObjectType::ScreenplayText: {
        d->exportScreenplay(_model);
        break;
    }

    case Domain::DocumentObjectType::ComicBookText: {
        d->exportComicBook(_model);
        break;
    }

    case Domain::DocumentObjectType::AudioplayText: {
        d->exportAudioplay(_model);
        break;
    }

    default:
        break;
    }
}

} // namespace ManagementLayer
