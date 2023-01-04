// 1971341, 김태하
// 턴제 RPG
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#include <Windows.h>

 /*색상 정의*/
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW1	6
#define GRAY1	7
#define GRAY2	8
#define BLUE2	9
#define GREEN2	10
#define CYAN2	11
#define RED2	12
#define MAGENTA2 13
#define YELLOW2	14
#define WHITE	15

#define ESC 0x1b
#define ENTER 13

#define UP  0x48 // Up key는 0xe0 + 0x48 두개의 값이 들어온다.
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d

#define UP2		'w'
#define DOWN2	's'
#define LEFT2	'a'
#define RIGHT2	'd'

#define SPECIAL1 0xe0 // 특수키는 0xe0 + key 값으로 구성된다.
#define SPECIAL2 0x00 // keypad 경우 0x00 + key 로 구성된다.

// 함수 원형 정의
void print_status();
void Main_Game();
void GameStart();
void fight_monster();
void calc_attack_status();
void make_character();
int check_key();
int avoid_probability();
void shopping();
void attack_cycle();

// 몬스터 상태
typedef struct monster_status {
	char name[50];	// 몬스터 이름
	int level;	// 몬스터의 레벨

	int G_EXP;  // 처치하였을 때 주는 경험치
	int G_Gold;	// 처치하였을 때 주는 골드

	int hp;		// 몬스터의 기본 체력
	int remain_hp;	// 몬스터의 남은 체력

	int attack;		// 몬스터의 공격력
}Monster;

// 몬스터 종류
Monster m[4] = {{"피카츄", 1, 50, 50, 100, 100, 10},
	{"달곰", 2, 100, 70, 700, 700, 60},
	{"메카티안", 3, 120, 100, 850, 850, 70} ,
	{"핑크빈", 4, 150, 150, 2000, 2000, 100} };

typedef struct skills {	// 스킬 종류들
	char name[50];		// 스킬 이름
	int mp;				// 스킬 사용시 소모 MP
	int level;			// 스킬 사용 가능 레벨
}Skills;

// 스킬 정의
Skills skills[4][4] = { {{ "기본 공격",1,1 }, { "", 1, 2 }, { "", 1, 3 }, { "", 1, 4 }} ,
	{{ "기본 공격", 0 ,1 }, { "강타", 20 , 2 }, { "인내심", 20, 3 }, { "최후의 일격", 40, 4 }} ,
		{{ "기본 공격", 0 ,1 }, { "난사", 20, 2 }, { "치명타", 30, 3 }, { "마법의 수정화살", 50, 4 }},
			{{ "기본 공격", 0 ,1 }, { "출혈", 20, 2 }, { "은신", 40, 3 }, { "암살", 60, 4 }}
};

// ======================================================== //

// 아이템
typedef struct items {
	int HP_potion;	// 일반 HP물약
	int MP_potion;	// 일반 MP물약
	int HP_potion2;	// 고급 HP물약
	int MP_potion2;	// 고급 MP물약
}Items;

// 공격력
typedef struct attack {
	int dmg;
	int max_dmg;	// 최대 공격력
	int min_dmg;	// 최소 공격력
}Attack;

// ======================================================== //

typedef struct staus {   // 캐릭터 능력치
	char name[12];		// 유저이름
	int type;			// 직업 종류
	char job[15];		// 직업 이름
	int level;			// 레벨
	int EXP;			// 경험치
	int max_EXP;		// 레벨업이 되기위한 경험치
	char weapon[50];	// 착용중인 무기
	Attack attack;		// 공격력
	int defense;		// 방어력
	int tmp_def;		// 임시 방어력
	int avoid;			// 공격 회피율
	int run_avoid;		// 도망 회피율
	int Gold;			// 재화
	Items items;		// 아이템
	int hp;				// 체력
	int remain_hp;		// 남은 체력
	int mp;				// 마나
	int remain_mp;		// 남은 마나
	int change_job;		// 전직 유무 확인
}Status;

// 암살자 스킬 턴
int poison = 0; 
int hide = 0;
// 전사 스킬 턴
int s_def = 0; 
int stun = 0;
// 궁수 스킬 턴
int cri = 0;

// 몬스터 스킬 턴
int m_stun = 0;

char buf[256]; // 공용으로 쓸 배열

int debug = 0; // 접속 확인

char character[4] = "옷";  //캐릭터 모양
int cx = 2, cy = 1;		  //캐릭터 초기 좌표
unsigned char input;	  // 키 입력 받을 때 사용하는 변수
int current_map = 0;	  // 맵 위치 설정


// 색깔 설정
void getcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}

//내가 원하는 위치로 커서 이동
void gotoxy(int x, int y)
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void removeCursor(void) { // 커서를 안보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

// 맵 3차원 배열
int map[3][19][40] =
{
	{// 마을
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,
	1 ,2 ,2 ,2 ,2 ,2 ,7 ,20,0 ,20,0 ,0 ,20,0 ,0 ,20,0 ,0 ,0 ,4 ,0 ,0 ,20,20,0 ,7 ,19,19,19,19,19,7 ,0, 0 ,21,21,0 ,0 ,1 ,3 ,
	1 ,2 ,2 ,2 ,2 ,2 ,7 ,0 ,20,20,0 ,20,0 ,0 ,0 ,20,0 ,15,4 ,4 ,4 ,15,0 ,20,0 ,7 ,19,16,16,19,19,7 ,0 ,21,21,21,21,21,1 ,3 ,
	1 ,7 ,2 ,2 ,2 ,2 ,7 ,0 ,0 ,0 ,20,20,0 ,20,20,0 ,0 ,15,15,4 ,15,15,20,0 ,20,7 ,19,19,19,19,19,7 ,0 ,21,21,21,21,21,1 ,3 ,
	1 ,7 ,2 ,2 ,2 ,2 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,22,22,22,22,22,7 ,7 ,7 ,7 ,19,19,19,16,19,7 ,21,21,21,21,0 ,21,1 ,3 ,
	1 ,7 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,7 ,22,22,25,22,22,7 ,2 ,2 ,7 ,19,19,19,19,19,7 ,21,21,21,21,6 ,21,1 ,3 ,
	1 ,7 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,7 ,7 ,2 ,2 ,2 ,7 ,7 ,2 ,2 ,7 ,16,16,19,19,19,7 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,7 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,7 ,19,19,16,16,19,7 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,7 ,19,19,19,19,19,7 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,20,0 ,0 ,5 ,0 ,20,20,0 ,7 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,7 ,19,19,19,16,19,7 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,20,20,5 ,4 ,5 ,0 ,0 ,0 ,7 ,2 ,2 ,2 ,18,1 ,1 ,1 ,1 ,1 ,18,2 ,2 ,2 ,2 ,2 ,7 ,19,16,19,19,19,7 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,0 ,5 ,4 ,4 ,4 ,5 ,20,20,7 ,2 ,2 ,2 ,1 ,16,16,19,19,19,1 ,2 ,2 ,2 ,2 ,2 ,7 ,19,19,19,19,19,7 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,5 ,4 ,4 ,4 ,4 ,4 ,5 ,0 ,7 ,2 ,2 ,2 ,1 ,19,19,16,16,19,1 ,2 ,2 ,2 ,2 ,2 ,18,1 ,1 ,1 ,1 ,1 ,18,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,0 ,9 ,10,11,12,13,0 ,20,7 ,2 ,2 ,2 ,1 ,16,19,17,19,19,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,20,14,14,14,14,14,7 ,7 ,7 ,2 ,2 ,2 ,1 ,19,19,16,16,16,1 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,20,14,14,14,8 ,14,2 ,2 ,2 ,2 ,2 ,2 ,18,1 ,1 ,1 ,1 ,1 ,18,2 ,2 ,2 ,2 ,2 ,18,1 ,1 ,1 ,1 ,1 ,18,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,0 ,14,14,14,8 ,14,24,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,2 ,7 ,19,19,16,16,19,7 ,2 ,2 ,2 ,2 ,2 ,2 ,1 ,3 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3
	},
	{//던전
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,
	1 ,23,23,23,1 ,23,23,23,23,23,23,23,1 ,23,23,23,1 ,23,23,23,23,1 ,23,23,23,23,23,23,23,23,32,1 ,1 ,23,23,21,23,23,1 ,3 ,
	1 ,23,30,23,1 ,23,1 ,1 ,1 ,1 ,1 ,23,23,23,1 ,23,23,23,23,23,23,1 ,23,1 ,1 ,1 ,1 ,23,1 ,1 ,1 ,23,1 ,23,21,21,21,23,1 ,3 ,
	1 ,23,23,23,23,23,1 ,23,23,23,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,1 ,23,1 ,23,23,23,23,23,23,23,23,1 ,21,21,21,21,21,1 ,3 ,
	1 ,23,23,23,23,23,23,23,1 ,23,23,23,23,1 ,23,23,23,23,31,1 ,23,1 ,23,1 ,23,23,1 ,1 ,1 ,1 ,1 ,23,1 ,21,21,0 ,21,21,1 ,3 ,
	1 ,23,23,23,1 ,23,1 ,1 ,1 ,1 ,1 ,1 ,23,1 ,23,1 ,23,1 ,1 ,1 ,23,1 ,23,23,1 ,23,1 ,23,23,32,1 ,23,1 ,21,21,28,21,21,1 ,3 ,
	1 ,23,23,23,1 ,23,1 ,23,23,23,23,1 ,23,23,23,1 ,23,1 ,32,1 ,23,1 ,1 ,23,1 ,23,1 ,23,1 ,1 ,1 ,23,1 ,1 ,23,23,23,1 ,1 ,3 ,
	1 ,23,23,23,1 ,23,23,23,1 ,1 ,23,1 ,1 ,1 ,1 ,1 ,23,1 ,23,1 ,23,23,23,23,1 ,23,1 ,23,1 ,23,23,23,23,1 ,23,23,23,1 ,1 ,3 ,
	1 ,23,23,23,1 ,1 ,1 ,1 ,1 ,23,23,23,23,23,23,1 ,23,23,23,1 ,1 ,1 ,1 ,1 ,1 ,23,1 ,23,1 ,23,1 ,23,23,23,1 ,23,1 ,23,1 ,3 ,
	1 ,23,23,23,1 ,23,23,23,1 ,23,1 ,1 ,1 ,1 ,23,1 ,1 ,1 ,1 ,1 ,23,23,23,23,23,23,1 ,23,1 ,23,1 ,1 ,1 ,23,1 ,23,1 ,23,1 ,3 ,
	1 ,23,23,23,1 ,23,1 ,23,1 ,23,23,23,23,1 ,23,23,23,23,31,1 ,23,1 ,1 ,1 ,1 ,1 ,1 ,23,1 ,23,23,23,1 ,23,1 ,23,1 ,23,1 ,3 ,
	1 ,23,23,23,23,23,1 ,23,23,23,1 ,1 ,23,1 ,1 ,1 ,1 ,1 ,1 ,1 ,23,1 ,23,23,23,23,1 ,23,1 ,1 ,1 ,23,1 ,23,1 ,23,1 ,23,1 ,3 ,
	1 ,23,23,23,23,23,1 ,1 ,1 ,1 ,1 ,23,23,1 ,23,30,23,23,23,23,23,1 ,23,23,1 ,23,1 ,23,23,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,3 ,
	1 ,1 ,23,1 ,1 ,23,1 ,23,23,31,1 ,1 ,23,23,23,1 ,1 ,23,1 ,1 ,1 ,1 ,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,3 ,
	1 ,23,23,23,1 ,23,1 ,23,1 ,1 ,1 ,1 ,1 ,30,1 ,1 ,1 ,23,23,23,23,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,3 ,
	1 ,23,23,23,1 ,23,1 ,23,1 ,23,23,23,1 ,1 ,1 ,23,23,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,23,1 ,1 ,1 ,23,1 ,23,1 ,3 ,
	1 ,23,26,23,1 ,23,23,23,23,23,1 ,23,23,23,23,23,1 ,23,1 ,31,1 ,23,23,23,1 ,23,23,23,1 ,32,1 ,23,23,23,23,23,23,23,1 ,3 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3
	},
	{// 보스맵
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3 ,
	1 ,19,19,19,19,19,19,19,19,19,19,19,19,19,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,19,19,19,19,16,19,19,19,16,16,16,19,19,1 ,3 ,
	1 ,19,16,16,19,19,19,19,19,19,16,16,19,19,7 ,27,27,27,27,27,27,27,27,27,7 ,19,19,19,19,19,19,19,19,19,19,19,19,19,1 ,3 ,
	1 ,19,19,19,19,19,19,19,19,19,19,16,19,19,7 ,27,27,27,27,27,27,27,27,27,7 ,19,19,19,19,16,16,16,19,19,19,19,19,19,1 ,3 ,
	1 ,19,19,19,16,16,16,19,19,19,19,19,19,19,7 ,27,27,27,27,33,27,27,27,27,7 ,19,19,19,19,19,19,19,16,19,19,19,19,19,1 ,3 ,
	1 ,19,19,19,19,19,19,19,19,16,16,16,19,19,7 ,27,27,27,27,27,27,27,27,27,7 ,19,16,16,16,19,19,19,19,19,19,19,19,19,1 ,3 ,
	1 ,19,19,19,19,19,19,19,19,19,19,19,19,19,7 ,27,27,27,27,27,27,27,27,27,7 ,19,19,19,19,16,19,19,19,19,19,19,16,16,1 ,3 ,
	1 ,19,19,19,19,19,19,19,19,19,19,16,19,19,7 ,27,27,27,27,27,27,27,27,27,7 ,19,19,19,19,19,19,19,19,19,19,19,19,19,1 ,3 ,
	1 ,16,19,16,16,16,19,19,19,19,19,19,19,19,19,7 ,7 ,7 ,27,27,27,7 ,7 ,7 ,19,19,19,16,16,16,19,19,19,19,19,19,19,19,1 ,3 ,
	1 ,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,1 ,27,27,27,1 ,19,19,19,16,19,19,19,19,19,19,19,16,16,16,19,19,1 ,3 ,
	1 ,19,19,16,19,19,19,19,19,16,16,16,19,19,16,16,19,1 ,27,27,27,1 ,19,19,16,16,19,19,19,16,16,16,19,19,19,19,16,19,1 ,3 ,
	1 ,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,1 ,27,27,27,1 ,19,19,19,19,19,19,19,19,19,19,19,16,19,19,19,19,1 ,3 ,
	1 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,27,27,27,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,7 ,1 ,3 ,
	1 ,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,1 ,3 ,
	1 ,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,1 ,3 ,
	1 ,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,1 ,3 ,
	1 ,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,29,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,27,1 ,3 ,
	1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,3
	}
};

// 맵 그리기 함수
void print_map() {
	int i, j;
	for (i = 0; i < 19; i++) {
		for (j = 0; j < 40; j++) {
			switch (map[current_map][i][j]) {
			
			case 0:
				getcolor(GRAY1, BLACK); printf("  "); getcolor(GRAY1, BLACK);
				break;
			case 1:
				getcolor(GRAY1, BLACK);  printf("■");		// 맵 구조
				break;
			case 2:
				getcolor(BLACK, YELLOW1); printf("▒"); getcolor(GRAY1, BLACK);	// 플레이어 다니는 길
				break;
			case 3:
				printf("\n");
				break;
			case 4:
				getcolor(RED2, BLACK); printf("■"); getcolor(GRAY1, BLACK);	// 상점 지붕용
				break;
			case 5:
				getcolor(RED2, BLACK); printf("▲"); getcolor(GRAY1, BLACK);	// 상점 지붕
				break;
			case 6:
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK); // 포탈
				break;
			case 7:
				getcolor(GREEN1, BLACK);  printf("♣");  getcolor(GRAY1, BLACK);	// 나무
				break;
			case 8:
				getcolor(BLACK, BLUE1); printf("▥"); getcolor(GRAY1, BLACK); // 상점 문
				break;
			case 9:
				getcolor(YELLOW2, CYAN1); printf(" S"); getcolor(GRAY1, BLACK);
				break;
			case 10:
				getcolor(YELLOW2, CYAN1); printf(" T"); getcolor(GRAY1, BLACK);
				break;
			case 11:
				getcolor(YELLOW2, CYAN1); printf(" O"); getcolor(GRAY1, BLACK);
				break;
			case 12:
				getcolor(YELLOW2, CYAN1); printf(" R"); getcolor(GRAY1, BLACK);
				break;
			case 13:
				getcolor(YELLOW2, CYAN1); printf(" E"); getcolor(GRAY1, BLACK);
				break;
			case 14:
				getcolor(YELLOW2, CYAN1); printf("〓"); getcolor(GRAY1, BLACK);	// 상점 외부
				break;
			case 15:
				getcolor(BLACK, GRAY1); printf("  "); getcolor(GRAY1, BLACK); //  병원 외부
				break;
			case 16:
				getcolor(WHITE, BLUE1);  printf("~~");  getcolor(GRAY1, BLACK);	// 물결
				break;
			case 17:
				getcolor(WHITE, BLUE1); printf("▣"); getcolor(GRAY1, BLACK); // 분수대
				break;
			case 18:
				getcolor(GRAY1, BLACK);  printf("●"); getcolor(GRAY1, BLACK);	// 분수대
				break;
			case 19:
				getcolor(WHITE, BLUE1); printf("  "); getcolor(GRAY1, BLACK); //  물
				break;
			case 20:
				getcolor(GREEN1, BLACK);  printf(",,");  getcolor(GRAY1, BLACK);// 풀
				break;
			case 21:
				getcolor(RED1, BLACK); printf("■"); getcolor(GRAY1, BLACK);	// 지붕용
				break;
			case 22:
				getcolor(BLACK, GRAY1); printf("〓"); getcolor(GRAY1, BLACK);	// 병원 외부
				break;
				// 던전
			case 23:
				getcolor(GRAY2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);	// 플레이어 다니는 길
				break;
				// 상점
			case 24:
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK);
				break;
				// 휴식
			case 25:
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK);
				break;
				// 던전 -> 마을
			case 26:
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK);
				break;
				// 보스 맵
			case 27:
				getcolor(GREEN2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
				break;
			case 28:
				// 던전 -> 보스맵
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK);
				break;
			case 29:
				// 보스 맵 -> 던전
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK);
				break;
			case 30:
				// 튜토리얼 몬스터
				getcolor(GREEN1, BLACK); printf("ⓐ"); getcolor(GRAY1, BLACK);
				break;
			case 31:
				// 몬스터2
				getcolor(YELLOW1, BLACK); printf("ⓑ"); getcolor(GRAY1, BLACK);
				break;
			case 32:
				// 몬스터3
				getcolor(BLUE1, BLACK); printf("ⓒ"); getcolor(GRAY1, BLACK);
				break;
			case 33:
				// 보스몹
				getcolor(RED1, BLACK); printf("㉿"); getcolor(GRAY1, BLACK);
				break;
			}
		}
	}
}

