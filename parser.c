#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* ---------- Symbol IDs --------------------------------------
   Terminals first (0 .. NUM_TERMINALS-1), then non-terminals.
   EPSILON is a sentinel used only inside RHS arrays.
   ----------------------------------------------------------- */
enum {
    T_ID, T_NUM, T_STR,
    T_ADADI, T_ASHRIYA, T_HARF, T_MATN, T_MANTIQI,
    T_FOR, T_WHILE, T_AGAR, T_WAGARNA,
    T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE,
    T_COMMA, T_DCOLON, T_ASSIGN,
    T_EQ, T_LT, T_GT, T_LE, T_GE, T_NE1, T_NE2,
    T_PLUS, T_MINUS, T_MUL, T_DIV,
    T_TRUE, T_FALSE, T_WAPAS,
    T_DOLLAR,
    NUM_TERMINALS,

    N_Function = NUM_TERMINALS,
    N_ArgList, N_ArgListP, N_Arg,
    N_Declaration, N_Type,
    N_IdentList, N_IdentListP,
    N_Stmt, N_ForStmt, N_OptExpr,
    N_WhileStmt, N_IfStmt, N_ElsePart,
    N_CompStmt, N_StmtList,
    N_Expr, N_ExprP,
    N_Rvalue, N_RvalueP,
    N_Compare,
    N_Mag, N_MagP,
    N_Term, N_TermP,
    N_Factor,
    NUM_SYMBOLS,

    SYM_EPSILON = 9999
};

/* ---------- Pretty names for tracing ------------------------ */
static const char *symName(int s) {
    switch (s) {
        case T_ID: return "id"; case T_NUM: return "num"; case T_STR: return "str";
        case T_ADADI: return "Adadi"; case T_ASHRIYA: return "Ashriya";
        case T_HARF: return "Harf"; case T_MATN: return "Matn"; case T_MANTIQI: return "Mantiqi";
        case T_FOR: return "for"; case T_WHILE: return "while";
        case T_AGAR: return "Agar"; case T_WAGARNA: return "Wagarna";
        case T_LPAREN: return "("; case T_RPAREN: return ")";
        case T_LBRACE: return "{"; case T_RBRACE: return "}";
        case T_COMMA: return ","; case T_DCOLON: return "::"; case T_ASSIGN: return ":=";
        case T_EQ: return "=="; case T_LT: return "<"; case T_GT: return ">";
        case T_LE: return "<="; case T_GE: return ">="; case T_NE1: return "!="; case T_NE2: return "<>";
        case T_PLUS: return "+"; case T_MINUS: return "-"; case T_MUL: return "*"; case T_DIV: return "/";
        case T_TRUE: return "True"; case T_FALSE: return "False"; case T_WAPAS: return "Wapas";
        case T_DOLLAR: return "$";
        case N_Function: return "Function";
        case N_ArgList: return "ArgList"; case N_ArgListP: return "ArgList'";
        case N_Arg: return "Arg";
        case N_Declaration: return "Declaration"; case N_Type: return "Type";
        case N_IdentList: return "IdentList"; case N_IdentListP: return "IdentList'";
        case N_Stmt: return "Stmt"; case N_ForStmt: return "ForStmt";
        case N_OptExpr: return "OptExpr";
        case N_WhileStmt: return "WhileStmt"; case N_IfStmt: return "IfStmt";
        case N_ElsePart: return "ElsePart";
        case N_CompStmt: return "CompStmt"; case N_StmtList: return "StmtList";
        case N_Expr: return "Expr"; case N_ExprP: return "Expr'";
        case N_Rvalue: return "Rvalue"; case N_RvalueP: return "Rvalue'";
        case N_Compare: return "Compare";
        case N_Mag: return "Mag"; case N_MagP: return "Mag'";
        case N_Term: return "Term"; case N_TermP: return "Term'";
        case N_Factor: return "Factor";
        case SYM_EPSILON: return "e";
        default: return "?";
    }
}

/* ---------- Productions ------------------------------------- */
typedef struct { int lhs; int rhs[12]; int rhsLen; } Production;

