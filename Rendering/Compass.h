#pragma once
#include "Graphics/DeviceCaching/PhysicalDeviceCache.h"
#include "Graphics/Rendering/Device.h"
#include "Graphics/Rendering/CommandBuffer.h"
#include "Graphics/Rendering/CommandPool.h"
#include "Graphics/Rendering/Semaphore.h"
#include "Graphics/Rendering/Fence.h"
#include "Graphics/MemoryManagement/Buffer.h"
#include "Graphics/Rendering/Queue.h"
#include "Graphics/MemoryManagement/Memory.h"
#include "Graphics/MemoryManagement/MappedMemory.h"
#include "Graphics/Camera.h"
#include "Graphics/Rendering/Surface.h"
#include "Graphics/Rendering/Pipeline.h"
#include "Graphics/Rendering/ComputePipeline.h"
#include "Graphics/MemoryManagement/MemoryPool.h"

#include "Rendering/Shaders/ShaderCache.h"
#include "Rendering/StorageCache.h"
#include "Rendering/ShaderLayoutDefinitions.h"
#include "GameData/ResourceCache.h"
#include "WorldManagement/WorldGrid.h"
#include "Rendering/DebugConsole.h"
#include "Math/LinearAlgebra.h"

#include "Multithreading/ThreadPool.h"

class Compass
{
public:
    static void drawCoordinateAxes(const Gfx::CameraPerspective& cam, 
        const ImVec2& position = ImVec2(50, 50), float scale = 1.0f, float size = 100.0f) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Calculate the center point of the axes
        ImVec2 center = ImVec2(position.x + size * 0.5f, position.y + size * 0.5f);

        // Define the 3D axis directions in world space
        glm::vec3 world_axes[3] = {
            glm::vec3(1.0f, 0.0f, 0.0f),  // X
            glm::vec3(0.0f, 1.0f, 0.0f),  // Y
            glm::vec3(0.0f, 0.0f, 1.0f)   // Z
        };

        // Colors for each axis
        ImU32 colors[3] = {
            IM_COL32(255, 0, 0, 255),    // Red for X
            IM_COL32(0, 255, 0, 255),    // Green for Y
            IM_COL32(0, 0, 255, 255)     // Blue for Z
        };

        // Labels for each axis
        const char* labels[3] = { "X", "Y", "Z" };

        // Project each axis direction to screen space
        for (int i = 0; i < 3; i++) {
            // Transform the axis direction to view space
            glm::vec4 view_axis = cam.getView() * glm::vec4(world_axes[i], 0.0f);

            // Project to clip space
            glm::vec4 clip_axis = cam.getProjection() * view_axis;

            // Perspective division
            if (clip_axis.w != 0.0f) {
                clip_axis.x /= clip_axis.w;
                clip_axis.y /= clip_axis.w;
            }

            // Convert to screen coordinates
            // Note: We flip Y because screen coordinates have origin at top-left
            ImVec2 screen_dir = ImVec2(clip_axis.x, -clip_axis.y);

            // Normalize and scale
            float length = sqrtf(screen_dir.x * screen_dir.x + screen_dir.y * screen_dir.y);
            if (length > 0.0f) {
                screen_dir.x = screen_dir.x / length * size * 0.4f;
                screen_dir.y = screen_dir.y / length * size * 0.4f;
            }

            // Calculate end point
            ImVec2 end_point = ImVec2(center.x + screen_dir.x, center.y + screen_dir.y);

            // Draw the axis line
            draw_list->AddLine(center, end_point, colors[i], 2.0f);

            // Draw arrowhead
            const float arrow_size = 6.0f;
            ImVec2 perpendicular = ImVec2(-screen_dir.y, screen_dir.x);
            float perp_length = sqrtf(perpendicular.x * perpendicular.x + perpendicular.y * perpendicular.y);
            if (perp_length > 0.0f) {
                perpendicular.x = perpendicular.x / perp_length * arrow_size;
                perpendicular.y = perpendicular.y / perp_length * arrow_size;
            }

            ImVec2 arrow1 = ImVec2(end_point.x - screen_dir.x * 0.2f + perpendicular.x,
                end_point.y - screen_dir.y * 0.2f + perpendicular.y);
            ImVec2 arrow2 = ImVec2(end_point.x - screen_dir.x * 0.2f - perpendicular.x,
                end_point.y - screen_dir.y * 0.2f - perpendicular.y);

            draw_list->AddLine(end_point, arrow1, colors[i], 2.0f);
            draw_list->AddLine(end_point, arrow2, colors[i], 2.0f);

            // Draw label
            ImVec2 label_pos = ImVec2(end_point.x + screen_dir.x * 0.1f,
                end_point.y + screen_dir.y * 0.1f);
            draw_list->AddText(label_pos, colors[i], labels[i]);
        }

        // Add a subtle background to make the axes stand out
        ImU32 bg_color = IM_COL32(0, 0, 0, 100);
        ImVec2 bg_min = ImVec2(position.x, position.y);
        ImVec2 bg_max = ImVec2(position.x + size, position.y + size);
        draw_list->AddRectFilled(bg_min, bg_max, bg_color, 5.0f);
        draw_list->AddRect(bg_min, bg_max, IM_COL32(255, 255, 255, 100), 5.0f);
    }

};

