/*
* File Name: scanner.c
* Compiler: gcc
* Version: 6.65
* Author: Justin De Vries, 040 495 834, Akash Bakshi 040 866 704
* Course: CST 8152 - Compilers, Lab Section 012
* Assignment: 2
* Date: 8 November 2018
* Professor: Sv. Ranev
* scanner_init(), malar_next_token(), get_next_state(), char_class(), aa_func02(), aa_func03(),
* aa_func05(), aa_func08(), aa_func10(), aa_func11(), aa_func12()
*/

/* The #define _CRT_SECURE_NO_WARNINGS should be used in MS Visual Studio projects
* to suppress the warnings about using "unsafe" functions like fopen()
* and standard sting library functions defined in string.h.
* The define does not have any effect in Borland compiler projects.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>  /* standard input / output */
#include <ctype.h>  /* conversion functions */
#include <stdlib.h> /* standard library functions and constants */
#include <string.h> /* string functions */
#include <limits.h> /* integer types constants */
#include <float.h>  /* floating-point types constants */

/*#define NDEBUG        to suppress assert() call */
#include <assert.h> /* assert() prototype */

/* project header files */
#include "buffer.h"
#include "token.h"
#include "table.h"

#define DEBUG /* for conditional processing */
#undef DEBUG

/* Global objects - variables */
/* This buffer is used as a repository for string literals.
It is defined in platy_st.c */
extern Buffer *str_LTBL; /*String literal table */
int line;                /* current line number of the source code */
extern int scerrnum;     /* defined in platy_st.c - run-time error number */

						 /* Local(file) global objects - variables */
static Buffer *lex_buf; /*pointer to temporary lexeme buffer*/
static pBuffer sc_buf;  /*pointer to input source buffer*/
						/* No other global variable declarations/definitiond are allowed */

						/* scanner.c static(local) function  prototypes */
static int char_class(char c);               /* character class function */
static int get_next_state(int, char, int *); /* state machine function */
static int iskeyword(char *kw_lexeme);       /*keywords lookup functuion */

											 /*Initializes scanner */