static Production prods[] = {
 /*  0 */ {N_Function,   {N_Type, T_ID, T_LPAREN, N_ArgList, T_RPAREN, N_CompStmt}, 6},
 /*  1 */ {N_ArgList,    {N_Arg, N_ArgListP}, 2},
 /*  2 */ {N_ArgList,    {SYM_EPSILON}, 1},
 /*  3 */ {N_ArgListP,   {T_COMMA, N_Arg, N_ArgListP}, 3},
 /*  4 */ {N_ArgListP,   {SYM_EPSILON}, 1},
 /*  5 */ {N_Arg,        {N_Type, T_ID}, 2},
 /*  6 */ {N_Declaration,{N_Type, N_IdentList, T_DCOLON}, 3},
 /*  7 */ {N_Type,       {T_ADADI}, 1},
 /*  8 */ {N_Type,       {T_ASHRIYA}, 1},
 /*  9 */ {N_Type,       {T_HARF}, 1},
 /* 10 */ {N_Type,       {T_MATN}, 1},
 /* 11 */ {N_Type,       {T_MANTIQI}, 1},
 /* 12 */ {N_IdentList,  {T_ID, N_IdentListP}, 2},
 /* 13 */ {N_IdentListP, {T_COMMA, N_IdentList}, 2},
 /* 14 */ {N_IdentListP, {SYM_EPSILON}, 1},
 /* 15 */ {N_Stmt,       {N_ForStmt}, 1},
 /* 16 */ {N_Stmt,       {N_WhileStmt}, 1},
 /* 17 */ {N_Stmt,       {N_IfStmt}, 1},
 /* 18 */ {N_Stmt,       {N_CompStmt}, 1},
 /* 19 */ {N_Stmt,       {N_Declaration}, 1},
 /* 20 */ {N_Stmt,       {N_Expr, T_DCOLON}, 2},
 /* 21 */ {N_Stmt,       {T_DCOLON}, 1},
 /* 22 */ {N_ForStmt,    {T_FOR, T_LPAREN, N_Expr, T_DCOLON, N_OptExpr, T_DCOLON, N_OptExpr, T_RPAREN, N_Stmt}, 9},
 /* 23 */ {N_OptExpr,    {N_Expr}, 1},
 /* 24 */ {N_OptExpr,    {SYM_EPSILON}, 1},
 /* 25 */ {N_WhileStmt,  {T_WHILE, T_LPAREN, N_Expr, T_RPAREN, N_Stmt}, 5},
 /* 26 */ {N_IfStmt,     {T_AGAR, T_LPAREN, N_Expr, T_RPAREN, N_Stmt, N_ElsePart}, 6},
 /* 27 */ {N_ElsePart,   {T_WAGARNA, N_Stmt}, 2},
 /* 28 */ {N_ElsePart,   {SYM_EPSILON}, 1},
 /* 29 */ {N_CompStmt,   {T_LBRACE, N_StmtList, T_RBRACE}, 3},
 /* 30 */ {N_StmtList,   {N_Stmt, N_StmtList}, 2},
 /* 31 */ {N_StmtList,   {SYM_EPSILON}, 1},
 /* 32 */ {N_Expr,       {N_Rvalue, N_ExprP}, 2},
 /* 33 */ {N_ExprP,      {T_ASSIGN, N_Expr}, 2},
 /* 34 */ {N_ExprP,      {SYM_EPSILON}, 1},
 /* 35 */ {N_Rvalue,     {N_Mag, N_RvalueP}, 2},
 /* 36 */ {N_RvalueP,    {N_Compare, N_Mag, N_RvalueP}, 3},
 /* 37 */ {N_RvalueP,    {SYM_EPSILON}, 1},
 /* 38 */ {N_Compare,    {T_EQ}, 1},
 /* 39 */ {N_Compare,    {T_LT}, 1},
 /* 40 */ {N_Compare,    {T_GT}, 1},
 /* 41 */ {N_Compare,    {T_LE}, 1},
 /* 42 */ {N_Compare,    {T_GE}, 1},
 /* 43 */ {N_Compare,    {T_NE1}, 1},
 /* 44 */ {N_Compare,    {T_NE2}, 1},
 /* 45 */ {N_Mag,        {N_Term, N_MagP}, 2},
 /* 46 */ {N_MagP,       {T_PLUS,  N_Term, N_MagP}, 3},
 /* 47 */ {N_MagP,       {T_MINUS, N_Term, N_MagP}, 3},
 /* 48 */ {N_MagP,       {SYM_EPSILON}, 1},
 /* 49 */ {N_Term,       {N_Factor, N_TermP}, 2},
 /* 50 */ {N_TermP,      {T_MUL, N_Factor, N_TermP}, 3},
 /* 51 */ {N_TermP,      {T_DIV, N_Factor, N_TermP}, 3},
 /* 52 */ {N_TermP,      {SYM_EPSILON}, 1},
 /* 53 */ {N_Factor,     {T_LPAREN, N_Expr, T_RPAREN}, 3},
 /* 54 */ {N_Factor,     {T_ID}, 1},
 /* 55 */ {N_Factor,     {T_NUM}, 1},
 /* 56 */ {N_Stmt,       {T_WAPAS, N_Expr, T_DCOLON}, 3},
 /* 57 */ {N_Factor,     {T_STR}, 1},
 /* 58 */ {N_Factor,     {T_TRUE}, 1},
 /* 59 */ {N_Factor,     {T_FALSE}, 1},
};
#define NUM_PRODS (int)(sizeof(prods)/sizeof(prods[0]))

