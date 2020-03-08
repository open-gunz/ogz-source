#pragma once
#include <functional>
#include <list>
#include "ZGameInterface.h"

template <typename T>
void Listen(const char* WidgetName, T&& Function)
{
	struct ListenerType : public MListener
	{
		ListenerType(T&& Function) : Function{std::forward<T>(Function)} {}

		virtual bool OnCommand(MWidget* Widget, const char* Message) override
		{
			return Function(Widget, Message);
		}

		std::decay_t<T> Function;
	};

	// If we're calling this function for the first time, we want to just allocate the listener in
	// a static local to avoid heap allocations. (This will be the case if Listen is called with a
	// lambda type, since they are all unique, so the current instantiation will be different.)
	// However, if it's called with the same type (e.g. a function pointer), we still want it to
	// work, so we heap allocate new instances.
	// We use a std::list for this in order to have stable pointers.
	static bool FirstCall = true;

	if (FirstCall)
	{
		static ListenerType Listener{std::forward<T>(Function)};
		ZGetGameInterface()->SetListenerWidget(WidgetName, &Listener);
		FirstCall = false;
	}
	else
	{
		static std::list<ListenerType> Listeners;
		Listeners.emplace_back(std::forward<T>(Function));
		ZGetGameInterface()->SetListenerWidget(WidgetName, &Listeners.back());
	}
}

template <typename T>
auto OnMessage(const char* MessageName, T&& Function)
{
	return[MessageName, Function = std::forward<T>(Function)](MWidget* Widget, const char* Message) {
		if (MWidget::IsMsg(Message, MessageName))
		{
			Function();
			return true;
		}
		return false;
	};
}

template <typename T>
auto ApplyToButtonState(T&& Function)
{
	return [Function = std::forward<T>(Function)](MWidget* Widget, const char* Message) {
		if (!MWidget::IsMsg(Message, MBTN_CLK_MSG))
			return false;

		Function(static_cast<MButton*>(Widget)->GetCheck());

		return true;
	};
}

inline auto SetToButtonState(bool& Dest)
{
	return ApplyToButtonState([&Dest](bool Src) { Dest = Src; });
}