int scanner_init(Buffer *psc_buf)
{
	if (b_isempty(psc_buf))
		return EXIT_FAILURE; /*1*/
							 /* in case the buffer has been read previously  */
	b_rewind(psc_buf);
	b_clear(str_LTBL);
	line = 1;
	sc_buf = psc_buf;
	return EXIT_SUCCESS;   /*0*/
						   /*   scerrnum = 0;  */ /*no need - global ANSI C */
}
/*
* Purpose: Read source file and generate token from input
* Author: Justin De Vries - 040 495 834, Akash Bakshi - 040 866 704
* History/Versions: 5.3
* Called functions: b_getc(), isspace(), b_retract(), b_mark(), b_getcoffset(), get_next_state(), b_allocate(), b_reset(), printf()
* Parameters:
*   void
* Algorithm:
*   Continually checks buffer for input and creates tokens based on what it finds
*/
Token malar_next_token(void)
{
	Token t = { 0 };     /* token to return after pattern recognition. Set all structure members to 0 */
	unsigned char c;   /* input symbol */
	int state = 0;     /* initial state of the FSM */
	short lexstart;    /*start offset of a lexeme in the input char buffer (array) */
	short lexend;      /*end   offset of a lexeme in the input char buffer (array)*/
	int accept = NOAS; /* type of state - initially not accepting */

	while (1)
	{
		c = b_getc(sc_buf);

		if (c == SEOF || c == '\0') {  /* check for Source end of file, set token, and return */
			t.code = SEOF_T;
			return t;
		}

		if (isspace(c)) {  /* Check for new lines and increment counter (for debugging) */
			if (c == '\n')
				line++;
			continue;
		}

		switch (c) { /* check for specific characters and set proper token */
		case ',':
			t.code = COM_T;
			return t;
		case '(':
			t.code = LPR_T;
			return t;
		case ')':
			t.code = RPR_T;
			return t;
		case '{':
			t.code = LBR_T;
			return t;
		case '}':
			t.code = RBR_T;
			return t;
		case '+':
			t.code = ART_OP_T;
			t.attribute.arr_op = PLUS;
			return t;
		case '-':
			t.code = ART_OP_T;
			t.attribute.arr_op = MINUS;
			return t;
		case '/':
			t.code = ART_OP_T;
			t.attribute.arr_op = DIV;
			return t;
		case '*':
			t.code = ART_OP_T;
			t.attribute.arr_op = MULT;
			return t;
		case '#':
			t.code = SCC_OP_T;
			return t;
		case '<':
			c = b_getc(sc_buf);

			if (c != '>') { /* set relationship operator if it's by itself */
				b_retract(sc_buf);
				t.code = REL_OP_T;
				t.attribute.rel_op = LT;
			}
			else {
				t.code = REL_OP_T;
				t.attribute.rel_op = NE;
			}
			return t;
		case '>':
			t.code = REL_OP_T;
			t.attribute.rel_op = GT;
			return t;
		case '=':
			c = b_getc(sc_buf);

			if (c != '=') { /* if there aren't two '=' signs then it's an assignment operator, otherwise its equals (==) */
				b_retract(sc_buf);
				t.code = ASS_OP_T;
			}
			else {
				t.code = REL_OP_T;
				t.attribute.rel_op = EQ;
			}
			return t;
		case '.':

			b_mark(sc_buf, b_getcoffset(sc_buf));

			if (b_getc(sc_buf) == 'A' && b_getc(sc_buf) == 'N' && b_getc(sc_buf) == 'D' && b_getc(sc_buf) == '.') { /* check for AND logical operator */
				t.code = LOG_OP_T;
				t.attribute.log_op = AND;
				return t;
			}
			else {
				b_reset(sc_buf);
			}
			if (b_getc(sc_buf) == 'O' && b_getc(sc_buf) == 'R' && b_getc(sc_buf) == '.') { /* check for OR logical operator */
				t.code = LOG_OP_T;
				t.attribute.log_op = OR;
				return t;
			}
			else {
				b_reset(sc_buf);
#ifdef DEBUG
				printf("\t***Not AND or OR");
#endif
				t.attribute.err_lex[0] = '.';
				t.attribute.err_lex[1] = '\0';
				t.code = ERR_T;
				b_reset(sc_buf);
				return t;
			}


		case '!':
			c = b_getc(sc_buf);

			if (c != '!') { /* if its not two '!!' then it's not a comment*/
#ifdef DEBUG
				printf("\t***Not a comment");
#endif
				t.attribute.err_lex[0] = '!';
				t.attribute.err_lex[1] = c;
				t.attribute.err_lex[2] = '\0';
				t.code = ERR_T;
				while (c != '\n' && c != '\0' && c != SEOF) {
					c = b_getc(sc_buf);
				}
				return t;
			}

			while (c != '\n') { /* it's a comment as long as it's on the same line */
				c = b_getc(sc_buf);

				if (c == '\0' || c == SEOF) {
					t.code = SEOF_T;
					return t;
				}
			}
			line++;
			continue;
		case ';':
			t.code = EOS_T;
			return t;
		default: /* if no cases met then read c, assign token as needed, likely a VID */
#ifdef DEBUG
			printf(" Alpha Numeric character: %c ", c);
#endif
			lexstart = b_mark(sc_buf, b_getcoffset(sc_buf) - 1);
			state = get_next_state(state, c, &accept);

			while (accept == NOAS) {
				c = b_getc(sc_buf);
				state = get_next_state(state, c, &accept);
			}

			if (accept == ASWR)
				b_retract(sc_buf);


			lexend = b_getcoffset(sc_buf);
			lex_buf = b_allocate(lexend - lexstart, 0, 'f');

			if (!lex_buf) {
#ifdef DEBUG
				printf("\t***no buffer (Line: 169)");
#endif                
				t.code = ERR_T;
				scerrnum = 1;
				strcpy(t.attribute.err_lex, "Error: ");
				return t;
			}

			b_reset(sc_buf);

			while (lexend > b_getcoffset(sc_buf)) {
				c = b_getc(sc_buf);
				b_addc(lex_buf, c);
			}

			b_compact(lex_buf, '\0');

			t = aa_table[state](b_location(lex_buf, 0));
			b_free(lex_buf);
			return t;

		}
	}
}
int get_next_state(int state, char c, int *accept)
{
	int col; /* Hold the column number*/
	int next; /* hold the next state*/
	col = char_class(c);
#ifdef DEBUG
	printf(" Column %d  EOFtate: %d", col, state);
#endif
	next = st_table[state][col];
#ifdef DEBUG
	printf("Input symbol: %c Row: %d Column: %d Next: %d \n", c, state, col, next);
#endif

	assert(next != IS);

#ifdef DEBUG
	if (next == IS)
	{
		printf("Scanner Error: Illegal state:\n");
		printf("Input symbol: %c Row: %d Column: %d\n", c, state, col);
		exit(1);
	}
#endif
	*accept = as_table[next];
	return next;
}