/* ---------- Parse table M[NonTerm][Term] = production idx ----
   Built from FIRST/FOLLOW computed by hand (see accompanying
   notes at the bottom of this file).
   ----------------------------------------------------------- */
static int  parseTable[NUM_SYMBOLS][NUM_TERMINALS];
static bool followSet [NUM_SYMBOLS][NUM_TERMINALS];   /* panic-mode sync */

/* helper: mark a bunch of terminals as being in FOLLOW(nt)        */
static void addFollow(int nt, const int *ts, int n) {
    for (int i = 0; i < n; i++) followSet[nt][ts[i]] = true;
}

static void initFollowSet(void) {
    for (int i = 0; i < NUM_SYMBOLS; i++)
        for (int j = 0; j < NUM_TERMINALS; j++)
            followSet[i][j] = false;

    int DOLLAR[]   = { T_DOLLAR };
    int RP[]       = { T_RPAREN };
    int RP_COMMA[] = { T_RPAREN, T_COMMA };
    int DCOL[]     = { T_DCOLON };
    int ID[]       = { T_ID };
    int RCB[]      = { T_RBRACE };
    int OPT_EXPR[] = { T_DCOLON, T_RPAREN };
    int EXPR_F[]   = { T_DCOLON, T_RPAREN };
    int RVAL_F[]   = { T_ASSIGN, T_DCOLON, T_RPAREN };
    int MAG_F[]    = { T_EQ,T_LT,T_GT,T_LE,T_GE,T_NE1,T_NE2,
                       T_ASSIGN, T_DCOLON, T_RPAREN };
    int TERM_F[]   = { T_PLUS,T_MINUS, T_EQ,T_LT,T_GT,T_LE,T_GE,T_NE1,T_NE2,
                       T_ASSIGN, T_DCOLON, T_RPAREN };
    int FACT_F[]   = { T_MUL,T_DIV, T_PLUS,T_MINUS,
                       T_EQ,T_LT,T_GT,T_LE,T_GE,T_NE1,T_NE2,
                       T_ASSIGN, T_DCOLON, T_RPAREN };
    int MAG_FIRST[]= { T_LPAREN,T_ID,T_NUM,T_STR,T_TRUE,T_FALSE };
    /* FOLLOW(Stmt) = FIRST(Stmt) U { }, Wagarna, $, ) } */
    int STMT_F[]   = { T_FOR,T_WHILE,T_AGAR,T_LBRACE,T_RBRACE,
                       T_ADADI,T_ASHRIYA,T_HARF,T_MATN,T_MANTIQI,
                       T_LPAREN,T_RPAREN,T_ID,T_NUM,T_STR,T_TRUE,T_FALSE,
                       T_DCOLON,T_WAPAS,T_WAGARNA,T_DOLLAR };

    addFollow(N_Function,    DOLLAR,   1);
    addFollow(N_ArgList,     RP,       1);
    addFollow(N_ArgListP,    RP,       1);
    addFollow(N_Arg,         RP_COMMA, 2);
    addFollow(N_IdentList,   DCOL,     1);
    addFollow(N_IdentListP,  DCOL,     1);
    addFollow(N_Type,        ID,       1);    /* Type is followed by an id */
    addFollow(N_StmtList,    RCB,      1);
    addFollow(N_OptExpr,     OPT_EXPR, 2);
    addFollow(N_Expr,        EXPR_F,   2);
    addFollow(N_ExprP,       EXPR_F,   2);
    addFollow(N_Rvalue,      RVAL_F,   3);
    addFollow(N_RvalueP,     RVAL_F,   3);
    addFollow(N_Compare,     MAG_FIRST,6);
    addFollow(N_Mag,         MAG_F,   10);
    addFollow(N_MagP,        MAG_F,   10);
    addFollow(N_Term,        TERM_F,  12);
    addFollow(N_TermP,       TERM_F,  12);
    addFollow(N_Factor,      FACT_F,  15);
    addFollow(N_Stmt,        STMT_F,  sizeof(STMT_F)/sizeof(int));
    addFollow(N_Declaration, STMT_F,  sizeof(STMT_F)/sizeof(int));
    addFollow(N_ForStmt,     STMT_F,  sizeof(STMT_F)/sizeof(int));
    addFollow(N_WhileStmt,   STMT_F,  sizeof(STMT_F)/sizeof(int));
    addFollow(N_IfStmt,      STMT_F,  sizeof(STMT_F)/sizeof(int));
    addFollow(N_CompStmt,    STMT_F,  sizeof(STMT_F)/sizeof(int));
    addFollow(N_ElsePart,    STMT_F,  sizeof(STMT_F)/sizeof(int));
}

