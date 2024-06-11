#include "pch.h"
#include "NetToBallLine.h"

BAKKESMOD_PLUGIN(NetToBallLine, "This is a simple plugin that draws a line from the net to the ball to assist in shooting practice", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void NetToBallLine::onLoad()
{
    _globalCvarManager = cvarManager;
    cvarManager->log("Plugin loaded!!!!!!!!!!!!");

    enabled = std::make_shared<bool>(false);
    cvarManager->registerCvar("net_to_ball_line_enabled", "0", "Enable NetToBallLine plugin")
        .bindTo(enabled);

    gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
        if (*enabled)
            RenderLine(canvas);
        });
}

void NetToBallLine::onUnload()
{
}

void NetToBallLine::RenderLine(CanvasWrapper canvas) {
    if (!gameWrapper->IsInFreeplay() && !gameWrapper->IsInCustomTraining())
        return;

    ServerWrapper game = gameWrapper->GetGameEventAsServer();
    if (game.IsNull())
        return;

    BallWrapper ball = game.GetBall();
    if (ball.IsNull())
        return;



    Vector ballLocation = ball.GetLocation();
    float ballRadius = ball.GetRadius();


    // Project the ball's location to screen coordinates
    Vector2 canvasSize = canvas.GetSize();
    Vector2 screenLocation = canvas.Project(ballLocation);

    // Check if the ball is within the screen boundaries
    if (screenLocation.X < 0 || screenLocation.X > canvasSize.X || screenLocation.Y < 0 || screenLocation.Y > canvasSize.Y)
        return;

    Vector netLocation = GetNetLocation(game);

    // Calculate the direction from the ball to the net and normalize it
    Vector direction = netLocation - ballLocation;
    float magnitude = sqrt(direction.X * direction.X + direction.Y * direction.Y + direction.Z * direction.Z);

    if (magnitude != 0) {
        direction.X /= magnitude;
        direction.Y /= magnitude;
        direction.Z /= magnitude;
    }

    // Calculate the entry and exit points on the ball
    Vector entryPoint = ballLocation + direction * 10 * ballRadius;
    Vector exitPoint = ballLocation - direction * ballRadius;

    LinearColor color;
    color.R = 0;
    color.G = 255;
    color.B = 0;
    color.A = 255;
    canvas.SetColor(color);

    // Draw the line from the ball location to the entry point (before the ball)
    canvas.DrawLine(canvas.Project(netLocation), canvas.Project(entryPoint));

    // Render the ball here if possible

    // Draw the green cross at the exit point
    LinearColor colorTwo;
    colorTwo.R = 0;
    colorTwo.G = 255;
    colorTwo.B = 0;
    colorTwo.A = 255;
    canvas.SetColor(colorTwo);

    // Draw 200 lines in a vertical cross pattern at the exit point
    for (int i = 0; i < 200; ++i) {
        float angle = i * (360.0f / 200.0f);
        float radian = angle * (3.14159f / 180.0f);
        float length = 5.0f; // Length of each line in the cross

        // Lines in the XZ plane
        Vector lineEndXZ;
        lineEndXZ.X = exitPoint.X + cos(radian) * length;
        lineEndXZ.Y = exitPoint.Y;
        lineEndXZ.Z = exitPoint.Z + sin(radian) * length;

        canvas.DrawLine(canvas.Project(exitPoint), canvas.Project(lineEndXZ));

        // Lines in the YZ plane
        Vector lineEndYZ;
        lineEndYZ.X = exitPoint.X;
        lineEndYZ.Y = exitPoint.Y + cos(radian) * length;
        lineEndYZ.Z = exitPoint.Z + sin(radian) * length;

        canvas.DrawLine(canvas.Project(exitPoint), canvas.Project(lineEndYZ));
    }
}

Vector NetToBallLine::GetNetLocation(ServerWrapper game) {
    // Assuming we are drawing to the center of the blue goal and the center of the orange goal.
    // The locations should be adjusted based on the specific map and goal dimensions.
    // Here is a rough approximation for most maps:
    Vector blueGoalLocation = Vector(0, -5120, 200);  // Center of the blue goal
    Vector orangeGoalLocation = Vector(0, 5120, 200); // Center of the orange goal

    // Get the ball location

    BallWrapper ball = game.GetBall();

    Vector ballLocation = ball.GetLocation();


    // Get your car location
    CarWrapper car = game.GetGameCar();
    Vector carLocation = car.GetLocation();

    // Determine if you are behind the ball relative to the goals
    // Assuming "behind" means closer to your own goal (blue goal)
    bool isBehindBall = (carLocation.Y > ballLocation.Y);

    // Return the appropriate goal location
    Vector netLocation = isBehindBall ? blueGoalLocation : orangeGoalLocation;
    return netLocation;
}

