#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/* =========================================================
   KEYWORDS  (all lowercase for case-insensitive matching)
   ========================================================= */
const char *KEYWORDS[62] = {
    "asm",      "wagarna",   "new",       "this",
    "auto",     "enum",      "operator",  "throw",
    "mantiqi",  "explicit",  "private",   "true",
    "break",    "export",    "protected", "try",
    "case",     "extern",    "public",    "typedef",
    "catch",    "false",     "register",  "typeid",
    "harf",     "ashriya",   "typename",  "adadi",
    "class",    "for",       "wapas",     "union",
    "const",    "dost",      "short",     "unsigned",
    "goto",     "signed",    "using",     "continue",
    "agar",     "sizeof",    "virtual",   "default",
    "inline",   "static",    "khali",     "delete",
    "volatile", "do",        "long",      "struct",
    "double",   "mutable",   "switch",    "while",
    "namespace","template",  "marqazi",   "matn",
    "input->",  "output<-"
};
#define NUM_KEYWORDS 62

/* =========================================================
   STATES
   ========================================================= */
enum States {
    S,                  /* Start */

    /* --- Identifier / Keyword paths --- */
    A1,                 /* Started with '_'  → will be F_ID */
    A2,                 /* Started with letter, NO '_' yet → could be keyword */
    A3,                 /* Letter-start, '_' seen → will be F_ID */

    /* --- Number states --- */
    B1,                 /* Integer digits */
    B2,                 /* Leading sign (+/-) */
    B3,                 /* Dot after integer */
    B4,                 /* Fractional digits */
    B5,                 /* E after int or frac */
    B6,                 /* Sign after E */
    B7,                 /* Exponent digits */

    /* --- String states --- */
    ST_STR,             /* saw opening " — collecting string body */
    ST_STR_ANY,         /* inside string, consuming chars */

    /* --- Operator lookahead states --- */
    OP_LT,              /* saw '<' */
    OP_GT,              /* saw '>' */
    OP_COL,             /* saw ':' */
    OP_EQ,              /* saw '=' */
    OP_PLS,             /* saw '+' */
    OP_MNS,             /* saw '-' */
    OP_PIP,             /* saw '|' */
    OP_AMP,             /* saw '&' */
    OP_EXC,             /* saw '!' */

    /* -------------------------------------------------------
       ACCEPT STATES  (everything >= F_KW is an accept state)
       ------------------------------------------------------- */
    F_KW,               /* Keyword  */
    F_KW_CANDIDATE,     /* Letter-only word: check keyword table; error if not found */
    F_ID,               /* Identifier (underscore confirmed) */
    F_NUM,              /* Number (int / float / exp) */
    F_STR,              /* String literal */

    /* Operators */
    F_LT,               /* <   */
    F_LTLT,             /* <<  */
    F_NEQ,              /* <>  */
    F_LEQ,              /* <=  */
    F_GT,               /* >   */
    F_GTGT,             /* >>  */
    F_GEQ,              /* >=  */
    F_ASGN,             /* =   */
    F_EQ,               /* ==  */
    F_CEQ,              /* :=  */
    F_MOD,              /* %   */
    F_ADD,              /* +   */
    F_INC,              /* ++  */
    F_SUB,              /* -   */
    F_DEC,              /* --  */
    F_OR,               /* ||  */
    F_AND,              /* &&  */
    F_MUL,              /* *   */
    F_DIV,              /* /   */
    F_BNE,              /* !=  */
    F_ADE,              /* +=  */
    F_SBE,              /* -=  */

    /* Punctuation */
    F_LPR,              /* (   */
    F_RPR,              /* )   */
    F_LSB,              /* [   */
    F_RSB,              /* ]   */
    F_LCB,              /* {   */
    F_RCB,              /* }   */
    F_COM,              /* ,   */
    F_COL,              /* ::  */

    STATE_COUNT
};

/* =========================================================
   INPUTS
   ========================================================= */
enum Inputs {
    /* Character classes that get consumed into the lexeme */
    IN_LTR,     /* a-z, A-Z  (excluding E/e, handled separately) */
    IN_DIG,     /* 0-9 */
    IN_DOT,     /* .  */
    IN_EXP,     /* E or e  (can be letter in identifiers too) */
    IN_UND,     /* _  */

    IN_LSB,     /* [  */
    IN_RSB,     /* ]  */
    IN_LCB,     /* {  */
    IN_RCB,     /* }  */
    IN_LPR,     /* (  */
    IN_RPR,     /* )  */
    IN_COL,     /* :  */
    IN_COM,     /* ,  */