static void initParseTable(void) {
    for (int i = 0; i < NUM_SYMBOLS; i++)
        for (int j = 0; j < NUM_TERMINALS; j++)
            parseTable[i][j] = -1;

    int typeT[] = {T_ADADI, T_ASHRIYA, T_HARF, T_MATN, T_MANTIQI};
    int cmpT[]  = {T_EQ, T_LT, T_GT, T_LE, T_GE, T_NE1, T_NE2};
    int exprFirst[] = {T_LPAREN, T_ID, T_NUM, T_STR, T_TRUE, T_FALSE};  /* extended */
    int nExpr = (int)(sizeof(exprFirst)/sizeof(int));

    /* Function */
    for (int i=0;i<5;i++) parseTable[N_Function][typeT[i]] = 0;

    /* ArgList / ArgList' */
    for (int i=0;i<5;i++) parseTable[N_ArgList][typeT[i]] = 1;
    parseTable[N_ArgList][T_RPAREN] = 2;
    parseTable[N_ArgListP][T_COMMA]  = 3;
    parseTable[N_ArgListP][T_RPAREN] = 4;

    /* Arg / Declaration */ 
    for (int i=0;i<5;i++) { parseTable[N_Arg][typeT[i]]=5; parseTable[N_Declaration][typeT[i]]=6; }

    /* Type */
    parseTable[N_Type][T_ADADI]=7;  parseTable[N_Type][T_ASHRIYA]=8;
    parseTable[N_Type][T_HARF]=9;   parseTable[N_Type][T_MATN]=10;
    parseTable[N_Type][T_MANTIQI]=11;

    /* IdentList / IdentList' */
    parseTable[N_IdentList][T_ID] = 12;
    parseTable[N_IdentListP][T_COMMA]  = 13;
    parseTable[N_IdentListP][T_DCOLON] = 14;

    /* Stmt */
    parseTable[N_Stmt][T_FOR]    = 15;
    parseTable[N_Stmt][T_WHILE]  = 16;
    parseTable[N_Stmt][T_AGAR]   = 17;
    parseTable[N_Stmt][T_LBRACE] = 18;
    for (int i=0;i<5;i++) parseTable[N_Stmt][typeT[i]] = 19;
    for (int i=0;i<nExpr;i++) parseTable[N_Stmt][exprFirst[i]] = 20;
    parseTable[N_Stmt][T_DCOLON] = 21;
    parseTable[N_Stmt][T_WAPAS]  = 56;   /* NEW: return statement */

    parseTable[N_ForStmt][T_FOR]       = 22;
    parseTable[N_WhileStmt][T_WHILE]   = 25;
    parseTable[N_IfStmt][T_AGAR]       = 26;

    /* OptExpr: FIRST(Expr) -> 23, else e on follow {::, )} */
    for (int i=0;i<nExpr;i++) parseTable[N_OptExpr][exprFirst[i]] = 23;
    parseTable[N_OptExpr][T_DCOLON] = 24;
    parseTable[N_OptExpr][T_RPAREN] = 24;

    /* ElsePart: Wagarna -> 27, else e on FOLLOW(Stmt) */
    parseTable[N_ElsePart][T_WAGARNA] = 27;
    int followStmt[] = { T_FOR,T_WHILE,T_AGAR,T_LBRACE,T_RBRACE,
                         T_ADADI,T_ASHRIYA,T_HARF,T_MATN,T_MANTIQI,
                         T_LPAREN,T_RPAREN,T_ID,T_NUM,T_STR,T_TRUE,T_FALSE,
                         T_DCOLON,T_WAPAS,T_DOLLAR };
    for (unsigned i=0;i<sizeof(followStmt)/sizeof(int);i++)
        parseTable[N_ElsePart][followStmt[i]] = 28;

    parseTable[N_CompStmt][T_LBRACE] = 29;

    /* StmtList: FIRST(Stmt) -> 30, } -> 31 */
    int firstStmt[] = { T_FOR,T_WHILE,T_AGAR,T_LBRACE,
                        T_ADADI,T_ASHRIYA,T_HARF,T_MATN,T_MANTIQI,
                        T_LPAREN,T_ID,T_NUM,T_STR,T_TRUE,T_FALSE,
                        T_DCOLON,T_WAPAS };
    for (unsigned i=0;i<sizeof(firstStmt)/sizeof(int);i++)
        parseTable[N_StmtList][firstStmt[i]] = 30;
    parseTable[N_StmtList][T_RBRACE] = 31;

    /* Expr / Expr' */
    for (int i=0;i<nExpr;i++) parseTable[N_Expr][exprFirst[i]] = 32;
    parseTable[N_ExprP][T_ASSIGN] = 33;
    parseTable[N_ExprP][T_DCOLON] = 34;
    parseTable[N_ExprP][T_RPAREN] = 34;

    /* Rvalue / Rvalue' */
    for (int i=0;i<nExpr;i++) parseTable[N_Rvalue][exprFirst[i]] = 35;
    for (int i=0;i<7;i++) parseTable[N_RvalueP][cmpT[i]] = 36;
    parseTable[N_RvalueP][T_ASSIGN] = 37;
    parseTable[N_RvalueP][T_DCOLON] = 37;
    parseTable[N_RvalueP][T_RPAREN] = 37;

    /* Compare */
    parseTable[N_Compare][T_EQ]=38;  parseTable[N_Compare][T_LT]=39;
    parseTable[N_Compare][T_GT]=40;  parseTable[N_Compare][T_LE]=41;
    parseTable[N_Compare][T_GE]=42;  parseTable[N_Compare][T_NE1]=43;
    parseTable[N_Compare][T_NE2]=44;

    /* Mag / Mag' */
    for (int i=0;i<nExpr;i++) parseTable[N_Mag][exprFirst[i]] = 45;
    parseTable[N_MagP][T_PLUS]  = 46;
    parseTable[N_MagP][T_MINUS] = 47;
    for (int i=0;i<7;i++) parseTable[N_MagP][cmpT[i]] = 48;
    parseTable[N_MagP][T_ASSIGN]=48; parseTable[N_MagP][T_DCOLON]=48; parseTable[N_MagP][T_RPAREN]=48;

    /* Term / Term' */
    for (int i=0;i<nExpr;i++) parseTable[N_Term][exprFirst[i]] = 49;
    parseTable[N_TermP][T_MUL] = 50;
    parseTable[N_TermP][T_DIV] = 51;
    parseTable[N_TermP][T_PLUS]=52; parseTable[N_TermP][T_MINUS]=52;
    for (int i=0;i<7;i++) parseTable[N_TermP][cmpT[i]] = 52;
    parseTable[N_TermP][T_ASSIGN]=52; parseTable[N_TermP][T_DCOLON]=52; parseTable[N_TermP][T_RPAREN]=52;

    /* Factor */
    parseTable[N_Factor][T_LPAREN] = 53;
    parseTable[N_Factor][T_ID]     = 54;
    parseTable[N_Factor][T_NUM]    = 55;
    parseTable[N_Factor][T_STR]    = 57;   /* NEW */
    parseTable[N_Factor][T_TRUE]   = 58;   /* NEW */
    parseTable[N_Factor][T_FALSE]  = 59;   /* NEW */
}

