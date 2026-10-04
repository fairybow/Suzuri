# Suzuri — A plain-text editor for creative writing
# Copyright (C) 2026 fairybow
#
# This program is free software, redistributable and/or modifiable under the
# terms of the GNU GPL v3. It's distributed in the hope that it will be useful
# but without any warranty (even the implied warranty of merchantability or
# fitness for a particular purpose)
#
# See the LICENSE file or visit <https://www.gnu.org/licenses/>

# Qt Test, one executable per test class. Each is a single .cpp named for its
# class, and includes the app's headers straight from src/: the app is
# header-only, so there is no library to link against.

find_package(Qt6 REQUIRED COMPONENTS Test)

# Builds <name>.cpp as a test executable and registers it with CTest. Every
# test gets what the app's own headers need to compile: the same include root,
# the same APP_DEBUG definition, Coco, and the same Qt modules.
function(app_add_test name)
    qt_add_executable(${name} ${name}.cpp)

    target_include_directories(${name} PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )

    if(WIN32)
        target_compile_definitions(${name} PRIVATE
            NOMINMAX
            WIN32_LEAN_AND_MEAN
        )
    endif()

    target_compile_definitions(${name} PRIVATE
        APP_DEBUG=$<IF:$<CONFIG:Debug>,1,0>
    )

    target_link_libraries(${name} PRIVATE
        Coco::Coco
        Qt6::Core
        Qt6::Gui
        Qt6::PdfWidgets
        Qt6::Widgets
        Qt6::Test
    )

    if(MSVC)
        # NB: /Zc:preprocessor is required by Coco's ASSERT (__VA_OPT__)
        target_compile_options(${name} PRIVATE
            /W3 /MP /sdl /permissive- /bigobj /EHsc /Zc:preprocessor
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${name} PRIVATE
            -Wall -Wextra -Wpedantic
        )
    endif()

    add_test(NAME ${name} COMMAND ${name})

    # No display is needed (or present, on a build server)
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

app_add_test(FileTypesTest)
app_add_test(TextCountsTest)