#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "neillsdl2.h"

#define MAXNUMTOKENS 500 /* Maximun instructions can be stored */
#define MAXTOKENSIZE 7 /* Maximun instruction size */
#define MAXNUM 120 /* Maximun amount of number can be visualized */
#define NUMSAFE 100 /* Maximun amount of number can be visualized with safe buffer */
#define MAXSTEP 14400 /* Maximun amount of steps can be visualized */
#define BARWIDTH 2 /* Define the original width of the data bar */
#define UNITHEIGHT 2 /* Define the original height of the data unit in bar chart */
#define CENTERX WWIDTH/2 
#define CENTERY WHEIGHT/2
#define PICTURESIZE 1 /* Factor to multiple the size of the picture */
#define MILLISECONDDELAY 80 /* The waiting time between frams of animation */
#define strsame(A,B) (strcmp(A, B)==0)
#define ERROR(PHRASE) {fprintf(stderr, "Fatal Error : %s\n", PHRASE); exit(2); }

typedef enum bool {false, true} bool;
typedef enum sorttype {BUBBLE, MERGE, QUICK} sorttype;
typedef enum errorcode {Parse_OK, error_BEGIN, error_INSTRUCTION, error_SET, error_RBRACE, error_SETINSTRCT, error_COLOR, error_TYPE, error_SORT, error_NUM, error_CNUM} errorcode;
typedef enum color {WHITE, RED, GREEN, BLUE} color;

struct prog{
    char wds[MAXNUMTOKENS][MAXTOKENSIZE]; /* Store all fo the instructions in the sort program */
    int steps[MAXSTEP][MAXNUM]; /* Store all of the steps needed to sort the input number */
    int cw; /* Current Line of the input code */
    int cstep; /* Current step */
    int cnum; /* Current number */
    int word_counter; /* Count the total number of words  */
    int brace_seeker; /* An index for check if there is the right amount of } */
    bool test_mode; /* A switch telling if the condition is in _test() function or not */
    errorcode error_code; /* To record the error_code detected */
    bool error_lock; /* Help maintain the most precise error code detected */
    SDL_Simplewin sw;
    SDL_Rect rectangle;
    color draw_color; /* store the desired color */
    sorttype type; /* store the desired sorting method */
    int x; /* X offset */
    int y; /* Y offset */
    int delay; /* delay time between frames */
};
typedef struct prog Program;

void _test(Program *p);
void _test_Program_load_in(Program *p, char *s);
void Program_init(Program *p);
void Program_load_in(Program *p, char *fname);
void check_right_brace(Program *p);
void MAIN(Program *p);
void INSTRCTLST(Program *p);
void INSTRUCTION(Program *p);
void SET(Program *p);
void SETINSTRCTLST(Program *p);
void SETINSTRCT(Program *p);
void HEIGHT(Program *p);
void WIDTH(Program *p);
void X(Program *p);
void Y(Program *p);
void COLOR(Program *p);
void TYPE(Program* p);
void SORT(Program *p);
void NUMLST(Program *p);
int NUM(Program *p);
void bubble_sort(Program *p);
void merge_sort(Program *p);
void merge_sort_recursive(Program *p, int *spare, int l, int r);
void merge(Program *p, int *spare, int l, int m, int r);
void quick_sort(Program *p);
void quick_sort_recursive(Program *p, int l, int r);
int partition(Program *p, int l, int r);
void sort_combine(Program *p);
void DELAY(Program *p);