// 능력치 초기화 함수
void init(Status* s, int type) {
	switch (type) {
	case 0:			// 기본 능력치
		s->level = 1;
		s->attack.dmg = 15;
		strcpy(s->job, "모험가");
		s->type = 0;
		strcpy(s->weapon, "주먹");
		s->tmp_def = 10;
		s->defense = 10;
		s->avoid = 10;
		s->run_avoid = 30;
		s->Gold = 70;
		s->hp = 100;
		s->remain_hp = 100;
		s->mp = 100;
		s->remain_mp = 100;
		s->EXP = 0;
		s->max_EXP = 100;
		s->items.HP_potion = 0;
		s->items.MP_potion = 0;
		s->items.HP_potion2 = 0;
		s->items.MP_potion2 = 0;
		s->change_job = 0;
		break;
	case 1:			// 전사 능력치
		s->attack.dmg = 40;
		strcpy(s->job, "전사");
		s->type = 1;
		strcpy(s->weapon, "검");
		s->tmp_def = 60;
		s->defense = 60;
		s->avoid = 10;
		s->run_avoid = 30;
		s->hp = 1000;
		s->remain_hp = 1000;
		s->mp = 100;
		s->remain_mp = 100;
		s->max_EXP = 150;
		s->change_job = 1;
		break;
	case 2:			// 궁수 능력치
		s->attack.dmg = 60;
		strcpy(s->job, "궁수");
		s->type = 2;
		strcpy(s->weapon, "활");
		s->defense = 40;
		s->avoid = 10;
		s->run_avoid = 30;
		s->hp = 700;
		s->remain_hp = 700;
		s->mp = 100;
		s->remain_mp = 100;
		s->max_EXP = 150;
		s->change_job = 1;
		break;
	case 3:				// 암살자 능력치
		s->attack.dmg = 70;
		strcpy(s->job, "암살자");
		s->type = 3;
		strcpy(s->weapon, "단검");
		s->defense = 40;

		s->avoid = 10;
		s->run_avoid = 30;
		s->hp = 600;
		s->remain_hp = 600;

		s->mp = 100;
		s->remain_mp = 100;

		s->max_EXP = 150;
		s->change_job = 1;
		break;
	}
}