    IN_LT,      /* <  */
    IN_GT,      /* >  */
    IN_EQ,      /* =  */
    IN_PLS,     /* +  */
    IN_MNS,     /* -  */
    IN_MOD,     /* %  */
    IN_PIP,     /* |  */
    IN_AMP,     /* &  */
    IN_EXC,     /* !  */
    IN_MUL,     /* *  */
    IN_DIV,     /* /  */

    IN_QUOTE,   /* "  — starts/ends a string literal */

    /* ---- delimiter: whitespace. advance() returns false for this ---- */
    IN_DELIM,   /* space / tab / newline — terminates a token, not consumed */

    INPUT_COUNT
};

/* =========================================================
   TRANSITION TABLE
   ========================================================= */
int TT[STATE_COUNT][INPUT_COUNT];

void initTT(void)
{
    for (int i = 0; i < STATE_COUNT; i++)
        for (int j = 0; j < INPUT_COUNT; j++)
            TT[i][j] = -1;

    /* ---------------------------------------------------------
       IDENTIFIERS & KEYWORDS
    --------------------------------------------------------- */
    TT[S][IN_UND]   = A1;
    TT[A1][IN_LTR]  = A1;
    TT[A1][IN_EXP]  = A1;
    TT[A1][IN_DIG]  = A1;
    TT[A1][IN_UND]  = A1;
    TT[A1][IN_DELIM]= F_ID;

    TT[S][IN_LTR]   = A2;
    TT[S][IN_EXP]   = A2;
    TT[A2][IN_LTR]  = A2;
    TT[A2][IN_EXP]  = A2;
    TT[A2][IN_DIG]  = A2;
    TT[A2][IN_UND]  = A3;
    TT[A2][IN_DELIM]= F_KW_CANDIDATE;

    TT[A3][IN_LTR]  = A3;
    TT[A3][IN_EXP]  = A3;
    TT[A3][IN_DIG]  = A3;
    TT[A3][IN_UND]  = A3;
    TT[A3][IN_DELIM]= F_ID;

    /* ---------------------------------------------------------
       NUMBERS
    --------------------------------------------------------- */
    TT[S][IN_DIG]   = B1;
    TT[B1][IN_DIG]  = B1;
    TT[B1][IN_DELIM]= F_NUM;

    TT[B2][IN_DIG]  = B1;

    TT[B1][IN_DOT]  = B3;
    TT[B3][IN_DIG]  = B4;
    TT[B4][IN_DIG]  = B4;
    TT[B4][IN_DELIM]= F_NUM;

    TT[B1][IN_EXP]  = B5;
    TT[B4][IN_EXP]  = B5;
    TT[B5][IN_PLS]  = B6;
    TT[B5][IN_MNS]  = B6;
    TT[B6][IN_DIG]  = B7;
    TT[B7][IN_DIG]  = B7;
    TT[B7][IN_DELIM]= F_NUM;

    /* ---------------------------------------------------------
       STRING LITERALS   "..."
    --------------------------------------------------------- */
    TT[S][IN_QUOTE] = ST_STR;   /* opening " */

    /* every input type is consumed inside a string */
    int strInputs[] = {
        IN_LTR, IN_DIG, IN_DOT, IN_EXP, IN_UND,
        IN_LSB, IN_RSB, IN_LCB, IN_RCB, IN_LPR, IN_RPR,
        IN_COL, IN_COM, IN_LT,  IN_GT,  IN_EQ,  IN_PLS,
        IN_MNS, IN_MOD, IN_PIP, IN_AMP, IN_EXC, IN_MUL,
        IN_DIV, IN_DELIM
    };
    int nStr = (int)(sizeof(strInputs) / sizeof(strInputs[0]));
    for (int i = 0; i < nStr; i++) {
        TT[ST_STR][strInputs[i]]     = ST_STR_ANY;
        TT[ST_STR_ANY][strInputs[i]] = ST_STR_ANY;
    }
    /* closing " → accept */
    TT[ST_STR][IN_QUOTE]     = F_STR;
    TT[ST_STR_ANY][IN_QUOTE] = F_STR;

    /* ---------------------------------------------------------
       PUNCTUATION
    --------------------------------------------------------- */
    TT[S][IN_LSB]   = F_LSB;
    TT[S][IN_RSB]   = F_RSB;
    TT[S][IN_LCB]   = F_LCB;
    TT[S][IN_RCB]   = F_RCB;
    TT[S][IN_LPR]   = F_LPR;
    TT[S][IN_RPR]   = F_RPR;
    TT[S][IN_COM]   = F_COM;
    TT[S][IN_MOD]   = F_MOD;
    TT[S][IN_MUL]   = F_MUL;
    TT[S][IN_DIV]   = F_DIV;

    /* ---------------------------------------------------------
       OPERATORS
    --------------------------------------------------------- */
    TT[S][IN_LT]        = OP_LT;
    TT[OP_LT][IN_LT]    = F_LTLT;
    TT[OP_LT][IN_GT]    = F_NEQ;
    TT[OP_LT][IN_EQ]    = F_LEQ;
    TT[OP_LT][IN_DELIM] = F_LT;

    TT[S][IN_GT]        = OP_GT;
    TT[OP_GT][IN_GT]    = F_GTGT;
    TT[OP_GT][IN_EQ]    = F_GEQ;
    TT[OP_GT][IN_DELIM] = F_GT;

    TT[S][IN_EQ]        = OP_EQ;
    TT[OP_EQ][IN_EQ]    = F_EQ;
    TT[OP_EQ][IN_DELIM] = F_ASGN;

    TT[S][IN_COL]       = OP_COL;
    TT[OP_COL][IN_EQ]   = F_CEQ;
    TT[OP_COL][IN_COL]  = F_COL;

    TT[S][IN_PLS]       = OP_PLS;
    TT[OP_PLS][IN_PLS]  = F_INC;
    TT[OP_PLS][IN_EQ]   = F_ADE;
    TT[OP_PLS][IN_DELIM]= F_ADD;
    TT[OP_PLS][IN_DIG]  = B1;

    TT[S][IN_MNS]       = OP_MNS;
    TT[OP_MNS][IN_MNS]  = F_DEC;
    TT[OP_MNS][IN_EQ]   = F_SBE;
    TT[OP_MNS][IN_DELIM]= F_SUB;
    TT[OP_MNS][IN_DIG]  = B1;

    TT[S][IN_PIP]       = OP_PIP;
    TT[OP_PIP][IN_PIP]  = F_OR;

    TT[S][IN_AMP]       = OP_AMP;
    TT[OP_AMP][IN_AMP]  = F_AND;

    TT[S][IN_EXC]       = OP_EXC;
    TT[OP_EXC][IN_EQ]   = F_BNE;
    TT[OP_EXC][IN_DELIM]= -1;
}