int main(int argc, char **argv)
{
    int i;
    int j;
    int print_step;
    char* fname;
    Program prog;

    /* Start test */
    prog.test_mode = true;
    _test(&prog);

    /* Turn off the test mode */
    prog.test_mode = false;

    /* Initialize the variables in prog */
    Program_init(&prog);

    if( argc == 2 ){
        fname = (char*)malloc((strlen(argv[1]) + 1 )* sizeof(char));
        fname = argv[1];
        
    }else{
        fprintf(stderr, "Please input valid command format : ./sortingViz SortData/sort50_bubble.srt\n");
        exit(EXIT_FAILURE);
    }

    /* Load the file Data into prog */
    Program_load_in(&prog, fname);

    /* Start interpreting and sorting */
    MAIN(&prog);

    /* Start drawing the animation of sorting */
    Neill_SDL_Init(&prog.sw);
    print_step = 0;
    do{
        SDL_Delay(prog.delay);
        /* Set drawing color */
        if(prog.draw_color == WHITE){
            Neill_SDL_SetDrawColour(&prog.sw, 255, 255, 255);
        }else if(prog.draw_color == RED){
            Neill_SDL_SetDrawColour(&prog.sw, 255, 0, 0);
        }else if(prog.draw_color == GREEN){
            Neill_SDL_SetDrawColour(&prog.sw, 0, 255, 0);
        }else if(prog.draw_color == BLUE){
            Neill_SDL_SetDrawColour(&prog.sw, 0, 0, 255);
        }else{
            Neill_SDL_SetDrawColour(&prog.sw, 255, 255, 255);
        }
        
        for(i = 0; i < prog.cnum; i++){
            for(j = 0; j < prog.steps[print_step][i]; j++){
                prog.rectangle.x = (i * prog.rectangle.w) + prog.x + CENTERX;
                prog.rectangle.y = (-(j * prog.rectangle.h)) - prog.y + CENTERY;
                SDL_RenderFillRect(prog.sw.renderer, &prog.rectangle);
            }
        }
        

        Neill_SDL_UpdateScreen(&prog.sw);

        /* Clear screen, & set draw colour to black */
        SDL_SetRenderDrawColor(prog.sw.renderer, 0, 0, 0, 255);
        SDL_RenderClear(prog.sw.renderer);
        /* Has anyone pressed ESC or killed the SDL window ?
        Must be called frequently - it's the only way of escaping  */
        Neill_SDL_Events(&prog.sw);

        if(print_step < prog.cstep){
            print_step ++;
        }
        
   }while(!prog.sw.finished);

    SDL_Quit();
    atexit(SDL_Quit);

    return 0;
}