// 알림창 틀
void notice_frame() {
	gotoxy(9, 9);  printf("┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
	gotoxy(9, 10); printf("┃                                                          ┃\n");
	gotoxy(9, 11); printf("┠━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\n");
	gotoxy(9, 12); printf("┃                                                          ┃\n");
	gotoxy(9, 13); printf("┃                                                          ┃\n");
	gotoxy(9, 14); printf("┃                                                          ┃\n");
	gotoxy(9, 15); printf("┃                                                          ┃\n");
	gotoxy(9, 16); printf("┃                                                          ┃\n");
	gotoxy(9, 17); printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n");
}

// 전직할때 표시해줄 내용들
void job_status(int x) {
	if (x == 13) {
		getcolor(RED1, BLACK);
		gotoxy(11, 12);
		printf("[전사]를 선택하셨습니다.                            ");
		gotoxy(11, 13);
		printf("HP : 1000 공격력 : 40 방어력 : 60                   ");
		gotoxy(11, 14);
		printf("스킬1 : 강타 (200의 고정피해)                       ");
		gotoxy(11, 15);
		printf("스킬2 : 방어 (방어력이 3턴간 50%%상승)              ");
		gotoxy(11, 16);
		printf("스킬3 : 일격 (400의 고정피해, 상대방 두턴동안 기절) ");
		getcolor(GRAY1, BLACK);
	}
	else if (x == 22) {
		getcolor(BLUE2, BLACK);
		gotoxy(11, 12);
		printf("[궁수]를 선택하셨습니다.                            ");
		gotoxy(11, 13);
		printf("HP : 700 공격력 : 60 방어력 : 40                    ");
		gotoxy(11, 14);
		printf("스킬1 : 난사 (추가 데미지가 들어간 기본공격을 2회)  ");
		gotoxy(11, 15);
		printf("스킬2 : 치명타 (치명타 3턴간 치명타 공격)           ");
		gotoxy(11, 16);
		printf("스킬3 : 마법의 수정화살 (300의 고정피해, 세턴 기절) ");
		getcolor(GRAY1, BLACK);
	}
	else if (x == 31) {
		getcolor(GREEN1, BLACK);
		gotoxy(11, 12);
		printf("[암살자]를 선택하셨습니다.                          ");
		gotoxy(11, 13);
		printf("HP : 600 공격력 : 70 방어력 : 40                    ");
		gotoxy(11, 14);
		printf("스킬1 : 출혈(두턴 동안 상대방 출혈효과)             ");
		gotoxy(11, 15);
		printf("스킬2 : 은신(공격을 한턴 무시, 체력 10%% 회복)      ");
		gotoxy(11, 16);
		printf("스킬3 : 암살(상대 체력이 20%% 이하일 경우 즉시 처치)");
		getcolor(GRAY1, BLACK);
	}
}

// 레벨2일때 전직을 위한 화면
int choose_job(Status* User) {
	system("cls");
	int x = 13;
	int y = 10;
	notice_frame();
	gotoxy(x, y); // ▶ 출력을 위해 x-2
	printf("▶ 전사");
	gotoxy(x + 12, y);
	printf("궁수");
	gotoxy(x + 21, y);
	printf("암살자");

	job_status(13); // 직업 내용 표시

	while (1) {
		int n = check_key();
		switch (n) {
		case 3:  // 왼쪽
			if (x > 13) {
				x -= 1;
				gotoxy(x, y);
				printf("  ");
				x -= 8;
				gotoxy(x, y);
				printf("▶");
				job_status(x);
			}
			break;
		case 4: // 오른쪽
			if (x < 30) {
				x -= 1;
				gotoxy(x, y);
				printf("  ");
				x += 10;
				gotoxy(x, y);
				printf("▶");
				job_status(x);
			}
			break;
		case 10:
			if (x == 13)return 1;
			else if (x == 22)return 2;
			else if (x == 31)return 3;
		}
	}
}

// 처치 후 보상 출력
void print_reward(Status* User, int m_num) {
	
	system("cls");
	notice_frame();
	gotoxy(11, 10); printf("보상");
	gotoxy(11, 13);

	printf("%d Gold를 획득하였습니다!", m[m_num].G_Gold);
	User->Gold += m[m_num].G_Gold;
	gotoxy(11, 14);
	printf("%d EXP를 획득하였습니다!", m[m_num].G_EXP);
	User->EXP += m[m_num].G_EXP;

	// 레벨업이 되는 경험치를 얻었을 경우
	if (User->EXP >= User->max_EXP) {
		system("cls");
		notice_frame();
		gotoxy(11, 10); printf("Level UP");
		gotoxy(11, 13); printf("Level %d로 레벨 업을 하였습니다!!", User->level + 1);

		User->hp += 500;
		User->remain_hp = User->hp; // 최대체력으로 회복
		User->remain_mp = User->mp; // 최대MP로 회복
		User->level += 1;			// 레벨 1 증가
		User->max_EXP += 100;		// 레벨업이 되기 위한 경험치 증가
		User->EXP = 0;

		// 레벨이 2가 되면 전직
		if (User->level == 2 && User->change_job == 0) {
			gotoxy(11, 14);
			printf("전직을 진행합니다!!");
		}
	}
}

// 유저 공격할때 공격력 계산
int calc_dmg(Status* User) {
	User->attack.min_dmg = (int)(User->attack.dmg * 0.8);
	User->attack.max_dmg = (int)(User->attack.dmg * 1.2);
	int dmg = User->attack.min_dmg + rand() % (User->attack.max_dmg - User->attack.min_dmg + 1);
	
	return (int)dmg;
}

// 스킬 턴 초기화
void init_skill() {
	// 암살자 스킬
	poison = 0;
	hide = 0;
	// 전사 스킬
	s_def = 0;
	stun = 0;
	// 궁수 스킬
	cri = 0;
	// 몬스터 스킬
	m_stun = 0;
}

// 레벨업, 무기 장착 등 공격력 상승했을 시 조정
void calc_attack_status(Status* User, int cal) {
	User->attack.dmg += cal;
	User->attack.min_dmg = (int)(User->attack.dmg * 0.8);
	User->attack.max_dmg = (int)(User->attack.dmg * 1.2);
}

// 보스 몹을 처치했을 경우
void print_end_msg() {
	system("cls");
	gotoxy(0, 3); getcolor(RED1, BLACK); printf("  ■■■■■  ■■■■  ■      ■ ■■■■"); getcolor(GRAY1, BLACK);
	gotoxy(0, 4); getcolor(RED1, BLACK); printf("  ■          ■    ■  ■■  ■■ ■      "); getcolor(GRAY1, BLACK);
	gotoxy(0, 5); getcolor(RED1, BLACK); printf("  ■  ■■■  ■■■■  ■  ■  ■ ■■■■"); getcolor(GRAY1, BLACK);
	gotoxy(0, 6); getcolor(RED1, BLACK); printf("  ■      ■  ■    ■  ■      ■ ■      "); getcolor(GRAY1, BLACK);
	gotoxy(0, 7); getcolor(RED1, BLACK); printf("  ■■■■■  ■    ■  ■      ■ ■■■■"); getcolor(GRAY1, BLACK);

	gotoxy(6, 10); getcolor(CYAN1, BLACK); printf("  ■■■■■■  ■            ■■■■■   ■■■■■   ■■■■   "); getcolor(GRAY1, BLACK);
	gotoxy(6, 11); getcolor(CYAN1, BLACK); printf("  ■            ■            ■           ■      ■   ■     ■  "); getcolor(GRAY1, BLACK);
	gotoxy(6, 12); getcolor(CYAN1, BLACK); printf("  ■            ■            ■           ■      ■   ■     ■  "); getcolor(GRAY1, BLACK);
	gotoxy(6, 13); getcolor(CYAN1, BLACK); printf("  ■            ■            ■■■■■   ■■■■■   ■■■■   "); getcolor(GRAY1, BLACK);
	gotoxy(6, 14); getcolor(CYAN1, BLACK); printf("  ■            ■            ■           ■      ■   ■     ■  "); getcolor(GRAY1, BLACK);
	gotoxy(6, 15); getcolor(CYAN1, BLACK); printf("  ■            ■            ■           ■      ■   ■      ■ "); getcolor(GRAY1, BLACK);
	gotoxy(6, 16); getcolor(CYAN1, BLACK); printf("  ■■■■■■  ■■■■■■  ■■■■■   ■      ■   ■       ■"); getcolor(GRAY1, BLACK);
	gotoxy(0, 20);
	exit(1);
}

// 처치했는지 확인하는 함수
void check_hp(Status* User, int hp, int skill_num,int m_num) {
	int type;
	int r_dmg = calc_dmg(User);
	if (hp <= 0) {
		if (m_num == 3) {
			printf("%s(을)를 처치하였습니다!\n", m[m_num].name);
			print_end_msg();
		}
		else {
			printf("%s(을)를 처치하였습니다!\n", m[m_num].name);
			m[m_num].remain_hp = m[m_num].hp;
			init_skill(); // 상태이상 초기화
			_getch();

			// 보상 출력
			print_reward(User, m_num);
			_getch();

			//처치 후 레벨이 2인경우 전직한 후 change_job변수에 1저장
			if (User->level == 2 && User->change_job == 0) {
				type = choose_job(User);
				init(User, type);
			}
			Main_Game(User);
		}
	}
}

// 유저의 기본 공격
void basic_attack(Status* User, int skill_num, int m_num) {
	int hp;
	// 무기와 레벨에 따른 공격력
	int r_dmg = calc_dmg(User);

	// 기절상태가 아닌 경우
	if (m_stun == 0) {
		// 치명타 공격인 경우
		if (cri >= 1) {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
			printf("%s(은)는 %d의 치명타 피해를 입었다!\n", m[m_num].name, r_dmg);
			r_dmg = r_dmg + rand() % 200 + 1;
		}
		else {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
			printf("%s(은)는 %d의 피해를 입었다!\n", m[m_num].name, r_dmg);
		}
		hp = m[m_num].remain_hp - r_dmg;

		// 체력 확인
		if (hp <= 0)check_hp(User, hp, skill_num, m_num);
		else m[m_num].remain_hp -= r_dmg; // 처치하지 않았을 때
	}
	else {
		printf("기절 상태입니다.");
		m_stun--;
	}
}

// 스킬 공격
void skill_attack(Status* User, int m_num) {
	int skill_num;
	int r_dmg;

	//스킬 출력
	for (int i = 1; i < 4; i++)
		printf("%d. %s [%d MP](Lv.%d이상)\n", i, skills[User->type][i].name, skills[User->type][i].mp, skills[User->type][i].level);
	skill_num = _getch() - '0';

	attack_cycle(User, m_num);

	// 기절상태가 아닌경우
	if (m_stun == 0) {
		// 스킬 사용 가능한 레벨인지 확인
		if (User->level < skills[User->type][skill_num].level) {
			printf("스킬 사용 가능 레벨이 아닙니다.\n");
			_getch();
			fight_monster(User, m_num);
		}

		// 전사 스킬
		else if (User->type == 1) {
			r_dmg = calc_dmg(User);
			switch (skill_num) {
			case 1: // 스킬 1 : 강타
				if (User->remain_mp - skills[User->type][skill_num].mp < 0) {
					printf("마나가 부족합니다!");
					break;
				}
				else {
					printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
					printf("%s(은)는 %s에게 %d의 고정피해를 입혔다!!\n", User->name, m[m_num].name, 200);
					m[m_num].remain_hp -= 200;
					User->remain_mp -= skills[User->type][skill_num].mp;
					break;
				}
			case 2: // 스킬 2 : 인내심
				if (User->remain_mp - skills[User->type][skill_num].mp < 0) {
					printf("마나가 부족합니다!");
					break;
				}
				else {
					printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
					printf("%s(은)는 세 턴동안 방어력이 50%% 상승합니다!!\n", User->name);
					User->defense += (int)(User->tmp_def / 2);
					User->remain_mp -= skills[User->type][skill_num].mp;
					s_def = 3;
					break;
				}
			case 3: // 스킬 3 : 최후의 일격
				if (User->remain_mp - skills[User->type][skill_num].mp < 0) {
					printf("마나가 부족합니다!");
					break;
				}
				else {
					printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
					printf("%s(은)는 %s에게 %d의 고정피해를 입혔다!!\n", User->name, m[m_num].name, 400);
					m[m_num].remain_hp -= 400;
					User->remain_mp -= skills[User->type][skill_num].mp;
					stun = 2;
					break;
				}
			}
			check_hp(User, m[m_num].remain_hp, skill_num, m_num);
		}
		// 궁수 스킬
		else if (User->type == 2) {
			switch (skill_num) {
			case 1: // 스킬 1: 난사
				r_dmg = calc_dmg(User);
				if (User->remain_mp - skills[User->type][skill_num].mp < 0) {
					printf("마나가 부족합니다!");
					break;
				}
				else {
					printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
					printf("%s(은)는 %s에게 추가피해를 입혔다!!\n", User->name, m[m_num].name);
					m[m_num].remain_hp -= r_dmg * 2;

					User->remain_mp -= skills[User->type][skill_num].mp;
					break;
				}
			case 2: // 스킬 1: 치명타
				r_dmg = calc_dmg(User);
				if (User->remain_mp - skills[User->type][skill_num].mp < 0) {
					printf("마나가 부족합니다!");
					break;
				}
				else {
					printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
					User->remain_mp -= skills[User->type][skill_num].mp;
					cri = 3;
					break;
				}
			case 3: // 스킬 3: 마법의 수정화살
				r_dmg = calc_dmg(User);
				if (User->remain_mp - skills[User->type][skill_num].mp < 0) {
					printf("마나가 부족합니다!");
					break;
				}
				else {
					printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
					printf("%s(은)는 %s에게 피해를 입혔다!!\n", User->name, m[m_num].name);
					m[m_num].remain_hp -= 300;
					User->remain_mp -= skills[User->type][skill_num].mp;
					stun = 3;
					break;
				}
			}
			check_hp(User, m[m_num].remain_hp, skill_num, m_num);
		}
		// 암살자 스킬
		else if (User->type == 3) {
			switch (skill_num) {
			case 1:		// 스킬 1 : 출혈
				r_dmg = calc_dmg(User);
				if (User->remain_mp - skills[User->type][skill_num].mp < 0) {
					printf("마나가 부족합니다!");
					break;
				}
				else {
					printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
					User->remain_mp -= skills[User->type][skill_num].mp;
					poison = 2;
					break;
				}
			case 2:		// 스킬 2 : 은신
				r_dmg = calc_dmg(User);
				if (User->remain_mp - skills[User->type][skill_num].mp < 0) {
					printf("마나가 부족합니다!");
					break;
				}
				else {
					printf("%s(은)는 %s(을)를 사용했다!\n", User->name, skills[User->type][skill_num].name);
					User->remain_mp -= skills[User->type][skill_num].mp;
					hide = 1;
					break;
				}
			case 3:		// 스킬 3 : 암살
				r_dmg = calc_dmg(User);
				if (User->remain_mp - skills[User->type][skill_num].mp < 0) {
					printf("마나가 부족합니다!");
					break;
				}
				else {
					printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", User->name, m[m_num].name, skills[User->type][skill_num].name);
					if (m[m_num].remain_hp <= (int)(m[m_num].hp * 0.2)) {
						printf("%s(은)는 암살에 성공하였다!!\n", User->name);
						User->remain_mp -= skills[User->type][skill_num].mp;
						m[m_num].remain_hp = 0;
					}
					else {
						printf("암살에 실패하였습니다...\n");
						printf("기본공격을 사용하였습니다.\n");
						User->remain_mp -= skills[User->type][skill_num].mp;
						m[m_num].remain_hp -= r_dmg;
					}
					break;
				}
			}
			check_hp(User, m[m_num].remain_hp, skill_num, m_num);
		}
	}
	else {
		printf("기절 상태입니다.");
		m_stun--;
	}
}

// 몬스터의 데미지 계산
int mon_dmg(Status* User, int m_num) {
	int min_dmg = (int)(m[m_num].attack * 0.8);
	int max_dmg = (int)(m[m_num].attack * 1.2);

	int dmg = min_dmg + rand() % (max_dmg - min_dmg - (User->defense / 4) + 1);

	int a = rand() % 100 + 1; // 확률
	int critical = rand() % 100 + 1; // 치명타

	// a 몬스터
	if (m_num == 0) {
		printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", m[m_num].name, User->name, "기본 공격");
		printf("%s(은)는 %d의 피해를 입었다!\n", User->name, (int)dmg);
		return (int)dmg;
	} 
	// b 몬스터 스킬
	else if (m_num == 1) {
		if (a >= 1 && a < 70) {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", m[m_num].name, User->name, "기본 공격");
			printf("%s(은)는 %d의 피해를 입었다!\n", User->name, (int)dmg);
			return (int)dmg;
		}
		else if (a >= 70 && a < 100) {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", m[m_num].name, User->name, "기본 공격");
			printf("%s(은)는 %d의 치명타 피해를 입었다!\n", User->name, (int)dmg + critical);
			return (int)dmg + critical;
		}
	}
	// c 몬스터 스킬
	else if (m_num == 2) {
		if (a >= 1 && a < 65) {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", m[m_num].name, User->name, "기본 공격");
			printf("%s(은)는 %d의 피해를 입었다!\n", User->name, (int)dmg);
			return (int)dmg;
		}
		else if (a >= 65 && a < 90) {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", m[m_num].name, User->name, "기본 공격");
			printf("%s(은)는 %d의 치명타 피해를 입었다!\n", User->name, (int)dmg + critical);
			return (int)dmg + critical;
		}
		else if (a >= 90 && a < 100) {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", m[m_num].name, User->name, "기본 공격");
			printf("%s(은)는 %d의 피해를 입었다!\n", User->name, (int)dmg);
			printf("%s(은)는 기절상태가 되었다!\n",User->name);
			m_stun = 1;
			return (int)dmg;
		}
	}
	else if (m_num == 3) {
		if (a >= 1 && a < 60) {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", m[m_num].name, User->name, "기본 공격");
			printf("%s(은)는 %d의 피해를 입었다!\n", User->name, (int)dmg);
			return (int)dmg;
		}
		else if (a >= 60 && a < 90) {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", m[m_num].name, User->name, "기본 공격");
			printf("%s(은)는 %d의 치명타 피해를 입었다!\n", User->name, (int)dmg + critical);
			return (int)dmg + critical;
		}
		else if (a >= 90 && a < 100) {
			printf("%s(은)는 %s에게 %s(을)를 사용했다!\n", m[m_num].name, User->name, "기본 공격");
			printf("%s(은)는 %d의 피해를 입었다!\n", User->name, (int)dmg);
			printf("%s(은)는 기절상태가 되었다!\n", User->name);
			m_stun = 1;
			return (int)dmg;
		}
	}
}

// 몬스터의 공격
void monster_attack(Status* User, int m_num) {
	int hp;
	int r_dmg = mon_dmg(User, m_num); // 몬스터 공격력 비율 계산

	hp = User->remain_hp - r_dmg;

	// 유저 체력이 0이하가 되는 경우
	if (hp <= 0) {

		system("cls");
		notice_frame();

		gotoxy(11, 10);
		printf(" 패배...");

		gotoxy(11, 13);
		printf("당신은 %s에게 패배하였습니다...", m[m_num].name);
		gotoxy(11, 14);
		printf("50 Gold를 잃었습니다...");
		_getch();

		// 몬스터 체력 복구
		m[m_num].remain_hp = m[m_num].hp;

		// 20골드 빼기
		User->Gold -= 50;
		if (User->Gold <= 0)User->Gold = 0;

		// 체력이 -일 수 없으므로 0으로 세팅
		User->remain_hp = 0;

		// 마을로 이동
		current_map = 0;
		cx = 2;
		cy = 1;
		Main_Game(User);
	}
	else User->remain_hp -= r_dmg;
}

// 공격 방법창 출력
void attack_way(Status* User) {
	gotoxy(2, 12); printf("┏공격 방법 ━━━━━━━━━━━━━┓");
	gotoxy(2, 13); printf("┃  1. 기본공격        2. 스킬        ┃");
	gotoxy(2, 14); printf("┃  3. 아이템          4. 도망친다    ┃");
	gotoxy(2, 15); printf("┗━━━━━━━━━━━━━━━━━━┛");
}

// 아이템 사용 화면
void Use_item(Status* User, int m_num) {
	int num, after_use;
	printf("1. HP포션(보유중 : %2d개)\n", User->items.HP_potion);
	printf("2. MP포션(보유중 : %2d개)\n", User->items.MP_potion);
	printf("3. 고급HP포션(보유중 : %2d개)\n", User->items.HP_potion2);
	printf("4. 고급MP포션(보유중 : %2d개)\n", User->items.MP_potion2);
	num = _getch() - '0';
	switch (num) {
	case 1:	// HP포션 사용
		// 포션이 없는 경우
		if (User->items.HP_potion == 0) {
			attack_cycle(User, m_num);
			printf("사용가능한 HP포션이 없습니다.\n");
			return;
		}
		else {	// 체력이 최대체력이 넘어가지 않도록 회복
			User->items.HP_potion--;
			after_use = User->remain_hp + 30;
			if (after_use >= User->hp)User->remain_hp = User->hp;
			else User->remain_hp += 30;
			attack_cycle(User, m_num);
			printf("체력을 30 회복하였습니다.");
			_getch();
		}
	case 2:	//MP포션 사용
		//mp가 없는 경우
		if (User->items.MP_potion == 0) {
			attack_cycle(User, m_num);
			printf("사용가능한 MP포션이 없습니다.\n");
			return;
		}
		else {	// MP가 최대가 넘어가지 않도록 회복
			User->items.MP_potion--;
			after_use = User->remain_mp + 30;
			if (after_use >= User->mp)User->remain_mp = User->mp;
			else User->remain_mp += 30;
			attack_cycle(User, m_num);
			printf("마나를 30 회복하였습니다.");
			_getch();
		}
	case 3:	// 고급HP포션 사용
	// 포션이 없는 경우
		if (User->items.HP_potion2 == 0) {
			attack_cycle(User, m_num);
			printf("사용가능한 HP포션이 없습니다.\n");
			return;
		}
		else {	// 체력이 최대체력이 넘어가지 않도록 회복
			User->items.HP_potion2--;
			after_use = User->remain_hp + 300;
			if (after_use >= User->hp)User->remain_hp = User->hp;
			else User->remain_hp += 300;
			attack_cycle(User, m_num);
			printf("체력을 300 회복하였습니다.");
			_getch();
		}
	case 4:	// 고급MP포션 사용
		//mp가 없는 경우
		if (User->items.MP_potion2 == 0) {
			attack_cycle(User, m_num);
			printf("사용가능한 MP포션이 없습니다.\n");
			return;
		}
		else {	// MP가 최대가 넘어가지 않도록 회복
			User->items.MP_potion2--;
			after_use = User->remain_mp + 100;
			if (after_use >= User->mp)User->remain_mp = User->mp;
			else User->remain_mp += 100;
			attack_cycle(User, m_num);
			printf("마나를 100 회복하였습니다.");
			_getch();
		}
	}
}

// 공격 방법 입력 받기
void fight_massage(Status* User, int m_num) {
	int num;
	
	num = _getch();
	num -= '0';
	switch (num) {
	case 1:	// 기본 공격
		basic_attack(User, 0, m_num);
		_getch();
		break;
	case 2:	// 스킬
		if (User->level >= 2) {
			skill_attack(User, m_num);
			_getch();
		}
		else {
			printf("전직 후 스킬 이용이 가능합니다.");
			_getch();
			fight_monster(User, m_num);
		}
		break;
	case 3: // 아이템
		Use_item(User, m_num);
		break;
	case 4: // 도망가기
		int run = avoid_probability(User);
		if (run == 1) {
			printf("도망 성공!");
			m[m_num].remain_hp = m[m_num].hp; // 도망쳤을 시 몬스터의 체력 다시 회복
			init_skill();				// 상태 이상 초기화
			_getch();
			Main_Game(User);
		}
		else if (run == 0) {
			printf("도망 가지 못했습니다!");
			_getch();
		}
		break;
	}
}


// 몬스터를 만났을 때 출력할 화면
int question_to_user(int m_num) {
	int x = 13;
	int y = 13;

	system("cls");
	notice_frame();
	gotoxy(11, 10);
	printf(" 야생의 %s(이)가 나타났다!\n", m[m_num].name);

	gotoxy(x - 2, y); printf("▶ 싸운다");
	gotoxy(x - 2, y + 1); printf("   도망간다");

	while (1) {
		int n = check_key();
		switch (n) {
		case 1:
			if (y > 13) {
				gotoxy(x - 2, y);
				printf(" ");
				gotoxy(x - 2, --y);
				printf("▶");
			}
			break;
		case 2:
			if (y < 14) {
				gotoxy(x - 2, y);
				printf(" ");
				gotoxy(x - 2, ++y);
				printf("▶");
			}
			break;
		case 10:
			return y - 12;
		}
	}
}

// 몬스터 그리기 함수
void print_monster(int m_num) {
	switch (m_num) {
	case 0:
		// a 몬스터
		gotoxy(54, 2); getcolor(YELLOW1, BLACK); printf("  ／) 　／)      "); getcolor(GRAY1, BLACK);
		gotoxy(54, 3); getcolor(YELLOW1, BLACK); printf(" / 　⌒　 ＼     "); getcolor(GRAY1, BLACK);
		gotoxy(54, 4); getcolor(YELLOW1, BLACK); printf("| ●_　●　 |／＼"); getcolor(GRAY1, BLACK);
		gotoxy(54, 5); getcolor(YELLOW1, BLACK); printf("(○ ～　 ○ |　／"); getcolor(GRAY1, BLACK);
		gotoxy(54, 6); getcolor(YELLOW1, BLACK); printf("と/ 　　 　 |く  "); getcolor(GRAY1, BLACK);
		gotoxy(54, 7); getcolor(YELLOW1, BLACK); printf("  ＼　 |_/　|／  "); getcolor(GRAY1, BLACK);
		gotoxy(54, 8); getcolor(YELLOW1, BLACK); printf("  　L/￣L/       "); getcolor(GRAY1, BLACK);
		break;
	case 1: 
		// b 몬스터
		gotoxy(54, 2);  getcolor(RED1, BLACK); printf(". /⌒＞―――＜⌒＼ "); getcolor(GRAY1, BLACK);
		gotoxy(54, 3);  getcolor(RED1, BLACK); printf(" ｜／　　　　　＼｜ "); getcolor(GRAY1, BLACK);
		gotoxy(54, 4);  getcolor(RED1, BLACK); printf("  Y　　　　　　　Y  "); getcolor(GRAY1, BLACK);
		gotoxy(54, 5);  getcolor(RED1, BLACK); printf("  | ●　　　 ●　 | "); getcolor(GRAY1, BLACK);
		gotoxy(54, 6);  getcolor(RED1, BLACK); printf("  ＼″(_人_) ″ ノ  "); getcolor(GRAY1, BLACK);
		gotoxy(54, 7);  getcolor(RED1, BLACK); printf("　 ＞――――イ     "); getcolor(GRAY1, BLACK);
		gotoxy(54, 8);  getcolor(RED1, BLACK); printf("　/　|　　　＼)     "); getcolor(GRAY1, BLACK);
		gotoxy(54, 9);  getcolor(RED1, BLACK); printf(" (＿ノ　/⌒)/⌒)    "); getcolor(GRAY1, BLACK);
		gotoxy(54, 10); getcolor(RED1, BLACK); printf("　 ＼＿(_／(_／     "); getcolor(GRAY1, BLACK);
		break;
		// c 몬스터
	case 2:
		gotoxy(54, 2); getcolor(CYAN2, BLACK); printf("☆　    ∧＿∧        "); getcolor(GRAY1, BLACK);
		gotoxy(54, 3); getcolor(CYAN2, BLACK); printf("　　　 (＊ω＊）☆    "); getcolor(GRAY1, BLACK);
		gotoxy(54, 4); getcolor(CYAN2, BLACK); printf("　　　 ¶と¶と＼     "); getcolor(GRAY1, BLACK);
		gotoxy(54, 5); getcolor(CYAN2, BLACK); printf("(( ((　／￣￣￣＼     "); getcolor(GRAY1, BLACK);
		gotoxy(54, 6); getcolor(CYAN2, BLACK); printf("　　 |) ○ ○ ○ (|   "); getcolor(GRAY1, BLACK);
		gotoxy(54, 7); getcolor(CYAN2, BLACK); printf("　 ／″ 　　ν.　　＼ "); getcolor(GRAY1, BLACK);
		gotoxy(54, 8); getcolor(CYAN2, BLACK); printf("  ／＿＿＿＿＿＿＿＿＼"); getcolor(GRAY1, BLACK);
		gotoxy(54, 9); getcolor(CYAN2, BLACK); printf("  ￣ ＼＿＼＿_／＿／￣"); getcolor(GRAY1, BLACK);
		break;
	case 3:
		// 보스 
		gotoxy(48, 2); getcolor(BLUE1, BLACK); printf("    ;' ':;,,　　　   ,;'':;,   "); getcolor(GRAY1, BLACK);
		gotoxy(48, 3); getcolor(BLUE1, BLACK); printf("　 ;'　　 ':;,.,.,,,;'　　';,  "); getcolor(GRAY1, BLACK);
		gotoxy(48, 4); getcolor(BLUE1, BLACK); printf(" ,:' ＼　 　　　　 ／ 　　 ',  "); getcolor(GRAY1, BLACK);
		gotoxy(48, 5); getcolor(BLUE1, BLACK); printf(":'　 ●　　　　　 ●　 　　  i."); getcolor(GRAY1, BLACK);
		gotoxy(48, 6); getcolor(BLUE1, BLACK); printf("i　 '''　(__人_)　　''' 　 　i "); getcolor(GRAY1, BLACK);
		gotoxy(48, 7); getcolor(BLUE1, BLACK); printf(" :　 　　　　　　　　　  　i   "); getcolor(GRAY1, BLACK);
		gotoxy(48, 8); getcolor(BLUE1, BLACK); printf("　`:, 　　　　　 　  　   /    "); getcolor(GRAY1, BLACK);
		gotoxy(48, 9); getcolor(BLUE1, BLACK); printf("　 ,:'　　　　　　  　 ｀:､    "); getcolor(GRAY1, BLACK);
		gotoxy(48, 9); getcolor(BLUE1, BLACK); printf("　 ,:'　　　　　　　  　 ｀:   "); getcolor(GRAY1, BLACK);
		break;
	}

}

// 몬스터 만났을 때의 함수
void meet_monster(Status* User, int m_num) {
	system("cls");
	int answer = question_to_user(m_num);

	switch (answer) {
	case 1:		// 싸운다
		fight_monster(User, m_num);
	case 2:		// 도망간다
		int run = avoid_probability(User);
		if (run == 1) {			// 도망 성공
			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 야생의 %s(이)가 나타났다!\n", m[m_num].name);
			gotoxy(11, 13); printf("도망 성공!\n");
			_getch();
			Main_Game(User);
		}
		else if (run == 0) {	// 도망 실패
			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 야생의 %s(이)가 나타났다!\n", m[m_num].name);
			gotoxy(11, 13); printf("도망 가지 못했습니다!");
			_getch();
			fight_monster(User, m_num);
		}
		break;
	}
}

// 유저의 체력과 마나 출력
void print_hpmp(Status* User, int m_num) {
	int hp, mp;
	int mon;
	int n = 0;

	// hp,mp 비율 계산
	hp = ((int)(User->remain_hp * 10) / User->hp);
	mp = ((int)(User->remain_mp * 10) / User->mp);

	mon = ((int)(m[m_num].remain_hp * 10) / m[m_num].hp);

	// ======유저일 경우======================================================
	gotoxy(4, 8); printf(" < %s >", User->name);
	gotoxy(4, 9); printf("HP :");
	// 현재 있는 체력 출력
	for (int i = 0; i < hp; i++) {
		gotoxy(10 + n, 9); getcolor(RED2, BLACK); printf("■"); getcolor(GRAY1, BLACK);
		n += 2;
	}
	// 남은 체력 출력
	n = 18;
	for (int i = 0; i < 10 - hp; i++) {
		gotoxy(10 + n, 9); getcolor(GRAY1, BLACK); printf("□"); getcolor(GRAY1, BLACK);
		n -= 2;
	}

	gotoxy(32, 9); printf("%3d / %3d", User->remain_hp, User->hp);
	n = 0;


	gotoxy(4, 10); printf("MP :");
	// 현재 있는 MP 출력
	for (int i = 0; i < mp; i++) {
		gotoxy(10 + n, 10); getcolor(BLUE2, BLACK); printf("■"); getcolor(GRAY1, BLACK);
		n += 2;
	}
	// 남은 MP 출력
	n = 18;
	for (int i = 0; i < 10 - mp; i++) {
		gotoxy(10 + n, 10); getcolor(GRAY1, BLACK); printf("□"); getcolor(GRAY1, BLACK);
		n -= 2;
	}
	gotoxy(32, 10); printf("%3d / %3d", User->remain_mp, User->mp);
	n = 0;

	// ==========================몬스터인 경우==============================================

	gotoxy(4, 2); printf(" < %s >", m[m_num].name);
	gotoxy(4, 3); printf("HP :");
	// 현재 있는 체력 출력
	for (int i = 0; i < mon; i++) {
		gotoxy(10 + n, 3); getcolor(RED2, BLACK); printf("■"); getcolor(GRAY1, BLACK);
		n += 2;
	}
	// 남은 체력 출력
	n = 18;
	for (int i = 0; i < 10 - mon; i++) {
		gotoxy(10 + n, 3); getcolor(GRAY1, BLACK); printf("□"); getcolor(GRAY1, BLACK);
		n -= 2;
	}
	gotoxy(32, 3); printf("%3d / %3d", m[m_num].remain_hp, m[m_num].hp);
}

// 유저 상태창과 몬스터 상태창 출력
void attack_cycle(Status* User, int m_num) {
	system("cls");
	print_hpmp(User, m_num);
	attack_way(User);
	print_monster(m_num);
	gotoxy(53, 22); printf("[ 아무 키를 눌러 확인 ]");
	gotoxy(0, 17); printf("============================== [ 전투 메세지 ] ================================\n\n");
	
}

void fight_monster(Status* User, int m_num) {
	while (1) {
		switch (User->type) {
		case 0:		// 전직 이전
			attack_cycle(User, m_num);
			fight_massage(User, m_num);

			attack_cycle(User, m_num);
			monster_attack(User, m_num);
			_getch();
			break;
		case 1:		// 전사일 경우
			// 방어력 상승이 안되어있을경우
			if (s_def == 0)User->defense = User->tmp_def;
			attack_cycle(User, m_num);
			fight_massage(User, m_num);

			attack_cycle(User, m_num);
			// 기절 스킬이 적용되고 있는경우
			if (stun >= 1) {
				printf("%s는 기절 상태입니다.", m[m_num].name);
				stun--;
				_getch();
			}
			else {
				monster_attack(User, m_num);
				_getch();
			}
			s_def--;
			break;
		case 2:		// 궁수일 경우

			attack_cycle(User, m_num);
			fight_massage(User, m_num);

			attack_cycle(User, m_num);
			// 기절 스킬이 적용되고 있는경우
			if (stun >= 1) {
				printf("%s는 기절 상태입니다.", m[m_num].name);
				stun--;
				_getch();
			}
			else {
				monster_attack(User, m_num);
				_getch();
			}
			cri--;
			break;
		case 3:		// 암살자일 경우
			attack_cycle(User, m_num);
			fight_massage(User, m_num);

			attack_cycle(User, m_num);

			// 일반적인 상황인 경우
			if (hide == 0 && poison == 0) {
				monster_attack(User, m_num);
				_getch();
			}
			else if (hide > 0) { // 은신중일 경우
				printf("은신으로 공격을 한번 회피하였습니다!\n");
				printf("전체체력의 10%% 회복합니다!\n");
				_getch();
				// 체력 회복했는데 최대 최력이 넘어갈 경우
				if (User->remain_hp + (int)(User->hp * 0.1) >= User->hp)
					User->remain_hp = User->hp;
				else
					User->remain_hp += (int)(User->hp * 0.1);

				hide--;
			}// 출혈 스킬이 적용중인 경우
			else if (poison > 0) {
				printf("%s(은)는 %d의 출혈 데미지를 입습니다.\n", m[m_num].name, (int)(m[m_num].hp * 0.1));
				m[m_num].remain_hp -= (int)(m[m_num].hp * 0.1);
				check_hp(User, m[m_num].remain_hp, 1, m_num);
				_getch();
				poison--;
				monster_attack(User, m_num);
				_getch();
				break;
			}
		}
	}
}


// 도망간다에서 확률 계산
int avoid_probability(Status* User) {
	int prob = (rand() % 100) + 1;			// 1~100까지
	
	if (prob > User->run_avoid)return 0;
	else if (1 <= prob <= User->run_avoid) {	// 1일 경우 도망성공
		return 1;
	}
	return 0;
}


// 경험치 표시
void print_exp(Status* User) {
	int exp;
	int n = 0;
	
	// 경험치 비율 계산
	exp = ((int)(User->EXP * 10) / User->max_EXP);

	// 현재 있는 경험치 출력
	for (int i = 0; i < exp; i++) {
		gotoxy(38 + n, 19); getcolor(YELLOW2, BLACK); printf("■"); getcolor(GRAY1, BLACK);
		n += 2;
	}
	// 남은 경험치 출력
	n = 18;
	for (int i = 0; i < 10 - exp; i++) {
		gotoxy(38 + n, 19); getcolor(GRAY1, BLACK); printf("□"); getcolor(GRAY1, BLACK);
		n -= 2;
	}
}

// 맵 아래의 상태창 출력
void print_status(Status* User) {
	gotoxy(2, 18); printf("┏━상태창━━━━━━━━━━━━━━━━━━━━━━━━━┓");
	gotoxy(2, 19); printf("┃  이름 : %s                                                 ", User->name); gotoxy(30, 19); printf("경험치"); gotoxy(62, 19); printf("┃");
	gotoxy(2, 20); printf("┃  체력 : %3d/%d   직업 : %6s    보유 골드 : %3d G        ", User->remain_hp, User->hp, User->job, User->Gold); gotoxy(62, 20); printf("┃");
	gotoxy(2, 21); printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
	gotoxy(2, 22); printf(" 이동 : 방향키 | 저장 : 'S' | 인벤토리 : 'I' | 세부상태 : 'D'");

	// 경험치 그래픽 출력
	print_exp(User);
}


// 파일 입출력을 이용한 상태 저장
void save_user(Status* User) {
	FILE* fp = fopen("user.txt", "w");
	fseek(fp, 0, SEEK_END); // 끝으로 이동
	fwrite(User, sizeof(Status), 1, fp);

	system("cls");
	notice_frame();

	gotoxy(11, 10);
	printf("저장 완료!");
	gotoxy(11, 13);
	printf("성공적으로 저장 되었습니다!");
	_getch();
	Main_Game(User);
}

// 예 아니오 골라주는 함수
int choice_ox() {
	int x = 13;
	int y = 15;
	while (1) {
		int n = check_key();
		switch (n) {
		case 1:
			if (y > 15) {
				gotoxy(x - 2, y);
				printf(" ");
				gotoxy(x - 2, --y);
				printf("▶");
			}
			break;
		case 2:
			if (y < 16) {
				gotoxy(x - 2, y);
				printf(" ");
				gotoxy(x - 2, ++y);
				printf("▶");
			}
			break;
		case 10:
			return y - 14;
		}
	}
}

// 휴식하기 함수
void rest(Status* User) {
	int ans;
	system("cls");

	notice_frame();

	gotoxy(11, 10);
	printf(" 상태 회복");
	gotoxy(11, 12);
	printf(" %s님의", User->name);
	gotoxy(11, 13);
	printf(" HP 및 MP 를 모두 회복시켜드립니다. [-20G]");
	gotoxy(11, 15); printf("▶ 네");
	gotoxy(11, 16); printf("   아니오");

	// 예 아니오 선택
	ans = choice_ox();

	// 예를 선택한 경우
	if (ans == 1 && User->Gold >= 20) {
		User->remain_hp = User->hp;
		User->remain_mp = User->mp;
		User->Gold -= 20;

		system("cls");

		notice_frame();
		gotoxy(11, 10);
		printf(" < 알 림 >");
		gotoxy(11, 13);
		printf("회복이 완료 되었습니다!\n");
		_getch();
	}
	// 예를 선택했지만 gold가 부족한 경우
	else if (ans == 1 && User->Gold < 20) {
		system("cls");

		notice_frame();
		gotoxy(11, 10);
		printf(" < 알 림 >");
		gotoxy(11, 13);
		printf("Gold가 부족합니다.\n");
		_getch();
	}
	system("cls");
}

// 포션, 무기, 방어구 중 선택
int choose_shop() {
	int x = 13 ,y = 13;
	system("cls");
	notice_frame();
	gotoxy(11, 10);
	printf(" < 상 점 >");

	gotoxy(x, y); printf("▶ 포션 상점");
	gotoxy(x, y + 1); printf("   무기 상점");
	gotoxy(x, y + 2); printf("   상점 나가기");
	gotoxy(x + 28, y + 3); printf("   [ ENTER키를 눌러 선택 ]");
	
	while (1) {
		int n = check_key();
		switch (n) {
		case 1:
			if (y > 13) {
				gotoxy(x, y);
				printf(" ");
				gotoxy(x, --y);
				printf("▶");
			}
			break;
		case 2:
			if (y < 15) {
				gotoxy(x, y);
				printf(" ");
				gotoxy(x, ++y);
				printf("▶");
			}
			break;
		case 10:		// 엔터 눌렀을 경우
			return y - 12;
		}
	}
}

// 포션 상점
int potion_shop() {
	int x = 13, y = 12;

	system("cls");
	notice_frame();
	gotoxy(11, 10);
	printf(" < 포션 상점 >");


	gotoxy(x, y); printf("▶ 고급 HP 포션(HP 300회복) - 50 Gold");
	gotoxy(x, y + 1); printf("   고급 MP 포션(MP 100회복) - 100 Gold");
	gotoxy(x, y + 2); printf("   HP 포션(HP 30회복) - 10 Gold");
	gotoxy(x, y + 3); printf("   MP 포션(MP 30회복) - 10 Gold");
	gotoxy(x, y + 4); printf("   뒤로 가기");
	gotoxy(x+28, y + 4); printf("   [ ENTER키를 눌러 구매 ]");

	while (1) {
		int n = check_key();
		switch (n) {
		case 1:
			if (y > 12) {
				gotoxy(x, y);
				printf(" ");
				gotoxy(x, --y);
				printf("▶");
			}
			break;
		case 2:
			if (y < 16) {
				gotoxy(x, y);
				printf(" ");
				gotoxy(x, ++y);
				printf("▶");
			}
			break;
		case 10:		// 엔터 눌렀을 경우
			return y - 11;
		}
	}
}

// 무기 상점
int weapon_shop(Status* User) {
	int x = 13, y = 12;
	system("cls");
	notice_frame();
	gotoxy(11, 10);
	printf(" < 무기 상점 >");
	
	if (strcmp(User->job, "전사")==0) {
		gotoxy(x, y); printf("▶ 롱소드 - 70 Gold, Lv.2 전용");
		gotoxy(x, y + 1); printf("   광휘의 검 - 100 Gold, Lv.3 전용");
		gotoxy(x, y + 2); printf("   B.F 소드 - 150 Gold, Lv.4 전용");
		gotoxy(x, y + 3); printf("   뒤로 가기");
		gotoxy(x + 28, y + 4); printf("   [ ENTER키를 눌러 구매 ]");
	}
	else if (strcmp(User->job, "궁수")==0) {
		gotoxy(x, y); printf("▶ 곡궁 - 70 Gold, Lv.2 전용");
		gotoxy(x, y + 1); printf("   돌풍 - 100 Gold, Lv.3 전용");
		gotoxy(x, y + 2); printf("   루난의 허리케인 - 150 Gold, Lv.4 전용");
		gotoxy(x, y + 3); printf("   뒤로 가기");
		gotoxy(x + 28, y + 4); printf("   [ ENTER키를 눌러 구매 ]");
	}
	else if (strcmp(User->job, "암살자")==0) {
		gotoxy(x, y); printf("▶ 도란의 검 - 70 Gold, Lv.2 전용");
		gotoxy(x, y + 1); printf("   톱날 단검 - 100 Gold, Lv.3 전용");
		gotoxy(x, y + 2); printf("   그림자 검 - 150 Gold, Lv.4 전용");
		gotoxy(x, y + 3); printf("   뒤로 가기");
		gotoxy(x + 28, y + 4); printf("   [ ENTER키를 눌러 구매 ]");
	}
	else {	// 모험가일 경우
		system("cls");
		notice_frame();
		gotoxy(11, 10);
		printf(" 이용 불가");
		gotoxy(11, 13);
		printf(" 모험가는 무기 상점 이용이 불가능합니다.");
		_getch();
		shopping(User);
	}
	while (1) {
		int n = check_key();
		switch (n) {
		case 1:
			if (y > 12) {
				gotoxy(x, y);
				printf(" ");
				gotoxy(x, --y);
				printf("▶");
			}
			break;
		case 2:
			if (y < 15) {
				gotoxy(x, y);
				printf(" ");
				gotoxy(x, ++y);
				printf("▶");
			}
			break;
		case 10:		// 엔터 눌렀을 경우
			return y - 11;
		}
	}
}

// 상점 화면
void shopping(Status* User) {
	int num, check;
	
	// 포션, 무기 상점 선택
	num = choose_shop();
	switch (num) {
		// 포션 상점
	case 1:
		// HP, MP 중 선택
		check = potion_shop();
		// HP포션일 경우(골드 10이상)
		if (check == 3 && User->Gold >= 10) {
			User->items.HP_potion++;
			User->Gold -= 10;

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}// MP포션일 경우(골드 10이상)
		else if (check == 4 && User->Gold >= 10) {
			User->items.MP_potion++;
			User->Gold -= 10;

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}	// 골드가 부족한 경우
		else if (check == 3 || check == 4 && User->Gold < 10) {
			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 실패");
			gotoxy(11, 13);
			printf("골드가 부족하여 구매에 실패하였습니다...");
			_getch();
			Main_Game(User);
		}
		// 고급 HP포션일 경우(골드 50이상)
		else if (check == 1 && User->Gold >= 50) {
			User->items.HP_potion2++;
			User->Gold -= 50;

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}
		// 고급 MP포션일 경우(골드 100이상)
		else if (check == 2 && User->Gold >= 100) {
			User->items.MP_potion++;
			User->Gold -= 100;

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}	// 골드가 부족한 경우
		else if ((check == 1 && User->Gold < 50) || (check == 2 && User->Gold < 100)) {
			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 실패");
			gotoxy(11, 13);
			printf("골드가 부족하여 구매에 실패하였습니다...");
			_getch();
			Main_Game(User);
		}
		else if (check == 5)shopping(User);
		break;

		// 무기 상점
	case 2:
		check = weapon_shop(User);
		// 전사 ===========================================================
		// 첫번째 무기
		if (check == 1 && User->type == 1 && User->Gold >= 70 && User->level >= 2) {
			strcpy(User->weapon, "롱소드");
			User->Gold -= 70;
			calc_attack_status(User, 20); // 무기 착용에 따른 공격력 조절

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}// 두번째 무기
		else if (check == 2 && User->type == 1 && User->Gold >= 100 && User->level >= 3) {
			strcpy(User->weapon, "광휘의 검");
			User->Gold -= 100;
			calc_attack_status(User, 10);

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}// 세번째 무기
		else if (check == 3 && User->type == 1 && User->Gold >= 150 && User->level >= 4) {
			strcpy(User->weapon, "B.F 소드");
			User->Gold -= 150;
			calc_attack_status(User, 20);

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}
		// 궁수 ===========================================================
		// 첫번째 무기
		else if (check == 1 && User->type == 2 && User->Gold >= 70 && User->level >= 2) {
			strcpy(User->weapon, "곡궁");
			User->Gold -= 70;
			calc_attack_status(User, 20);

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}// 두번째 무기
		else if (check == 2 && User->type == 2 && User->Gold >= 100 && User->level >= 3) {
			strcpy(User->weapon, "돌풍");
			User->Gold -= 100;
			calc_attack_status(User, 10);

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}// 세번째 무기
		else if (check == 3 && User->type == 2 && User->Gold >= 150 && User->level >= 4) {
			strcpy(User->weapon, "루난의 허리케인");
			User->Gold -= 150;
			calc_attack_status(User, 20);

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}
		// 암살자 ===========================================================
		// 첫번째 무기
		else if (check == 1 && User->type == 3 && User->Gold >= 70 && User->level >= 2) {
			strcpy(User->weapon, "도란의 검");
			User->Gold -= 70;
			calc_attack_status(User, 20);

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}// 두번째 무기
		else if (check == 2 && User->type == 3 && User->Gold >= 100 && User->level >= 3) {
			strcpy(User->weapon, "톱날단검");
			User->Gold -= 100;
			calc_attack_status(User, 10);

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}// 세번째 무기
		else if (check == 3 && User->type == 3 && User->Gold >= 150 && User->level >= 4) {
			strcpy(User->weapon, "그림자 검");
			User->Gold -= 150;
			calc_attack_status(User, 20);

			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 완료!");
			gotoxy(11, 13);
			printf("성공적으로 구매를 완료하였습니다!");

			_getch();
			Main_Game(User);
		}
		else if (check == 4)shopping(User);
		else {
			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" 구매 실패");
			gotoxy(11, 13);
			printf("골드가 부족하거나 레벨이 낮아 구매에 실패하였습니다...");
			_getch();
			Main_Game(User);
		}
		break;
	}
	Main_Game(User);
}

