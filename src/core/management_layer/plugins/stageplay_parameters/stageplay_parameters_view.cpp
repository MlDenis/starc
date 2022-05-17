#include "stageplay_parameters_view.h"

#include <business_layer/templates/stageplay_template.h>
#include <business_layer/templates/templates_facade.h>
#include <ui/design_system/design_system.h>
#include <ui/widgets/card/card.h>
#include <ui/widgets/check_box/check_box.h>
#include <ui/widgets/combo_box/combo_box.h>
#include <ui/widgets/scroll_bar/scroll_bar.h>
#include <ui/widgets/text_field/text_field.h>

#include <QGridLayout>
#include <QScrollArea>
#include <QStandardItemModel>


namespace Ui {

class StageplayParametersView::Implementation
{
public:
    explicit Implementation(QWidget* _parent);


    QScrollArea* content = nullptr;

    Card* stageplayInfo = nullptr;
    QVBoxLayout* infoLayout = nullptr;
    TextField* header = nullptr;
    CheckBox* printHeaderOnTitlePage = nullptr;
    TextField* footer = nullptr;
    CheckBox* printFooterOnTitlePage = nullptr;
    CheckBox* overrideCommonSettings = nullptr;
    ComboBox* stageplayTemplate = nullptr;
    CheckBox* showBlockNumbers = nullptr;
    CheckBox* continueBlockNumbers = nullptr;
};

StageplayParametersView::Implementation::Implementation(QWidget* _parent)
    : content(new QScrollArea(_parent))
    , stageplayInfo(new Card(_parent))
    , infoLayout(new QVBoxLayout)
    , header(new TextField(stageplayInfo))
    , printHeaderOnTitlePage(new CheckBox(stageplayInfo))
    , footer(new TextField(stageplayInfo))
    , printFooterOnTitlePage(new CheckBox(stageplayInfo))
    , overrideCommonSettings(new CheckBox(stageplayInfo))
    , stageplayTemplate(new ComboBox(_parent))
    , showBlockNumbers(new CheckBox(_parent))
    , continueBlockNumbers(new CheckBox(_parent))
{
    QPalette palette;
    palette.setColor(QPalette::Base, Qt::transparent);
    palette.setColor(QPalette::Window, Qt::transparent);
    content->setPalette(palette);
    content->setFrameShape(QFrame::NoFrame);
    content->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    content->setVerticalScrollBar(new ScrollBar);

    stageplayInfo->setResizingActive(false);

    header->setSpellCheckPolicy(SpellCheckPolicy::Manual);
    footer->setSpellCheckPolicy(SpellCheckPolicy::Manual);

    stageplayTemplate->setSpellCheckPolicy(SpellCheckPolicy::Manual);
    stageplayTemplate->setModel(BusinessLayer::TemplatesFacade::stageplayTemplates());
    stageplayTemplate->hide();

    showBlockNumbers->hide();
    continueBlockNumbers->setEnabled(false);
    continueBlockNumbers->hide();

    infoLayout->setDirection(QBoxLayout::TopToBottom);
    infoLayout->setContentsMargins({});
    infoLayout->setSpacing(0);
    infoLayout->addWidget(header);
    infoLayout->addWidget(printHeaderOnTitlePage);
    infoLayout->addWidget(footer);
    infoLayout->addWidget(printFooterOnTitlePage);
    infoLayout->addWidget(overrideCommonSettings, 1, Qt::AlignTop);
    infoLayout->addWidget(stageplayTemplate);
    {
        auto layout = new QHBoxLayout;
        layout->setContentsMargins({});
        layout->setSpacing(0);
        layout->addWidget(showBlockNumbers);
        layout->addWidget(continueBlockNumbers);
        layout->addStretch();
        infoLayout->addLayout(layout);
    }
    stageplayInfo->setLayoutReimpl(infoLayout);

    //
    // TODO: С лёту не завелось, т.к. при отображении скрытых виджетов, виджеты, которые были видны,
    // сжимаются лейаутом, что даёт некрасивый эффект дёргания (собственно это актуально и для
    // диалогов, просто там это не так сильно заметно как на больших карточках).
    //
    // Во время экспериментов не помогли ни фиксация размера виджета, ни установка минимального
    // размера строки лейаута, ни разные полтики лейута, надо смотреть код лейаута и искать лазейку,
    // как заставить его не сжимать некоторые из виджетов
    //
    stageplayInfo->setResizingActive(true);

    QWidget* contentWidget = new QWidget;
    content->setWidget(contentWidget);
    content->setWidgetResizable(true);
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(stageplayInfo);
    layout->addStretch();
    contentWidget->setLayout(layout);
}


// ****


StageplayParametersView::StageplayParametersView(QWidget* _parent)
    : Widget(_parent)
    , d(new Implementation(this))
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(d->content);
    setLayout(layout);

