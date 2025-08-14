#include <stdio.h>
#include <conio.h>
#include <windows.h>
// ▉ ▲ ▓ █ ■🔺🔽🔼△🔷🔶●⬜◭△🛆▴
/*  
    1. 난이도 선택 (맵크기)
    2. 시야 범위 선택 or 아이템 획득 시 범위 증가, 감소
    3. regame
    4. time 기록 측정
    5. 옵션 (아이콘 선택)
*/ 
#define UP_KEY     72
#define LEFT_KEY   75
#define DOWN_KEY   80
#define RIGHT_KEY  77

#define PATH 0 // 길
#define WALL 1 // 벽

#define EASY_WIDTH  25
#define EASY_HEIGHT 25
#define NORMAL_WIDTH  50
#define NORMAL_HEIGHT 29
#define HARD_WIDTH  50
#define HARD_HEIGHT 35

#define MAX_WIDTH  194//133
#define MAX_HEIGHT 50

#define VISION_RANGE 5  // 시야 범위

#define EASY_MAP_FILE_PATH      "Map\\EasyMap.txt"
#define NORMAL_MAP_FILE_PATH    "Map\\NormalMap.txt"
#define HARD_MAP_FILE_PATH      "Map\\HardMap.txt"

void Goto_XY(int x, int y, char* str);
void HideCursor(int cursor);
void MoveUser(COORD *userPos, unsigned char *wall);
void ShowUser(COORD *userPos, unsigned char *wall, int newX, int newY);
void TriggerAltEnter(void);
unsigned char* LoadMazeText(int mapSelect);
void Setup(void);
void InitMap(COORD uPos, unsigned char *wall);
int SelectMaze(void);
void UpdateCharacterMap(COORD *user, unsigned char *wall, int xPos, int yPos);

COORD mapSize = {0, 0};
int main(void)
{
    COORD userPos = {1, 1};
    Setup();
    unsigned char* wallData = LoadMazeText(SelectMaze());

    InitMap(userPos, wallData); // 초기 맵 출력
    Goto_XY(userPos.X, userPos.Y, "△");
    while(1)
    {
        MoveUser(&userPos, wallData);
        Sleep(10);
    }
    free(wallData);
    return 0;
}

void InitMap(COORD uPos, unsigned char *wall)
{
    for(int y=0 ; y<mapSize.Y ; y++)
    {
        for(int x=0 ; x<mapSize.X ; x++)
        {
            if(/*y == 0 || y == mapSize.Y-1 || x == 0 || x == mapSize.X-1 ||*/  (x <= uPos.X+VISION_RANGE && y <= uPos.Y+VISION_RANGE))
            {
                if(*(wall+(y*mapSize.X+x)) == WALL)
                    Goto_XY(x, y, "██");
            }
            
            else
                Goto_XY(x, y, "  ");
        }
    }
}

int SelectMaze(void)
{
    Goto_XY(-3, -3, "> Easy");
    Goto_XY(-2, -2, "Normal");
    Goto_XY(-2, -1, "Hard");
    int idx = 3;
    int prevIdx = idx;
    while(1)
    {
        if(GetAsyncKeyState(VK_RETURN) & 0x8000)                return idx;

        else if(GetAsyncKeyState(VK_UP) & 0x8000 && idx < 3)    idx++;
        
        else if(GetAsyncKeyState(VK_DOWN) & 0x8000 && idx > 1)  idx--;
    
        else 
            continue;
    
        Goto_XY(-3, -prevIdx, " ");
        Goto_XY(-3, -idx, ">");
        
        prevIdx = idx;
        Sleep(100);
    }
}

unsigned char* LoadMazeText(int mapSelect)
{
    char *mazeMapFilePath[] = {HARD_MAP_FILE_PATH, NORMAL_MAP_FILE_PATH, EASY_MAP_FILE_PATH};
    COORD size[] = {{HARD_WIDTH, HARD_HEIGHT}, {NORMAL_WIDTH, NORMAL_HEIGHT}, {EASY_WIDTH, EASY_HEIGHT}};
    mapSelect--; // index 맞추기 위함 (1,2,3) -> (0,1,2)
    mapSize = size[mapSelect];

    unsigned char *wall = (unsigned char*)malloc(sizeof(unsigned char) * (size[mapSelect].X*size[mapSelect].Y)); // 1:벽, 0:길
    FILE *map = fopen(mazeMapFilePath[mapSelect], "r");

    if(map == NULL)
    {
        printf("\n   Failed to open file. \n");
        getch();
        exit(1);
    }

    int ch;
    int len = 0;
    while ((ch = fgetc(map)) != EOF)
    {
        if(ch == '0' || ch == '1')
            wall[len++] = ch - '0';
    
        else if(ch == '\n' || ch == '\r')
            continue;

        else if(len > size[mapSelect].X * size[mapSelect].Y)
        {
            printf("Map size mismatch. Expected %d, got %d\n", size[mapSelect].X * size[mapSelect].Y, len);
            getch();
            exit(1);
        }
        
        else 
        {
            printf("Invalid character: %c\n", ch);
            getch();
            exit(1);
        }
    }
    return wall;
}
// void MoveUser(COORD *userPos, unsigned char *wall)   // getch()를 사용하니 키보드 반복 입력 지연 발생
// {
//     int key = getch();
    
//     if(key == 0 || key == 224)
//     {
//         int newX = userPos->X;
//         int newY = userPos->Y;
//         key = getch();
//         if(key == UP_KEY)           newY--;
//         else if(key == DOWN_KEY)    newY++;
//         else if(key == RIGHT_KEY)   newX++;
//         else if(key == LEFT_KEY)    newX--;

