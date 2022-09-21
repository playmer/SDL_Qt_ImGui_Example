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

#include "glad/glad.h"



  static char const* Source(GLenum source)
  {
    switch (source)
    {
    case GL_DEBUG_SOURCE_API: return "DEBUG_SOURCE_API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "DEBUG_SOURCE_WINDOW_SYSTEM";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "DEBUG_SOURCE_SHADER_COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY: return "DEBUG_SOURCE_THIRD_PARTY";
    case GL_DEBUG_SOURCE_APPLICATION: return "DEBUG_SOURCE_APPLICATION";
    case GL_DEBUG_SOURCE_OTHER: return "DEBUG_SOURCE_OTHER";
    default: return "unknown";
    }
  }

  static char const* Severity(GLenum severity)
  {
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH: return "DEBUG_SEVERITY_HIGH";
    case GL_DEBUG_SEVERITY_MEDIUM: return "DEBUG_SEVERITY_MEDIUM";
    case GL_DEBUG_SEVERITY_LOW: return "DEBUG_SEVERITY_LOW";
    case GL_DEBUG_SEVERITY_NOTIFICATION: return "DEBUG_SEVERITY_NOTIFICATION";
    default: return "unknown";
    }
  }


  static char const* Type(GLenum type)
  {
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR: return "DEBUG_TYPE_ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEBUG_TYPE_DEPRECATED_BEHAVIOR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "DEBUG_TYPE_UNDEFINED_BEHAVIOR";
    case GL_DEBUG_TYPE_PORTABILITY: return "DEBUG_TYPE_PORTABILITY";
    case GL_DEBUG_TYPE_PERFORMANCE: return "DEBUG_TYPE_PERFORMANCE";
    case GL_DEBUG_TYPE_MARKER: return "DEBUG_TYPE_MARKER";
    case GL_DEBUG_TYPE_PUSH_GROUP: return "DEBUG_TYPE_PUSH_GROUP";
    case GL_DEBUG_TYPE_POP_GROUP: return "DEBUG_TYPE_POP_GROUP";
    case GL_DEBUG_TYPE_OTHER: return "DEBUG_TYPE_OTHER";
    default: return "unknown";
    }
  }

  static
    void APIENTRY messageCallback(GLenum source,
      GLenum type,
      GLuint id,
      GLenum severity,
      GLsizei length,
      const GLchar* message,
      const void* userParam)
  {
    if (GL_DEBUG_SEVERITY_NOTIFICATION == severity)
    {
      return;
    }

    printf("GL DEBUG CALLBACK:\n    Source = %s\n    type = %s\n    severity = %s\n    message = %s\n",
      Source(source),
      Type(type),
      Severity(severity),
      message);
  }


struct color
{
    Uint8 r, g, b, a;
};

    
const float vertices[] = {
    -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
};  

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "} \0";

class QSdlWindow : public QWindow
{
public:
    QSdlWindow() = default;
    ~QSdlWindow() override = default;
    

    // GL Stuff, needs to be factored out.
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;
    unsigned int VAO;
    unsigned int VBO;

    void Initialize()
    {
        mWindowId = reinterpret_cast<void*>(winId());
        mWindow = SDL_CreateWindowFrom(mWindowId);
        printf("Window {%p}\n", mWindow);

        mGlContext = SDL_GL_CreateContext(mWindow);
        SDL_GL_MakeCurrent(mWindow, mGlContext);
        
        gladLoadGLLoader(SDL_GL_GetProcAddress);

        
        glEnable(GL_DEBUG_OUTPUT);

        // FIXME: This doesn't work on Apple when I tested it, need to look into this more on 
        // other platforms, and maybe only enable it in dev builds.
        #if defined(_WIN32)
            glDebugMessageCallback(messageCallback, this);
        #endif
        
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        
        glUseProgram(shaderProgram);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);  
        