// 세부상태창 출력
void detail_status(Status* User) {
	system("cls");

	notice_frame();

	gotoxy(11, 10);
	printf(" %s님의 세부 상태창 [%s]",User->name,User->job);
	gotoxy(11, 12);
	printf("HP [ %4d / %3d ]   MP [ %3d / %3d ]\n", User->remain_hp, User->hp,User->remain_mp, User->mp);
	gotoxy(11, 13);
	printf("레벨 : %d [ %d / %d ]\n",User->level, User->EXP, User->max_EXP);
	gotoxy(11, 14);
	printf("착용중인 무기 : %s\n", User->weapon);
	gotoxy(11, 15);
	printf("공격력 : %d\n", User->attack.dmg);
	gotoxy(11, 16);
	printf("보유 골드 : %d Gold\n", User->Gold);
	_getch();
	Main_Game(User);
}

// 인벤토리 출력
void print_inventory(Status* User) {
	system("cls");
	notice_frame();

	gotoxy(11, 10);
	printf(" 인벤토리");

	gotoxy(11, 12);
	printf(" [ 보유중 ]");
	gotoxy(11, 13);
	printf(" HP포션 : %3d 개\n", User->items.HP_potion);
	gotoxy(11, 14);
	printf(" MP포션 : %3d 개\n", User->items.MP_potion);
	gotoxy(11, 15);
	printf(" 고급 HP포션 : %3d 개\n", User->items.HP_potion2);
	gotoxy(11, 16);
	printf(" 고급 MP포션 : %3d 개\n", User->items.MP_potion2);

	_getch();
	Main_Game(User);
}

