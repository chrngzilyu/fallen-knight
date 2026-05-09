#include "Game.h"

int main() {
    srand((unsigned)time(nullptr));
    initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);

    Game game;
    game.getAssets().loadAll();
    game.getSounds().loadAllSfx();
    game.run();

    closegraph();
    return 0;
}
