#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "neillsdl2.h"
#include "stack.h"
#include "Linked/specific.h"

#define MAXNUMTOKENS 500 /* Maximun instructions can be stored */
#define MAXTOKENSIZE 7 /* Maximun instruction size */
#define NUMVAR 26 /* Maximun var can be store (26 upper case words) */
#define strsame(A,B) (strcmp(A, B)==0)
#define ERROR(PHRASE) {fprintf(stderr, "Fatal Error : %s\n", PHRASE); exit(2); }
#define PI 3.14159265
#define CENTERX ( WWIDTH )/2 
#define CENTERY ( WHEIGHT )/2
#define MAXSTEP 10000  /* Maximun vertices can be drawn */
#define PICTURESIZE 1 /* Factor to multiple the size of the picture */

typedef enum errorcode {Parse_OK, error_BEGIN, error_INSTRUCTION, error_DO, error_SET, error_VAR, error_RBRACE, error_VAR_notset} errorcode;

struct var{
    float num;
    bool set;
}; /* A struct to store the VARs declared in the turtle program. 
    num is for the stored value. set record whether the variable is set or not. 
    false => not set; true => set */
typedef struct var var;

struct prog{
    char wds[MAXNUMTOKENS][MAXTOKENSIZE]; /* Store all fo the instructions in the turtle program */
    var vars[NUMVAR]; /* Store the VARs declared in the turtle program */
    int steps[MAXSTEP][2]; /* Store all of the drawing points in the graph */
    int cw; /* Current Line of the input code */
    int cstep; /* Current step */
    int word_counter; /* Count the total number of words  */
    int brace_seeker; /* An index for check if there is the right amount of } */
    int direction; /* Store the current direction of the turtle in degree */
    int x; /* Record the x position of the turtle */
    int y; /* Record the y position of the turtle */
    stack *s; /* Stack for POLISH */
    stacktype d, g1, g2;
    bool test_mode; /* A switch telling if the condition is in _test() function or not */
    errorcode error_code; /* To record the error_code detected */
    bool error_lock; /* Help maintain the most precise error code detected */
};
typedef struct prog Program;

void _test(Program *p);
void _test_Program_load_in(Program *p, char *f);
void Program_init(Program *p);
void Program_load_in(Program *p, char *fname);
void MAIN(Program *p);
void INSTRCTLST(Program *p);
void INSTRUCTION(Program *p);
void FD(Program *p);
void LT(Program *p);
void RT(Program *p);
float VARNUM(Program *p);
void DO(Program *p);
void SET(Program *p);
void POLISH(Program *p);
bool isoperator(char *c);
char VAR(Program *p);
void check_right_brace(Program *p);
void OP(Program *p);


int main(int argc, char **argv)
{
    
    char* fname;
    Program prog;
    SDL_Simplewin sw;
    int i;

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
        fprintf(stderr, "Please input valid command format : ./interp filename\n");
        exit(EXIT_FAILURE);
    }

    /* Load the file Data into prog */
    Program_load_in(&prog, fname);

    /* Start of the parsing and interpreting */
    MAIN(&prog);

    /* Start drawing the points stored in prog.steps[][] */
    Neill_SDL_Init(&sw);
    do{

        /* Set drawing color into white */
        Neill_SDL_SetDrawColour(&sw, 255, 255, 255);
        
        for(i = 0; i < prog.cstep; i++){
            SDL_RenderDrawLine(sw.renderer, prog.steps[i][0], prog.steps[i][1], prog.steps[i+1][0], prog.steps[i+1][1]);
        }
        
        Neill_SDL_UpdateScreen(&sw);

        /* Has anyone pressed ESC or killed the SDL window ?
        Must be called frequently - it's the only way of escaping  */
        Neill_SDL_Events(&sw);

   }while(!sw.finished);
    
    SDL_Quit();
    atexit(SDL_Quit);

    return 0;
}