// 맵 이동 함수
void nextmap(Status* User) {
	// 마을에서 던전으로 갈 경우( 체력이 있어야 입장 )
	if (map[current_map][cy][cx / 2] == 6 && User->remain_hp != 0) {
		system("cls");
		current_map = 1;
		print_map();
		print_status(User);
		cx = 4;
		cy = 15;
		gotoxy(cx, cy);
		printf("%s", character);
	}	// 체력이 없을 경우
	else if(map[current_map][cy][cx / 2] == 6 && User->remain_hp == 0) {
		system("cls");
		notice_frame();
		gotoxy(11, 10);
		printf(" 입장 불가");
		gotoxy(11, 13);
		printf("체력이 부족하여 던전에 입장하실 수 없습니다!");
		_getch();

		system("cls");
		print_map();
		print_status(User);
		cx = 72;
		cy = 6;
		gotoxy(cx, cy);
		printf("%s", character);
		
	}

	// 상점에 갈 경우
	if (map[current_map][cy][cx / 2] == 24) {
		system("cls");
		shopping(User);
		print_status(User);
		cx = 16;
		cy = 15;
		gotoxy(cx, cy);
		printf("%s", character);
	}
	// 휴식할 경우
	if (map[current_map][cy][cx / 2] == 25) {
		system("cls");
		rest(User);
		print_map();
		print_status(User);
		cx = 38;
		cy = 6;
		gotoxy(cx, cy);
		printf("%s", character);
	}
	// 던전에서 마을로 갈 경우
	if (map[current_map][cy][cx / 2] == 26) {
		system("cls");
		current_map = 0;
		print_map();
		print_status(User);
		cx = 72;
		cy = 6;
		gotoxy(cx, cy);
		printf("%s", character);
	}
	// 던전에서 보스맵으로 갈 경우
	if (map[current_map][cy][cx / 2] == 28) {
		if (User->level == 4) {
			system("cls");
			current_map = 2;
			print_map();
			print_status(User);
			cx = 38;
			cy = 15;
			gotoxy(cx, cy);
			printf("%s", character);
		}
		// 입장이 불가능한 경우
		else {
			system("cls");
			notice_frame();

			gotoxy(11, 10); printf(" 입장 불가");
			gotoxy(11, 13); printf(" 보스 던전은 레벨이 4인 경우에만 입장 가능합니다.");
			_getch();
			system("cls");
			print_map();
			print_status(User);
			cx = 70;
			cy = 6;
			gotoxy(cx, cy);
			printf("%s", character);
		}
	}

	// 보스맵에서 던전으로 갈 경우
	if (map[current_map][cy][cx / 2] == 29) {
		system("cls");
		current_map = 1;
		print_map();
		print_status(User);
		cx = 70;
		cy = 6;
		gotoxy(cx, cy);
		printf("%s", character);
	}
	// 튜토리얼 몹과 마주쳤을 경우
	if (map[current_map][cy][cx / 2] == 30) {
		system("cls");
		meet_monster(User, 0);
	}
	if (map[current_map][cy][cx / 2] == 31) {
		system("cls");
		meet_monster(User, 1);
	}
	if (map[current_map][cy][cx / 2] == 32) {
		system("cls");
		meet_monster(User, 2);
	}
	if (map[current_map][cy][cx / 2] == 33) {
		system("cls");
		meet_monster(User, 3);
	}
}

