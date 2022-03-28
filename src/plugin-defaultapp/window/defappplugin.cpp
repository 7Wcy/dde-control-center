#include "defappdetailwidget.h"
#include "defappplugin.h"
#include "defappmodel.h"
#include "defappworker.h"
#include "addbuttonwidget.h"
#include <QApplication>

#include <DFloatingButton>
#include <DStyle>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

struct DATE {
    QString name = "";
    QString displayName = "";
    QString icon = "";
    DefAppWorker::DefaultAppsCategory category;
    DATE(DefAppWorker::DefaultAppsCategory _category, const QString &_name, const QString &_displayName, const QString _icon)
        : category(_category)
        , name(_name)
        , displayName(_displayName)
        , icon(_icon)
    {}
};

QString DefAppPlugin::name() const
{
    return QStringLiteral("Default Applications");
}

ModuleObject *DefAppPlugin::module()
{
    QList<DATE> moduleInfo = {
        DATE(DefAppWorker::DefaultAppsCategory::Browser, "Webpage","Webpage", "dcc_browser"),
        DATE(DefAppWorker::DefaultAppsCategory::Mail, "Mail","Mail", "dcc_mail"),
        DATE(DefAppWorker::DefaultAppsCategory::Text, "Text","Text", "dcc_text"),
        DATE(DefAppWorker::DefaultAppsCategory::Music, "Music","Music","dcc_music"),
        DATE(DefAppWorker::DefaultAppsCategory::Video, "Video","Video","dcc_video"),
        DATE(DefAppWorker::DefaultAppsCategory::Picture, "Picture","Picture","dcc_photo"),
        DATE(DefAppWorker::DefaultAppsCategory::Terminal, "Terminal","Terminal", "dcc_terminal"),
    };
    // 一级页面
    DefAppModule *moduleRoot = new DefAppModule;
    moduleRoot->setChildType(ModuleObject::ChildType::VList);

    for (DATE iter : moduleInfo) {
        // 二级按钮页
        DefAppsButtonModule *moduleDefaultApps = new DefAppsButtonModule(iter.category, iter.name, iter.displayName,
                                                                         QIcon::fromTheme(QString(iter.icon)),
                                                                         moduleRoot->model(), moduleRoot->work());

        moduleDefaultApps->setChildType(ModuleObject::ChildType::Page);

        // 三级页面
        DefappDetailModule *defappDetail = new DefappDetailModule(iter.category, moduleRoot->model(), moduleRoot->work());
        defappDetail->setChildType(ModuleObject::ChildType::Page);
        moduleDefaultApps->appendChild(defappDetail);

        connect(moduleDefaultApps, &DefAppsButtonModule::onButtonClicked, moduleDefaultApps, [moduleDefaultApps] {
            moduleDefaultApps->activeChild(0);
        });
        moduleRoot->appendChild(moduleDefaultApps);
    }
    return moduleRoot;
}

DefAppModule::DefAppModule(QObject *parent)
    : ModuleObject("defapp", tr("Default Applications"), tr("Default Applications"), QIcon::fromTheme("dcc_nav_defapp"), parent)
    , m_model(new DefAppModel(this))
    , m_work(new DefAppWorker(m_model, this))
    , m_defApps(nullptr)
{
    m_work->moveToThread(qApp->thread());
    m_model->moveToThread(qApp->thread());
    m_work->onGetListApps();
}

DefAppModule::~DefAppModule() {
    m_model->deleteLater();
    m_work->deleteLater();
}

DefAppsButtonModule::DefAppsButtonModule(DefAppWorker::DefaultAppsCategory category, const QString &name, const QString &displayName, const QIcon &icon,  DefAppModel *model, DefAppWorker *work)
    : ModuleObject(name, displayName, icon)
    , m_category(category)
    , m_model(model)
    , m_work(work)
{

}

DefAppsButtonModule::~DefAppsButtonModule()
{
}

QWidget *DefAppsButtonModule::page(){
    DefappDetailWidget *defDetail = new DefappDetailWidget(m_category);
    defDetail->setModel(m_model);

    return defDetail;
}

QWidget *DefAppsButtonModule::extraButton()
{
    AddButtonWidget *button = new AddButtonWidget(m_category);
    button->setModel(m_model);
    connect(button, &AddButtonWidget::requestCreateFile, m_work, &DefAppWorker::onCreateFile);
    return button;
}


// 三级页面
DefappDetailModule::DefappDetailModule(DefAppWorker::DefaultAppsCategory category, DefAppModel *model, DefAppWorker *work)
    : ModuleObject(nullptr)
{
    m_category = category;
    m_model = model;
    m_work = work;
}

QWidget *DefappDetailModule::page()
{
    DefappDetailWidget *defDetail = new DefappDetailWidget(m_category);
    defDetail->setModel(m_model);
    //设置默认程序
    connect(defDetail, &DefappDetailWidget::requestSetDefaultApp, m_work, &DefAppWorker::onSetDefaultApp);
    connect(defDetail, &DefappDetailWidget::requestDelUserApp, m_work, &DefAppWorker::onDelUserApp);
    return defDetail;
}
