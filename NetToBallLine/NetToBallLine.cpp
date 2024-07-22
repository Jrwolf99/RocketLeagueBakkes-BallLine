#include <random>
#include <cmath>
#include "pch.h"
#include "NetToBallLine.h"
#include "bakkesmod/wrappers/canvaswrapper.h"
#include "bakkesmod/wrappers/arraywrapper.h"
#include "CinderBloccRenderingTools/Objects/Line.h"
#include "CinderBloccRenderingTools/Objects/Frustum.h"
#include "CinderBloccRenderingTools/Objects/Circle2D.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef NUM_SEGMENTS
#define NUM_SEGMENTS 50
#define NUM_SEGMENTSF 50.0f
#endif

BAKKESMOD_PLUGIN(NetToBallLine, "Shooting by Wolf", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

bool NetToBallLine::IsBallColliding(const Vector &point, const Vector &topLeft, const Vector &bottomRight)
{
    // the extra margin is there so that the ball doesn't have to be exactly at the goal to count as a goal
    bool is_collided = point.X <= topLeft.X + 50 && point.X >= bottomRight.X - 50 &&
                       point.Z <= topLeft.Z + 50 && point.Z >= bottomRight.Z - 50 &&
                       point.Y <= topLeft.Y + 100 && point.Y >= bottomRight.Y + 50;

    return is_collided;
}

void NetToBallLine::StartTimer()
{
    startTime = std::chrono::steady_clock::now(); // Record the start time
    timerIsRunning = true;
}

void NetToBallLine::StopTimer()
{

    timerIsRunning = false;
    startTime = std::chrono::steady_clock::now();
}

void NetToBallLine::ScoreGoal(CanvasWrapper &canvas, Vector2 goalLocation)
{

    if (goal_state != 0)
        return;

    goalCount++;
    std::mt19937 gen(std::random_device{}());

    bool whitelist_quadrants[4];
    for (int i = 0; i < 4; ++i)
    {
        std::string whitelist_quadrant_name = "whitelist_quadrant_" + std::to_string(i + 1);
        whitelist_quadrants[i] = cvarManager->getCvar(whitelist_quadrant_name).getBoolValue();
    }

    if (!whitelist_quadrants[0] && !whitelist_quadrants[1] && !whitelist_quadrants[2] && !whitelist_quadrants[3])
    {
        whitelist_quadrants[0] = true;
        whitelist_quadrants[1] = true;
        whitelist_quadrants[2] = true;
        whitelist_quadrants[3] = true;
    }

    std::vector<int> available_quadrants;
    for (int i = 0; i < 4; ++i)
    {
        if (whitelist_quadrants[i])
        {
            available_quadrants.push_back(i + 1);
        }
    }

    std::uniform_int_distribution<int> dist(0, available_quadrants.size() - 1);

    goal_state = 1;
    goal_current_color_state = goal_success_color;

    gameWrapper->SetTimeout([this, available_quadrants, gen, dist](GameWrapper *gw) mutable
                            {
        int random_index = dist(gen);
        deciding_quadrant = available_quadrants[random_index];
        goal_state = 0;
        goal_current_color_state = goal_default_color; }, (*goal_reset_time) / 1000.0f);
}
bool NetToBallLine::IsInScreenLocation(Vector2 screenLocation, Vector2 canvasSize)
{

    if (screenLocation.X < 0 || screenLocation.X > canvasSize.X || screenLocation.Y < 0 || screenLocation.Y > canvasSize.Y)
    {
        cvarManager->log("    ");
        cvarManager->log("screenLocation: " + std::to_string(screenLocation.X) + " " + std::to_string(screenLocation.Y));
        cvarManager->log("screenLocation.X < 0: " + std::to_string(screenLocation.X < 0));
        cvarManager->log("screenLocation.X > canvasSize.X: " + std::to_string(screenLocation.X > canvasSize.X));
        cvarManager->log("screenLocation.Y < 0: " + std::to_string(screenLocation.Y < 0));
        cvarManager->log("screenLocation.Y > canvasSize.Y: " + std::to_string(screenLocation.Y > canvasSize.Y));

        cvarManager->log("Canvas Size: " + std::to_string(canvasSize.X) + " " + std::to_string(canvasSize.Y));
        cvarManager->log("    ");
        return false;
    }
    return true;
}

void NetToBallLine::onLoad()
{
    _globalCvarManager = cvarManager;
    cvarManager->log("Plugin loaded!");

    shoot_enabled = std::make_shared<bool>(false);
    show_ball_to_net_line = std::make_shared<bool>(false);
    show_dot_on_ball = std::make_shared<bool>(false);
    show_scoreboard = std::make_shared<bool>(true);
    scoreboard_show_background = std::make_shared<bool>(false);
    goal_reset_time = std::make_shared<int>(400);

    cvarManager->registerCvar("shooting_enabled", "1", "Enable shooting practice")
        .bindTo(shoot_enabled);

    cvarManager->registerCvar("show_scoreboard", "1", "Show Scoreboard")
        .bindTo(show_scoreboard);

    cvarManager->registerCvar("scoreboard_color", "#000000CC", "color of overlay");
    cvarManager->registerCvar("scoreboard_text_color", "#FFFFFF", "color of overlay text");
    cvarManager->registerCvar("scoreboard_show_background", "0", "Show background of overlay");

    cvarManager->registerCvar("goal_reset_time", "4000", "Time it takes for goal to reset after scoring (in milliseconds)")
        .bindTo(goal_reset_time);

    cvarManager->registerCvar("show_ball_to_net_line", "0", "Show Ball to Net Line")
        .bindTo(show_ball_to_net_line);

    cvarManager->registerCvar("show_dot_on_ball", "0", "Show Dot on Ball")
        .bindTo(show_dot_on_ball);

    for (int i = 0; i < 4; i++)
    {
        std::string whitelist_quadrant_name = "whitelist_quadrant_" + std::to_string(i + 1);
        cvarManager->registerCvar(whitelist_quadrant_name, "true", "Whitelist Quadrant " + std::to_string(i + 1));
    }

    gameWrapper->RegisterDrawable([this](CanvasWrapper canvas)
                                  { 

                                    if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining())
                                    {
                                        NetToBallLine::StopTimer();
                                        goalCount = 0;
                                        return;
                                    }
                                    
                                      if (*shoot_enabled)
                                      {
                                          RenderShootingGame(canvas);
                                      }

                                        if (*show_dot_on_ball)
                                        {
                                            RenderDotOnBall(canvas);
                                        }

                                        if (*shoot_enabled && *show_scoreboard)
                                        {
                                             HandleGameUI(canvas);
                                        }

                                      if (*show_ball_to_net_line)
                                      {
                                          RenderBallToNetLine(canvas);
                                      } });
}

