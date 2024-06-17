#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include <random>

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

class NetToBallLine : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow, public BakkesMod::Plugin::PluginWindow
{

	std::shared_ptr<bool> shoot_enabled;

	virtual void onLoad();
	virtual void onUnload();

	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "NetToBallLine";

	std::chrono::steady_clock::time_point startTime;
	bool timerIsRunning;

	int goalCount = 0;
	bool previous_collision = false;
	int deciding_quadrant = 1;

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;

	void StartTimer();
	void StopTimer();
	void ScoreGoal(CanvasWrapper &canvas);
	bool IsInScreenLocation(Vector2 screenLocation, Vector2 canvasSize);
	void renderShootingGame(CanvasWrapper canvas);
	void DrawGoals(CanvasWrapper &canvas, int quadrant);
	void handleGameUI(CanvasWrapper canvas);
	bool isBallColliding(const Vector& point, const Vector& topLeft, const Vector& bottomRight);
};
