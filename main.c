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

#define WIDTH  26
#define HEIGHT 29

#define MAX_WIDTH  194//133
#define MAX_HEIGHT 50

#define VISION_RANGE 5  // 시야 범위
#define MAZE_MAP_FILE_PATH "Map\\2.txt" //"Map\\50x35 map.txt"

void Goto_XY(int x, int y, char* str);
void HideCursor(int cursor);
void MoveUser(COORD *userPos, unsigned char *wall);
void ShowUser(COORD *userPos, unsigned char *wall, int newX, int newY);
void TriggerAltEnter(void);
void LoadMazeText(unsigned char *wall);
void Setup(void);
void InitMap(COORD uPos, unsigned char *wall);

void UpdateCharacterMap(COORD *user, unsigned char *wall, int xPos, int yPos);

int main(void)
{
    COORD userPos = {0, 1};
    unsigned char *wallData = (unsigned char*)malloc(sizeof(unsigned char) * (WIDTH*HEIGHT)); // 1:벽, 0:길
    Setup();
    LoadMazeText(wallData);

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
    for(int y=0 ; y<HEIGHT ; y++)
    {
        for(int x=0 ; x<WIDTH ; x++)
        {
            if(/*y == 0 || y == HEIGHT-1 || x == 0 || x == WIDTH-1 ||*/ (x <= uPos.X+VISION_RANGE && y <= uPos.Y+VISION_RANGE))
            {
                if(*(wall+(y*WIDTH+x)) == WALL)
                    Goto_XY(x, y, "██");
            }
            
            else
                Goto_XY(x, y, "  ");
        }
    }
}

void LoadMazeText(unsigned char *wall)
{
    FILE *map = fopen(MAZE_MAP_FILE_PATH, "r");

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

        else if(len > WIDTH * HEIGHT)
        {
            printf("Map size mismatch. Expected %d, got %d\n", WIDTH * HEIGHT, len);
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
    if(*(wall + (newY * WIDTH + newX)) != WALL)
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
            if(user->Y+VISION_RANGE <= HEIGHT-1 && x >= 0 && x < WIDTH && *(wall + ((user->Y+VISION_RANGE) * WIDTH + x)) == WALL)
                Goto_XY(x, user->Y+VISION_RANGE, "  ");
            if(user->Y-(VISION_RANGE+1) >= 0 && x >= 0 && x < WIDTH && *(wall + ((user->Y-(VISION_RANGE+1)) * WIDTH + x)) == WALL)
                Goto_XY(x, user->Y-(VISION_RANGE+1), "██");
        }
    }
    else if(user->Y - yPos == -1)   // 아래
    {
        for(int x=user->X-VISION_RANGE ; x<=user->X+VISION_RANGE ; x++)
        {
            if(user->Y-VISION_RANGE >= 0 && x >= 0 && x < WIDTH && *(wall + ((user->Y-VISION_RANGE) * WIDTH + x)) == WALL)
                Goto_XY(x, user->Y-VISION_RANGE, "  ");
            if(user->Y+(VISION_RANGE+1) <= HEIGHT-1 && x >= 0 && x < WIDTH && *(wall + ((user->Y+(VISION_RANGE+1)) * WIDTH + x)) == WALL)
                Goto_XY(x, user->Y+(VISION_RANGE+1), "██");
        }
    }
    else if(user->X - xPos == 1)   // 왼쪽
    {
        for(int y=user->Y-VISION_RANGE ; y<=user->Y+VISION_RANGE ; y++)
        {
            if(user->X+VISION_RANGE <= WIDTH-1 && y >= 0 && y < HEIGHT && *(wall + (y * WIDTH + (user->X+VISION_RANGE))) == WALL)
                Goto_XY(user->X+VISION_RANGE, y, "  ");
            if(user->X-(VISION_RANGE+1) >= 0 && y >= 0 && y < HEIGHT && *(wall + (y * WIDTH + (user->X-(VISION_RANGE+1)))) == WALL)
                Goto_XY(user->X-(VISION_RANGE+1), y, "██");
        }
    }
    else if(user->X - xPos == -1)   // 오른쪽
    {
        for(int y=user->Y-VISION_RANGE ; y<=user->Y+VISION_RANGE ; y++)
        {
            if(user->X-VISION_RANGE >= 0 && y >= 0 && y < HEIGHT && *(wall + (y * WIDTH + (user->X-VISION_RANGE))) == WALL)
                Goto_XY(user->X-VISION_RANGE, y, "  ");
            if(user->X+(VISION_RANGE+1) <= WIDTH-1 && y >= 0 && y < HEIGHT && *(wall + (y * WIDTH + (user->X+(VISION_RANGE+1)))) == WALL)
                Goto_XY(user->X+(VISION_RANGE+1), y, "██");
        }
    }
    else
        return;
}

void Goto_XY(int x, int y, char* str)
{
    COORD map = {MAX_WIDTH/2-WIDTH, MAX_HEIGHT/2-(HEIGHT/2)};//{MAX_WIDTH/2-WIDTH, MAX_HEIGHT/2-(HEIGHT/2)};
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