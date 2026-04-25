#pragma once
#include <cstdint>

namespace Platform {
    enum class CursorMode {
        Normal,
        Hidden,
        Disabled  // For raw motion input
    };

    struct Position
    {
        int32_t x;
        int32_t y;
    };

    struct Extent
    {
        int32_t width, height;
    };

    struct WindowAttributes
    {
        enum class Type {
            // Window behavior
            Resizable,                  // bool
            Visible,                    // bool
            Decorated,                  // bool
            Focused,                    // bool
            AutoIconify,                // bool
            Floating,                   // bool
            Maximized,                  // bool
            CenterCursor,               // bool
            TransparentFramebuffer,     // bool
            FocusOnShow,                // bool

            // Input mode
            CursorMode,                 // CursorMode enum
        };

        // Window behavior
        bool resizable = true;                  // GLFW_RESIZABLE
        bool visible = true;                    // GLFW_VISIBLE
        bool decorated = true;                  // GLFW_DECORATED (title bar, borders)
        bool focused = true;                    // GLFW_FOCUSED
        bool autoIconify = true;                // GLFW_AUTO_ICONIFY
        bool floating = false;                  // GLFW_FLOATING (always on top)
        bool maximized = false;                 // GLFW_MAXIMIZED
        bool centerCursor = false;              // GLFW_CENTER_CURSOR
        bool transparentFramebuffer = false;    // GLFW_TRANSPARENT_FRAMEBUFFER
        bool focusOnShow = true;                // GLFW_FOCUS_ON_SHOW

        CursorMode cursorMode = CursorMode::Normal;

        static WindowAttributes defaultAtr() {
            return WindowAttributes();
        }

        static WindowAttributes firstPersonGameMaximisedAtr() {
            WindowAttributes atr;
            atr.centerCursor = true;
            atr.cursorMode = CursorMode::Disabled;
            atr.maximized = true;
            return atr;
        }

        static WindowAttributes firstPersonGameMinimisedAtr() {
            WindowAttributes atr;
            atr.centerCursor = true;
            atr.cursorMode = CursorMode::Disabled;
            return atr;
        }
    };
}