/* ---------- Token reader -----------------------------------
   token.txt lines look like:  <lexeme,CLASS>
   lexeme may itself contain a comma (e.g. <,,PUNCT_COMMA>),
   so we split on the LAST comma inside the < > pair.
   ----------------------------------------------------------- */
typedef struct { int term; char lexeme[128]; } Token;
static Token tokens[20000];
static int   numTokens = 0;

static int classify(const char *lex, const char *cls) {
    if (!strcmp(cls,"KEYWORD")) {
        char b[128]; int i;
        for (i=0; lex[i] && i<127; i++) b[i] = (char)tolower((unsigned char)lex[i]);
        b[i] = 0;
        if (!strcmp(b,"adadi"))   return T_ADADI;
        if (!strcmp(b,"ashriya")) return T_ASHRIYA;
        if (!strcmp(b,"harf"))    return T_HARF;
        if (!strcmp(b,"matn"))    return T_MATN;
        if (!strcmp(b,"mantiqi")) return T_MANTIQI;
        if (!strcmp(b,"for"))     return T_FOR;
        if (!strcmp(b,"while"))   return T_WHILE;
        if (!strcmp(b,"agar"))    return T_AGAR;
        if (!strcmp(b,"wagarna")) return T_WAGARNA;
        if (!strcmp(b,"true"))    return T_TRUE;
        if (!strcmp(b,"false"))   return T_FALSE;
        if (!strcmp(b,"wapas"))   return T_WAPAS;
        if (!strcmp(b,"marqazi")) return T_ID;   /* allowed as function-name identifier */
        return -1;  /* keyword not used by grammar */
    }
    if (!strcmp(cls,"IDENTIFIER")) return T_ID;
    if (!strcmp(cls,"NUMBER"))     return T_NUM;
    if (!strcmp(cls,"STRING"))     return T_STR;
    if (!strcmp(cls,"PUNCT_LPAREN")) return T_LPAREN;
    if (!strcmp(cls,"PUNCT_RPAREN")) return T_RPAREN;
    if (!strcmp(cls,"PUNCT_LBRACE")) return T_LBRACE;
    if (!strcmp(cls,"PUNCT_RBRACE")) return T_RBRACE;
    if (!strcmp(cls,"PUNCT_COMMA"))  return T_COMMA;
    if (!strcmp(cls,"PUNCT_DCOLON")) return T_DCOLON;
    if (!strcmp(cls,"OP_CEQ"))    return T_ASSIGN;
    if (!strcmp(cls,"OP_ASSIGN")) return T_ASSIGN;  /* fall-back — treat = as := if grammar uses := */
    if (!strcmp(cls,"OP_EQ"))  return T_EQ;
    if (!strcmp(cls,"OP_LT"))  return T_LT;
    if (!strcmp(cls,"OP_GT"))  return T_GT;
    if (!strcmp(cls,"OP_LEQ")) return T_LE;
    if (!strcmp(cls,"OP_GEQ")) return T_GE;
    if (!strcmp(cls,"OP_BNE")) return T_NE1;
    if (!strcmp(cls,"OP_NEQ")) return T_NE2;
    if (!strcmp(cls,"OP_ADD")) return T_PLUS;
    if (!strcmp(cls,"OP_SUB")) return T_MINUS;
    if (!strcmp(cls,"OP_MUL")) return T_MUL;
    if (!strcmp(cls,"OP_DIV")) return T_DIV;
    return -1;
}

