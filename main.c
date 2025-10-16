#include <stdio.h>
#include <conio.h>
#include <windows.h>    // COORD
#include <time.h>       // time()
#include <string.h>     // memset()
// ▉ ▲ ▓ █ ■🔺🔽🔼△🔷🔶●⬜◭△🛆▴
/*  
    1. 난이도 선택 (맵크기) [완]
    2. 시야 범위 선택 or 아이템 획득 시 범위 증가, 감소
    3. regame
    4. time 기록 측정
    5. 옵션 (아이콘 선택)
*/ 
#define UP_KEY     72
#define LEFT_KEY   75
#define DOWN_KEY   80
#define RIGHT_KEY  77


#define EASY_WIDTH  25
#define EASY_HEIGHT 25
#define NORMAL_WIDTH  51
#define NORMAL_HEIGHT 29
#define HARD_WIDTH 81
#define HARD_HEIGHT 45


#define MAX_WIDTH  194//133
#define MAX_HEIGHT 50

#define VISION_RANGE 5  // 시야 범위

typedef enum
{
    MAP_WALL,
    MAP_EMPTY,
    MAP_VISITED
} MapFlag;

typedef enum
{
    DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_LEFT,
	DIRECTION_RIGHT
} Direction;

const int DIR[4][2] = {{0,-2}, {0,2}, {-2,0}, {2,0}};
COORD mapSize = {0, 0};

void Goto_XY(int x, int y, char* str);
void GetMazeSize(void);
void GetMaze(unsigned char map[][mapSize.X]);
int InRange(int y, int x);
void HideCursor(int cursor);
void MoveUser(COORD *userPos, unsigned char map[][mapSize.X]);
void ShowUser(COORD *userPos, unsigned char map[][mapSize.X], int newX, int newY);
void TriggerAltEnter(void);
void Setup(void);
void InitMap(COORD uPos, unsigned char map[][mapSize.X]);
int SelectMaze(void);
void UpdateVisionMap(COORD *user, unsigned char map[][mapSize.X], int xPos, int yPos);

void ShuffleArray(int array[], int size)
{
    int r, temp;

    for(int i=0 ; i<(size-1) ; i++)
    {
        r = i + (rand() % (size - i));
        temp = array[r];
        array[r] = array[i];
        array[i] = temp;
    }
}

void GenerateMaze(int y, int x, unsigned char map[][mapSize.X])
{
    int nx, ny;
    int directions[4] = {
        DIRECTION_UP,
        DIRECTION_DOWN,
		DIRECTION_LEFT,
		DIRECTION_RIGHT
    };

    map[y][x] = MAP_VISITED;    // 방문한 정점 표시
    ShuffleArray(directions, 4);

    for(int i=0 ; i<4 ; i++)
    {
        // 다음 위치 구하기
        nx = x + DIR[directions[i]][0];
        ny = y + DIR[directions[i]][1];
        
        if(InRange(ny, nx) && map[ny][nx] == MAP_WALL)
        {
            GenerateMaze(ny, nx, map);

            // y축 이동일 시 정점 사이에 길 뚫기
            if(ny != y)
                map[(ny+y)/2][nx] = MAP_EMPTY;

            // x축 이동일 시 정점 사이에 길 뚫기
            else
                map[ny][(nx+x)/2] = MAP_EMPTY;

            map[ny][nx] = MAP_EMPTY;    // 이동 정점에 길 뚷기
        }
    }
}

COORD getRandomStartingPoint(void)
{
	int x = 1 + rand() % (mapSize.X - 1);
	int y = 1 + rand() % (mapSize.Y - 1);
	if (x % 2 == 0)
		x--;
	if (y % 2 == 0)
		y--;
	return (COORD){x, y};
}

int InRange(int y, int x)
{
    return (y < mapSize.Y-1 && y > 0) && (x < mapSize.X-1 && x > 0);
}

int RandomNumber(int num)
{
    return rand() % num + 1;
}