/*
* Purpose: This function will go through the char and determine which column to read
* Author: Justin De Vries
* Version: 1.0
* Called function:
* Parameters : char c
* Return value: int
*/
int char_class(char c)
{
	int val; /* hold the value based on condition to store column num*/

	if (isalpha(c)) {
#ifdef DEBUG
		printf(" Returning 0 ");
#endif
		val = 0; /* if alpha numeric*/
	}
	else if (isdigit(c) && c != '0')
	{
		val = 2; /* if it's digit*/

	}
	else if (c == '0')
		val = 1; /*if 0 go to column 1*/
	else if (c == '.')
		val = 3; /*if dot go to column 3*/
	else if (c == '$')
		val = 4; /*if $ sign go to column 4*/
	else if (c == '"') {
#ifdef DEBUG
		printf("** String Literal Char Class ** \n");
#endif
		val = 6; /* if we have a quote go to string literal column 6*/
	}
	else if (c == SEOF || c == '\0')
		val = 7; /* if seof or end of string go to seof column*/
	else
		val = 5; /* if not any of the above go to other column */


	return val;
}

/*
* Purpose: This function will check for keyboard and AVID
* Author: Justin De Vries
* Version: 1.0
* Called function:
* Parameters : char *lexeme
* Return value: Token
*/

Token aa_func02(char *lexeme) {
	Token t;
	int check = iskeyword(lexeme);
	short i;
#ifdef DEBUG
	printf("called aa_func02 (AVID))", check);
#endif
	
	if (check == -1) /* if isn't keyword AVID*/
		t.code = AVID_T;
	else { /*if is keyword set code and store value*/
		t.code = KW_T;
		t.attribute.kwt_idx = check;
		return t;
	}

	if (strlen(lexeme) < VID_LEN) /* if lexeme is less than VID_LEN proceed to store*/
		strcpy(t.attribute.vid_lex, lexeme);
	else {/* if it's longer than store and add '\0' */
		t.attribute.vid_lex[VID_LEN] = '\0';
		for (i = 0; i < VID_LEN; i++)
			t.attribute.vid_lex[i] = lexeme[i];
	}


	return t;
}


/*
* Purpose: This function will handle variable string identifier
* Author: Justin De Vries
* Version: 1.0
* Called function:
* Parameters : char *lexeme
* Return value: Token
*/
Token aa_func03(char *lexeme)
{
	Token t;
	int i, size = strlen(lexeme);

	t.code = SVID_T;

	if (size < VID_LEN) {
		strncpy(t.attribute.vid_lex, lexeme, size); /* if size is less than max store*/
		t.attribute.err_lex[size] = '\0';
	}
	else {
		strncpy(t.attribute.vid_lex, lexeme, (VID_LEN - 1)); /* copy everything but last*/

		for (i = 0; i < VID_LEN; i++) {
			t.attribute.vid_lex[i] = lexeme[i];
		}
		t.attribute.vid_lex[VID_LEN - 1] = '$';
		t.attribute.vid_lex[VID_LEN] = '\0'; /* add null terminating */
	}
	return t;


}