static int readTokens(const char *path, FILE *errOut) {
    FILE *f = fopen(path,"r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *lt = strchr(line,'<');
        char *gt = strrchr(line,'>');
        if (!lt || !gt || gt <= lt) continue;
        *gt = 0;
        char *inner = lt + 1;
        char *comma = strrchr(inner, ',');   /* last comma = separator */
        if (!comma) continue;
        *comma = 0;
        const char *lex = inner;
        const char *cls = comma + 1;
        int t = classify(lex, cls);
        if (t == -1) {
            fprintf(errOut, "Warning: token <%s,%s> not recognised by grammar — skipped\n", lex, cls);
            continue;
        }
        tokens[numTokens].term = t;
        strncpy(tokens[numTokens].lexeme, lex, 127);
        tokens[numTokens].lexeme[127] = 0;
        numTokens++;
    }
    tokens[numTokens].term = T_DOLLAR;
    strcpy(tokens[numTokens].lexeme, "$");
    numTokens++;
    fclose(f);
    return 1;
}

/* ---------- Parse tree -------------------------------------- */
typedef struct TreeNode {
    int sym;
    char lexeme[128];
    struct TreeNode **kids;
    int nKids;
} TreeNode;

static TreeNode *newNode(int sym, const char *lex) {
    TreeNode *n = (TreeNode*)calloc(1, sizeof(TreeNode));
    n->sym = sym;
    if (lex) { strncpy(n->lexeme, lex, 127); n->lexeme[127]=0; }
    return n;
}
static void addKid(TreeNode *p, TreeNode *c) {
    p->kids = (TreeNode**)realloc(p->kids, (p->nKids+1)*sizeof(TreeNode*));
    p->kids[p->nKids++] = c;
}
static void printTree(TreeNode *n, int depth, FILE *out) {
    for (int i=0;i<depth;i++) fputs("  ", out);
    if (n->sym == SYM_EPSILON)            fprintf(out, "e\n");
    else if (n->sym < NUM_TERMINALS) {
        if (n->lexeme[0]) fprintf(out, "%s  [%s]\n", symName(n->sym), n->lexeme);
        else              fprintf(out, "%s\n", symName(n->sym));
    } else                                fprintf(out, "%s\n", symName(n->sym));
    for (int i=0;i<n->nKids;i++) printTree(n->kids[i], depth+1, out);
}

/* ---------- Driver (stack-based LL(1) algorithm) ------------ */
#define STACK_MAX 20000
static int        stk[STACK_MAX];
static TreeNode  *stkNode[STACK_MAX];
static int        sp = 0;

static void push(int s, TreeNode *n) { stk[sp] = s; stkNode[sp] = n; sp++; }
static void popS(void)               { sp--; }

