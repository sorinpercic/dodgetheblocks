#include "raylib.h"
#include <stdlib.h>
#define STORAGE_DATA_FILE   "db/score.data"

typedef enum GameScreen {LOGO = 0, TITLE, GAMEPLAY, ENDING} GameScreen;
typedef enum {
    STORAGE_POSITION_SCORE = 0,
    STORAGE_POSITION_HISCORE = 1
} StorageData;

static bool SaveStorageValue(unsigned int position, int value);
static int LoadStorageValue(unsigned int position);
typedef struct FallingRectangle {
    Rectangle rect;
    float speedY;
} FallingRectangle;

const int MAX_RECTANGLES = 100;
FallingRectangle blocks[1000];

int main(void) {
    // Variables
    const int screenWidth = 1440;
    const int screenHeight = 800;
    GameScreen currentScreen = LOGO;
    int framesCounter = 0;
    int gameplayFrames = 0;
    int blockCount = 0;
    int score = 0;
    int hiscore = 0 ;

    InitWindow(screenWidth, screenHeight, "Dodge the Blocks");
    hiscore = LoadStorageValue(STORAGE_POSITION_HISCORE);
    InitAudioDevice();
    Sound fxWav = LoadSound("assets/die.wav");
    Texture2D character = LoadTexture("assets/Character.png");
    Texture2D background = LoadTexture("assets/img.png");
    Vector2 characterPos = {screenWidth / 2, screenHeight - character.height};
    Rectangle characterRectangle = {characterPos.x, characterPos.y, character.width, character.height};
    Vector2 characterCenter = {character.width / 2 , character.height / 2};
    SetTargetFPS(60);
    SetSoundVolume(fxWav, 0.1);
    while (!WindowShouldClose()) {

        switch (currentScreen) {

            case LOGO:
                framesCounter++;
                if (framesCounter > 120) {
                    currentScreen = TITLE;
                }
                break;
            case TITLE:
                if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)) {

                    currentScreen = GAMEPLAY;
                }
                break;
            case GAMEPLAY:
                gameplayFrames++;
                // Movement
                if (IsKeyDown(KEY_A)) characterPos.x -= 5;
                if (IsKeyDown(KEY_D)) characterPos.x += 5;


                if (characterPos.x < 0) characterPos.x = 0;
                if (characterPos.x > screenWidth - character.width) characterPos.x = screenWidth - character.width;

                // Spawn blocks
                if (gameplayFrames % 60 == 0 && blockCount < MAX_RECTANGLES) {
                    blocks[blockCount].rect.x = GetRandomValue(0, screenWidth - 40);
                    blocks[blockCount].rect.y = 0;
                    blocks[blockCount].rect.width = 40;
                    blocks[blockCount].rect.height = 40;
                    blocks[blockCount].speedY = GetRandomValue(2, 4);
                    blockCount++;
                }

                //Score
                if (gameplayFrames % 60 == 0 ) {
                    score++;
                }

                for (int i = 0; i < blockCount; i++) {
                    blocks[i].rect.y += blocks[i].speedY;


                    if (CheckCollisionRecs(blocks[i].rect, characterRectangle)){
                        if (score > hiscore) {
                            hiscore = score;
                            SaveStorageValue(STORAGE_POSITION_HISCORE, hiscore);
                        }

                        SaveStorageValue(STORAGE_POSITION_SCORE, score);
                        PlaySound(fxWav);
                        currentScreen = ENDING;
                        break;
                    }
                }

                break;
            case ENDING:

                if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)) {


                    blockCount = 0;
                    gameplayFrames = 0;
                    score = 0;


                    // Reset character position
                    characterPos = (Vector2){screenWidth / 2, screenHeight - character.height};
                    characterRectangle.x = characterPos.x;
                    characterRectangle.y = characterPos.y;

                    // Reset blocks (empty the array)
                    for (int i = 0; i < MAX_RECTANGLES; i++) {
                        blocks[i].rect.y = -1000;
                    }

                    currentScreen = TITLE;
                }
                break;
            default:
                break;
        }


        characterRectangle.x = characterPos.x;
        characterRectangle.y = characterPos.y;

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (currentScreen) {
            case LOGO:
                DrawText("Dodge the Blocks", 550, 300, 40, BLACK);
                DrawText("Wait to load the game...", 625, 400,20, BLACK);
                break;
            case TITLE:
                DrawRectangle(0, 0, screenWidth, screenHeight, SKYBLUE);
                DrawText("DODGE THE BLOCKS", 550, 300, 40, BLACK);
                DrawText(TextFormat("HIGHEST-SCORE: %i", hiscore), 600,400,30, RED);
                DrawText("Press ENTER or TAP to play the game", 650, 625, 10, BLACK);
                break;
            case GAMEPLAY:
                DrawRectangle(0, 0, screenWidth, screenHeight, RAYWHITE);
                //DrawFPS(0, 0);

                // Draw the character
                DrawTexturePro(character, (Rectangle){0, 0, character.width, character.height},
                    (Rectangle){characterPos.x, characterPos.y, character.width * 1.25f, character.height * 1.25f},
                    characterCenter, 0, RAYWHITE);


                for (int i = 0; i < blockCount; i++) {
                    DrawRectangleRec(blocks[i].rect, BLACK);
                }


                DrawText(TextFormat("Blocks: %i", blockCount), 10, 10, 20, BLACK);
                DrawText(TextFormat("SCORE: %i", score), 1300, 10, 20, BLACK);


                if (gameplayFrames < 120) {
                    DrawText("Press A or D to move", 550, 300, 20, BLACK);
                }
                break;
            case ENDING:
                DrawRectangle(0, 0, screenWidth, screenHeight, RAYWHITE);
                DrawText("You have died, TAP or press ENTER to RETURN to the Title Screen", 400 , 300, 20, BLACK);
                break;
            default:
                break;
        }

        EndDrawing();
    }
    UnloadSound(fxWav);
    UnloadTexture(character);

    CloseAudioDevice();
    CloseWindow();
}

