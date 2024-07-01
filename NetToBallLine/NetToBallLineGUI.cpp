#include "pch.h"
#include "NetToBallLine.h"

std::string NetToBallLine::GetPluginName()
{
	return "NetToBallLine";
}

void NetToBallLine::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext *>(ctx));
}

void NetToBallLine::RenderSettings()
{
	ImGui::TextUnformatted("Shooting Practice:");

	if (ImGui::Checkbox("Enable Shooting Practice", shoot_enabled.get()))
	{
		cvarManager->getCvar("shooting_enabled").setValue(*shoot_enabled);
		if (*shoot_enabled)
		{
			NetToBallLine::StartTimer();
		}
		else
		{
			NetToBallLine::StopTimer();
			goalCount = 0;
		}
	}

	ImGui::TextUnformatted("Show  Ball to Net Line:");
	if (ImGui::Checkbox("Show Ball to Net Line", show_ball_to_net_line.get()))
	{
		cvarManager->getCvar("show_ball_to_net_line").setValue(*show_ball_to_net_line);
	}

	ImGui::TextUnformatted("Show Dot on Ball:");
	if (ImGui::Checkbox("Show Dot on Ball", show_dot_on_ball.get()))
	{
		cvarManager->getCvar("show_dot_on_ball").setValue(*show_dot_on_ball);
	}
}

// Do ImGui rendering here
void NetToBallLine::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

// Name of the menu that is used to toggle the window.
std::string NetToBallLine::GetMenuName()
{
	return "nettoballline";
}

// Title to give the menu
std::string NetToBallLine::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
// this was a dupped piece of code?
// void NetToBallLine::SetImGuiContext(uintptr_t ctx)
//{
//	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
//}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool NetToBallLine::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool NetToBallLine::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void NetToBallLine::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void NetToBallLine::OnClose()
{
	isWindowOpen_ = false;
}
