#include "menu_view.h"

#include "about_application_dialog.h"

#include <ui/design_system/design_system.h>
#include <ui/widgets/drawer/drawer.h>
#include <ui/widgets/label/link_label.h>
#include <ui/widgets/scroll_bar/scroll_bar.h>
#include <utils/3rd_party/WAF/Animation/Animation.h>
#include <utils/helpers/color_helper.h>
#include <utils/logging.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QBoxLayout>
#include <QKeyEvent>
#include <QScrollArea>


namespace Ui {

class MenuView::Implementation
{
public:
    explicit Implementation(QWidget* _parent);


    QScrollArea* menuPage = nullptr;

    Drawer* drawer = nullptr;
    QAction* signIn = nullptr;
    QAction* projects = nullptr;
    QAction* createProject = nullptr;
    QAction* openProject = nullptr;
    QAction* project = nullptr;
    QAction* saveProject = nullptr;
    QAction* saveProjectAs = nullptr;
    QAction* exportCurrentDocument = nullptr;
    QAction* importProject = nullptr;
    QAction* fullScreen = nullptr;
    QAction* settings = nullptr;

    QAction* writingStatistics = nullptr;
    QAction* writingSprint = nullptr;
    QAction* chat = nullptr;
    QAction* notifications = nullptr;

