cmake_minimum_required(VERSION 3.15)

include(FetchContent)
FetchContent_Declare(
    vcpkg
    GIT_REPOSITORY https://github.com/playmer/vcpkg.git
    GIT_TAG        origin/playmer/AllTheThings
)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    FetchContent_Populate(vcpkg)
    set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
endif()

project(SDL_Qt_ImGui_Example C CXX)

#add_executable(SDL_Qt_ImGui_Example)

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(qtadvanceddocking CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

find_package(Vulkan)

#add_executable(SDL_Qt_ImGui_Example)
qt_add_executable(SDL_Qt_ImGui_Example)

target_sources(SDL_Qt_ImGui_Example
PRIVATE
    main.cpp

    Renderers/OpenGL3_3Renderer.cpp
    Renderers/OpenGL3_3Renderer.hpp
    Renderers/Renderer.cpp
    Renderers/Renderer.hpp

    vcpkg.json
)

target_include_directories(SDL_Qt_ImGui_Example 
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(SDL_Qt_ImGui_Example 
PRIVATE 
    SDL2::SDL2 
    SDL2::SDL2main
    imgui::imgui
    imguizmo::imguizmo
    Qt::Widgets
    glad::glad
    ads::qtadvanceddocking
)

if (${Vulkan_FOUND})
    find_package(vk-bootstrap CONFIG REQUIRED)
    find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)

    target_sources(SDL_Qt_ImGui_Example
    PRIVATE
        Renderers/VkRenderer.cpp
        Renderers/VkRenderer.hpp
    )

    target_compile_definitions(SDL_Qt_ImGui_Example PUBLIC HAVE_VULKAN)

    target_include_directories(SDL_Qt_ImGui_Example PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(SDL_Qt_ImGui_Example 
    PUBLIC 
        vk-bootstrap::vk-bootstrap
        Vulkan::Vulkan
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
    )
endif()

#install(TARGETS SDL_Qt_ImGui_Example
#    BUNDLE  DESTINATION .
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)
#
#qt_generate_deploy_app_script(
#    TARGET SDL_Qt_ImGui_Example
#    FILENAME_VARIABLE deploy_script
#    NO_UNSUPPORTED_PLATFORM_ERROR
#)
#install(SCRIPT ${deploy_script})

if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    find_package(directx-headers CONFIG REQUIRED)

    target_sources(SDL_Qt_ImGui_Example
    PRIVATE
        Renderers/Dx11Renderer.cpp
        Renderers/Dx11Renderer.hpp
        Renderers/Dx12Renderer.cpp
        Renderers/Dx12Renderer.hpp
    )

    # Qt Deployment stuff
    find_program(TOOL_WINDEPLOYQT_DEBUG NAMES windeployqt.debug.bat)
    find_program(TOOL_WINDEPLOYQT NAMES windeployqt)

    add_custom_command(TARGET SDL_Qt_ImGui_Example POST_BUILD
        COMMAND $<IF:$<CONFIG:DEBUG>,${TOOL_WINDEPLOYQT_DEBUG},${TOOL_WINDEPLOYQT}>
                $<TARGET_FILE:SDL_Qt_ImGui_Example>
        COMMENT "Running windeployqt..."
    )

    target_link_libraries(SDL_Qt_ImGui_Example
    PRIVATE
        Microsoft::DirectX-Headers
        d3d12.lib
        dxgi.lib
        dxguid.lib
        d3dcompiler.lib
        Winmm.lib
    )

elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    find_program(TOOL_MACDEPLOYQT_DEBUG NAMES macdeployqt.debug.bat)
    find_program(TOOL_MACDEPLOYQT NAMES macdeployqt)

    #add_custom_command(TARGET SDL_Qt_ImGui_Example POST_BUILD
    #    COMMAND $<IF:$<CONFIG:DEBUG>,${TOOL_MACDEPLOYQT_DEBUG},${TOOL_MACDEPLOYQT}>
    #            $<TARGET_FILE_NAME:SDL_Qt_ImGui_Example>
    #    COMMENT "Running macdeployqt..."
    #)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
endif()