void _test(Program *p)
{
    char good1[] = "{ FD 20 RT 45 FD 30 LT 60 }";
    /* good2 is the same as spiral2_c.ttl */
    char good2[] = "{ DO A FROM 1 TO 50 { FD A RT 30 DO B FROM 1 TO 8 { SET C := A 5 / ; FD C RT 45 } } }";
    /* good3 is the same as spiral1_c.ttl */
    char good3[] = "{ DO A FROM 1 TO 100 { SET C := A 1.5 * ; FD C RT 62 } }";
    char good4[] = "{ SET B := 5 4 + 3 * 9 / ; }";
    char good5[] = "{ SET K := 12 6 - 3 / 4 5 + - ; }";
    char good6[] = "{ SET C := 1.5 ; }";
    char good7[] = "{ SET A := 0.5 ; SET B := 1 ; SET C := 1.5 ; SET D := 2 ; SET E := 2.2 ; SET F := 3.3 ; SET G := 8.4 2 / ; SET H := 2 7 - 2 * ; }";
    
    char error1[] = " FD 20 RT 45 FD 30 LT 60 }";
    char error2[] = "{ FD 20 RRR 45 FD 30 LT 60 }";
    char error3[] = "{ FD 20 RT 45 FD30 LT 60 }";
    char error4[] = "{ DO A START 1 TO 50 { FD A RT 30 DO B FROM 1 TO 8 { SET C := A 5 / ; FD C RT 45 } } }";
    char error5[] = "{ DO A FROM 1 GOTO 50 { FD A RT 30 DO B FROM 1 TO 8 { SET C := A 5 / ; FD C RT 45 } } }";
    char error6[] = "{ DO A FROM 1 TO 50 { FD A RT 30 DO B FROM 1 TO 8 { SET C == A 5 / ; FD C RT 45 } } }";
    char error7[] = "{ DO KK FROM 1 TO 50 { FD A RT 30 DO B FROM 1 TO 8 { SET C := A 5 / ; FD C RT 45 } } }";
    char error8[] = "{ FD 20 RT 45 FD 30 LT 60 ";
    char error9[] = "{ DO A FROM 1 TO 50 { FD A RT 30 DO B FROM 1 TO 8 { SET C := A 5 / ; FD C RT 45 } ";
    char error10[] = "{ FD B RT 40 FD 20 LT 50 }";
    char error11[] = "{ DO A FROM 1 TO 100 { SET C := B 1.5 * ; FD C RT 62 } }";
    
    /* Check the parser can pass the right turtle program */
    Program_init(p); 
    _test_Program_load_in(p, good1);
    MAIN(p);
    assert(p->error_code == Parse_OK);

    Program_init(p); 
    _test_Program_load_in(p, good2);
    MAIN(p);
    assert(p->error_code == Parse_OK);

    Program_init(p); 
    _test_Program_load_in(p, good3);
    MAIN(p);
    assert(p->error_code == Parse_OK);

    Program_init(p); 
    _test_Program_load_in(p, good4);
    MAIN(p);
    assert(p->error_code == Parse_OK);

    Program_init(p); 
    _test_Program_load_in(p, good5);
    MAIN(p);
    assert(p->error_code == Parse_OK);

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

    Program_init(p); 
    _test_Program_load_in(p, error3);
    MAIN(p);
    assert(p->error_code == error_INSTRUCTION);

    /* Test the error detection for <DO> */
    Program_init(p); 
    _test_Program_load_in(p, error4);
    MAIN(p);
    assert(p->error_code == error_DO);

    Program_init(p); 
    _test_Program_load_in(p, error5);
    MAIN(p);
    assert(p->error_code == error_DO);

    /* Test the error detection for <SET> */
    Program_init(p); 
    _test_Program_load_in(p, error6);
    MAIN(p);
    assert(p->error_code == error_SET);

    /* Test the error detection for <VAR> */
    Program_init(p); 
    _test_Program_load_in(p, error7);
    MAIN(p);
    assert(p->error_code == error_VAR);

    /* Test the error detection for "}" */
    Program_init(p); 
    _test_Program_load_in(p, error8);
    MAIN(p);
    assert(p->error_code == error_RBRACE);

    Program_init(p); 
    _test_Program_load_in(p, error9);
    MAIN(p);
    assert(p->error_code == error_RBRACE);

    /* Test the function of <SET> and <POLISH> */
    Program_init(p); 
    _test_Program_load_in(p, good3);
    MAIN(p);
    assert(p->error_code == Parse_OK);
    assert( fabs(p->vars['C' - 'A'].num - 148.5) < 0.001);
    /* 148.5 = 99 * 1.5 */

    Program_init(p); 
    _test_Program_load_in(p, good4);
    MAIN(p);
    assert(p->error_code == Parse_OK);
    assert(fabs(p->d - 3) < 0.01);
    assert( fabs(p->vars['B' - 'A'].num - 3) < 0.001 );

    Program_init(p); 
    _test_Program_load_in(p, good5);
    MAIN(p);
    assert(p->error_code == Parse_OK);
    assert(fabs(p->d - (-7)) < 0.01);
    assert( fabs(p->vars['K' - 'A'].num - (-7)) < 0.001 );

    Program_init(p); 
    _test_Program_load_in(p, good6);
    MAIN(p);
    assert(p->error_code == Parse_OK);
    assert( fabs(p->vars['C' - 'A'].num - 1.5) < 0.001 );

    Program_init(p); 
    _test_Program_load_in(p, good7);
    MAIN(p);
    assert(p->error_code == Parse_OK);
    assert( fabs(p->vars['A' - 'A'].num -  0.5)  < 0.001 );
    assert( fabs(p->vars['B' - 'A'].num -  1  )  < 0.001 );
    assert( fabs(p->vars['C' - 'A'].num -  1.5)  < 0.001 );
    assert( fabs(p->vars['D' - 'A'].num -  2  )  < 0.001 );
    assert( fabs(p->vars['E' - 'A'].num -  2.2)  < 0.001 );
    assert( fabs(p->vars['F' - 'A'].num -  3.3)  < 0.001 );
    assert( fabs(p->vars['G' - 'A'].num -  4.2)  < 0.001 );
    assert( fabs(p->vars['H' - 'A'].num - (-10)) < 0.001 );

    /* Test the error detection for use of unset variable */
    Program_init(p); 
    _test_Program_load_in(p, error10);
    MAIN(p);
    assert(p->error_code == error_VAR_notset);

    Program_init(p); 
    _test_Program_load_in(p, error11);
    MAIN(p);
    assert(p->error_code == error_VAR_notset);

    /* Test isoperator */
    assert(isoperator("*"));
    assert(isoperator("/"));
    assert(!isoperator("&"));
    assert(!isoperator("1"));
    assert(!isoperator("++"));
}

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