void _test(Program *p)
{
    char good1[] = "{ SET { X -100 Y -200 HEIGHT 10 WIDTH 20 TYPE BUBBLE COLOR BLUE DELAY 10 } SORT { 2 8 4 7 5 6 1 9 3 10 } }";
    char good2[] = "{ SORT { 2 8 4 7 5 6 1 9 3 10 } SET { Y 30 WIDTH 2 HEIGHT 5 DELAY 20 TYPE QUICK COLOR RED X 20 } }";
    char good3[] = "{ SET { X 23 WIDTH 10 Y 22 DELAY 5 TYPE MERGE HEIGHT 3 } SORT { 2 8 4 7 5 6 1 9 3 10 } }";

    char error1[] = " SET { X 23 WIDTH 10 Y 22 DELAY 5 TYPE MERGE HEIGHT 3 } SORT { 2 8 4 7 5 6 1 9 3 10 } }";
    char error2[] = "{ OAT { X 23 WIDTH 10 Y 22 DELAY 5 TYPE MERGE HEIGHT 3 } SORT { 2 8 4 7 5 6 1 9 3 10 } }";
    char error3[] = "{ SET X 23 WIDTH 10 Y 22 DELAY 5 TYPE MERGE HEIGHT 3 } SORT { 2 8 4 7 5 6 1 9 3 10 } }";
    char error4[] = "{ SET { X 23 WIDTH 10 YYY 22 DELAY 5 TYPE MERGE HEIGHT 3 } SORT { 2 8 4 7 5 6 1 9 3 10 } }";
    char error5[] = "{ SET { X 23 BYEBYEWIDTH 10 Y 22 DELAY 5 TYPE MERGE HEIGHT 3 } SORT { 2 8 4 7 5 6 1 9 3 10 } }";
    char error6[] = "{ SET { COLOR GRAY X -100 Y -200 HEIGHT 10 WIDTH 20 TYPE BUBBLE DELAY 10 } SORT { 2 8 4 7 5 6 1 9 3 10 } }";
    char error7[] = "{ SET { TYPE SLEEP X 23 WIDTH 10 Y 22 DELAY 5 HEIGHT 3 } SORT { 2 8 4 7 5 6 1 9 3 10 } }";
    char error8[] = "{ SORT : 2 8 4 7 5 6 1 9 3 10 } SET { Y 30 WIDTH 2 HEIGHT 5 DELAY 20 TYPE QUICK COLOR RED X 20 } }";
    char error9[] = "{ SORT { A 8 4 7 5 6 1 9 3 10 } SET { Y 30 WIDTH 2 HEIGHT 5 DELAY 20 TYPE QUICK COLOR RED X 20 } }";
    char error10[] = "{ SET { X 23 WIDTH 10 Y 22 DELAY 5 TYPE MERGE HEIGHT 3 } SORT { 2 8 4 7 5 6 1 9 3 10 } ";
    /* Check the parser can pass the right sorting visualization program and sort function */

    /* test the bubble_sort() and the other settings for drawing and data storage  */
    Program_init(p); 
    _test_Program_load_in(p, good1);
    MAIN(p);
    assert(p->error_code == Parse_OK);

    assert(p->type == BUBBLE);
    assert(p->draw_color == BLUE);
    assert(p->rectangle.h == 10);
    assert(p->rectangle.w == 20);
    assert(p->delay == 10);
    assert(p->x == -100);
    assert(p->y == -200);

    assert(p->steps[0][0] == 2);
    assert(p->steps[0][1] == 8);
    assert(p->steps[0][2] == 4);
    assert(p->steps[0][3] == 7);
    assert(p->steps[0][4] == 5);
    assert(p->steps[0][5] == 6);
    assert(p->steps[0][6] == 1);
    assert(p->steps[0][7] == 9);
    assert(p->steps[0][8] == 3);
    assert(p->steps[0][9] == 10);
    assert(p->cnum == 10);
    
    assert(p->steps[p->cstep][0] == 1);
    assert(p->steps[p->cstep][1] == 2);
    assert(p->steps[p->cstep][2] == 3);
    assert(p->steps[p->cstep][3] == 4);
    assert(p->steps[p->cstep][4] == 5);
    assert(p->steps[p->cstep][5] == 6);
    assert(p->steps[p->cstep][6] == 7);
    assert(p->steps[p->cstep][7] == 8);
    assert(p->steps[p->cstep][8] == 9);
    assert(p->steps[p->cstep][9] == 10);

    /* test the quick_sort() and the other settings for drawing and data storage  */
    Program_init(p); 
    _test_Program_load_in(p, good2);
    MAIN(p);
    assert(p->error_code == Parse_OK);

    assert(p->type == QUICK);
    assert(p->draw_color == RED);
    assert(p->rectangle.h == 5);
    assert(p->rectangle.w == 2);
    assert(p->delay == 20);
    assert(p->x == 20);
    assert(p->y == 30);

    assert(p->steps[p->cstep][0] == 1);
    assert(p->steps[p->cstep][1] == 2);
    assert(p->steps[p->cstep][2] == 3);
    assert(p->steps[p->cstep][3] == 4);
    assert(p->steps[p->cstep][4] == 5);
    assert(p->steps[p->cstep][5] == 6);
    assert(p->steps[p->cstep][6] == 7);
    assert(p->steps[p->cstep][7] == 8);
    assert(p->steps[p->cstep][8] == 9);
    assert(p->steps[p->cstep][9] == 10);

    /* test the merge_sort() and the other settings for drawing and data storage  */
    Program_init(p); 
    _test_Program_load_in(p, good3);
    MAIN(p);
    assert(p->error_code == Parse_OK);

    assert(p->type == MERGE);
    assert(p->draw_color == WHITE);
    assert(p->rectangle.h == 3);
    assert(p->rectangle.w == 10);
    assert(p->delay == 5);
    assert(p->x == 23);
    assert(p->y == 22);

    assert(p->steps[p->cstep][0] == 1);
    assert(p->steps[p->cstep][1] == 2);
    assert(p->steps[p->cstep][2] == 3);
    assert(p->steps[p->cstep][3] == 4);
    assert(p->steps[p->cstep][4] == 5);
    assert(p->steps[p->cstep][5] == 6);
    assert(p->steps[p->cstep][6] == 7);
    assert(p->steps[p->cstep][7] == 8);
    assert(p->steps[p->cstep][8] == 9);
    assert(p->steps[p->cstep][9] == 10);

    /* Test the error detection for the "{" in the begining of the input turtle program  */
    Program_init(p); 
    _test_Program_load_in(p, error1);
    MAIN(p);
    assert(p->error_code == error_BEGIN);

    /* Test the error detection for <INSTRUCTION> */
    Program_init(p); 
    _test_Program_load_in(p, error2);
    MAIN(p);
    assert(p->error_code == error_INSTRUCTION);

    /* Test the error detection for <SET> */
    Program_init(p); 
    _test_Program_load_in(p, error3);
    MAIN(p);
    assert(p->error_code == error_SET);

    /* Test the error detection for <SETINSTRCT> */
    Program_init(p); 
    _test_Program_load_in(p, error4);
    MAIN(p);
    assert(p->error_code == error_SETINSTRCT);

    Program_init(p); 
    _test_Program_load_in(p, error5);
    MAIN(p);
    assert(p->error_code == error_SETINSTRCT);

    /* Test the error detection for <COLOR> */
    Program_init(p); 
    _test_Program_load_in(p, error6);
    MAIN(p);
    assert(p->error_code == error_COLOR);

    /* Test the error detection for <TYPE> */
    Program_init(p); 
    _test_Program_load_in(p, error7);
    MAIN(p);
    assert(p->error_code == error_TYPE);

    /* Test the error detection for <SORT> */
    Program_init(p); 
    _test_Program_load_in(p, error8);
    MAIN(p);
    assert(p->error_code == error_SORT);

    /* Test the error detection for <NUM> */
    Program_init(p); 
    _test_Program_load_in(p, error9);
    MAIN(p);
    assert(p->error_code == error_NUM);

    /* Test the error detection for "}" */
    Program_init(p); 
    _test_Program_load_in(p, error10);
    MAIN(p);
    assert(p->error_code == error_RBRACE);
}

