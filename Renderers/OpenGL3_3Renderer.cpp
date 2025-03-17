#include <cstdio>

#define SDL_FUNCTION_POINTER_IS_VOID_POINTER
#include "SDL3/SDL.h"

#include "glad/glad.h"

#include "Renderers/Renderer.hpp"
#include "Renderers/OpenGL3_3Renderer.hpp"


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


static void APIENTRY messageCallback(GLenum source,
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


std::unique_ptr<Renderer> CreateOpenGL3_3Renderer(SDL_Window* aWindow)
{
    return std::unique_ptr<Renderer>(new OpenGL3_3Renderer(aWindow));
}

OpenGL3_3Renderer::OpenGL3_3Renderer(SDL_Window* aWindow)
	: Renderer{ aWindow }
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * TriangleVerts.size(), TriangleVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  
}

void OpenGL3_3Renderer::Initialize()
{
}

void OpenGL3_3Renderer::Update()
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
}

void OpenGL3_3Renderer::Resize(unsigned int aWidth, unsigned int aHeight)
{

}
