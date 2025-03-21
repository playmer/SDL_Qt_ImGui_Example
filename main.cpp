#include "QApplication"
#include "QLabel"
#include "QTreeView"
#include "QTabWidget"
#include "QFileSystemModel"
#include "QtGui/QStandardItemModel.h"

#include "QMainWindow"
#include "QDockWidget"
#include "QTextEdit"
#include "QMenuBar"
#include "QToolBar"
#include "QWindow"
#include "QTimer"
#include "QResizeEvent"

#include "SDL3/SDL.h"

#include "Renderers/Renderer.hpp"

#include "DockManager.h"

class DockOwningMainWindow : public QMainWindow
{
public:

    explicit DockOwningMainWindow(QWidget* parent = nullptr) :
        QMainWindow(parent)
    {
        // Create the dock manager after the ui is setup. Because the
        // parent parameter is a QMainWindow the dock manager registers
        // itself as the central widget as such the ui must be set up first.
        mDockManager = new ads::CDockManager(this);

        //// Create example content label - this can be any application specific
        //// widget
        //QLabel* l = new QLabel();
        //l->setWordWrap(true);
        //l->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        //l->setText("Lorem ipsum dolor sit amet, consectetuer adipiscing elit. ");
        //
        //// Create a dock widget with the title Label 1 and set the created label
        //// as the dock widget content
        //ads::CDockWidget* DockWidget = new ads::CDockWidget("Label 1");
        //DockWidget->setWidget(l);

        // Add the toggleViewAction of the dock widget to the menu to give
        // the user the possibility to show the dock widget if it has been closed
        //ui->menuView->addAction(DockWidget->toggleViewAction());

        // Add the dock widget to the top dock widget area
        //mDockManager->addDockWidget(ads::TopDockWidgetArea, DockWidget);
    }

    ~DockOwningMainWindow()
    {
    }

    ads::CDockManager* GetDockManager()
    {
        return mDockManager;
    }

private:

    // The main container for docking
    ads::CDockManager* mDockManager;
};


    
class QSdlWindow : public QWindow
{
public:
    QSdlWindow(RendererType aType, const char* aRendererBackend)
        : mType{ aType }
        , mRendererBackend{ aRendererBackend }
    {
    }

    ~QSdlWindow() override = default;
    

    // GL Stuff, needs to be factored out.

    void Initialize()
    {
        SDL_PropertiesID window_props = SDL_CreateProperties();

        if ((RendererType::VkRenderer == mType) || 
            ((RendererType::SdlRenderRenderer == mType) && (strcmp(mRendererBackend, "vulkan") == 0)))
        {
#ifdef HAVE_VULKAN
            SDL_SetBooleanProperty(window_props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
#endif
            SDL_SetBooleanProperty(window_props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, false);
            setSurfaceType(QSurface::VulkanSurface);
        }
        else if ((RendererType::OpenGL3_3Renderer == mType)
            || (RendererType::SdlRenderRenderer == mType) && (
            (strcmp(mRendererBackend, "opengles2") == 0)
            || (strcmp(mRendererBackend, "opengl") == 0)))
        {
            SDL_SetBooleanProperty(window_props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);

#ifdef HAVE_VULKAN
            SDL_SetBooleanProperty(window_props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, false);
#endif
            setSurfaceType(QSurface::OpenGLSurface);
        }
        else if (RendererType::Dx12Renderer == mType
            || RendererType::Dx11Renderer == mType)
        {
            setSurfaceType(QSurface::Direct3DSurface);
        }

        mWindowId = reinterpret_cast<void*>(winId());

        SDL_SetPointerProperty(window_props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, mWindowId);
        mWindow = SDL_CreateWindowWithProperties(window_props);
        mRenderer = CreateRenderer(mWindow, mType, mRendererBackend);

        requestUpdate();
    }

    void Update()
    {
        mRenderer->Update();
        requestUpdate();
    }


    bool event(QEvent* event) override
    {
        if (event->type() == QEvent::UpdateRequest)
        {
            Update();
            return false;
        }
        else 
        {
            return QWindow::event(event);
        }
    }

    void exposeEvent(QExposeEvent*) override
    {
        requestUpdate();
    }

    void resizeEvent(QResizeEvent* aEvent) override
    {
        printf("ResizeEvent: {%d, %d}\n",aEvent->size().width(), aEvent->size().height());
        aEvent->accept();

        if (mRenderer)
        {
            mRenderer->Resize(aEvent->size().width(), aEvent->size().height());
        }
    }

    void keyPressEvent(QKeyEvent* aEvent) override
    {

    }

    void focusInEvent(QFocusEvent*) override
    {

    }

    void focusOutEvent(QFocusEvent*) override
    {

    }

    Renderer* GetRenderer()
    {
        return mRenderer.get();
    }

private:
    SDL_Window* mWindow = nullptr;
    void* mWindowId = nullptr;
    std::unique_ptr<Renderer> mRenderer;
    RendererType mType;
    const char* mRendererBackend;
};


void sdl_event_loop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {

    }
    
    QTimer::singleShot(0, []()
    {
        sdl_event_loop();
    });
}