/* Initialize the variables in p */
void Program_init(Program *p)
{
    int i;
    p->cw = 0;
    p->word_counter = 0;
    p->error_code = Parse_OK;
    p->error_lock = false;
    p->cstep = 0;
    p->x = CENTERX;
    p->y = CENTERY;
    p->direction = 0;
    p->steps[0][0] = p->x;
    p->steps[0][1] = p->y;
    /* Initialize the words stored */
    for(i=0; i<MAXNUMTOKENS; i++){
        p->wds[i][0] = '\0';
    }

    /* Initialize the VARs stored */
    for(i=0; i < NUMVAR; i++){
        p->vars[i].num = 0;
        p->vars[i].set = false;
    }
}

/* Load the Data of the input FILE into prog before parsing */
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
}

/* Parse and interpret INSTRCTLST */
void INSTRCTLST(Program *p)
{
    if(strsame(p->wds[p->cw], "}")){
        return;
    }
    INSTRUCTION(p);
    p->cw = p->cw +1;
    INSTRCTLST(p);
}

/* Parse and interpret INSTRUCTION */
void INSTRUCTION(Program *p)
{
    if(strsame(p->wds[p->cw], "FD")){
        FD(p);
    }else if(strsame(p->wds[p->cw], "LT")){
        LT(p);
    }else if(strsame(p->wds[p->cw], "RT")){
        RT(p);
    }else if(strsame(p->wds[p->cw], "DO")){
        DO(p);
    }else if(strsame(p->wds[p->cw], "SET")){
        SET(p);
    }else{
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
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

/* Parse and interpret FD */
void FD(Program *p)
{
    int x;
    int y;
    float num;
    p->cw = p->cw +1;
    num = VARNUM(p);
    /* Compute the moving in x and y, through resolution of vector by trigonometric functions */
    /* PI / 180 is a constant to convert degree into radian */

    x = num*cos(p->direction * PI / 180) * PICTURESIZE;
    y = num*sin(p->direction * PI / 180) * PICTURESIZE; 

    p->cstep ++;
    p->x += x;
    p->y += y;
    p->steps[p->cstep][0] = p->x;
    p->steps[p->cstep][1] = p->y;
}

/* Parse and interpret LT */
void LT(Program *p)
{
    float num;
    p->cw = p->cw +1;
    num = VARNUM(p);
    p->direction -= num;
}

/* Parse and interpret RT */
void RT(Program *p)
{
    float num;
    p->cw = p->cw +1;
    num = VARNUM(p);
    p->direction += num;
}

/* Parse and interpret DO */
void DO(Program *p)
{
    int f;
    int t;
    int cw_reset;
    int brace_seeker_reset;
    int index;
    
    p->cw = p->cw +1;
    index = VAR(p) - 'A'; /* Convert char VAR(p) into integer [0-26] */
    p->cw = p->cw +1;
    if(!strsame(p->wds[p->cw], "FROM")){
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_DO;
                p->error_lock = true;
            }
            return;
        }
        ERROR("Not a valid format for: DO <VAR> FROM <VARNUM> TO <VARNUM> {}");
    }
    p->cw = p->cw +1;
    f = VARNUM(p);
    p->cw = p->cw +1;
    if(!strsame(p->wds[p->cw], "TO")){
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_DO;
                p->error_lock = true;
            }
            return;
        }
        ERROR("Not a valid format for: DO <VAR> FROM <VARNUM> TO <VARNUM> {}");
    }
    p->cw = p->cw +1;
    t = VARNUM(p);
    p->cw = p->cw +1;
    if(!strsame(p->wds[p->cw], "{")){
        /* In test_mode, if something wrong in the test data, 
        it will not cause an error_exit. It only update the error_code and return
        , handing the job to assert() in _test() */
        if(p->test_mode == true){
            if(p->error_lock == false){
                p->error_code = error_DO;
                p->error_lock = true;
            }
            return;
        }
        ERROR("Not a valid format for: DO <VAR> FROM <VARNUM> TO <VARNUM> {}");
    }else{
        check_right_brace(p);
    }
    brace_seeker_reset = p->brace_seeker;
    cw_reset = p->cw;
    for(;f < t; f++){
        p->vars[index].num = f;
        p->vars[index].set = true;
        p->cw = p->cw +1;
        INSTRCTLST(p);
        p->brace_seeker = brace_seeker_reset;
        p->cw = cw_reset;
    }
}

