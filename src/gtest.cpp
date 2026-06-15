#include "raylib.h"

 Color colors[21] = {
        DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
        GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
        GREEN, SKYBLUE, PURPLE, BEIGE };
int currentcol;
int main() {
    InitWindow(640,640,"GEMU - Test");
    SetTargetFPS(2);
    while(!WindowShouldClose){
        BeginDrawing();
        ClearBackground(BLACK);
        for(int rows=0; rows<64; rows++){
            for(int cols = 0; cols<64; cols++){
                DrawRectangle(cols*10, rows*10, 10,10, colors[currentcol]);
                currentcol = (currentcol + 1) % 21;
            }
        }
        EndDrawing();
    }
    CloseWindow();
}