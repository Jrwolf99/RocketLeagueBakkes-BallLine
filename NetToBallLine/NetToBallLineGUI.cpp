#include "pch.h"
#include "NetToBallLine.h"

std::string NetToBallLine::GetPluginName() {
	return "NetToBallLine";
}

void NetToBallLine::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> NetToBallLine
void NetToBallLine::RenderSettings() {
	ImGui::TextUnformatted("Net To Ball Line - This is an assist line to help with shooting training.");
	
	// Render the toggle button
	if (ImGui::Checkbox("Enable NetToBallLine", enabled.get())) {
		// The state is automatically managed by the shared pointer
		cvarManager->getCvar("net_to_ball_line_enabled").setValue(*enabled);
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
//void NetToBallLine::SetImGuiContext(uintptr_t ctx)
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