/* Parse and interpret SET */
void SET(Program *p)
{
    int index;
    p->cw = p->cw +1;
    index = VAR(p) - 'A'; /* Convert char VAR(p) into integer [0-26] */
    p->cw = p->cw +1;
    if(!strsame(p->wds[p->cw], ":=")){
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
        ERROR("Not a valid format for: SET <VAR> := <POLISH>");
    }
    p->cw = p->cw +1;
    /* The stack methods conducted are all used to implement reverse polish expression of arithmetic */
    p->s = stack_init();
    POLISH(p);
    assert(stack_pop(p->s, &p->d));
    /* Store the value of the VAR into the chart vars[] with corresponding index */
    p->vars[index].num = p->d;
    p->vars[index].set = true;
    if(stack_peek(p->s, &p->d) == true){
        fprintf(stderr, "Stack still had items on it ?\n");
        exit(EXIT_FAILURE);
    }
   stack_free(p->s);
}

/* Parse and interpret POLISH */
void POLISH(Program *p)
{
    if(isoperator(p->wds[p->cw])){
        OP(p);
        p->cw = p->cw +1;
        POLISH(p);
    }else if(strsame(p->wds[p->cw], ";")){
        return;
    }else{
        p->d = VARNUM(p);
        stack_push(p->s, p->d);
        p->cw = p->cw +1;
        POLISH(p);
    }
}

/* Parse and interpret OP */
void OP(Program *p)
{
    assert(stack_pop(p->s, &p->g2));
    assert(stack_pop(p->s, &p->g1));
     switch (p->wds[p->cw][0])
    {
    case '+' :
        p->d = p->g1 + p->g2;
        break;
    case '-' :
        p->d = p->g1 - p->g2;
        break;
    case '*' :
        p->d = p->g1 * p->g2;
        break;
    case '/' :
        p->d = p->g1 / p->g2;
        break;
    
    default:
        fprintf(stderr, "Can't understand that operator ? \n");
        exit(EXIT_FAILURE);
        break;
    }
    stack_push(p->s, p->d);
}

/* A function to check whether the input string is a operator or not */
bool isoperator(char *c)
{
    if(strsame(c, "+") || strsame(c, "-") || strsame(c, "*") || strsame(c, "/")){ 
        return true;
    }else{
        return false; 
    }
}

/* Parse and inperpret VAR with the char read in the program returned */
char VAR(Program *p)
{
    char i;
    char v[MAXTOKENSIZE];
    v[0] = '\0';
    v[1] = '\0';

    for(i = 'A'; i <= 'Z'; i++){
        v[0] = i;
        if( strsame(p->wds[p->cw], v) ){
            return i;
        }
    }
    /* In test_mode, if something wrong in the test data, 
    it will not cause an error_exit. It only update the error_code and return
    , handing the job to assert() in _test() */
    if(p->test_mode == true){
        if(p->error_lock == false){
            p->error_code = error_VAR;
            p->error_lock = true;
        }
        return 'A';
    }
    ERROR("Need a uppercase character for VAR in grammer");
}

/* A function parses the VARNUM with the value of VARNUM returned */
float VARNUM(Program *p)
{
    float num;
    int index;
    if(sscanf(p->wds[p->cw], "%f", &num) == 1){
        return num;
    }else{
        index = VAR(p) - 'A';
        if(p->vars[index].set == false){
            /* In test_mode, if something wrong in the test data, 
            it will not cause an error_exit. It only update the error_code and return
            , handing the job to assert() in _test() */
            if(p->test_mode == true){
                if(p->error_lock == false){
                    p->error_code = error_VAR_notset;
                    p->error_lock = true;
                }
                return p->vars[index].num;
            }
            ERROR("A VAR have not been set before use");
        }
        return p->vars[index].num;
    } 
}

/* Check if there is right amount of } in the program */
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

