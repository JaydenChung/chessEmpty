cmake_minimum_required(VERSION 3.2.0)
project(gameboard VERSION 0.1.0 LANGUAGES C CXX)

# check for macOS
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOS TRUE)
endif()

# for filesystem functionality from C++20
set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

if(MACOS)
    find_package(glfw3 REQUIRED)
    include_directories(${GLFW_INCLUDE_DIRS})
endif()

include(CTest)
enable_testing()

if(MACOS)
    set(MAIN_FILE "main_macos.cpp")
    set(IMPL_FILE "imgui/imgui_impl_glfw.cpp")
else()
    set(MAIN_FILE "main_win32.cpp")
    set(IMPL_FILE "imgui/imgui_impl_win32.cpp")
endif()

add_executable(gameboard Application.cpp
                          imgui/imgui_demo.cpp
                          imgui/imgui_draw.cpp
                          imgui/imgui_tables.cpp
                          imgui/imgui_widgets.cpp
                          imgui/imgui.cpp
                          imgui/imgui_impl_opengl3.cpp
                          classes/Bit.cpp
                          classes/BitHolder.cpp
                          classes/Game.cpp
                          classes/Sprite.cpp
                          classes/Square.cpp
                          classes/ChessSquare.cpp
                          classes/TicTacToe.cpp
                          classes/Chess.cpp
                          ${MAIN_FILE}
                          ${IMPL_FILE}
                )

if(MACOS)
    target_link_libraries(gameboard ${OPENGL_gl_LIBRARY} glfw)
else()
    target_link_libraries(gameboard ${OPENGL_gl_LIBRARY})
endif(

)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