    Subtitle2LinkLabel* appName = nullptr;
    Body2LinkLabel* appVersion = nullptr;
    Body2Label* aboutAppSpacer = nullptr;
    Body2LinkLabel* aboutApp = nullptr;
    QGridLayout* appInfoLayout = nullptr;
};

MenuView::Implementation::Implementation(QWidget* _parent)
    : menuPage(new QScrollArea(_parent))
    , drawer(new Drawer(_parent))
    , signIn(new QAction)
    , projects(new QAction)
    , createProject(new QAction)
    , openProject(new QAction)
    , project(new QAction)
    , saveProject(new QAction)
    , saveProjectAs(new QAction)
    , exportCurrentDocument(new QAction)
    , importProject(new QAction)
    , fullScreen(new QAction)
    , settings(new QAction)
    , writingStatistics(new QAction)
    , writingSprint(new QAction)
    , chat(new QAction)
    , notifications(new QAction)
    , appName(new Subtitle2LinkLabel(_parent))
    , appVersion(new Body2LinkLabel(_parent))
    , aboutAppSpacer(new Body2Label(_parent))
    , aboutApp(new Body2LinkLabel(_parent))
    , appInfoLayout(new QGridLayout)
{
    //
    // Настроим страницу меню
    //
    QPalette palette;
    palette.setColor(QPalette::Base, Qt::transparent);
    palette.setColor(QPalette::Window, Qt::transparent);
    menuPage->setPalette(palette);
    menuPage->setFrameShape(QFrame::NoFrame);
    menuPage->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    menuPage->setVerticalScrollBar(new ScrollBar);

    drawer->addAction(signIn);
    drawer->addAction(projects);
    drawer->addAction(createProject);
    drawer->addAction(openProject);
    drawer->addAction(project);
    drawer->addAction(saveProject);
    drawer->addAction(saveProjectAs);
    drawer->addAction(importProject);
    drawer->addAction(exportCurrentDocument);
    drawer->addAction(fullScreen);
    drawer->addAction(settings);

    drawer->setAccountActions({
        writingStatistics,
        writingSprint,
        chat,
        notifications,
    });

    signIn->setIconText(u8"\U000F0004");
    signIn->setCheckable(false);
    signIn->setVisible(false);
    //
    projects->setIconText(u8"\U000f024b");
    projects->setCheckable(true);
    projects->setChecked(true);
    //
    createProject->setIconText(u8"\U000f0415");
    createProject->setCheckable(false);
    //
    openProject->setIconText(u8"\U000f0770");
    openProject->setCheckable(false);
    //
    project->setIconText(u8"\U000f00be");
    project->setCheckable(true);
    project->setVisible(false);
    project->setSeparator(true);
    //
    saveProject->setIconText(u8"\U000f0193");
    saveProject->setCheckable(false);
    saveProject->setEnabled(false);
    saveProject->setVisible(false);
    //
    saveProjectAs->setIconText(" ");
    saveProjectAs->setCheckable(false);
    saveProjectAs->setVisible(false);
    //
    exportCurrentDocument->setIconText(u8"\U000f0207");
    exportCurrentDocument->setCheckable(false);
    exportCurrentDocument->setEnabled(false);
    exportCurrentDocument->setVisible(false);
    //
    importProject->setIconText(u8"\U000f02fa");
    importProject->setCheckable(false);
    importProject->setVisible(false);
    //
    fullScreen->setIconText(u8"\U000F0293");
    fullScreen->setCheckable(false);
    fullScreen->setVisible(false);
    fullScreen->setSeparator(true);
    //
    settings->setIconText(u8"\U000f0493");
    settings->setCheckable(false);
    settings->setVisible(true);
    settings->setSeparator(true);

    QActionGroup* actions = new QActionGroup(_parent);
    actions->addAction(projects);
    actions->addAction(project);
    actions->addAction(settings);

    writingStatistics->setIconText(u8"\U000F0127");
    writingStatistics->setVisible(false);
    //
    writingSprint->setIconText(u8"\U000F13AB");
    //
    chat->setIconText(u8"\U000F0368");
    chat->setVisible(false);
    //
    notifications->setIconText(u8"\U000F009A");
    notifications->setVisible(false);

    appName->setText("Story Architect");
    appName->setLink(QUrl("https://starc.app"));
    appVersion->setLink(QUrl("https://starc.app/blog/"));
    aboutAppSpacer->setText(" - ");

    appInfoLayout->setContentsMargins({});
    appInfoLayout->setSpacing(0);
    appInfoLayout->addWidget(appName, 0, 0, 1, 4, Qt::AlignLeft);
    appInfoLayout->addWidget(appVersion, 1, 0);
    appInfoLayout->addWidget(aboutAppSpacer, 1, 1);
    appInfoLayout->addWidget(aboutApp, 1, 2);
    appInfoLayout->setColumnStretch(3, 1);

    auto menuPageContentWidget = new QWidget;
    menuPage->setWidget(menuPageContentWidget);
    menuPage->setWidgetResizable(true);
    auto layout = new QVBoxLayout;
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(drawer);
    layout->addStretch();
    layout->addLayout(appInfoLayout);
    menuPageContentWidget->setLayout(layout);
}

// ****


MenuView::MenuView(QWidget* _parent)
    : StackWidget(_parent)
    , d(new Implementation(this))
{
#ifdef CLOUD_SERVICE_MANAGER
    d->signIn->setVisible(true);
    d->projects->setSeparator(true);
#endif
    setCurrentWidget(d->menuPage);


    connect(d->drawer, &Drawer::accountPressed, this, &MenuView::accountPressed);
    connect(d->signIn, &QAction::triggered, this, &MenuView::signInPressed);
    connect(d->projects, &QAction::triggered, this, &MenuView::projectsPressed);
    connect(d->createProject, &QAction::triggered, this, &MenuView::createProjectPressed);
    connect(d->openProject, &QAction::triggered, this, &MenuView::openProjectPressed);
    connect(d->project, &QAction::triggered, this, &MenuView::projectPressed);
    connect(d->saveProject, &QAction::triggered, this, &MenuView::saveProjectChangesPressed);
    connect(d->saveProjectAs, &QAction::triggered, this, &MenuView::saveProjectAsPressed);
    connect(d->importProject, &QAction::triggered, this, &MenuView::importPressed);
    connect(d->exportCurrentDocument, &QAction::triggered, this,
            &MenuView::exportCurrentDocumentPressed);
    connect(d->fullScreen, &QAction::triggered, this, &MenuView::fullscreenPressed);
    connect(d->settings, &QAction::triggered, this, &MenuView::settingsPressed);
    //
    connect(d->writingSprint, &QAction::triggered, this, &MenuView::writingSprintPressed);
    //
    connect(d->aboutApp, &Body2LinkLabel::clicked, this, [this] {
        auto dialog = new AboutApplicationDialog(parentWidget());
        connect(dialog, &Ui::AboutApplicationDialog::disappeared, dialog,
                &Ui::AboutApplicationDialog::deleteLater);
        dialog->showDialog();
    });

    connect(this, &MenuView::accountPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::projectsPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::createProjectPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::openProjectPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::projectPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::saveProjectAsPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::importPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::exportCurrentDocumentPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::fullscreenPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::settingsPressed, this, &MenuView::closeMenu);
    connect(this, &MenuView::helpPressed, this, &MenuView::closeMenu);
    //
    connect(this, &MenuView::writingSprintPressed, this, &MenuView::closeMenu);


    setVisible(false);


    updateTranslations();
    designSystemChangeEvent(nullptr);
}

MenuView::~MenuView() = default;

void MenuView::setAccountVisible(bool _visible)
{
    d->drawer->setAccountVisible(_visible);
}

void MenuView::setAvatar(const QPixmap& _avatar)
{
    d->drawer->setAvatar(_avatar);
}

void MenuView::setAccountName(const QString& _name)
{
    d->drawer->setAccountName(_name);
}

void MenuView::setAccountEmail(const QString& _email)
{
    d->drawer->setAccountEmail(_email);
}

void MenuView::setSignInVisible(bool _visible)
{
    d->signIn->setVisible(_visible);
}

void MenuView::checkProjects()
{
    QSignalBlocker signalBlocker(this);
    d->projects->setChecked(true);
}

void MenuView::setProjectActionsVisible(bool _visible)
{
    d->project->setVisible(_visible);
    d->saveProject->setVisible(_visible);
    d->saveProjectAs->setVisible(_visible);
    d->exportCurrentDocument->setVisible(_visible);
    d->importProject->setVisible(_visible);
    d->fullScreen->setVisible(_visible);
    d->drawer->updateGeometry();
}

void MenuView::checkProject()
{
    QSignalBlocker signalBlocker(this);
    d->project->setChecked(true);
}

void MenuView::setProjectTitle(const QString& _title)
{
    d->project->setText(_title);
}

void MenuView::checkSettings()
{
    QSignalBlocker signalBlocker(this);
    d->importProject->setChecked(true);
}

void MenuView::markChangesSaved(bool _saved)
{
    d->saveProject->setEnabled(!_saved);
    d->saveProject->setText(_saved ? tr("All changes saved") : tr("Save changes"));
    update();
}

void MenuView::setCurrentDocumentExportAvailable(bool _available)
{
    d->exportCurrentDocument->setEnabled(_available);
}

void MenuView::closeMenu()
{
    Log::info("Hide menu");
    WAF::Animation::sideSlideOut(this);
}

QSize MenuView::sizeHint() const
{
    return d->drawer->sizeHint();
}

void MenuView::updateTranslations()
{
    d->signIn->setText(tr("Sign in"));
    d->projects->setText(tr("Stories"));
    d->createProject->setText(tr("Create story"));
    d->openProject->setText(tr("Open story"));
    d->saveProject->setText(d->saveProject->isEnabled() ? tr("Save changes")
                                                        : tr("All changes saved"));
    d->saveProject->setWhatsThis(
        QKeySequence(QKeySequence::Save).toString(QKeySequence::NativeText));
    d->saveProjectAs->setText(tr("Save current story as..."));
    d->importProject->setText(tr("Import..."));
    d->importProject->setWhatsThis(QKeySequence("Alt+I").toString(QKeySequence::NativeText));
    d->exportCurrentDocument->setText(tr("Export current document..."));
    d->exportCurrentDocument->setWhatsThis(
        QKeySequence("Alt+E").toString(QKeySequence::NativeText));
    d->fullScreen->setText(tr("Toggle full screen"));
    d->fullScreen->setWhatsThis(
        QKeySequence(QKeySequence::FullScreen).toString(QKeySequence::NativeText));
    d->settings->setText(tr("Application settings"));

    d->writingSprint->setToolTip(tr("Show writing sprint timer"));

    d->appVersion->setText(
        QString("%1 %2").arg(tr("Version"), QCoreApplication::applicationVersion()));
    d->aboutApp->setText(tr("About"));
}

void MenuView::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    StackWidget::designSystemChangeEvent(_event);

    setBackgroundColor(Ui::DesignSystem::color().primary());

    for (auto label : std::vector<Widget*>{
             d->appName,
             d->appVersion,
             d->aboutAppSpacer,
             d->aboutApp,
         }) {
        label->setBackgroundColor(Ui::DesignSystem::color().primary());
        label->setTextColor(ColorHelper::transparent(Ui::DesignSystem::color().onPrimary(),
                                                     Ui::DesignSystem::disabledTextOpacity()));
    }
    d->appInfoLayout->setContentsMargins(
        Ui::DesignSystem::layout().px16(), Ui::DesignSystem::layout().px24(),
        Ui::DesignSystem::layout().px16(), Ui::DesignSystem::layout().px16());
    d->appInfoLayout->setVerticalSpacing(Ui::DesignSystem::layout().px8());
}

void MenuView::keyPressEvent(QKeyEvent* _event)
{
    if (_event->key() == Qt::Key_Escape) {
        closeMenu();
    }

    StackWidget::keyPressEvent(_event);
}

} // namespace Ui