bool SaveStorageValue( unsigned int position, int value) {
    bool success = false;
    int dataSize = 0;
    unsigned int newDataSize = 0;
    unsigned char *fileData = LoadFileData(STORAGE_DATA_FILE, &dataSize);
    unsigned char *newFileData = NULL;

    if (fileData != NULL) {
         if (dataSize <= (position*sizeof(int))) {
             newDataSize = (position+1)*sizeof(int);
             newFileData = (unsigned char *)RL_REALLOC(fileData, newDataSize);

         if (newFileData != NULL) {
             int *dataPtr =  (int *)newFileData;
             dataPtr[position] = value;
         }
         else {
             TraceLog(LOG_WARNING,"FILEIO: [%s] Failed to realloc data (%u), position in bytes (%u) bigger than actual file size", STORAGE_DATA_FILE, dataSize, position*sizeof(int));

             newFileData = fileData;
             newDataSize = dataSize;

         }

         }
        else {
            newFileData = fileData;
            newDataSize = dataSize;
            int *dataPtr =  (int *)newFileData;
            dataPtr[position] = value;


        }

        success = SaveFileData(STORAGE_DATA_FILE, newFileData, newDataSize);
        RL_FREE(newFileData);

        TraceLog(LOG_INFO, "FILEIO: [%s] saved the storage value: %i", STORAGE_DATA_FILE, value);
    }
    else {
        TraceLog(LOG_INFO, "FILEIO: [%s] File created successfully", STORAGE_DATA_FILE);

        dataSize = (position +1)*sizeof(int);
        fileData = (unsigned char *)RL_MALLOC(dataSize);
        int *dataPtr =  (int *)fileData;
        dataPtr[position] = value;

        success = SaveFileData(STORAGE_DATA_FILE, fileData, dataSize);

        UnloadFileData(STORAGE_DATA_FILE);

        TraceLog(LOG_INFO, "FILEIO: [%s] Saved storage value: %i", STORAGE_DATA_FILE, value);
    }

    return success;
}

int LoadStorageValue(unsigned int position) {
    int value =0;
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(STORAGE_DATA_FILE, &dataSize);

    if (fileData != NULL) {
        if (dataSize < ((int)(position *4))) {
            TraceLog(LOG_INFO, "FILEIO: [%s] Failed to find storage position: %i",STORAGE_DATA_FILE, position);

        }
        else {
            int *dataPtr =  (int *)fileData;
            value = dataPtr[position];
        }
        UnloadFileData(fileData);

        TraceLog(LOG_INFO, "FILEIO: [%s] Loaded storage value : %i ", STORAGE_DATA_FILE, value);

    }

    return value;
}

