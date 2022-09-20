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

#include "SDL2/SDL.h"

struct color
{
    Uint8 r, g, b, a;
};

class QSdlWindow : public QWindow
{
public:
    QSdlWindow() = default;
    ~QSdlWindow() override = default;

    void Initialize()
    {
        mWindowId = reinterpret_cast<void*>(winId());
        mWindow = SDL_CreateWindowFrom(mWindowId);
        mRenderer = SDL_CreateRenderer(mWindow, -1, 0);
        printf("Window {%p}, Renderer {%p}\n", mWindow, mRenderer);
    }

    void Update()
    {
        if (reinterpret_cast<void*>(winId()) != mWindowId)
        {
            SDL_DestroyRenderer(mRenderer);
            SDL_DestroyWindow(mWindow);

            mWindowId = reinterpret_cast<void*>(winId());

            mWindow = SDL_CreateWindowFrom(mWindowId);
            mRenderer = SDL_CreateRenderer(mWindow, -1, 0);
            printf("WindowId Changeed! new Window {%p}, Renderer {%p}\n", mWindow, mRenderer);
        }

        SDL_Event event;
        //Handle events on queue
        while(SDL_PollEvent(&event) != 0)
        {
            printf("Event from Window {%p}\n", mWindow);
        }

        int width, height;
        SDL_GetWindowSize(mWindow, &width, &height);

        //Clear screen
        SDL_SetRenderDrawColor(mRenderer, mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a);
        SDL_RenderClear(mRenderer);

        //Render red filled quad
        SDL_Rect fillRect = { width / 4, height / 4, width / 2, height / 2 };
        SDL_SetRenderDrawColor(mRenderer, mQuadColor.r, mQuadColor.g, mQuadColor.b, mQuadColor.a);
        SDL_RenderFillRect(mRenderer, &fillRect);
        
        SDL_Rect outlineRect = { width / 6, height / 6, width * 2 / 3, height * 2 / 3 };
        SDL_SetRenderDrawColor(mRenderer, mQuadOutlineColor.r, mQuadOutlineColor.g, mQuadOutlineColor.b, mQuadOutlineColor.a);
        SDL_RenderDrawRect(mRenderer, &outlineRect);

        //Draw blue horizontal line
        SDL_SetRenderDrawColor(mRenderer, mHorizontalLineColor.r, mHorizontalLineColor.g, mHorizontalLineColor.b, mHorizontalLineColor.a);
        SDL_RenderDrawLine(mRenderer, 0, height / 2, width, height / 2);

        //Draw vertical line of yellow dots
        SDL_SetRenderDrawColor(mRenderer, mVerticalDotsColor.r, mVerticalDotsColor.g, mVerticalDotsColor.b, mVerticalDotsColor.a);
        for( int i = 0; i < height; i += 4 )
        {
            SDL_RenderDrawPoint(mRenderer, width / 2, i);
        }

        //Update screen
        SDL_RenderPresent( mRenderer );

        printf("Window {%p}, Renderer {%p}\n", mWindow, mRenderer);
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
        //Initialize();
        requestUpdate();
    }

    void resizeEvent(QResizeEvent* aEvent) override
    {

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
    
    
    color mClearColor = {0xFF, 0xFF, 0xFF, 0xFF};
    color mQuadColor = {0xFF, 0x00, 0x00, 0xFF};
    color mQuadOutlineColor = {0x00, 0xFF, 0x00, 0xFF};
    color mHorizontalLineColor = {0x00, 0x00, 0xFF, 0xFF};
    color mVerticalDotsColor = {0xFF, 0xFF, 0x00, 0xFF};

private:
    SDL_Window* mWindow = nullptr;
    SDL_Renderer* mRenderer = nullptr;
    void* mWindowId = nullptr;
};

int main(int argc, char *argv[])
{
    SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "1");
    //SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "1");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }

    QApplication app(argc, argv);

    // Make a window. This type has support for docking and gives a 
    // central window in the middle of the docking panels that doesn't move.
    auto window = new QMainWindow;

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

    auto textEdit = new QTextEdit;
    
    centralTabs->addTab(textEdit, "File 2");

    
    {
        auto sdlWindow = new QSdlWindow();
        auto sdlWidget = QWidget::createWindowContainer(sdlWindow);
        centralTabs->addTab(sdlWidget, "SdlWindow");
        sdlWidget->setWindowTitle("SdlWindow");
        sdlWidget->setMinimumSize(480, 320);
        sdlWindow->Initialize();


    
        //QTimer::singleShot(0, [sdlWindow]()
        //{
        //    sdlWindow->Update();
        //});
    }

    {
        auto sdlWindow = new QSdlWindow();
        auto sdlWidget = QWidget::createWindowContainer(sdlWindow);
        centralTabs->addTab(sdlWidget, "SdlWindow2");
        sdlWidget->setWindowTitle("SdlWindow2");
        sdlWidget->setMinimumSize(480, 320);
        sdlWindow->mClearColor = { 0x00, 0x00, 0x00, 0xFF };
        sdlWindow->mQuadColor = { 0x00, 0x00, 0xFF, 0xFF };
        sdlWindow->Initialize();


        //QTimer::singleShot(0, [sdlWindow]()
        //{
        //    sdlWindow->Update();
        //});
    }




    window->show();
  
    return QApplication::exec();
}