        glGenVertexArrays(1, &VAO);  
        
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);  

        requestUpdate();
    }

    void Update()
    {
        SDL_GL_MakeCurrent(mWindow, mGlContext);
        gladLoadGLLoader(SDL_GL_GetProcAddress);

        // Rendering
        int width, height;
        SDL_GetWindowSize(mWindow, &width, &height);

        glViewport(0, 0, width, height);
        glClearColor(mClearColor.r * mClearColor.a, mClearColor.g * mClearColor.a, mClearColor.b * mClearColor.a, mClearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(mWindow);

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
        //Initialize();
        requestUpdate();
    }

    void resizeEvent(QResizeEvent* aEvent) override
    {
        printf("ResizeEvent: {%d, %d}\n",aEvent->size().width(), aEvent->size().height());
        aEvent->accept();
        //printf("Window {%p}\n", aEvent->accept());
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
    
    
    color mClearColor = {0x00, 0x00, 0xFF, 0xFF};
    color mQuadColor = {0xFF, 0x00, 0x00, 0xFF};
    color mQuadOutlineColor = {0x00, 0xFF, 0x00, 0xFF};
    color mHorizontalLineColor = {0x00, 0x00, 0xFF, 0xFF};
    color mVerticalDotsColor = {0xFF, 0xFF, 0x00, 0xFF};

private:
    SDL_Window* mWindow = nullptr;
    void* mWindowId = nullptr;
    SDL_GLContext mGlContext;
};

void openglinit()
{
    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.3 + GLSL 130
    const char* glsl_version = "#version 330";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

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
    SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_OPENGL, "1");
    //SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "1");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }

    openglinit();

    QApplication app(argc, argv);

    // Make a window. This type has support for docking and gives a 
    // central window in the middle of the docking panels that doesn't move.
    auto window = new QMainWindow;

    //QDockWidget *dockWidget = new QDockWidget(window);
    //dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea |
    //                    Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //
    //window->setCentralWidget(dockWidget);

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
        window->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    
    //{
    //    auto sdlWindow = new QSdlWindow();
    //    auto sdlWidget = QWidget::createWindowContainer(sdlWindow);
    //
    //    window->addDockWidget(Qt::LeftDockWidgetArea, new QDockWidget("SdlWindow", sdlWidget));
    //    sdlWidget->setWindowTitle("SdlWindow");
    //    sdlWidget->setMinimumSize(480, 320);
    //    sdlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //    sdlWindow->mClearColor = { 0x00, 0x00, 0xFF, 0xFF };
    //    sdlWindow->mQuadColor = { 0x00, 0x00, 0xFF, 0xFF };
    //    sdlWindow->Initialize();
    //}
    {
        auto sdlWindow = new QSdlWindow();
        auto dockWidget = new QDockWidget("SdlWindow1", window);
        auto sdlWidget = QWidget::createWindowContainer(sdlWindow);
        dockWidget->setWidget(sdlWidget);

        window->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
        sdlWidget->setWindowTitle("SdlWindow1");
        sdlWidget->setMinimumSize(480, 320);
        sdlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sdlWindow->mClearColor = { 0x00, 0x00, 0xFF, 0xFF };
        sdlWindow->mQuadColor = { 0x00, 0x00, 0xFF, 0xFF };
        sdlWindow->Initialize();
    }

    {
        auto sdlWindow = new QSdlWindow();
        auto dockWidget = new QDockWidget("SdlWindow2", window);
        auto sdlWidget = QWidget::createWindowContainer(sdlWindow);
        dockWidget->setWidget(sdlWidget);

        window->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
        sdlWidget->setWindowTitle("SdlWindow2");
        sdlWidget->setMinimumSize(480, 320);
        sdlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sdlWindow->mClearColor = { 0xFF, 0x00, 0x00, 0xFF };
        sdlWindow->mQuadColor = { 0x00, 0x00, 0xFF, 0xFF };
        sdlWindow->Initialize();
    }

    QTimer::singleShot(0, []()
    {
        sdl_event_loop();
    });

    window->show();
  
    return QApplication::exec();
}