/* initialize the input Program */
void Program_init(Program *p)
{
    int i;
    int j;
    p->cw = 0;
    p->word_counter = 0;
    p->error_code = Parse_OK;
    p->error_lock = false;
    p->cstep = 0;
    p->cnum = 0;
    p->rectangle.w = BARWIDTH;
    p->rectangle.h = UNITHEIGHT;
    p->type = BUBBLE;
    p->delay = MILLISECONDDELAY;
    p->draw_color = WHITE;

    /* Initialize the words stored */
    for(i=0; i<MAXNUMTOKENS; i++){
        p->wds[i][0] = '\0';
    }

    /* Initialize the sorting steps */
    for(j = 0; j < MAXSTEP; j++){
        for(i = 0; i < MAXNUM; i++){
            p->steps[j][i] = 0;
        }
    }
}

/* Load the data into Program */
void Program_load_in(Program *p, char *fname)
{
    int i;
    FILE *fp = NULL;
    if((fp = fopen(fname, "r")) == NULL){
            fprintf(stderr, "Can not open %s\n", fname);
            exit(EXIT_FAILURE);
    }
    /* scan the file and save the words into prog.wds[][] */
    i=0;
    while(fscanf(fp, "%s", p->wds[i++])==1 && i<MAXNUMTOKENS){
        p->word_counter ++;
    }
    /* make brace_seeker start from the end of program */
    p->brace_seeker = p->word_counter;
    fclose(fp);
}

/* data loading for _test() */
void _test_Program_load_in(Program *p, char *s)
{
    int i;
    int offset;
    /* scan the testing string and save the words into prog.wds[][] */
    i=0;
    while(sscanf(s, "%s%n", p->wds[i++], &offset)==1 && i<MAXNUMTOKENS){
        s += offset;
        p->word_counter ++;
    }
    /* make brace_seeker start from the end of program */
    p->brace_seeker = p->word_counter;
}

/* Parse and interpret MAIN */
void MAIN(Program *p)
{
    if(!strsame(p->wds[p->cw], "{")){
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_BEGIN;
                p->error_lock = true;
            }
            return;
        }
        ERROR("No { statement found for begining the program");
    }
    /* If there is a {, there must be a }. For a more precise debug. */
    check_right_brace(p);
    p->cw = p->cw +1;
    INSTRCTLST(p);
    /* Start to sort the input numbers */
    sort_combine(p);
}

/* parse and interpret INSTRCTLST */
void INSTRCTLST(Program *p)
{
    if(strsame(p->wds[p->cw], "}")){
        return;
    }
    INSTRUCTION(p);
    p->cw = p->cw +1;
    INSTRCTLST(p);
}

/* parse and interpret INSTRUCTION */
void INSTRUCTION(Program *p)
{
    if(strsame(p->wds[p->cw], "SET")){
        SET(p);
    }else if(strsame(p->wds[p->cw], "SORT")){
        SORT(p);
    }else{
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_INSTRUCTION;
                p->error_lock = true;
            }
            return;
        }
        ERROR("Not a valid instruction");
    }
}

void SET(Program *p)
{
    p->cw = p->cw +1;
    if(!strsame(p->wds[p->cw], "{")){
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_SET;
                p->error_lock = true;
            }
            return;
        }
        ERROR("Not a valid format for: SET {}");
    }else{
        check_right_brace(p);
    }
    p->cw = p->cw +1;
    SETINSTRCTLST(p);
}

void SETINSTRCTLST(Program *p)
{
    if(strsame(p->wds[p->cw], "}")){
        return;
    }
    SETINSTRCT(p);
    p->cw = p->cw +1;
    SETINSTRCTLST(p);
}

