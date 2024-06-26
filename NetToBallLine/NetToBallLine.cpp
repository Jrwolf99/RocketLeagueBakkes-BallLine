#include <random>

#include "pch.h"
#include "NetToBallLine.h"
#include "bakkesmod/wrappers/canvaswrapper.h"
#include "bakkesmod/wrappers/arraywrapper.h"
#include "CinderBloccRenderingTools/Objects/Line.h"

BAKKESMOD_PLUGIN(NetToBallLine, "This is a simple plugin that draws a line from the net to the ball to assist in shooting practice", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

bool NetToBallLine::IsBallColliding(const Vector &point, const Vector &topLeft, const Vector &bottomRight)
{
    // the extra margin is there so that the ball doesn't have to be exactly at the goal to count as a goal
    bool is_collided = point.X <= topLeft.X + 50 && point.X >= bottomRight.X - 50 &&
                       point.Z <= topLeft.Z + 50 && point.Z >= bottomRight.Z - 50 &&
                       point.Y <= topLeft.Y + 50 && point.Y >= bottomRight.Y - 50;

    if (!is_collided)
    {
        previous_collision = false;
    }

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

void NetToBallLine::ScoreGoal(CanvasWrapper &canvas)
{

    LinearColor colors;
    colors.R = 0;
    colors.G = 255;
    colors.B = 0;
    colors.A = 255;
    canvas.SetColor(colors);

    if (previous_collision)
        return;

    goalCount++;
    previous_collision = true;
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, 4);

    if (dist(gen) == deciding_quadrant)
    {
        std::uniform_int_distribution<int> dist_second(1, 4);
        deciding_quadrant = dist_second(gen);
    }
    else
    {
        deciding_quadrant = dist(gen);
    }
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

    cvarManager->registerCvar("shooting_enabled", "0", "Enable shooting practice")
        .bindTo(shoot_enabled);

    gameWrapper->RegisterDrawable([this](CanvasWrapper canvas)
                                  {
        if (*shoot_enabled)
        {
            RenderShootingGame(canvas);
        } });
}

void NetToBallLine::onUnload()
{
}

void NetToBallLine::DrawGoals(CanvasWrapper &canvas, int quadrant)
{

    // RT::Vector topLefttt(880, 5120, 600);
    // RT::Vector topRighttt(-880, 5120, 600);

    // Creating a line with thickness 3.0 using the specified vectors
    // RT::Line thickLine(topLefttt, topRighttt, 10.0f);

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
            ScoreGoal(canvas);
        }
        break;
    case 2:

        if (IsBallColliding(ballLocation, midTop, midRight))
        {
            ScoreGoal(canvas);
        }
        break;
    case 3:

        if (IsBallColliding(ballLocation, midMid, bottomRight))
        {
            ScoreGoal(canvas);
        }
        break;
    case 4:
        if (IsBallColliding(ballLocation, midLeft, midBottom))

        {
            ScoreGoal(canvas);
        }
        break;
    }

    switch (quadrant)
    {
    case 1:
        if (!IsInScreenLocation(p1, canvas.GetSize()) ||
            !IsInScreenLocation(p5, canvas.GetSize()) ||
            !IsInScreenLocation(p9, canvas.GetSize()) ||
            !IsInScreenLocation(p7, canvas.GetSize()))
            return;
        DrawNetLine(p1, p5, canvas);
        DrawNetLine(p5, p9, canvas);
        DrawNetLine(p9, p7, canvas);
        DrawNetLine(p7, p1, canvas);
        break;
    case 2:
        if (!IsInScreenLocation(p7, canvas.GetSize()) ||
            !IsInScreenLocation(p9, canvas.GetSize()) ||
            !IsInScreenLocation(p6, canvas.GetSize()) ||
            !IsInScreenLocation(p2, canvas.GetSize()))
            return;
        DrawNetLine(p7, p9, canvas);
        DrawNetLine(p9, p6, canvas);
        DrawNetLine(p6, p2, canvas);
        DrawNetLine(p2, p7, canvas);
        break;
    case 3:
        if (!IsInScreenLocation(p9, canvas.GetSize()) ||
            !IsInScreenLocation(p8, canvas.GetSize()) ||
            !IsInScreenLocation(p3, canvas.GetSize()) ||
            !IsInScreenLocation(p6, canvas.GetSize()))
            return;
        DrawNetLine(p9, p6, canvas);
        DrawNetLine(p6, p3, canvas);
        DrawNetLine(p3, p8, canvas);
        DrawNetLine(p8, p9, canvas);
        break;
    case 4:
        if (!IsInScreenLocation(p5, canvas.GetSize()) ||
            !IsInScreenLocation(p9, canvas.GetSize()) ||
            !IsInScreenLocation(p8, canvas.GetSize()) ||
            !IsInScreenLocation(p4, canvas.GetSize()))
            return;
        DrawNetLine(p5, p9, canvas);
        DrawNetLine(p9, p8, canvas);
        DrawNetLine(p8, p4, canvas);
        DrawNetLine(p4, p5, canvas);
        break;
    }
}

void NetToBallLine::RenderShootingGame(CanvasWrapper canvas)
{
    if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining())
    {
        NetToBallLine::StopTimer();
        goalCount = 0;
        return;
    }

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

    HandleGameUI(canvas);

    DrawGoals(canvas, deciding_quadrant);
}

void NetToBallLine::HandleGameUI(CanvasWrapper canvas)
{
    float elapsedTime = 0.0f;

    if (timerIsRunning)
    {
        auto now = std::chrono::steady_clock::now();
        elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
    }

    canvas.SetColor(255, 255, 0, 255);

    int elapsedTimeInt = static_cast<int>(elapsedTime);
    int minutes = elapsedTimeInt / 60;
    int seconds = elapsedTimeInt % 60;
    std::string timerText = "Timer: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    canvas.SetPosition(Vector2{1650, 400});
    canvas.DrawString(timerText, 2, 2);
    canvas.SetPosition(Vector2{1650, 450});
    canvas.DrawString("Goals: " + std::to_string(goalCount), 2, 2);
}

void NetToBallLine::DrawNetLine(Vector2 &p1, Vector2 &p2, CanvasWrapper &canvas)
{
    LinearColor colors;
    colors.R = 255;
    colors.G = 255;
    colors.B = 0;
    colors.A = 255;
    canvas.SetColor(colors);

    if (!IsInScreenLocation(p1, canvas.GetSize()) && !IsInScreenLocation(p2, canvas.GetSize()))
        return;

    // grab the car point vector2
    Vector cameraLocation = gameWrapper->GetCamera().GetLocation();

    //  middle of the points
    Vector2 middle = (p1 + p2) / 2;

    // draw line from the camera to the middle of the points
    canvas.DrawLine(canvas.Project(cameraLocation), middle, 3);
    canvas.DrawLine(p1, p2, 3);
}