    connect(d->header, &TextField::textChanged, this,
            [this] { emit headerChanged(d->header->text()); });
    connect(d->printHeaderOnTitlePage, &CheckBox::checkedChanged, this,
            &StageplayParametersView::printHeaderOnTitlePageChanged);
    connect(d->footer, &TextField::textChanged, this,
            [this] { emit footerChanged(d->footer->text()); });
    connect(d->printFooterOnTitlePage, &CheckBox::checkedChanged, this,
            &StageplayParametersView::printFooterOnTitlePageChanged);
    connect(d->overrideCommonSettings, &CheckBox::checkedChanged, this,
            &StageplayParametersView::overrideCommonSettingsChanged);
    connect(d->stageplayTemplate, &ComboBox::currentIndexChanged, this,
            [this](const QModelIndex& _index) {
                const auto templateId
                    = _index.data(BusinessLayer::TemplatesFacade::kTemplateIdRole).toString();
                emit stageplayTemplateChanged(templateId);
            });
    connect(d->showBlockNumbers, &CheckBox::checkedChanged, this,
            &StageplayParametersView::showBlockNumbersChanged);
    connect(d->continueBlockNumbers, &CheckBox::checkedChanged, this,
            &StageplayParametersView::continueBlockNumbersChanged);

    connect(d->overrideCommonSettings, &CheckBox::checkedChanged, this, [this](bool _checked) {
        d->stageplayTemplate->setVisible(_checked);
        d->showBlockNumbers->setVisible(_checked);
        // d->continueBlockNumbers->setVisible(_checked);
    });
    connect(d->showBlockNumbers, &CheckBox::checkedChanged, d->continueBlockNumbers,
            &CheckBox::setEnabled);

    updateTranslations();
    designSystemChangeEvent(nullptr);
}

StageplayParametersView::~StageplayParametersView() = default;

QWidget* StageplayParametersView::asQWidget()
{
    return this;
}

void StageplayParametersView::setHeader(const QString& _header)
{
    d->header->setText(_header);
}

void StageplayParametersView::setPrintHeaderOnTitlePage(bool _print)
{
    d->printHeaderOnTitlePage->setChecked(_print);
}

void StageplayParametersView::setFooter(const QString& _footer)
{
    d->footer->setText(_footer);
}

void StageplayParametersView::setPrintFooterOnTitlePage(bool _print)
{
    d->printFooterOnTitlePage->setChecked(_print);
}

void StageplayParametersView::setOverrideCommonSettings(bool _override)
{
    d->overrideCommonSettings->setChecked(_override);
}

void StageplayParametersView::setStageplayTemplate(const QString& _templateId)
{
    using namespace BusinessLayer;
    for (int row = 0; row < TemplatesFacade::stageplayTemplates()->rowCount(); ++row) {
        auto item = TemplatesFacade::stageplayTemplates()->item(row);
        if (item->data(TemplatesFacade::kTemplateIdRole).toString() != _templateId) {
            continue;
        }

        d->stageplayTemplate->setCurrentIndex(item->index());
        break;
    }
}

void StageplayParametersView::setShowBlockNumbers(bool _show)
{
    d->showBlockNumbers->setChecked(_show);
}

void StageplayParametersView::setContinueBlockNumbers(bool _continue)
{
    d->continueBlockNumbers->setChecked(_continue);
}

void StageplayParametersView::updateTranslations()
{
    d->header->setLabel(tr("Header"));
    d->printHeaderOnTitlePage->setText(tr("Print header on title page"));
    d->footer->setLabel(tr("Footer"));
    d->printFooterOnTitlePage->setText(tr("Print footer on title page"));
    d->overrideCommonSettings->setText(tr("Override common settings for this stageplay"));
    d->stageplayTemplate->setLabel(tr("Template"));
    d->showBlockNumbers->setText(tr("Show block numbers"));
    d->continueBlockNumbers->setText(tr("Continue block numbers through document"));
}

void StageplayParametersView::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    Widget::designSystemChangeEvent(_event);

    setBackgroundColor(Ui::DesignSystem::color().surface());

    d->content->widget()->layout()->setContentsMargins(
        QMarginsF(Ui::DesignSystem::layout().px24(), Ui::DesignSystem::layout().topContentMargin(),
                  Ui::DesignSystem::layout().px24(), Ui::DesignSystem::layout().px24())
            .toMargins());

    d->stageplayInfo->setBackgroundColor(DesignSystem::color().background());
    for (auto textField : std::vector<TextField*>{
             d->header,
             d->footer,
             d->stageplayTemplate,
         }) {
        textField->setBackgroundColor(Ui::DesignSystem::color().onBackground());
        textField->setTextColor(Ui::DesignSystem::color().onBackground());
    }
    for (auto combobox : {
             d->stageplayTemplate,
         }) {
        combobox->setPopupBackgroundColor(Ui::DesignSystem::color().background());
    }
    for (auto checkBox : {
             d->printHeaderOnTitlePage,
             d->printFooterOnTitlePage,
             d->overrideCommonSettings,
             d->showBlockNumbers,
             d->continueBlockNumbers,
         }) {
        checkBox->setBackgroundColor(Ui::DesignSystem::color().background());
        checkBox->setTextColor(Ui::DesignSystem::color().onBackground());
    }
    d->infoLayout->setSpacing(static_cast<int>(Ui::DesignSystem::layout().px16()));
    d->infoLayout->setContentsMargins(0, static_cast<int>(Ui::DesignSystem::layout().px24()), 0,
                                      static_cast<int>(Ui::DesignSystem::layout().px12()));
}

} // namespace Ui