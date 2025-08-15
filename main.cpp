#include "Game.hpp"

int main(int argc, char* argv[])
{
    Program* game = new Game("Renderer");
    game->Init();

    while (game->isRunning)
    {
        game->HandleEvents();
        game->HandleInput();
        game->Update();
        game->Render();
    }

	return 0;
} 