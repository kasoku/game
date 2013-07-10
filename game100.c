/* gcc game100.c -o game100 -lcurses */
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>

#define X_MIN 10  /* ゲームエリアの最小X座標 */
#define X_MAX 20  /* ゲームエリアの最大X座標 */

#define ENEMY_INTERVAL  200  /* 敵の出現間隔 */
#define ENEMY_SPEED  10  /* 敵の移動間隔 */

typedef struct Node {  /* ノードの定義 */
    int x;  /* X座標 */
    int y;  /* Y座標 */
    char c;  /* 画面に表示する文字 */
    struct Node *prev;
    struct Node *next;
} Node;

Node *EnemyList;  /* 敵リスト */
int ShipX, ShipY;  /* 自機の座標 */
int Score = 0;  /* 得点 */
int GameOver = 0;  /* ゲームオーバーフラグ */

/* aからbまでの間の乱数を返す */
int getRandomNumber(int a , int b)
{
    return rand() % (b - a + 1) + a;
}

/* ノードの生成 */
Node *getNewNode(void)
{
    Node *node;

    if ((node = malloc(sizeof(Node))) == NULL) {
        exit(EXIT_FAILURE);
    }

    return node;
}

/* リストの先頭にノードを追加 */
void addTop(Node *node, Node *newNode)
{
    newNode->next = node;
    newNode->prev = node->prev;
    newNode->next->prev = newNode;
    newNode->prev->next = newNode;
} 

/* 指定ノードを削除 */
void deleteNode(Node *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
}

/* 初期化 */
void initialize(void)
{
    /* 画面の初期化 */
    initscr();
    noecho();
    wtimeout(stdscr, 10);  /* キー入力は 0.01秒で時間切れ */

    /* 自機の初期位置 */
    ShipX = (X_MIN + X_MAX) / 2;  /* 横位置はゲームエリア中央 */
    ShipY = LINES - 1;            /* 縦位置は最終行 */

    /* 敵リスト初期化(ダミーノード) */
    EnemyList = getNewNode();
    EnemyList->next = EnemyList->prev = EnemyList;
}

/* ゲームエリアの外側に壁を描く */
void drawWall(void)
{
    int y;

    for (y = 0; y < LINES; y++) {
        mvaddstr(y, X_MIN - 1, "+");
        mvaddstr(y, X_MAX + 1, "+");
    }
}

/* 敵ノードの追加 */
void addEnemy(void)
{
    Node *node;
    node = getNewNode();
    node->x = getRandomNumber(X_MIN, X_MAX);
    node->y = 0;
    node->c = '@';
    addTop(EnemyList, node);
}

/* 敵の移動 */
void moveEnemy(void)
{
    Node *node, *n;
    for (node = EnemyList->next; node != EnemyList; node = n) {
        n = node->next;
        node->y += 1;
        if(node->y == ShipY && node->x == ShipX) {
            deleteNode(node);
            Score ++;
        } else if(node->y > ShipY) {
            GameOver = 1;
        }
    }
}

/* 得点の表示 */
void drawScore(void)
{
    mvprintw(0, X_MAX + 3, "SCORE:%5d", Score);
}

/* リスト内の全ノードを表示 */
void drawList(Node *list)
{
    Node *node;

    for (node = list->next; node != list; node = node->next) {
        mvprintw(node->y, node->x, "%c", node->c);
    }
}

/* 自機を表示 */
void drawShip(void)
{
    mvaddstr(ShipY, ShipX, "A");
    move(ShipY, ShipX);
}

/* 後始末 */
void terminate(void)
{
    Node *node, *n;
    
    /* 全ノードの削除 */
    for (node = EnemyList->next; node != EnemyList; node = n) {
        n = node->next;  /* 次のノードの場所を覚えておく */
        deleteNode(node);
    }

    free(EnemyList);  /* ダミーノードの削除 */

    endwin();  /* cursesの終了処理 */
    printf("SCORE:%5d\n", Score);  /* スコアの表示 */
}

int main(void)
{
    int count_add = ENEMY_INTERVAL;  /* 敵生成カウンタ */
    int count_move = ENEMY_SPEED;  /* 敵移動カウンタ */
    char c;

    /* 初期化 */
    initialize();

    /* ゲームのメインループ */
    do {
        c = getch();  /* キーボードから１文字得る */
        switch (c) {  /* キー入力にしたがって分岐 */
        case 'q':  /* 強制終了 */
            GameOver = 1;
            break;
        case '4':
            if(X_MIN < ShipX) {
                ShipX += -1;
                break;
            } else {
                break;
            }
        case '5':
            if (X_MAX > ShipX) {
                ShipX += 1;
                break;
            } else {
                break;
            }
        default: break;
        }

        count_add--;  /* 敵生成カウンタを減算 */
        if (count_add == 0) {  /* 敵生成のタイミングに達したら */
            addEnemy();  /* 新たな敵を生成 */
            count_add = ENEMY_INTERVAL;  /* 敵生成カウンタをリセット */
            count_add -= Score * 5;  /* 得点に従って敵生成を早める */
        }

        count_move--;  /* 敵移動カウンタを減算 */
        if (count_move == 0) {  /* 敵移動のタイミングに達したら */
            moveEnemy();  /* 全ての敵をひとつ下に移動 */
            count_move = ENEMY_SPEED;  /* 敵移動カウンタリセット */
        }

        erase();  /* 画面消去 */
        drawWall();  /* 壁の表示 */
        drawScore();  /* 得点の表示 */
        drawList(EnemyList);  /* 敵の表示 */
        drawShip();  /* 自機の表示 */
    } while (GameOver != 1);  /* ゲームオーバーでなければ繰り返し */

    /* 後始末 */
    terminate();

    return 0;
}