void InitMap(COORD uPos, unsigned char map[][mapSize.X])
{
    for(int y=0 ; y<mapSize.Y ; y++)
    {
        for(int x=0 ; x<mapSize.X ; x++)
        {
            if(/*y == 0 || y == mapSize.Y-1 || x == 0 || x == mapSize.X-1 ||*/  (x <= uPos.X+VISION_RANGE && y <= uPos.Y+VISION_RANGE))
            {
                if(map[y][x] == MAP_WALL)
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
    int idx = 0;
    int prevIdx = idx;
    while(1)
    {
        if(GetAsyncKeyState(VK_RETURN) & 0x8000)                return idx;

        else if(GetAsyncKeyState(VK_UP) & 0x8000 && idx > 0)    idx--;
        
        else if(GetAsyncKeyState(VK_DOWN) & 0x8000 && idx < 2)  idx++;
    
        else 
            continue;
    
        Goto_XY(-3, prevIdx-3, " ");
        Goto_XY(-3, idx-3, ">");
        
        prevIdx = idx;
        Sleep(100);
    }
}

void GetMazeSize(void)
{
    COORD size[] = {{EASY_WIDTH, EASY_HEIGHT}, {NORMAL_WIDTH, NORMAL_HEIGHT}, {HARD_WIDTH, HARD_HEIGHT}};
    int mapSelect = SelectMaze();
    mapSize = size[mapSelect];
}

void GetMaze(unsigned char map[][mapSize.X])
{
    COORD startPoint = getRandomStartingPoint();

    GenerateMaze(startPoint.Y, startPoint.X, map);  // 미로 맵 생성
}

void MoveUser(COORD *userPos, unsigned char map[][mapSize.X])
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


    ShowUser(userPos, map, newX, newY);
    lastMoveTime = now;
}

void ShowUser(COORD *userPos, unsigned char map[][mapSize.X], int newX, int newY)
{
    if(map[newY][newX] != MAP_WALL)
    {
        Goto_XY(userPos->X, userPos->Y, "  ");
        Goto_XY(newX, newY, "△");
        UpdateVisionMap(userPos, map, newX, newY);
        userPos->X = newX;
        userPos->Y = newY;
    }
}

void UpdateVisionMap(COORD *user, unsigned char map[][mapSize.X], int xPos, int yPos)
{
    if(user->Y - yPos == 1)    // 위
    {
        for(int x=user->X-VISION_RANGE ; x<=(user->X+VISION_RANGE) ; x++)
        {   
            if(user->Y+VISION_RANGE <= mapSize.Y-1 && x >= 0 && x < mapSize.X && map[user->Y+VISION_RANGE][x]/**(wall + ((user->Y+VISION_RANGE) * mapSize.X + x))*/ == MAP_WALL)
                Goto_XY(x, user->Y+VISION_RANGE, "  ");
            if(user->Y-(VISION_RANGE+1) >= 0 && x >= 0 && x < mapSize.X && map[user->Y-(VISION_RANGE+1)][x] == MAP_WALL)
                Goto_XY(x, user->Y-(VISION_RANGE+1), "██");
        }
    }
    else if(user->Y - yPos == -1)   // 아래
    {
        for(int x=user->X-VISION_RANGE ; x<=user->X+VISION_RANGE ; x++)
        {
            if(user->Y-VISION_RANGE >= 0 && x >= 0 && x < mapSize.X && map[user->Y-VISION_RANGE][x] == MAP_WALL)
                Goto_XY(x, user->Y-VISION_RANGE, "  ");
            if(user->Y+(VISION_RANGE+1) <= mapSize.Y-1 && x >= 0 && x < mapSize.X && map[user->Y+(VISION_RANGE+1)][x] == MAP_WALL)
                Goto_XY(x, user->Y+(VISION_RANGE+1), "██");
        }
    }
    else if(user->X - xPos == 1)   // 왼쪽
    {
        for(int y=user->Y-VISION_RANGE ; y<=user->Y+VISION_RANGE ; y++)
        {
            if(user->X+VISION_RANGE <= mapSize.X-1 && y >= 0 && y < mapSize.Y && map[y][user->X+VISION_RANGE] == MAP_WALL)
                Goto_XY(user->X+VISION_RANGE, y, "  ");
            if(user->X-(VISION_RANGE+1) >= 0 && y >= 0 && y < mapSize.Y && map[y][user->X-(VISION_RANGE+1)] == MAP_WALL)
                Goto_XY(user->X-(VISION_RANGE+1), y, "██");
        }
    }
    else if(user->X - xPos == -1)   // 오른쪽
    {
        for(int y=user->Y-VISION_RANGE ; y<=user->Y+VISION_RANGE ; y++)
        {
            if(user->X-VISION_RANGE >= 0 && y >= 0 && y < mapSize.Y && map[y][user->X-VISION_RANGE] == MAP_WALL)
                Goto_XY(user->X-VISION_RANGE, y, "  ");
            if(user->X+(VISION_RANGE+1) <= mapSize.X-1 && y >= 0 && y < mapSize.Y && map[y][user->X+(VISION_RANGE+1)] == MAP_WALL)
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
    srand(time(NULL));
    Sleep(100);
}

int main(void)
{
    COORD userPos = {1, 1};
    Setup();
    GetMazeSize();
    unsigned char mazeMap[mapSize.Y][mapSize.X];
    memset(mazeMap, MAP_WALL, sizeof(mazeMap)); // 배열 0으로 초기화
    GetMaze(mazeMap);


    InitMap(userPos, mazeMap); // 초기 맵 출력
    Goto_XY(userPos.X, userPos.Y, "△");
    while(1)
    {
        MoveUser(&userPos, mazeMap);
        Sleep(10);
    }
    return 0; 
}