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
	std::shared_ptr<bool> show_ball_to_net_line;
	std::shared_ptr<bool> show_dot_on_ball;
	std::shared_ptr<bool> show_scoreboard;
	std::shared_ptr<bool> scoreboard_show_background;

	std::shared_ptr<int> goal_reset_time;
	std::shared_ptr<LinearColor> scoreboard_color;
	std::shared_ptr<LinearColor> scoreboard_text_color;


	virtual void onLoad();
	virtual void onUnload();

	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "Shooting By Wolf";

	std::chrono::steady_clock::time_point startTime;
	bool timerIsRunning;

	int goalCount = 0;
	int goal_state = 0; // 0 for default, 1 for success, 2 for fail
	int deciding_quadrant = 1;


	LinearColor goal_current_color_state = { 255, 255, 255, 255 };
	LinearColor goal_default_color = { 255, 255, 255, 255 };
	LinearColor goal_success_color = { 0, 255, 0, 255 };
	LinearColor goal_fail_color = { 255, 0, 0, 255 };
	LinearColor curved_line_color = { 168, 172, 186, 255 };

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
	void RenderShootingGame(CanvasWrapper canvas);
	void RenderBallToNetLine(CanvasWrapper canvas);
	void RenderDotOnBall(CanvasWrapper canvas);
	void DrawGoals(CanvasWrapper &canvas, int quadrant);
	void HandleGameUI(CanvasWrapper canvas);
	bool IsBallColliding(const Vector &point, const Vector &topLeft, const Vector &bottomRight);
	void DrawNetLine(Vector &v1, Vector &v2, CanvasWrapper &canvas, int num_segments = 50, bool curved = false);
	Vector Lerp(const Vector& a, const Vector& b, float t);
	void DrawPartialLine(Vector &v1, Vector &v2, CanvasWrapper &canvas, int num_segments, bool curved);
	bool IsPointIntersectingBall(Vector &point, CanvasWrapper &canvas);
};