void SETINSTRCT(Program *p)
{
    if(strsame(p->wds[p->cw], "HEIGHT")){
        HEIGHT(p);
    }else if(strsame(p->wds[p->cw], "WIDTH")){
        WIDTH(p);
    }else if(strsame(p->wds[p->cw], "COLOR")){
        COLOR(p);
    }else if(strsame(p->wds[p->cw], "TYPE")){
        TYPE(p);
    }else if(strsame(p->wds[p->cw], "X")){
        X(p);
    }else if(strsame(p->wds[p->cw], "Y")){
        Y(p);
    }else if(strsame(p->wds[p->cw], "DELAY")){
        DELAY(p);
    }else{
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_SETINSTRCT;
                p->error_lock = true;
            }
            return;
        }
        printf("%s", p->wds[p->cw]);
        ERROR("Not a valid SETINSTRCT");
    }
}

void HEIGHT(Program *p)
{
    p->cw = p->cw +1;
    p->rectangle.h = NUM(p);
}

void WIDTH(Program *p)
{
    p->cw = p->cw +1;
    p->rectangle.w = NUM(p);
}

void X(Program *p){
    p->cw = p->cw +1;
    p->x = NUM(p);
}

void Y(Program *p){
    p->cw = p->cw +1;
    p->y = NUM(p);
}

void DELAY(Program *p){
    p->cw = p->cw +1;
    p->delay = NUM(p);
}

void COLOR(Program *p)
{
    p->cw = p->cw +1;
    if(strsame(p->wds[p->cw], "WHITE")){
        p->draw_color = WHITE;
    }else if(strsame(p->wds[p->cw], "RED")){
        p->draw_color = RED;
    }else if(strsame(p->wds[p->cw], "GREEN")){
        p->draw_color = RED;
    }else if(strsame(p->wds[p->cw], "BLUE")){
        p->draw_color = BLUE;
    }else{
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_COLOR;
                p->error_lock = true;
            }
            return;
        }
        ERROR("Not a valid COLOR which need to be either WHITE, RED, GREEN, or BLUE");
    }
}

void TYPE(Program *p)
{
    p->cw = p->cw +1;
    if(strsame(p->wds[p->cw], "BUBBLE")){
        p->type = BUBBLE;
    }else if(strsame(p->wds[p->cw], "MERGE")){
        p->type = MERGE;
    }else if(strsame(p->wds[p->cw], "QUICK")){
        p->type = QUICK;
    }else{
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_TYPE;
                p->error_lock = true;
            }
            return;
        }
        ERROR("Not a valid sort TYPE which need to be either BUBBLE, MERGE, or QUICK");
    }
}

void SORT(Program *p)
{
    p->cw = p->cw +1;
    if(!strsame(p->wds[p->cw], "{")){
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_SORT;
                p->error_lock = true;
            }
            return;
        }
        ERROR("Not a valid format for: SORT {}");
    }else{
        check_right_brace(p);
    }
    p->cw = p->cw +1;
    NUMLST(p);
}

void NUMLST(Program *p)
{
    if(strsame(p->wds[p->cw], "}")){
        return;
    }
    p->steps[p->cstep][p->cnum] = NUM(p);
    p->cnum ++;
    if(p->cnum > NUMSAFE){
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_CNUM;
                p->error_lock = true;
            }
            return;
        }
        ERROR("The amount of input number exceed the limit, Maximun amount is 100");
    }
    p->cw = p->cw +1;
    NUMLST(p);
}

int NUM(Program *p)
{
    int num;
    if(sscanf(p->wds[p->cw], "%d", &num) == 1){
        return num;
    }else{
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_NUM;
                p->error_lock = true;
            }
            return 0;
        }
        ERROR("Expect integer numbers in SORT {Num1, Num2, Num3, Num4}");
    }
}

void check_right_brace(Program *p)
{
    bool check_brace = false;
    char brace[] = "}";
    p->brace_seeker --;
    while( (p->brace_seeker > 0) && !strsame(p->wds[p->brace_seeker], "}")  ){
        p->brace_seeker --;
    }
    if(strsame(p->wds[p->brace_seeker], "}")){
            check_brace = true;
    }
    if(check_brace == false){
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_RBRACE;
                p->error_lock = true;
            }
            /* Without a proper amount of "}", the recursion can end up no stop, 
            so in test_mode, the missing "}" should be add back to the turtle program before return */
            memcpy(p->wds[p->word_counter++], brace, sizeof(brace));
            return;
        }
        ERROR("Expect more }");
    }
}