// 이동 가능한지 확인 후 이동
void movechar(char* character, int* cx, int* cy, int direction) {

	switch (direction) {
	case LEFT:
		// 가려는 곳
		if (map[current_map][*cy][*cx / 2 - 1] == 2 || map[current_map][*cy][*cx / 2 - 1] == 6 || map[current_map][*cy][*cx / 2 - 1] == 23 || map[current_map][*cy][*cx / 2 - 1] == 24 || map[current_map][*cy][*cx / 2 - 1] == 25 || map[current_map][*cy][*cx / 2 - 1] == 26 || map[current_map][*cy][*cx / 2 - 1] == 27 || map[current_map][*cy][*cx / 2 - 1] == 28 || map[current_map][*cy][*cx / 2 - 1] == 29 ||  map[current_map][*cy][*cx / 2 - 1] == 30 || map[current_map][*cy][*cx / 2 - 1] == 31 || map[current_map][*cy][*cx / 2 - 1] == 32 || map[current_map][*cy][*cx / 2 - 1] == 33 ){
			gotoxy(*cx, *cy);
			// 마을 맵 바닥
			if (map[current_map][*cy][*cx / 2] == 2) {
				getcolor(BLACK, YELLOW1); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 포탈
			if (map[current_map][*cy][*cx / 2] == 6 || map[current_map][*cy][*cx / 2] == 24 || map[current_map][*cy][*cx / 2] == 25 || map[current_map][*cy][*cx / 2] == 26 || map[current_map][*cy][*cx / 2] == 28 || map[current_map][*cy][*cx / 2] == 29) {
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK);
			}
			// 던전맵 바닥
			if (map[current_map][*cy][*cx / 2] == 23) {
				getcolor(GRAY2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 보스맵
			if (map[current_map][*cy][*cx / 2] == 27) {
				getcolor(GREEN2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 몹 처치 후 그 위치 23(바닥) 으로 바꿔줌
			if (map[current_map][*cy][*cx / 2] == 30 || map[current_map][*cy][*cx / 2] == 31 || map[current_map][*cy][*cx / 2] == 32) {
				getcolor(GRAY2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
				map[current_map][*cy][*cx / 2] = 23;
			}
			// 보스와 전투 중 도망쳤을 경우 다시 출력
			if (map[current_map][*cy][*cx / 2] == 33) {
				getcolor(RED1, BLACK); printf("㉿"); getcolor(GRAY1, BLACK);
			}
			gotoxy(*cx - 2, *cy);
			*cx -= 2;
			printf("%s", character);
		}
		break;
	case RIGHT:
		// 가려는 곳
		if (map[current_map][*cy][*cx / 2 + 1] == 2 || map[current_map][*cy][*cx / 2 + 1] == 6 || map[current_map][*cy][*cx / 2 + 1] == 23 || map[current_map][*cy][*cx / 2 + 1] == 24 || map[current_map][*cy][*cx / 2 + 1] == 25 || map[current_map][*cy][*cx / 2 + 1] == 26 || map[current_map][*cy][*cx / 2 + 1] == 27 || map[current_map][*cy][*cx / 2 + 1] == 28 || map[current_map][*cy][*cx / 2 + 1] == 29 || map[current_map][*cy][*cx / 2 + 1] == 30 || map[current_map][*cy][*cx / 2 + 1] == 31 || map[current_map][*cy][*cx / 2 + 1] == 32 || map[current_map][*cy][*cx / 2 + 1] == 33) { 
			gotoxy(*cx, *cy);
			// 마을 맵 바닥
			if (map[current_map][*cy][*cx / 2] == 2) {
				getcolor(BLACK, YELLOW1); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 포탈
			if (map[current_map][*cy][*cx / 2] == 6 || map[current_map][*cy][*cx / 2] == 24 || map[current_map][*cy][*cx / 2] == 25 || map[current_map][*cy][*cx / 2] == 26 || map[current_map][*cy][*cx / 2] == 28 || map[current_map][*cy][*cx / 2] == 29) {
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK);
			}
			// 던전맵 바닥
			if (map[current_map][*cy][*cx / 2] == 23) {
				getcolor(GRAY2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 보스맵
			if (map[current_map][*cy][*cx / 2] == 27) {
				getcolor(GREEN2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 몹 처치 후 그 위치 23(바닥) 으로 바꿔줌
			if (map[current_map][*cy][*cx / 2] == 30 || map[current_map][*cy][*cx / 2] == 31 || map[current_map][*cy][*cx / 2] == 32) {
				getcolor(GRAY2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
				map[current_map][*cy][*cx / 2] = 23;
			}
			// 보스와 전투 중 도망쳤을 경우 다시 출력
			if (map[current_map][*cy][*cx / 2] == 33) {
				getcolor(RED1, BLACK); printf("㉿"); getcolor(GRAY1, BLACK);
			}
			gotoxy(*cx + 2, *cy);
			*cx += 2;
			printf("%s", character);
		}
		break;
	case UP:
		// 가려는 곳
		if (map[current_map][*cy - 1][*cx / 2] == 2 || map[current_map][*cy - 1][*cx / 2] == 6 || map[current_map][*cy - 1][*cx / 2] == 23 || map[current_map][*cy - 1][*cx / 2] == 24 || map[current_map][*cy - 1][*cx / 2] == 25 || map[current_map][*cy - 1][*cx / 2] == 26 || map[current_map][*cy - 1][*cx / 2] == 27 || map[current_map][*cy - 1][*cx / 2] == 28 || map[current_map][*cy - 1][*cx / 2] == 29 || map[current_map][*cy - 1][*cx / 2] == 30 || map[current_map][*cy - 1][*cx / 2] == 31 || map[current_map][*cy - 1][*cx / 2] == 32 || map[current_map][*cy - 1][*cx / 2] == 33) {
			gotoxy(*cx, *cy);
			// 마을 맵 바닥
			if (map[current_map][*cy][*cx / 2] == 2) {
				getcolor(BLACK, YELLOW1); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 포탈
			if (map[current_map][*cy][*cx / 2] == 6 || map[current_map][*cy][*cx / 2] == 24 || map[current_map][*cy][*cx / 2] == 25 || map[current_map][*cy][*cx / 2] == 26 || map[current_map][*cy][*cx / 2] == 28 || map[current_map][*cy][*cx / 2] == 29) {
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK);
			}
			// 던전맵 바닥
			if (map[current_map][*cy][*cx / 2] == 23) {
				getcolor(GRAY2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 보스맵
			if (map[current_map][*cy][*cx / 2] == 27) {
				getcolor(GREEN2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 몹 처치 후 그 위치 23(바닥) 으로 바꿔줌
			if (map[current_map][*cy][*cx / 2] == 30 || map[current_map][*cy][*cx / 2] == 31 || map[current_map][*cy][*cx / 2] == 32) {
				getcolor(GRAY2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
				map[current_map][*cy][*cx / 2] = 23;
			}
			// 보스와 전투 중 도망쳤을 경우 다시 출력
			if (map[current_map][*cy][*cx / 2] == 33) {
				getcolor(RED1, BLACK); printf("㉿"); getcolor(GRAY1, BLACK);
			}
			gotoxy(*cx, *cy - 1);
			*cy -= 1;
			printf("%s", character);
		}
		break;
	case DOWN:
		// 가려는 곳
		if (map[current_map][*cy + 1][*cx / 2] == 2 || map[current_map][*cy + 1][*cx / 2] == 6 || map[current_map][*cy + 1][*cx / 2] == 23 || map[current_map][*cy + 1][*cx / 2] == 24 || map[current_map][*cy + 1][*cx / 2] == 25 || map[current_map][*cy + 1][*cx / 2] == 26 || map[current_map][*cy + 1][*cx / 2] == 27 || map[current_map][*cy + 1][*cx / 2] == 28 || map[current_map][*cy + 1][*cx / 2] == 29 || map[current_map][*cy + 1][*cx / 2] == 30 || map[current_map][*cy + 1][*cx / 2] == 31 || map[current_map][*cy + 1][*cx / 2] == 32 || map[current_map][*cy + 1][*cx / 2] == 33) {

			gotoxy(*cx, *cy);
			// 마을 맵 바닥
			if (map[current_map][*cy][*cx / 2] == 2) {
				getcolor(BLACK, YELLOW1); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 포탈
			if (map[current_map][*cy][*cx / 2] == 6 || map[current_map][*cy][*cx / 2] == 24 || map[current_map][*cy][*cx / 2] == 25 || map[current_map][*cy][*cx / 2] == 26 || map[current_map][*cy][*cx / 2] == 28 || map[current_map][*cy][*cx / 2] == 29) {
				getcolor(CYAN2, BLACK); printf("▣"); getcolor(GRAY1, BLACK);
			}
			// 던전맵
			if (map[current_map][*cy][*cx / 2] == 23) {
				getcolor(GRAY2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 보스맵
			if (map[current_map][*cy][*cx / 2] == 27) {
				getcolor(GREEN2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
			}
			// 몹 처치 후 그 위치 23(바닥) 으로 바꿔줌
			if (map[current_map][*cy][*cx / 2] == 30 || map[current_map][*cy][*cx / 2] == 31 || map[current_map][*cy][*cx / 2] == 32) {
				getcolor(GRAY2, BLACK); printf("▒"); getcolor(GRAY1, BLACK);
				map[current_map][*cy][*cx / 2] = 23;
			}
			// 보스와 전투 중 도망쳤을 경우 다시 출력
			if (map[current_map][*cy][*cx / 2] == 33) {
				getcolor(RED1, BLACK); printf("㉿"); getcolor(GRAY1, BLACK);
			}
			gotoxy(*cx, *cy + 1);
			*cy += 1;
			printf("%s", character);
		}
		break;
	}
}

// 다른 메뉴들
void check_menu(Status* User) {	
	switch (input) {
	case 'i': // 인벤토리
		print_inventory(User);
		break;
	case 's': // 저장
		save_user(User);
		break;
	case 'd': // 세부 상태
		detail_status(User);
		break;
	case ESC: // 게임 종료
		system("cls");
		GameStart();
	}
}

// 캐릭터 움직이는 함수
void move(Status* User) {
	// 캐릭터 현재 위치 출력
	gotoxy(cx, cy);
	printf("%s", character);
	while (1) {
		if (_kbhit()) {
			input = _getch();
			if (input == SPECIAL1 || input == SPECIAL2) {
				input = _getch();
				movechar(character, &cx, &cy, input);	// 캐릭터 이동
				nextmap(User);		// 포탈에 있는지 계속해서 확인
			}
			else {
				check_menu(User);   // 인벤토리, 저장, 세부 상태창 키 입력 하는지 확인
			}
		}
	}
}

// 메인 게임 함수
void Main_Game(Status* User) {
	while (1) {
		system("cls");
		print_map();		// 맵 출력
		print_status(User);	// 맵 밑에 있는 유저 상태창
		move(User);			// 캐릭터 이동
	}
}


// 파일에 몇개의 데이타가 있는지 계산
// data 수 = file 크기 / 구조체크기
// file 크기 구하기 file 끝에서 ftell(fp) / sizeof(Phone)
int getnumberofdata(FILE* fp)
{
	int filesize;
	int n;
	fseek(fp, 0, SEEK_END); // 끝으로 이동
	filesize = ftell(fp); // 파일 크기가 return 됨
	n = filesize / sizeof(Status);
	return n;
}

// 이름 중복 확인
void check_name(Status* User) {
	Status data;
	FILE* fp = fopen("user.txt", "r");
	int ndata = getnumberofdata(fp);
	fseek(fp, 0, SEEK_SET);

	// 이름이 같은지 조사 후 있으면 중복
	for (int n = 0; n < ndata; n++) {
		fread(&data, sizeof(data), 1, fp);
		if (strcmp(data.name, buf) == 0) {
			system("cls");
			notice_frame();

			gotoxy(11, 10);
			printf("이름 중복");
			gotoxy(11, 12);
			printf("캐릭터 이름이 중복됩니다.");
			_getch();

			fseek(fp, 0, SEEK_SET);
			fclose(fp);
			make_character(User);
		}
	}
	fclose(fp);
}

// 이름 재확인 함수
int recheck(Status* User) {
	int x = 11, y = 14;

	system("cls");
	notice_frame();
	gotoxy(11, 10);
	printf("재확인");
	gotoxy(11, 12);
	printf("정말 %s(으)로 하시겠습니까?", User->name);	// 이름 재확인

	gotoxy(x, y); printf("▶ 네");
	gotoxy(x, y + 1); printf("   아니오");

	while (1) {
		int n = check_key();
		switch (n) {
		case 1:
			if (y > 14) {
				gotoxy(x, y);
				printf(" ");
				gotoxy(x, --y);
				printf("▶");
			}
			break;
		case 2:
			if (y < 15) {
				gotoxy(x, y);
				printf(" ");
				gotoxy(x, ++y);
				printf("▶");
			}
			break;
		case 10:		// 엔터 눌렀을 경우
			return y - 13;
		}
	}

}

// 이름 만드는 함수
void make_character(Status* User) {
	unsigned char ch;
	while (1) {

		notice_frame();

		gotoxy(11, 10);
		printf("캐릭터 생성");
		gotoxy(11, 12);
		printf("캐릭터의 이름을 입력해주세요(최대 6자)");
		gotoxy(11, 14); printf(">> ");
		scanf("%s", buf);				// 사용자 이름 입력 받고

		// 문자 총길이가 배열의 크기를 넘어갈 경우
		if (strlen(buf) > 12) {
			system("cls");
			notice_frame();

			gotoxy(11, 10);
			printf(" < 알 림 >");
			gotoxy(11, 12);
			printf("캐릭터의 이름이 6자를 넘었습니다.");
			memset(buf, 0, sizeof(buf));
			_getch();
			make_character(User);
		}

		check_name(User);				// 이름 중복 확인

		if (buf[0] & 0x80) {			// 이름이 한글일 경우 영어키로 전환
			keybd_event(VK_HANGEUL, 0, 0, 0);
			keybd_event(VK_HANGEUL, 0, KEYEVENTF_KEYUP, 0);
		}

		strcpy(User->name, buf);		// 사용자 구조체로 저장
		return;
	}
}

// 캐릭터 생성 성공 알림
void success_char() {
	system("cls");
	notice_frame();
	gotoxy(11, 10); printf("캐릭터 생성 성공!");
	gotoxy(11, 13); printf("캐릭터가 성공적으로 생성되었습니다!");
}

// 첫 캐릭터 생성 함수
void new_Start() {					// 1번 새로 시작했을 경우 캐릭터 생성

	Status* User = (Status*)malloc(sizeof(Status));	// 유저 생성(동적 생성)
	
	system("cls");					// 화면 리셋
	int ok;							// 닉네임 확인하기 위한 변수
	do {							
		system("cls");
		make_character(User);		// 캐릭터 생성
		ok = recheck(User);			// 이름을 원하지 않을 경우 0 리턴 : 다시 입력
	} while (ok != 1);

	success_char();
	_getch();
	init(User, 0);
	Main_Game(User);
}


// 불러오기
void load_Start() {
	char name[20];	// 이름을 받을 변수
	Status User; 
	int n;
	FILE* fp = fopen("user.txt", "r");

	// 데이터 개수
	int ndata = getnumberofdata(fp);

	// 데이터가 없는 경우
	if (ndata == 0) { 
		notice_frame();
		gotoxy(11, 10);
		printf(" < 알 림 >");
		gotoxy(11, 13);
		printf("파일이 없습니다.");
		_getch();
		system("cls");
		return;
	}

	notice_frame();
	gotoxy(11, 10);
	printf(" < 알 림 >");
	gotoxy(11, 12);
	printf("캐릭터 이름을 입력하세요.");
	gotoxy(11, 14);
	printf(">>");
	scanf("%s", name);

	// 문자 총길이가 배열의 크기를 넘어갈 경우
	if (strlen(name) > 12) {
		system("cls");
		notice_frame();

		gotoxy(11, 10);
		printf(" < 알 림 >");
		gotoxy(11, 12);
		printf("캐릭터의 이름이 6자를 넘었습니다.");
		_getch();
		load_Start();
	}

	// 데이터 안에 입력한 이름이 같은지 확인
	fseek(fp, 0, SEEK_SET);
	for (n = 0; n < ndata; n++) {
		fread(&User, sizeof(Status), 1, fp);
		if (strcmp(User.name, name) == 0) {
			system("cls");
			notice_frame();
			gotoxy(11, 10);
			printf(" < 알 림 >");
			gotoxy(11, 13);
			printf("%s님 환영합니다!", User.name);
			_getch();
			break;
		}
	}
	// 찾았는데 데이터가 없을 경우
	if (n >= ndata) {
		system("cls");
		notice_frame();
		gotoxy(11, 10);
		printf(" < 알 림 >");
		gotoxy(11, 13);
		printf("찾으시는 유저가 없습니다.");
		fclose(fp);
		_getch();
		system("cls");
		
		return;
	}

	if (User.name[0] & 0x80) {			// 이름이 한글일 경우 영어키로 전환
		keybd_event(VK_HANGEUL, 0, 0, 0);
		keybd_event(VK_HANGEUL, 0, KEYEVENTF_KEYUP, 0);
	}

	Main_Game(&User);
	fclose(fp);
}

void titleShape() {
	gotoxy(28, 15); printf("┏━━━━━━━━━━━┓\n");
	gotoxy(28, 16); printf("┃                      ┃\n");
	gotoxy(28, 17); printf("┃                      ┃\n");
	gotoxy(28, 18); printf("┃                      ┃\n");
	gotoxy(28, 19); printf("┃                      ┃\n");
	gotoxy(28, 20); printf("┗━━━━━━━━━━━┛\n");
}

void first_text() {
	gotoxy(0, 3);  printf("  ■■■■■    ■               ■ ■   ■■■■    ■■■■■ ■■■■■■   ");
	gotoxy(0, 4);  printf("  ■            ■  ■■■■■   ■ ■   ■      ■  ■      ■ ■             ");
	gotoxy(0, 5);  printf("  ■■■■■  ■■      ■       ■ ■   ■      ■  ■      ■ ■             ");
	gotoxy(0, 6);  printf("  ■            ■     ■■    ■■ ■   ■     ■   ■■■■■ ■             ");
	gotoxy(0, 7);  printf("  ■■■■■    ■    ■  ■     ■ ■   ■■■■    ■         ■   ■■■■  ");
	gotoxy(0, 8);  printf("                     ■    ■    ■ ■   ■     ■   ■         ■         ■  ");
	gotoxy(0, 9);  printf("  ■                ■      ■   ■ ■   ■      ■  ■         ■         ■  ");
	gotoxy(0, 10); printf("  ■■■■■■■■               ■ ■   ■       ■ ■         ■■■■■ ■  ");
}

void explain() {
	system("cls");

	gotoxy(6, 1); printf("< 게임 설명 >");
	gotoxy(6, 2);  printf("턴제 RPG는 레벨업을 통해 능력치를 올리고 보스와의 전투에서 승리하는");
	gotoxy(6, 3); printf("것이 목표인 게임입니다.");
	
	gotoxy(6, 5); printf("< 직업 설명 >");
	getcolor(RED2, BLACK);
	gotoxy(6, 6); printf("[전사] HP : 1000 공격력 : 40 방어력 : 60");
	gotoxy(6, 7); printf("HP : 1000 공격력 : 40 방어력 : 60");
	gotoxy(6, 8); printf("스킬1 : 강타 (200의 고정피해)");
	gotoxy(6, 9); printf("스킬2 : 방어 (방어력이 3턴간 50%%상승)");
	gotoxy(6, 10); printf("스킬3 : 일격 (400의 고정피해, 상대방 두턴동안 기절)");
	getcolor(GRAY1, BLACK);

	getcolor(BLUE2, BLACK);
	gotoxy(6, 11); printf("[궁수] HP : 700 공격력 : 60 방어력 : 40");
	gotoxy(6, 12); printf("스킬1 : 난사 (추가 데미지가 들어간 기본공격을 2회)  ");
	gotoxy(6, 13); printf("스킬2 : 치명타 (치명타 3턴간 치명타 공격)           ");
	gotoxy(6, 14); printf("스킬3 : 마법의 수정화살 (300의 고정피해, 세턴 기절) ");
	getcolor(GRAY1, BLACK);

	getcolor(GREEN1, BLACK);
	gotoxy(6, 15); printf("[암살자] HP : 600 공격력 : 70 방어력 : 40");
	gotoxy(6, 16); printf("HP : 600 공격력 : 70 방어력 : 40                    ");
	gotoxy(6, 17); printf("스킬1 : 출혈(두턴 동안 상대방 출혈효과)             ");
	gotoxy(6, 18); printf("스킬2 : 은신(공격을 한턴 무시, 체력 10%% 회복)      ");
	gotoxy(6, 19); printf("스킬3 : 암살(상대 체력이 20%% 이하일 경우 즉시 처치)");
	getcolor(GRAY1, BLACK);

	gotoxy(6, 21); printf("< 조작 키 >");
	gotoxy(6, 22); printf("이동 : 방향 키 | 저장 'S' | 인벤토리 'I' | 세부 상태창 'D'");
	_getch();
	system("cls");
	GameStart();
}

// 키 확인
int check_key() {
	unsigned char ch;
	while (1) {
		if (_kbhit() == 1) {  // 키보드가 눌려져 있으면
			ch = _getch(); // key 값을 읽는다
			if (ch == SPECIAL1 || ch == SPECIAL2) { // 만약 특수키
				// 예를 들어 UP key의 경우 0xe0 0x48 두개의 문자가 들어온다.
				ch = _getch();
				switch (ch) {
				case UP:
					return 1;
				case DOWN:
					return 2;
				case LEFT:
					return 3;
				case RIGHT:
					return 4;
				}
			}
			else {
				// 특수 문자가 아니지만 AWSD를 방향키 대신 사용하는 경우 처리
				switch (ch) {
				case UP2:
					return 1;
				case DOWN2:
					return 2;
				case LEFT2:
					return 3;
				case RIGHT2:
					return 4;
				case ENTER:
					return 10;
				}
			}
		}
	}
}

int MainTitle() {
	int x = 36;
	int y = 16;
	removeCursor();

	titleShape();
	first_text(); // 메인 화면에 "턴제 RPG 출력"
	gotoxy(x - 2, y); // ▶ 출력을 위해 x-2
	printf("▶ 게임시작");
	gotoxy(x, y + 1);
	printf(" 불러오기");
	gotoxy(x, y + 2);
	printf(" 게임설명 ");
	gotoxy(x, y + 3);
	printf("   종료 ");


	// 키 입력받고 ▶ 이동
	while (1) {
		int n = check_key();
		switch (n) {
		case 1:
			if (y > 16) {
				gotoxy(x - 2, y);
				printf(" ");
				gotoxy(x - 2, --y);
				printf("▶");
			}
			break;
		case 2:
			if (y < 19) {
				gotoxy(x - 2, y);
				printf(" ");
				gotoxy(x - 2, ++y);
				printf("▶");
			}
			break;
		case 10:
			return y - 15;
		}
	}
}

void GameStart() {
	while (1) {
		int menu_num = MainTitle(); // 메뉴 선택 변수
		system("cls");
		switch (menu_num) {
		case 1:
			new_Start();
			break;
		case 2:
			load_Start();
			break;
		case 3:
			explain();
			break;
		case 4:
			exit(1);
		}
	}
}

int main() {
	srand((unsigned)time(0));
	GameStart();
}