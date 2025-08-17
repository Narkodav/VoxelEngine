#pragma once
#include <vector>
#include <string>
#include <format>
#include <mutex>
#include "imgui.h"

class DebugConsole
{
private:
	std::mutex m_mutex;
    std::vector<std::string> m_logs;
    bool m_scrollToBottom = false;

    void trimLogs()
    {
        if (m_logs.size() > 1000)
            m_logs.erase(m_logs.begin());
    }

    void copyLogsToClipboard()
    {
        ImGui::LogToClipboard();
        for (const auto& log : m_logs)
            ImGui::LogText("%s\n", log.c_str());
        ImGui::LogFinish();
    }

public:

    void log(const std::string& message)
    {
		std::lock_guard<std::mutex> lock(m_mutex);
        m_logs.push_back(message);
        trimLogs();
        m_scrollToBottom = true;
    }

    template<typename... Args>
    void log(std::format_string<Args...> fmt, Args&&... args)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logs.push_back(std::format(fmt, std::forward<Args>(args)...));
        trimLogs();
        m_scrollToBottom = true;
    }

    void clearLogs() { 
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logs.clear(); 
    }

    void draw()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 window_pos = ImVec2(
            viewport->WorkPos.x + viewport->WorkSize.x - PAD,
            viewport->WorkPos.y + PAD
        );

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.35f);

        if (ImGui::Begin("Debug Console", nullptr , ImGuiWindowFlags_NoMove))
        {
            ImGui::BeginChild("Scrolling", ImVec2(
                ImGui::GetContentRegionAvail().x, 
                ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), 
                0, ImGuiWindowFlags_HorizontalScrollbar);
            for (const auto& log : m_logs)
                ImGui::TextUnformatted(log.c_str());

            if (m_scrollToBottom)
            {
                ImGui::SetScrollHereY(1.0f);
                m_scrollToBottom = false;
            }
            ImGui::EndChild();

            if (ImGui::SmallButton("Clear")) m_logs.clear();
            ImGui::SameLine();
            if (ImGui::SmallButton("Copy")) copyLogsToClipboard();
        }
        ImGui::End();
    }
};