void NetToBallLine::onUnload()
{
}

void NetToBallLine::DrawGoals(CanvasWrapper &canvas, int quadrant)
{

    Vector topLeft(880, 5120, 600);

    Vector topRight(-880, 5120, 600);

    Vector bottomRight(-880, 5120, 0);

    Vector bottomLeft(880, 5120, 0);

    Vector midLeft(880, 5120, 300);

    Vector midRight(-880, 5120, 300);

    Vector midTop(0, 5120, 600);

    Vector midBottom(0, 5120, 0);

    Vector midMid(0, 5120, 300);

    // Project points to 2D
    Vector2 p1 = canvas.Project(topLeft);
    Vector2 p2 = canvas.Project(topRight);
    Vector2 p3 = canvas.Project(bottomRight);
    Vector2 p4 = canvas.Project(bottomLeft);
    Vector2 p5 = canvas.Project(midLeft);
    Vector2 p6 = canvas.Project(midRight);
    Vector2 p7 = canvas.Project(midTop);
    Vector2 p8 = canvas.Project(midBottom);
    Vector2 p9 = canvas.Project(midMid);

    const float lineWidth = 3;

    Vector ballLocation = gameWrapper->GetGameEventAsServer().GetBall().GetLocation();

    switch (quadrant)
    {
    case 1:
        if (IsBallColliding(ballLocation, topLeft, midMid))
        {
            ScoreGoal(canvas, p1);
        }
        break;
    case 2:

        if (IsBallColliding(ballLocation, midTop, midRight))
        {
            ScoreGoal(canvas, p7);
        }
        break;
    case 3:

        if (IsBallColliding(ballLocation, midMid, bottomRight))
        {
            ScoreGoal(canvas, p9);
        }
        break;
    case 4:
        if (IsBallColliding(ballLocation, midLeft, midBottom))

        {
            ScoreGoal(canvas, p5);
        }
        break;
    }

    switch (quadrant)
    {
    case 1:
        DrawNetLine(topLeft, midLeft, canvas);
        DrawNetLine(midLeft, midMid, canvas);
        DrawNetLine(midMid, midTop, canvas);
        DrawNetLine(midTop, topLeft, canvas);
        break;
    case 2:
        DrawNetLine(midTop, midMid, canvas);
        DrawNetLine(midMid, midRight, canvas);
        DrawNetLine(midRight, topRight, canvas);
        DrawNetLine(topRight, midTop, canvas);
        break;
    case 3:
        DrawNetLine(midMid, midRight, canvas);
        DrawNetLine(midRight, bottomRight, canvas);
        DrawNetLine(bottomRight, midBottom, canvas);
        DrawNetLine(midBottom, midMid, canvas);
        break;
    case 4:
        DrawNetLine(midLeft, midMid, canvas);
        DrawNetLine(midMid, midBottom, canvas);
        DrawNetLine(midBottom, bottomLeft, canvas);
        DrawNetLine(bottomLeft, midLeft, canvas);
        break;
    }

          if (goal_current_color_state == goal_success_color)
        {
            canvas.SetColor(goal_success_color);

            // bottom right of the screen
            canvas.SetPosition(Vector2{1920 - 550, 1080 - 90});
            canvas.DrawString("SCORE!", 5, 5);
        }
}