static void fmtStack(char *buf, size_t n) {
    buf[0] = 0;
    for (int i=0; i<sp; i++) {
        strncat(buf, symName(stk[i]), n - strlen(buf) - 2);
        strncat(buf, " ",             n - strlen(buf) - 2);
    }
}
static void fmtInput(char *buf, size_t n, int pos) {
    buf[0] = 0;
    for (int i=pos; i<numTokens; i++) {
        strncat(buf, symName(tokens[i].term), n - strlen(buf) - 2);
        strncat(buf, " ",                     n - strlen(buf) - 2);
    }
}

static int parse(FILE *trace, FILE *err, TreeNode **rootOut) {
    TreeNode *root = newNode(N_Function, NULL);
    sp = 0;
    push(T_DOLLAR, NULL);
    push(N_Function, root);

    int pos = 0, step = 0, hadError = 0;

    /* Monitor-style trace: Step | Action | Stack | Input */
    fprintf(trace, "%-4s | %-4s | %-55s | %s\n", "Step", "Act", "Stack", "Input");
    fprintf(trace, "-----+------+---------------------------------------------------------+------------------------------------------\n");

    while (sp > 0) {
        int X = stk[sp-1];
        int a = tokens[pos].term;

        char sbuf[2048], ibuf[2048];
        fmtStack(sbuf, sizeof sbuf);
        fmtInput(ibuf, sizeof ibuf, pos);

        /* ------ terminal on top of stack ------------------- */
        if (X < NUM_TERMINALS) {
            if (X == a) {
                fprintf(trace, "%-4d | %-4s | %-55.55s | %-40.40s  match %s\n",
                        ++step, "MTCH", sbuf, ibuf, symName(X));
                if (stkNode[sp-1]) strncpy(stkNode[sp-1]->lexeme, tokens[pos].lexeme, 127);
                popS();
                if (X != T_DOLLAR) pos++;
                else break;
            } else {
                /* Terminal mismatch: pop the stray terminal off the stack */
                fprintf(trace, "%-4d | %-4s | %-55.55s | %-40.40s  mismatch: expected %s, pop stack\n",
                        ++step, "POP", sbuf, ibuf, symName(X));
                fprintf(err, "Syntax error: expected '%s' on input '%s' (lexeme \"%s\") — popped terminal from stack\n",
                        symName(X), symName(a), tokens[pos].lexeme);
                hadError = 1;
                popS();
            }
            continue;
        }

        /* ------ nonterminal on top of stack ---------------- */
        int p = parseTable[X][a];
        if (p >= 0) {
            /* US — update stack with production */
            TreeNode *parent = stkNode[sp-1];
            popS();
            Production *P = &prods[p];

            char prodStr[256]; prodStr[0] = 0;
            snprintf(prodStr, sizeof prodStr, "%s ->", symName(P->lhs));
            if (P->rhsLen == 1 && P->rhs[0] == SYM_EPSILON)
                strncat(prodStr, " e", sizeof(prodStr)-strlen(prodStr)-1);
            else for (int i=0;i<P->rhsLen;i++) {
                strncat(prodStr, " ",              sizeof(prodStr)-strlen(prodStr)-1);
                strncat(prodStr, symName(P->rhs[i]), sizeof(prodStr)-strlen(prodStr)-1);
            }
            fprintf(trace, "%-4d | %-4s | %-55.55s | %-40.40s  %s\n",
                    ++step, "US", sbuf, ibuf, prodStr);

            if (P->rhsLen == 1 && P->rhs[0] == SYM_EPSILON) {
                if (parent) addKid(parent, newNode(SYM_EPSILON, NULL));
            } else {
                TreeNode *kids[12];
                for (int i=0;i<P->rhsLen;i++) {
                    kids[i] = newNode(P->rhs[i], NULL);
                    if (parent) addKid(parent, kids[i]);
                }
                for (int i=P->rhsLen-1;i>=0;i--) push(P->rhs[i], kids[i]);
            }
        }
        else if (followSet[X][a] || a == T_DOLLAR) {
            /* P — lookahead is in FOLLOW(X): sync by popping X */
            fprintf(trace, "%-4d | %-4s | %-55.55s | %-40.40s  no rule (%s, %s); sync: pop %s\n",
                    ++step, "POP", sbuf, ibuf, symName(X), symName(a), symName(X));
            fprintf(err, "Syntax error: unexpected '%s' (lexeme \"%s\") while parsing '%s' — synchronised by popping\n",
                    symName(a), tokens[pos].lexeme, symName(X));
            hadError = 1;
            popS();
        }
        else {
            /* S — skip the offending input token */
            fprintf(trace, "%-4d | %-4s | %-55.55s | %-40.40s  no rule (%s, %s); scan input\n",
                    ++step, "SCAN", sbuf, ibuf, symName(X), symName(a));
            fprintf(err, "Syntax error: unexpected '%s' (lexeme \"%s\") while parsing '%s' — scanned input\n",
                    symName(a), tokens[pos].lexeme, symName(X));
            hadError = 1;
            if (a == T_DOLLAR) { popS(); }
            else               { pos++; }
        }
    }
    *rootOut = root;
    return hadError;
}

