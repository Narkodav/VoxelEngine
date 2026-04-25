#pragma once
#ifdef _WIN32
#include "Graphics/PlatformManagement/WindowWin32.h"
#else
#include "Graphics/PlatformManagement/WindowXlib.h"
#endif

namespace Platform
{
#ifdef _WIN32
	using WindowImpl = Win32::Window;
#else
	using WindowImpl = Xlib::Window;
#endif

	class Window : public WindowImpl
	{
	public:
		using EventManager = WindowImpl::EventManager;

		using WindowImpl::WindowImpl;

		template<auto E, typename Callback>
		Window& registerCallback(Callback&& callback)
		{
			m_windowEvents.set<E>(std::forward<Callback>(callback));
			return *this;
		}

		template<auto E, typename Callback>
		Window& registerCallback(auto callback, auto& handler)
		{
			m_windowEvents.set<E>(std::forward<Callback>(callback), handler);
			return *this;
		}

		template<auto E, typename... Args>
		Window& emitEvent(Args... args) {
			m_windowEvents.emit<E>(std::forward<Args>(args)...);
			return *this;
		};

	};
}