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

#include "SDL2/SDL.h"

#include "imgui.h"


#include "Renderers/Renderer.hpp"

constexpr RendererType cChosenRenderer = RendererType::VkRenderer;
    
class QSdlWindow : public QWindow
{
public:
    QSdlWindow() = default;
    ~QSdlWindow() override = default;
    

    // GL Stuff, needs to be factored out.

    void Initialize(RendererType aType)
    {
        if (RendererType::VkRenderer == aType)
        {
            #ifdef HAVE_VULKAN
                SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "1");
            #endif
                
            SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "0");
        }
        else if (RendererType::OpenGL3_3Renderer == aType)
        {
            SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "1");

            #ifdef HAVE_VULKAN
                SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "0");
            #endif
        }


        mWindowId = reinterpret_cast<void*>(winId());
        mWindow = SDL_CreateWindowFrom(mWindowId);

        mRenderer = CreateRenderer(aType, mWindow);

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

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }

    QApplication app(argc, argv);

    // Make a window. This type has support for docking and gives a 
    // central window in the middle of the docking panels that doesn't move.
    auto window = new QMainWindow;
    //window->setCentralWidget(nullptr);
    auto centralWidget = new QWidget();
    centralWidget->setMaximumWidth(0);
    centralWidget->setMaximumHeight(0);
    window->setCentralWidget(centralWidget);
    window->centralWidget()->hide();

    // Enables "infinite docking".
    window->setDockNestingEnabled(true);

    // Sets the default window size.
    window->resize(850, 700);

    // Used to store widgets in the central widget.
    auto centralTabs = new QTabWidget;
    window->setCentralWidget(centralTabs);
    
    // Adding a menuTab layer for menu options.
    auto menuTabs = new QMenuBar(centralTabs);
    window->setMenuBar(menuTabs);
    
    // Adding a toolTab layer for tools options.
    auto toolTabs = new QToolBar(centralTabs);
    window->addToolBar(toolTabs);
    
    // Actions that will handle the events from our buttons.
    toolTabs->addAction("Play");
    toolTabs->addAction("Pause");
    toolTabs->addAction("Stop");
    
    // You can move the tabs around.
    centralTabs->setMovable(true);
    // Tabs get close buttons.
    centralTabs->setTabsClosable(true);
    // When there are two many tabs, this will make buttons to scroll 
    // through them.
    centralTabs->setUsesScrollButtons(true);
    
    // Add tab options for the menu bar layer.
    auto fileMenu = new QMenu("File");
    fileMenu->addMenu(new QMenu("Open"));
    
    menuTabs->addMenu(fileMenu);
    
    {
        auto dockWidget = new QDockWidget("Text Edit", window);
        auto textEdit = new QTextEdit;
        dockWidget->setWidget(textEdit);
        window->addDockWidget(Qt::TopDockWidgetArea, dockWidget);
    }

    {
        auto sdlWindow = new QSdlWindow();
        auto dockWidget = new QDockWidget("SdlWindow1", window);
        auto sdlWidget = QWidget::createWindowContainer(sdlWindow);
        dockWidget->setWidget(sdlWidget);

        window->addDockWidget(Qt::TopDockWidgetArea, dockWidget);
        sdlWidget->setWindowTitle("SdlWindow1");
        sdlWidget->setMinimumSize(480, 320);
        sdlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sdlWindow->Initialize(cChosenRenderer);
        sdlWindow->GetRenderer()->mClearColor = { 0x00, 0x00, 0xFF, 0xFF };
        sdlWindow->GetRenderer()->mTriangleColor = { 0x00, 0x00, 0xFF, 0xFF };
    }

    {
        auto sdlWindow = new QSdlWindow();
        auto dockWidget = new QDockWidget("SdlWindow2", window);
        auto sdlWidget = QWidget::createWindowContainer(sdlWindow);
        dockWidget->setWidget(sdlWidget);

        window->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);
        sdlWidget->setWindowTitle("SdlWindow2");
        sdlWidget->setMinimumSize(480, 320);
        sdlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sdlWindow->Initialize(cChosenRenderer);
        sdlWindow->GetRenderer()->mClearColor = { 0xFF, 0x00, 0x00, 0xFF };
        sdlWindow->GetRenderer()->mTriangleColor = { 0x00, 0x00, 0xFF, 0xFF };
    }

    QTimer::singleShot(0, []()
    {
        sdl_event_loop();
    });

    window->show();
  
    return QApplication::exec();
}