void createSdlWindow(DockOwningMainWindow* aMainWindow, RendererType aType, const char* aRendererBackend, ads::DockWidgetArea aArea, color aClearColor)
{
    auto sdlWindow = new QSdlWindow(aType, aRendererBackend);
    auto dockWidget = new ads::CDockWidget("", aMainWindow);
    dockWidget->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromContent);
    //dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto sdlWidget = QWidget::createWindowContainer(sdlWindow);
    dockWidget->setWidget(sdlWidget);

    aMainWindow->GetDockManager()->addDockWidget(aArea, dockWidget);
    sdlWidget->setMinimumSize(10, 10);
    sdlWidget->setBaseSize(480, 320);
    sdlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sdlWindow->Initialize();
    sdlWindow->GetRenderer()->mClearColor = aClearColor;
    sdlWindow->GetRenderer()->mTriangleColor = { 0x00, 0x00, 0xFF, 0xFF };
    sdlWidget->setWindowTitle(sdlWindow->GetRenderer()->Name());
    dockWidget->setWindowTitle(sdlWindow->GetRenderer()->Name());
}

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }

    QApplication app(argc, argv);

    // Make a window. This type has support for docking and gives a 
    // central window in the middle of the docking panels that doesn't move.
    auto window = new DockOwningMainWindow;
    //window->setCentralWidget(nullptr);
    //auto centralWidget = new QWidget();
    //window->setCentralWidget(centralWidget);
    //window->centralWidget()->setBaseSize(0, 0);
    //window->centralWidget()->setMinimumSize(0, 0);
    //window->centralWidget()->setMaximumSize(0,0);
    //window->centralWidget()->hide();

    // Enables "infinite docking".
    window->setDockNestingEnabled(true);

    // Sets the default window size.
    window->resize(850, 700);

    //// Used to store widgets in the central widget.
    //auto centralTabs = new QTabWidget;
    //window->setCentralWidget(centralTabs);
    //
    //// Adding a menuTab layer for menu options.
    //auto menuTabs = new QMenuBar(centralTabs);
    //window->setMenuBar(menuTabs);
    //
    //// Adding a toolTab layer for tools options.
    //auto toolTabs = new QToolBar(centralTabs);
    //window->addToolBar(toolTabs);
    //
    //// Actions that will handle the events from our buttons.
    //toolTabs->addAction("Play");
    //toolTabs->addAction("Pause");
    //toolTabs->addAction("Stop");
    //
    //// You can move the tabs around.
    //centralTabs->setMovable(true);
    //// Tabs get close buttons.
    //centralTabs->setTabsClosable(true);
    //// When there are two many tabs, this will make buttons to scroll 
    //// through them.
    //centralTabs->setUsesScrollButtons(true);
    //
    //// Add tab options for the menu bar layer.
    //auto fileMenu = new QMenu("File");
    //fileMenu->addMenu(new QMenu("Open"));
    //
    //menuTabs->addMenu(fileMenu);
    
    //{
    //    auto dockWidget = new QDockWidget("Text Edit", window);
    //    auto textEdit = new QTextEdit;
    //    dockWidget->setWidget(textEdit);
    //    window->addDockWidget(Qt::TopDockWidgetArea, dockWidget);
    //}

    //#if WIN32
    //    createSdlWindow(window, RendererType::Dx11Renderer, nullptr, ads::TopDockWidgetArea, {0x00, 0xFF, 0x00, 0xFF});
    //    createSdlWindow(window, RendererType::Dx12Renderer, nullptr, ads::BottomDockWidgetArea, { 0xFF, 0x00, 0xFF, 0xFF });
    //#endif // WIN32
    //
    //createSdlWindow(window, RendererType::VkRenderer, nullptr, ads::LeftDockWidgetArea, { 0x00, 0x00, 0xFF, 0xFF });
    //createSdlWindow(window, RendererType::OpenGL3_3Renderer, nullptr, ads::RightDockWidgetArea, { 0xFF, 0x00, 0x00, 0xFF });

#if WIN32
    createSdlWindow(window, RendererType::Dx11Renderer, nullptr, ads::TopDockWidgetArea, { 0x00, 0xFF, 0x00, 0xFF });
    createSdlWindow(window, RendererType::Dx12Renderer, nullptr, ads::TopDockWidgetArea, { 0xFF, 0x00, 0xFF, 0xFF });
#endif // WIN32

    createSdlWindow(window, RendererType::VkRenderer, nullptr, ads::TopDockWidgetArea, { 0x00, 0x00, 0xFF, 0xFF });
    createSdlWindow(window, RendererType::OpenGL3_3Renderer, nullptr, ads::TopDockWidgetArea, { 0xFF, 0x00, 0x00, 0xFF });

    auto drivers = SDL_GetNumRenderDrivers();

    printf("Drivers Start\n;");
    for (size_t i = 0; i < drivers; ++i) {
        printf("%s\n", SDL_GetRenderDriver(i));
    }
    printf("Drivers End\n;");

    for (size_t i = 0; i < drivers; ++i) {
        createSdlWindow(window, RendererType::SdlRenderRenderer, SDL_GetRenderDriver(i), ads::BottomDockWidgetArea, {0x00, 0x00, 0x00, 0xFF});
    }

    QTimer::singleShot(0, []()
    {
        sdl_event_loop();
    });

    window->show();
  
    return QApplication::exec();
}
