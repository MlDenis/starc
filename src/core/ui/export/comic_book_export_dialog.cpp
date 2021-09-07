#include "comic_book_export_dialog.h"

#include <business_layer/export/comic_book/comic_book_export_options.h>
#include <business_layer/templates/comic_book_template.h>
#include <business_layer/templates/templates_facade.h>
#include <ui/design_system/design_system.h>
#include <ui/widgets/button/button.h>
#include <ui/widgets/check_box/check_box.h>
#include <ui/widgets/combo_box/combo_box.h>
#include <ui/widgets/label/label.h>
#include <ui/widgets/text_field/text_field.h>

#include <QEvent>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QStringListModel>


namespace Ui {

class ComicBookExportDialog::Implementation
{
public:
    explicit Implementation(QWidget* _parent);


    ComboBox* fileFormat = nullptr;
    ComboBox* comicBookTemplate = nullptr;
    CheckBox* printTitlePage = nullptr;
    CheckBox* printFolders = nullptr;
    CheckBox* useWordsInPageHeadings = nullptr;
    CheckBox* printInlineNotes = nullptr;
    CheckBox* printReviewMarks = nullptr;
    TextField* watermark = nullptr;

    QHBoxLayout* buttonsLayout = nullptr;
    CheckBox* openDocumentAfterExport = nullptr;
    Button* cancelButton = nullptr;
    Button* exportButton = nullptr;
};

ComicBookExportDialog::Implementation::Implementation(QWidget* _parent)
    : fileFormat(new ComboBox(_parent))
    , comicBookTemplate(new ComboBox(_parent))
    , printTitlePage(new CheckBox(_parent))
    , printFolders(new CheckBox(_parent))
    , useWordsInPageHeadings(new CheckBox(_parent))
    , printInlineNotes(new CheckBox(_parent))
    , printReviewMarks(new CheckBox(_parent))
    , watermark(new TextField(_parent))
    , buttonsLayout(new QHBoxLayout)
    , openDocumentAfterExport(new CheckBox(_parent))
    , cancelButton(new Button(_parent))
    , exportButton(new Button(_parent))
{
    using namespace BusinessLayer;

    fileFormat->setSpellCheckPolicy(SpellCheckPolicy::Manual);
    auto formatsModel = new QStringListModel({ "PDF", "DOCX" /*, "FDX", "Fontain" */ });
    fileFormat->setModel(formatsModel);
    fileFormat->setCurrentIndex(formatsModel->index(0, 0));

    comicBookTemplate->setSpellCheckPolicy(SpellCheckPolicy::Manual);
    comicBookTemplate->setModel(TemplatesFacade::comicBookTemplates());
    for (int row = 0; row < TemplatesFacade::comicBookTemplates()->rowCount(); ++row) {
        auto item = TemplatesFacade::comicBookTemplates()->item(row);
        if (item->data(TemplatesFacade::kTemplateIdRole).toString()
            != TemplatesFacade::comicBookTemplate().id()) {
            continue;
        }

        comicBookTemplate->setCurrentIndex(item->index());
        break;
    }

    printTitlePage->hide();
    useWordsInPageHeadings->hide();

    for (auto checkBox : { /*printTitlePage,*/ printFolders, useWordsInPageHeadings,
                           printReviewMarks, openDocumentAfterExport }) {
        checkBox->setChecked(true);
    }

    watermark->setSpellCheckPolicy(SpellCheckPolicy::Manual);

    buttonsLayout->setContentsMargins({});
    buttonsLayout->setSpacing(0);
    buttonsLayout->addWidget(openDocumentAfterExport, 0, Qt::AlignVCenter);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(exportButton);
}


// ****


ComicBookExportDialog::ComicBookExportDialog(QWidget* _parent)
    : AbstractDialog(_parent)
    , d(new Implementation(this))
{
    setAcceptButton(d->exportButton);
    setRejectButton(d->cancelButton);

    int row = 0;
    contentsLayout()->addWidget(d->fileFormat, row++, 0);
    contentsLayout()->addWidget(d->comicBookTemplate, row++, 0);
    contentsLayout()->addWidget(d->printTitlePage, row++, 0);
    contentsLayout()->addWidget(d->printFolders, row++, 0);
    contentsLayout()->addWidget(d->useWordsInPageHeadings, row++, 0);
    contentsLayout()->addWidget(d->printInlineNotes, row++, 0);
    contentsLayout()->addWidget(d->printReviewMarks, row++, 0);
    contentsLayout()->addWidget(d->watermark, row++, 0);
    contentsLayout()->addLayout(d->buttonsLayout, row++, 0);

    connect(d->fileFormat, &ComboBox::currentIndexChanged, this, [this] {
        auto isComicBookTemplateVisible = true;
        auto isPrintFoldersVisible = true;
        auto isPrintInlineNotesVisible = true;
        auto isPrintReviewMarksVisible = true;
        auto isWatermarkVisible = true;
        switch (d->fileFormat->currentIndex().row()) {
        //
        // PDF
        //
        default:
        case 0: {
            //
            // ... всё видимое
            //
            break;
        }

        //
        // DOCX
        //
        case 1: {
            isWatermarkVisible = false;
            break;
        }
        }
        d->comicBookTemplate->setVisible(isComicBookTemplateVisible);
        d->printFolders->setVisible(isPrintFoldersVisible);
        d->printInlineNotes->setVisible(isPrintInlineNotesVisible);
        d->printReviewMarks->setVisible(isPrintReviewMarksVisible);
        d->watermark->setVisible(isWatermarkVisible);
    });

    connect(d->exportButton, &Button::clicked, this, &ComicBookExportDialog::exportRequested);
    connect(d->cancelButton, &Button::clicked, this, &ComicBookExportDialog::canceled);

    updateTranslations();
    designSystemChangeEvent(nullptr);
}

ComicBookExportDialog::~ComicBookExportDialog() = default;

BusinessLayer::ComicBookExportOptions ComicBookExportDialog::exportOptions() const
{
    BusinessLayer::ComicBookExportOptions options;
    options.fileFormat = static_cast<BusinessLayer::ComicBookExportFileFormat>(
        d->fileFormat->currentIndex().row());
    options.templateId = d->comicBookTemplate->currentIndex()
                             .data(BusinessLayer::TemplatesFacade::kTemplateIdRole)
                             .toString();
    options.printTiltePage = d->printTitlePage->isChecked();
    options.printFolders = d->printFolders->isChecked();
    options.useWordsInPageHeadings = d->useWordsInPageHeadings->isChecked();
    options.printInlineNotes = d->printInlineNotes->isChecked();
    options.printReviewMarks = d->printReviewMarks->isChecked();
    options.watermark = d->watermark->text();
    options.watermarkColor = QColor(100, 100, 100, 30);
    return options;
}

bool ComicBookExportDialog::openDocumentAfterExport() const
{
    return d->openDocumentAfterExport->isChecked();
}

QWidget* ComicBookExportDialog::focusedWidgetAfterShow() const
{
    return d->fileFormat;
}

QWidget* ComicBookExportDialog::lastFocusableWidget() const
{
    return d->exportButton;
}

void ComicBookExportDialog::updateTranslations()
{
    setTitle(tr("Export comic book"));

    d->fileFormat->setLabel(tr("Format"));
    d->comicBookTemplate->setLabel(tr("Template"));
    d->printTitlePage->setText(tr("Print title page"));
    d->printFolders->setText(tr("Print folders"));
    d->useWordsInPageHeadings->setText(tr("Print panels numbers in form of words"));
    d->printInlineNotes->setText(tr("Print inline notes"));
    d->printReviewMarks->setText(tr("Print review marks"));
    d->watermark->setLabel(tr("Watermark"));

    d->openDocumentAfterExport->setText(tr("Open document after export"));
    d->exportButton->setText(tr("Export"));
    d->cancelButton->setText(tr("Cancel"));
}

void ComicBookExportDialog::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    AbstractDialog::designSystemChangeEvent(_event);