void NetToBallLine::RenderShootingGame(CanvasWrapper canvas)
{

    ServerWrapper game = gameWrapper->GetGameEventAsServer();
    if (game.IsNull())
        return;

    BallWrapper ball = game.GetBall();
    if (ball.IsNull())
        return;

    if (!timerIsRunning)
    {
        NetToBallLine::StartTimer();
    }

    DrawGoals(canvas, deciding_quadrant);
}

void NetToBallLine::RenderBallToNetLine(CanvasWrapper canvas)
{
    ServerWrapper game = gameWrapper->GetGameEventAsServer();
    if (game.IsNull())
        return;

    BallWrapper ball = game.GetBall();
    if (ball.IsNull())
        return;

    Vector ballLocation = game.GetBall().GetLocation();
    Vector netLocation;

    switch (deciding_quadrant)
    {
    case 1:
        netLocation = Vector(440, 5120, 450);
        break;
    case 2:
        netLocation = Vector(-440, 5120, 450);
        break;
    case 3:
        netLocation = Vector(-440, 5120, 150);
        break;
    case 4:
        netLocation = Vector(440, 5120, 150);
        break;
    default:
        netLocation = Vector(0, 5120, 300);
        break;
    }

    DrawNetLine(ballLocation, netLocation, canvas, 200, true);
}

// todo: implement this
void NetToBallLine::RenderDotOnBall(CanvasWrapper canvas)
{
    ServerWrapper game = gameWrapper->GetGameEventAsServer();
    if (game.IsNull())
        return;

    BallWrapper ball = game.GetBall();
    if (ball.IsNull())
        return;

    Vector ballLocation = game.GetBall().GetLocation();
}

void NetToBallLine::HandleGameUI(CanvasWrapper canvas)
{
    float elapsedTime = 0.0f;

    if (timerIsRunning)
    {
        auto now = std::chrono::steady_clock::now();
        elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
    }

    Vector2 topLeft = Vector2{1630, 380};
    Vector2 bottomLeft = Vector2{1630, 500};
    Vector2 topRight = Vector2{1860, 380};
    Vector2 bottomRight = Vector2{1860, 500};

    if (*scoreboard_show_background)
    {
        LinearColor my_color = cvarManager->getCvar("scoreboard_color").getColorValue();
        canvas.FillTriangle(topLeft, bottomLeft, topRight, my_color);
        canvas.FillTriangle(topRight, bottomLeft, bottomRight, my_color);
    }

    canvas.SetColor(cvarManager->getCvar("scoreboard_text_color").getColorValue());

    int elapsedTimeInt = static_cast<int>(elapsedTime);
    int minutes = elapsedTimeInt / 60;
    int seconds = elapsedTimeInt % 60;
    std::string timerText = "Timer: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    canvas.SetPosition(Vector2{1650, 400});
    canvas.DrawString(timerText, 2, 2);
    canvas.SetPosition(Vector2{1650, 450});
    canvas.DrawString("Goals: " + std::to_string(goalCount), 2, 2);
}