/*
* Purpose: This function will handle floats
* Author: Akash  Bakshi
* Version: 1.0
* Called function:
* Parameters : char *lexeme
* Return value: Token
*/
Token aa_func08(char* lexeme)
{
	Token t;

	double num = atof(lexeme);

	if (((num >= 0 && strlen(lexeme) > 7) && (num < FLT_MIN || num > FLT_MAX)) || (num < 0)) { /* if isn't in range call error*/
		t = aa_table[11](lexeme);
	}
	else {
		t.code = FPL_T; /*if in range store and mark code*/
		t.attribute.flt_value = (float)num;
	}


	return t;


}

/*
* Purpose: This function will handle integer literal(IL)
* Author: Akash Bakshi
* Version: 1.0
* Called function:
* Parameters : char *lexeme
* Return value: Token
*/
Token aa_func05(char* lexeme)
{
	Token t;
	long num = atol(lexeme);

	if (num > SHRT_MAX || num < SHRT_MIN) {
		t = aa_table[11](lexeme); /* if out of range call error*/
	}
	else {
		t.code = INL_T;
		t.attribute.int_value = num; /* if not set code and value*/
	}


	return t;
}
/*
* Purpose: This function will handle String literal
* Author: Akash Bakshi
* Version: 1.0
* Called function:
* Parameters : char *lexeme
* Return value: Token
*/

Token aa_func10(char* lexeme)
{
	Token t;
	unsigned int i;

	t.attribute.str_offset = b_limit(str_LTBL); /*set offset of addc*/
	t.code = STR_T; /* set code*/

	for (i = 0; i < strlen(lexeme); i++) { /* loop through length*/
		if(lexeme[i] == '"') /* if we get a " continue*/
			continue;
		if (lexeme[i] == '\n') /* if new line increment*/
			line++;
		b_addc(str_LTBL, lexeme[i]); /* add to str_LTBL*/
	}
	b_addc(str_LTBL, '\0'); /*add null terminating*/

	return t;
}

/*
* Purpose: This function will handle errors without retract
* Author: Justin Di Vries
* Version: 1.0
* Called function:
* Parameters : char *lexeme
* Return value: Token
*/
Token aa_func11(char *lexeme) {
	Token t;

	int len = strlen(lexeme);

	t.code = ERR_T;

	if (len < ERR_LEN) { /* if less than length then simply add to err_lex*/
		strncpy(t.attribute.err_lex, lexeme, len);
		t.attribute.err_lex[len] = '\0';
	}
	else { /* else add everything but last 3 and add dots to it as requested*/
		strncpy(t.attribute.err_lex, lexeme, ERR_LEN - 3);
		t.attribute.err_lex[ERR_LEN - 3] = '.';
		t.attribute.err_lex[ERR_LEN - 2] = '.';
		t.attribute.err_lex[ERR_LEN - 1] = '.';
		t.attribute.err_lex[ERR_LEN] = '\0';
	}

	return t;
}


/*
* Purpose: This function will handle errors with retract
* Author: Justin Di Vries
* Version: 1.0
* Called function:
* Parameters : char *lexeme
* Return value: Token
*/
Token aa_func12(char *lexeme)
{
	Token t;
	int len = strlen(lexeme);

	t.code = ERR_T;

	if (len < ERR_LEN) { /*if less than max add to err_lex*/
		strncpy(t.attribute.err_lex, lexeme, len);
		t.attribute.err_lex[len] = '\0';
	}
	else { /* if greater add everything but leave last 3 characters as a ...*/
		strncpy(t.attribute.err_lex, lexeme, ERR_LEN - 3);
		t.attribute.err_lex[ERR_LEN - 3] = '.';
		t.attribute.err_lex[ERR_LEN - 2] = '.';
		t.attribute.err_lex[ERR_LEN - 1] = '.';
		t.attribute.err_lex[ERR_LEN] = '\0';
	}
	b_retract(sc_buf);/*retract */

	return t;
}

/*
* Purpose: This function checks for keywords
* Author: Justin Di Vries
* Version: 1.0
* Called function:
* Parameters : char *kw_lexeme
* Return value: int
*/

int iskeyword(char *kw_lexeme)
{
	int i;

	if (kw_lexeme == NULL)
		return -1;

	for (i = 0; i < KWT_SIZE; i++)
	{
		if (strcmp(kw_table[i], kw_lexeme) == 0)
		{
			return i;
		}
	}
	return -1;
}