    auto titleMargins = Ui::DesignSystem::label().margins().toMargins();
    titleMargins.setTop(Ui::DesignSystem::layout().px8());
    titleMargins.setBottom(0);

    for (auto textField :
         QVector<TextField*>{ d->fileFormat, d->comicBookTemplate, d->watermark }) {
        textField->setBackgroundColor(Ui::DesignSystem::color().onBackground());
        textField->setTextColor(Ui::DesignSystem::color().onBackground());
    }

    for (auto checkBox : { d->printTitlePage, d->printFolders, d->useWordsInPageHeadings,
                           d->printInlineNotes, d->printReviewMarks, d->openDocumentAfterExport }) {
        checkBox->setBackgroundColor(Ui::DesignSystem::color().background());
        checkBox->setTextColor(Ui::DesignSystem::color().onBackground());
    }

    for (auto button : { d->exportButton, d->cancelButton }) {
        button->setBackgroundColor(Ui::DesignSystem::color().secondary());
        button->setTextColor(Ui::DesignSystem::color().secondary());
    }

    contentsLayout()->setSpacing(static_cast<int>(Ui::DesignSystem::layout().px8()));
    d->buttonsLayout->setContentsMargins(QMarginsF(0.0, Ui::DesignSystem::layout().px24(),
                                                   Ui::DesignSystem::layout().px16(),
                                                   Ui::DesignSystem::layout().px8())
                                             .toMargins());
}

} // namespace Ui