/* =========================================================
   charToInput
   ========================================================= */
int charToInput(char c)
{
    if (c == 'E' || c == 'e') return IN_EXP;
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return IN_LTR;
    if ( c >= '0' && c <= '9')                             return IN_DIG;

    switch (c) {
        case '_':  return IN_UND;
        case '.':  return IN_DOT;
        case '[':  return IN_LSB;
        case ']':  return IN_RSB;
        case '{':  return IN_LCB;
        case '}':  return IN_RCB;
        case '(':  return IN_LPR;
        case ')':  return IN_RPR;
        case ':':  return IN_COL;
        case ',':  return IN_COM;
        case '<':  return IN_LT;
        case '>':  return IN_GT;
        case '=':  return IN_EQ;
        case '+':  return IN_PLS;
        case '-':  return IN_MNS;
        case '%':  return IN_MOD;
        case '|':  return IN_PIP;
        case '&':  return IN_AMP;
        case '!':  return IN_EXC;
        case '*':  return IN_MUL;
        case '/':  return IN_DIV;
        case '"':  return IN_QUOTE;
        case ' ':
        case '\t':
        case '\n':
        case '\r': return IN_DELIM;
        default:   return -1;
    }
}

/* =========================================================
   accept / advance helpers
   ========================================================= */
bool isAccept(int state)
{
    return (state >= F_KW && state < STATE_COUNT);
}

/* Returns true if this character should be consumed into the lexeme.
   IN_DELIM is normally a lookahead-only delimiter — NOT consumed.
   Exception: inside a string literal, spaces ARE consumed. */
bool advance(int inputCol, int currentState)
{
    bool insideString = (currentState == ST_STR || currentState == ST_STR_ANY);
    if (insideString && inputCol == IN_DELIM) return true;
    return (inputCol != IN_DELIM && inputCol >= 0);
}

/* =========================================================
   Token class name
   ========================================================= */