void NetToBallLine::DrawNetLine(Vector &v1, Vector &v2, CanvasWrapper &canvas, int num_segs, bool curved)
{
    DrawPartialLine(v1, v2, canvas, num_segs, curved);
    DrawPartialLine(v2, v1, canvas, num_segs, curved);
}

Vector NetToBallLine::Lerp(const Vector &a, const Vector &b, float t)
{
    return a + (b - a) * t;
}

void NetToBallLine::DrawPartialLine(Vector &v1, Vector &v2, CanvasWrapper &canvas, int num_segs, bool curved)
{

    if (curved)
    {
        num_segs /= 2;
    }
    Vector(*all_segments)[2] = new Vector[num_segs][2];
    Vector stopping_vector = v2;

    Vector control_point = (v1 + v2) / 2;
    control_point.Z += std::abs(v2.Z - v1.Z) * 0.9f; // Adjust Z to create an arch

    for (int i = 0; i < num_segs; i++)
    {
        float t0 = i / static_cast<float>(num_segs);
        float t1 = (i + 1) / static_cast<float>(num_segs);

        // Calculate points on the Bezier curve
        Vector p0 = Lerp(Lerp(v1, control_point, t0), Lerp(control_point, v2, t0), t0);
        Vector p1 = Lerp(Lerp(v1, control_point, t1), Lerp(control_point, v2, t1), t1);

        all_segments[i][0] = p0;
        all_segments[i][1] = p1;
    }

    for (int i = 0; i < num_segs; i++)
    {
        if (IsPointIntersectingBall(all_segments[i][0], canvas))
        {
            stopping_vector = all_segments[i][0];
            break;
        }
        else if (IsPointIntersectingBall(all_segments[i][1], canvas))
        {
            stopping_vector = all_segments[i][1];
            break;
        }
    }

    if (curved)
    {
        canvas.SetColor(curved_line_color);

        for (int i = 0; i < num_segs; i++)
        {
            RT::Line line(all_segments[i][0], all_segments[i][1], 3.0f);

            RT::Frustum frustum(canvas, gameWrapper->GetCamera());
            line.DrawWithinFrustum(canvas, frustum);
        }
    }
    else
    {

        canvas.SetColor(goal_current_color_state);
        RT::Line line(v1, stopping_vector, 3.0f);

        RT::Frustum frustum(canvas, gameWrapper->GetCamera());
        line.DrawWithinFrustum(canvas, frustum);
    }

    delete[] all_segments;
}
// derived from the equation to find projected radius using the fov of the camera
bool NetToBallLine::IsPointIntersectingBall(Vector &point, CanvasWrapper &canvas)
{
    Vector ballLocation = gameWrapper->GetGameEventAsServer().GetBall().GetLocation();
    Vector cameraLocation = gameWrapper->GetCamera().GetLocation();

    float distanceFromCamera = (ballLocation - cameraLocation).magnitude();

    float fovDegrees = static_cast<float>(gameWrapper->GetCamera().GetFOV());
    float fovRadians = fovDegrees * (static_cast<float>(M_PI) / 180.0f);

    float focalLength = 1920.0f / (2.0f * std::tan(fovRadians / 2.0f));

    float ballRadius = gameWrapper->GetGameEventAsServer().GetBall().GetRadius();
    float projectedRadius = (ballRadius * focalLength) / distanceFromCamera;

    Vector2 point2D = canvas.Project(point);
    Vector2 ballLocation2D = canvas.Project(ballLocation);
    float distance = std::sqrt(std::pow(static_cast<float>(point2D.X - ballLocation2D.X), 2.0f) + std::pow(static_cast<float>(point2D.Y - ballLocation2D.Y), 2.0f));

    return distance < projectedRadius;
}