//         if(*(wall+(newY*WIDTH+newX)) != WALL)
//         {
//             Goto_XY(userPos->X, userPos->Y, " ");   // 기존 위치 지우기
//             userPos->X = newX;
//             userPos->Y = newY;
//             Goto_XY(userPos->X, userPos->Y, "#");   // 새로운 위치 갱신
//         }
//     }
// }
void MoveUser(COORD *userPos, unsigned char *wall)
{
    static DWORD lastMoveTime = 0;
    DWORD now = GetTickCount();

    const DWORD moveInterval = 40; // 이동속도 조절 (ms)

    if(now - lastMoveTime < moveInterval)
        return;

    int newX = userPos->X;
    int newY = userPos->Y;

    if(GetAsyncKeyState(VK_UP) & 0x8000)         newY--;
    else if(GetAsyncKeyState(VK_DOWN) & 0x8000)  newY++;
    else if(GetAsyncKeyState(VK_LEFT) & 0x8000)  newX--;
    else if(GetAsyncKeyState(VK_RIGHT) & 0x8000) newX++;
    else
        return;


    ShowUser(userPos, wall, newX, newY);
    lastMoveTime = now;
}

void ShowUser(COORD *userPos, unsigned char *wall, int newX, int newY)
{
    if(*(wall + (newY * mapSize.X + newX)) != WALL)
    {
        Goto_XY(userPos->X, userPos->Y, "  ");
        Goto_XY(newX, newY, "△");
        UpdateCharacterMap(userPos, wall, newX, newY);
        userPos->X = newX;
        userPos->Y = newY;
    }
}

void UpdateCharacterMap(COORD *user, unsigned char *wall, int xPos, int yPos)
{
    if(user->Y - yPos == 1)    // 위
    {
        for(int x=user->X-VISION_RANGE ; x<=(user->X+VISION_RANGE) ; x++)
        {   
            if(user->Y+VISION_RANGE <= mapSize.Y-1 && x >= 0 && x < mapSize.X && *(wall + ((user->Y+VISION_RANGE) * mapSize.X + x)) == WALL)
                Goto_XY(x, user->Y+VISION_RANGE, "  ");
            if(user->Y-(VISION_RANGE+1) >= 0 && x >= 0 && x < mapSize.X && *(wall + ((user->Y-(VISION_RANGE+1)) * mapSize.X + x)) == WALL)
                Goto_XY(x, user->Y-(VISION_RANGE+1), "██");
        }
    }
    else if(user->Y - yPos == -1)   // 아래
    {
        for(int x=user->X-VISION_RANGE ; x<=user->X+VISION_RANGE ; x++)
        {
            if(user->Y-VISION_RANGE >= 0 && x >= 0 && x < mapSize.X && *(wall + ((user->Y-VISION_RANGE) * mapSize.X + x)) == WALL)
                Goto_XY(x, user->Y-VISION_RANGE, "  ");
            if(user->Y+(VISION_RANGE+1) <= mapSize.Y-1 && x >= 0 && x < mapSize.X && *(wall + ((user->Y+(VISION_RANGE+1)) * mapSize.X + x)) == WALL)
                Goto_XY(x, user->Y+(VISION_RANGE+1), "██");
        }
    }
    else if(user->X - xPos == 1)   // 왼쪽
    {
        for(int y=user->Y-VISION_RANGE ; y<=user->Y+VISION_RANGE ; y++)
        {
            if(user->X+VISION_RANGE <= mapSize.X-1 && y >= 0 && y < mapSize.Y && *(wall + (y * mapSize.X + (user->X+VISION_RANGE))) == WALL)
                Goto_XY(user->X+VISION_RANGE, y, "  ");
            if(user->X-(VISION_RANGE+1) >= 0 && y >= 0 && y < mapSize.Y && *(wall + (y * mapSize.X + (user->X-(VISION_RANGE+1)))) == WALL)
                Goto_XY(user->X-(VISION_RANGE+1), y, "██");
        }
    }
    else if(user->X - xPos == -1)   // 오른쪽
    {
        for(int y=user->Y-VISION_RANGE ; y<=user->Y+VISION_RANGE ; y++)
        {
            if(user->X-VISION_RANGE >= 0 && y >= 0 && y < mapSize.Y && *(wall + (y * mapSize.X + (user->X-VISION_RANGE))) == WALL)
                Goto_XY(user->X-VISION_RANGE, y, "  ");
            if(user->X+(VISION_RANGE+1) <= mapSize.X-1 && y >= 0 && y < mapSize.Y && *(wall + (y * mapSize.X + (user->X+(VISION_RANGE+1)))) == WALL)
                Goto_XY(user->X+(VISION_RANGE+1), y, "██");
        }
    }
    else
        return;
}

void Goto_XY(int x, int y, char* str)
{
    COORD map = {MAX_WIDTH/2-mapSize.X, MAX_HEIGHT/2-(mapSize.Y/2)};
    COORD pos = {x*2+map.X, y+map.Y};

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    printf("%s", str);
}

void TriggerAltEnter(void)
{
    // 콘솔창 전체화면을 위해 Alt+Enter 누르기
    keybd_event(VK_MENU, 0x38, 0, 0);
    keybd_event(VK_RETURN, 0x1C, 0, 0);

    keybd_event(VK_RETURN, 0x1C, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_MENU, 0x38, KEYEVENTF_KEYUP, 0);
}

void HideCursor(int cursor)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = cursor;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void Setup(void)
{
    TriggerAltEnter();              // 콘솔창 전체화면
    HideCursor(FALSE);              // 커서 숨기기
    SetConsoleOutputCP(CP_UTF8);    // 콘솔창을 UTF-8로 설정
    Sleep(100);
}