const char *className(int state)
{
    switch (state) {
        case F_KW:   return "KEYWORD";
        case F_ID:   return "IDENTIFIER";
        case F_NUM:  return "NUMBER";
        case F_STR:  return "STRING";
        case F_LT:   return "OP_LT";
        case F_LTLT: return "OP_LTLT";
        case F_NEQ:  return "OP_NEQ";
        case F_LEQ:  return "OP_LEQ";
        case F_GT:   return "OP_GT";
        case F_GTGT: return "OP_GTGT";
        case F_GEQ:  return "OP_GEQ";
        case F_ASGN: return "OP_ASSIGN";
        case F_EQ:   return "OP_EQ";
        case F_CEQ:  return "OP_CEQ";
        case F_MOD:  return "OP_MOD";
        case F_ADD:  return "OP_ADD";
        case F_INC:  return "OP_INC";
        case F_SUB:  return "OP_SUB";
        case F_DEC:  return "OP_DEC";
        case F_OR:   return "OP_OR";
        case F_AND:  return "OP_AND";
        case F_MUL:  return "OP_MUL";
        case F_DIV:  return "OP_DIV";
        case F_BNE:  return "OP_BNE";
        case F_ADE:  return "OP_ADE";
        case F_SBE:  return "OP_SBE";
        case F_LPR:  return "PUNCT_LPAREN";
        case F_RPR:  return "PUNCT_RPAREN";
        case F_LSB:  return "PUNCT_LBRACKET";
        case F_RSB:  return "PUNCT_RBRACKET";
        case F_LCB:  return "PUNCT_LBRACE";
        case F_RCB:  return "PUNCT_RBRACE";
        case F_COM:  return "PUNCT_COMMA";
        case F_COL:  return "PUNCT_DCOLON";
        default:     return "UNKNOWN";
    }
}

/* =========================================================
   isKeyword  (case-insensitive)
   ========================================================= */
bool isKeyword(const char *lexeme)
{
    char lower[256];
    int i = 0;
    while (lexeme[i] && i < 255) {
        lower[i] = (char)tolower((unsigned char)lexeme[i]);
        i++;
    }
    lower[i] = '\0';

    for (int k = 0; k < NUM_KEYWORDS; k++) {
        if (strcmp(lower, KEYWORDS[k]) == 0)
            return true;
    }
    return false;
}

/* =========================================================
   MAIN
   ========================================================= */
int main(int argc, char *argv[])
{
    initTT();

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    FILE *sourceFile = fopen(argv[1], "r");
    FILE *tokenFile  = fopen("Lexer/token.txt",  "w");
    FILE *errorFile  = fopen("Lexer/error.txt",  "w");

    if (!sourceFile || !tokenFile || !errorFile) {
        fprintf(stderr, "Error opening files.\n");
        return 1;
    }

    int  ch = fgetc(sourceFile);
    char lexeme[256];

    while (ch != EOF) {

        /* Skip whitespace between tokens */
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            ch = fgetc(sourceFile);
            continue;
        }

        int currentState = S;
        int lexPtr = 0;
        memset(lexeme, 0, sizeof(lexeme));

        while (!isAccept(currentState) && currentState != -1) {

            char c        = (char)ch;
            int  inputCol = charToInput(c);

            if (inputCol == -1) {
                if (lexPtr < 255) lexeme[lexPtr++] = c;
                currentState = -1;
                ch = fgetc(sourceFile);
                break;
            }

            int newState = TT[currentState][inputCol];

            if (advance(inputCol, currentState)) {
                if (lexPtr < 255) lexeme[lexPtr++] = c;
                ch = fgetc(sourceFile);
                if (ch == EOF) {
                    currentState = newState;
                    break;
                }
            }

            currentState = newState;
        }

        if (isAccept(currentState)) {

            if (currentState == F_KW_CANDIDATE) {
                if (isKeyword(lexeme)) {
                    currentState = F_KW;
                    fprintf(tokenFile, "<%s,KEYWORD>\n", lexeme);
                } else {
                    fprintf(errorFile,
                            "Lexical Error: '%s' is not a keyword and has no underscore"
                            " (invalid identifier)\n", lexeme);
                }
            } else {
                fprintf(tokenFile, "<%s,%s>\n",
                        lexeme, className(currentState));
            }

        } else {
            fprintf(errorFile, "Lexical Error: '%s' (dead state)\n", lexeme);
            if (lexPtr == 0) ch = fgetc(sourceFile);
        }
    }

    fclose(sourceFile);
    fclose(tokenFile);
    fclose(errorFile);

    printf("Done. Check token.txt and error.txt.\n");
    return 0;
}