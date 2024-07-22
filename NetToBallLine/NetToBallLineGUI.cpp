#include "pch.h"
#include "NetToBallLine.h"

std::string NetToBallLine::GetPluginName()
{
	return "Shooting By Wolf";
}

void NetToBallLine::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext *>(ctx));
}

void NetToBallLine::RenderSettings()
{
	if (ImGui::CollapsingHeader("Settings"))
	{
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
		ImGui::Indent();

		if (*shoot_enabled)
		{
			ImGui::NewLine();
			ImGui::Text("Goal Reset Time (ms):");
			if (ImGui::SliderInt("", goal_reset_time.get(), 1000, 10000))
			{
				cvarManager->getCvar("goal_reset_time").setValue(*goal_reset_time);
			}

			ImGui::NewLine();

			if (*shoot_enabled && ImGui::CollapsingHeader("Show Only Certain Quadrants?"))
			{
				for (int i = 0; i < 4; i++)
				{
					std::string whitelist_quadrant_name = "whitelist_quadrant_" + std::to_string(i + 1);
					bool whitelist_quadrant = cvarManager->getCvar(whitelist_quadrant_name).getBoolValue();
					std::string quadrant_names[] = {"Top Left", "Top Right", "Bottom Right", "Bottom Left"};

					if (ImGui::Checkbox(quadrant_names[i].c_str(), &whitelist_quadrant))
					{
						cvarManager->getCvar(whitelist_quadrant_name).setValue(whitelist_quadrant);
					}
				}
			}

			ImGui::NewLine();

			if (ImGui::Checkbox("Show Scoreboard", show_scoreboard.get()))
			{
				cvarManager->getCvar("show_scoreboard").setValue(*show_scoreboard);
			}

			if (*show_scoreboard)
			{
				ImGui::Indent();

				if (ImGui::Checkbox("Show Scoreboard Background", scoreboard_show_background.get()))
				{
					cvarManager->getCvar("scoreboard_show_background").setValue(*scoreboard_show_background);
				}
				if (*scoreboard_show_background)
				{
					LinearColor backgroundColor = cvarManager->getCvar("scoreboard_color").getColorValue() / 255;
					if (ImGui::ColorEdit4("Background Color", &backgroundColor.R))
					{
						cvarManager->getCvar("scoreboard_color").setValue(backgroundColor * 255);
					}
				}

				LinearColor textColor = cvarManager->getCvar("scoreboard_text_color").getColorValue() / 255;
				if (ImGui::ColorEdit4("Text Color", &textColor.R))
				{
					cvarManager->getCvar("scoreboard_text_color").setValue(textColor * 255);
				}
			}

			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::Unindent();

			if (*shoot_enabled && ImGui::CollapsingHeader("Extra Features"))
			{
				if (ImGui::Checkbox("Show Ball to Net Line", show_ball_to_net_line.get()))
				{
					cvarManager->getCvar("show_ball_to_net_line").setValue(*show_ball_to_net_line);
				}

				// if (ImGui::Checkbox("Show Dot on Ball", show_dot_on_ball.get()))
				// {
				// 	cvarManager->getCvar("show_dot_on_ball").setValue(*show_dot_on_ball);
				// }
			}
		}
	}
	ImGui::Unindent();
}

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