/* ---------- main -------------------------------------------- */
int main(int argc, char *argv[]) {
    initParseTable();
    initFollowSet();
    
    const char *tokPath = (argc > 1) ? argv[1] : "Lexer/token.txt";
    FILE *err   = fopen("Parser/parse_error.txt", "w");
    FILE *trace = fopen("Parser/parse_trace.txt", "w");
    FILE *tree  = fopen("Parser/parse_tree.txt",  "w");
    if (!err || !trace || !tree) { fprintf(stderr,"output file open failed\n"); return 1; }

    if (!readTokens(tokPath, err)) {
        fprintf(stderr, "cannot open %s\n", tokPath);
        return 1;
    }

    TreeNode *root = NULL;
    int rc = parse(trace, err, &root);
    if (root) printTree(root, 0, tree);

    fclose(err); fclose(trace); fclose(tree);
    printf("Parser done. Check parse_trace.txt, parse_tree.txt, parse_error.txt.  %s\n",
           rc == 0 ? "Parse OK." : "Parse finished with errors.");
    return rc;
}

/* ============================================================
   FIRST / FOLLOW (computed by hand — used to fill parseTable)
   ------------------------------------------------------------
   FIRST(Type)       = { Adadi, Ashriya, Harf, Matn, Mantiqi }
   FIRST(Factor)     = { (, id, number }
   FIRST(Term/Mag/Rvalue/Expr) = FIRST(Factor)
   FIRST(Compare)    = { ==, <, >, <=, >=, !=, <> }
   FIRST(Expr')      = { :=, e }
   FIRST(Rvalue')    = FIRST(Compare) U { e }
   FIRST(Mag')       = { +, -, e }
   FIRST(Term')      = { *, /, e }
   FIRST(IdentList)  = { id }
   FIRST(IdentList') = { ,, e }
   FIRST(ArgList)    = FIRST(Type) U { e }
   FIRST(ArgList')   = { ,, e }
   FIRST(Declaration)= FIRST(Type)
   FIRST(Stmt)       = { for, while, Agar, {, FIRST(Type), (, id, number, :: }
   FIRST(StmtList)   = FIRST(Stmt) U { e }
   FIRST(OptExpr)    = FIRST(Expr) U { e }
   FIRST(ElsePart)   = { Wagarna, e }

   FOLLOW(Function)  = { $ }
   FOLLOW(ArgList)   = { ) }
   FOLLOW(ArgList')  = { ) }
   FOLLOW(Arg)       = { , ) }
   FOLLOW(IdentList) = { :: }
   FOLLOW(IdentList')= { :: }
   FOLLOW(Type)      = { id }
   FOLLOW(Declaration)= FOLLOW(Stmt)
   FOLLOW(Stmt)      = FIRST(Stmt) U { }, Wagarna, $ }
                     = { for,while,Agar,{,},type-kws,(,id,num,::,Wagarna,$ }
                       (also ) from FOLLOW(OptExpr))
   FOLLOW(ForStmt)   ⊆ FOLLOW(Stmt)
   FOLLOW(WhileStmt) ⊆ FOLLOW(Stmt)
   FOLLOW(IfStmt)    ⊆ FOLLOW(Stmt)
   FOLLOW(CompStmt)  ⊆ FOLLOW(Stmt)
   FOLLOW(ElsePart)  = FOLLOW(IfStmt)
   FOLLOW(StmtList)  = { } }
   FOLLOW(OptExpr)   = { ::, ) }
   FOLLOW(Expr)      = { ::, ) }
   FOLLOW(Expr')     = FOLLOW(Expr)
   FOLLOW(Rvalue)    = { :=, ::, ) }
   FOLLOW(Rvalue')   = FOLLOW(Rvalue)
   FOLLOW(Compare)   = FIRST(Mag) = { (, id, number }
   FOLLOW(Mag)       = FIRST(Rvalue') \ {e} U FOLLOW(Rvalue)
                     = { compare-ops, :=, ::, ) }
   FOLLOW(Mag')      = FOLLOW(Mag)
   FOLLOW(Term)      = { +, -, compare-ops, :=, ::, ) }
   FOLLOW(Term')     = FOLLOW(Term)
   FOLLOW(Factor)    = { *, /, +, -, compare-ops, :=, ::, ) }
   ============================================================ */
