cmake_minimum_required(VERSION 3.26)

#include(FetchContent)
#FetchContent_Declare(
#    vcpkg
#    GIT_REPOSITORY https://github.com/playmer/vcpkg.git
#    GIT_TAG        origin/playmer/AllTheThings
#)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    #FetchContent_Populate(vcpkg)
    
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/deps/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

project(SDL3_Qt_Example C CXX)

add_subdirectory(deps/SDL EXCLUDE_FROM_ALL)

#add_executable(SDL3_Qt_Example)

find_package(glad CONFIG REQUIRED)
find_package(qtadvanceddocking-qt6 CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

find_package(Vulkan)

#add_executable(SDL3_Qt_Example)
qt_add_executable(SDL3_Qt_Example)

target_sources(SDL3_Qt_Example
PRIVATE
    main.cpp

    Renderers/OpenGL3_3Renderer.cpp
    Renderers/OpenGL3_3Renderer.hpp
    Renderers/Renderer.cpp
    Renderers/Renderer.hpp
    
    Renderers/SdlRenderRenderer.cpp
    Renderers/SdlRenderRenderer.hpp

    Renderers/SdlGpuRenderer.cpp
    Renderers/SdlGpuRenderer.hpp
    vcpkg.json
)

target_include_directories(SDL3_Qt_Example 
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(SDL3_Qt_Example 
PRIVATE 
    SDL3::SDL3 
    Qt::Widgets
    glad::glad
    ads::qtadvanceddocking-qt6
)

if (${Vulkan_FOUND})
    find_package(vk-bootstrap CONFIG REQUIRED)
    find_package(VulkanMemoryAllocator CONFIG REQUIRED)

    target_sources(SDL3_Qt_Example
    PRIVATE
        Renderers/VkRenderer.cpp
        Renderers/VkRenderer.hpp
    )

    target_compile_definitions(SDL3_Qt_Example PUBLIC HAVE_VULKAN)

    target_include_directories(SDL3_Qt_Example PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(SDL3_Qt_Example 
    PUBLIC 
        vk-bootstrap::vk-bootstrap
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
    )
endif()

#install(TARGETS SDL3_Qt_Example
#    BUNDLE  DESTINATION .
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)
#
#qt_generate_deploy_app_script(
#    TARGET SDL3_Qt_Example
#    FILENAME_VARIABLE deploy_script
#    NO_UNSUPPORTED_PLATFORM_ERROR
#)
#install(SCRIPT ${deploy_script})

if (${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    find_package(directx-headers CONFIG REQUIRED)

    target_sources(SDL3_Qt_Example
    PRIVATE
        Renderers/Dx11Renderer.cpp
        Renderers/Dx11Renderer.hpp
        Renderers/Dx12Renderer.cpp
        Renderers/Dx12Renderer.hpp
    )

    # Qt Deployment stuff
    find_program(TOOL_WINDEPLOYQT_DEBUG NAMES windeployqt.debug.bat)
    find_program(TOOL_WINDEPLOYQT NAMES windeployqt)

    add_custom_command(TARGET SDL3_Qt_Example POST_BUILD
        COMMAND $<IF:$<CONFIG:DEBUG>,${TOOL_WINDEPLOYQT_DEBUG},${TOOL_WINDEPLOYQT}>
                $<TARGET_FILE:SDL3_Qt_Example>
        COMMENT "Running windeployqt..."
    )

    target_link_libraries(SDL3_Qt_Example
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

    #add_custom_command(TARGET SDL3_Qt_Example POST_BUILD
    #    COMMAND $<IF:$<CONFIG:DEBUG>,${TOOL_MACDEPLOYQT_DEBUG},${TOOL_MACDEPLOYQT}>
    #            $<TARGET_FILE_NAME:SDL3_Qt_Example>
    #    COMMENT "Running macdeployqt..."
    #)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
endif()