/* Perform bubble sort on the data in Program p */
void bubble_sort(Program *p)
{
    int i;
    int j;
    int changes;

    do{
        changes = 0;
        for(i = 0; i < p->cnum - 1; i++){
            if(p->steps[p->cstep][i] > p->steps[p->cstep][i+1]){
                for(j = 0; j < p->cnum; j++){
                    p->steps[p->cstep + 1][j] = p->steps[p->cstep][j];
                }
                p->steps[p->cstep + 1][i] = p->steps[p->cstep][i+1];
                p->steps[p->cstep + 1][i + 1] = p->steps[p->cstep][i];
                changes++;
                p->cstep ++;
            }
        }
    }while(changes);
}

/* Perform merge sort on the data in Program p */
void merge_sort(Program *p)
{
    int spare[MAXNUM];
    merge_sort_recursive(p, spare, 0, p->cnum -1);
}

/* inner recursion of merge_sort */
void merge_sort_recursive(Program *p, int *spare, int l, int r)
{
    int m;
    if(l != r){
        m = (l+r)/2;
        merge_sort_recursive(p, spare, l, m);
        merge_sort_recursive(p, spare, m+1, r);
        merge(p, spare, l, m, r);
    }
}

/* inner funtion of merge_sort to perform merge */
void merge(Program *p, int *spare, int l, int m, int r)
{
    int j;
    int s1, s2, d;
        s1 = l;
        s2 = m+1;
        d = l;
    for(j = 0; j < p->cnum; j++){
            p->steps[p->cstep + 1][j] = p->steps[p->cstep][j];
        }
    do{
        if(p->steps[p->cstep][s1] < p->steps[p->cstep][s2]){
            spare[d++] = p->steps[p->cstep][s1++];
        }else{
            spare[d++] = p->steps[p->cstep][s2++];
        }
        for(j = 0; j < p->cnum; j++){
            p->steps[p->cstep + 1][j] = p->steps[p->cstep][j];
        }
        p->cstep ++;
    }while((s1 <= m) && (s2 <= r));
    if(s1 > m){
        memcpy(&spare[d], &p->steps[p->cstep][s2], sizeof(spare[0])*(r-s2+1));
    }else{
        memcpy(&spare[d], &p->steps[p->cstep][s1], sizeof(spare[0])*(m-s1+1));
    }
    memcpy(&p->steps[p->cstep][l], &spare[l], (r-l+1)*sizeof(spare[0]));   
}

/* Perform quick sort on the data in Program p */
void quick_sort(Program *p)
{
    quick_sort_recursive(p, 0, p->cnum - 1);
}

/* inner recursion of quick_sort */
void quick_sort_recursive(Program *p, int l, int r)
{
    int pivpoint;
    pivpoint = partition(p, l, r);
    if(l < pivpoint){
        quick_sort_recursive(p, l, pivpoint-1);
    }
    if(r > pivpoint){
        quick_sort_recursive(p, pivpoint+1, r);
    }
}

/* inner funtion of quick_sort to perform partition */
int partition(Program *p, int l, int r)
{
    int j;
    int piv;
    piv =  p->steps[p->cstep][l];
    while(l<r){
        while(piv < p->steps[p->cstep][r] && l<r){
            for(j = 0; j < p->cnum; j++){
                p->steps[p->cstep + 1][j] = p->steps[p->cstep][j];
            }
            p->cstep ++;
            r--;
        } 
        if(r!=l){
        p->steps[p->cstep][l] = p->steps[p->cstep][r];
        l++;
        }
        /* Left -> Right Scan */
        while(piv > p->steps[p->cstep][l] && l<r){
            for(j = 0; j < p->cnum; j++){
            p->steps[p->cstep + 1][j] = p->steps[p->cstep][j];
            }
            p->cstep ++;
            l++;
        } 
        if(r!=l){
        p->steps[p->cstep][r] = p->steps[p->cstep][l];
        r--;
        }
    }
    p->steps[p->cstep][r] = piv;
    return r;
}

/* Perform the specific sorting method according to the input setting */
void sort_combine(Program *p){
    switch (p->type)
    {
    case BUBBLE:
        bubble_sort(p);
        break;
    case MERGE:
        merge_sort(p);
        break;
    case QUICK:
        quick_sort(p);
        break;
    default:
        bubble_sort(p);
        break;
    }
}

