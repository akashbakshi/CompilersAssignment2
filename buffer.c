/*
* File Name: buffer.c
* Compiler: gcc
* Version: 3.6
* Author: Justin De Vries, 040 495 834
* Course: CST 8152 - Compilers, Lab Section 012
* Assignment: 1
* Date: 2 October 2018
* Professor: Sv. Ranev
* b_allocate(), b_addc(), b_clear(), b_free(), b_isfull(), b_limit(), b_capacity(), b_mark()
* b_mode(), b_incfactor(), b_load(), b_isempty(), b_getc(), b_eob(), b_print(), b_compact(), b_rflag()
* b_retract(), b_reset(), b_getcoffset(), b_rewind(), b_location()
*/
#include "buffer.h"
/*
* Purpose: Allocate memory for the buffer on the heap.
* Author: Justin De Vries, 040 495 834
* History/Versions: 8.2
* Called functions: calloc(), malloc(), free(), 
* Parameters: 
*   init_capacity
*       type: short
*       value: any non-negative short
*   inc_factor
*       type: char
*       value: any non-negative number
*   o_mode
*       type: char
*       value: a, m, or f
* Algorithm: 
*   Check to see if init_capacity is 0 or more or below SHRT_MAX.  Return NULL if not.
*   Allocate memory and check for NULL if it didn't work.
*   Assign appropriate values based on o_mode and return NULL if constraints aren't met.
*   If constraints are proper then allocate memory to cb_head and update capacity.
*   If allocation fails return NULL
*/
Buffer* b_allocate (short init_capacity, char inc_factor, char o_mode){

    /*  Create pointer to Buffer struct */
    Buffer* temp;

    /*  Check for init_cap < 0 or greater than or equal to SHRT_MAX */
    if (init_capacity < MIN_CAP){
        return NULL;
    }
    if (init_capacity > MAXPOS){
        return NULL;
    }

    /*  Assign memory to temp, return NULL on failure */
    temp = (Buffer*)calloc(1,sizeof(Buffer));
    
    if (temp == NULL){
        free(temp);
        return NULL;
    }

    /*  If inc_factor is zero then force o_mode to f to avoid overflow */
    if (inc_factor == MIN_VAL) 
        o_mode = 'f';

    /* Check for mode and apply values as appropriate, if mode and constraints don't match then return NULL */
    if (o_mode == 'f'){
        if (!init_capacity){
            return NULL;
        }
            temp->mode = FIXED;
            temp->inc_factor = MIN_INC;
    }
    if (o_mode == 'a'){
        if ((unsigned char)inc_factor){
            temp->mode = ADD;
            temp->inc_factor = (unsigned char)inc_factor;
        } else {
            return NULL;
        }
    }
    if (o_mode == 'm'){
        if (inc_factor > MIN_INC && inc_factor < MAX_MULTIINC){
            temp->mode = MULTI;
            temp->inc_factor = inc_factor;
        } else {
            return NULL;
        }
    }
    
    /* Assign memory to char array for storage of input from file, return NULL on failure */
    temp->cb_head = (char*)malloc(init_capacity * sizeof(char));

    if (temp->cb_head == NULL){
        free(temp->cb_head);
        return NULL;
    }

    /* Assign capacity and flag after working through modes and constraints, then return the Buffer */
    temp->capacity = init_capacity;
    temp->flags = DEFAULT_FLAGS;
    
    return temp;
}
/*
* Purpose: Add input from file to char array
* Author: Justin De Vries 040 495 834
* History/Versions: 10.4
* Called functions: realloc()
* Parameters: 
*   pBD
*       type: pBuffer (struct)
*       value: Current state of the Buffer struct
*   symbol
*       type: char
*       value: Current char from file input
* Algorithm: 
*   Checks for empty buffer and full capacity, if the add offset and the capacity are equal it will
*   attempt to increase the capacity based on operating mode, on FIXED mode it will simply return due to being full.
*   If there is an updated capacity to be assigned it will do so, reallocate the size of the buffer appropriately,
*   return NULL on failure and prevent the add offset from exceeding the capacity to avoid overflow
*   Returns the updated state of the Buffer if all criteria are met.
*/
pBuffer b_addc (pBuffer const pBD, char symbol){

    char* temp; /* char array for memory allocation without affecting the Buffer */
    short avail; /* Holds the amount of available capacity to work with*/
    short newInc = 0; /* Holds how much the buffer can be safely incremented. */
    short newCap = MIN_CAP; /* Hold the value of the new capacity value */

    if (pBD == NULL) {
        return pBD;
    }

    pBD->flags &= RESET_R_FLAG;

    if (pBD->capacity == SHRT_MAX){
        return NULL;
    }

    /* Does the offset match the capacity?  If so then modify the capacity according to the operational mode */
    if (pBD->addc_offset == pBD->capacity){

        if(pBD->mode == FIXED){ return NULL; } /* No modification necessary */
        if (pBD->mode == ADD){ /* Simply increase the capacity by the increment value provided */
            newCap = pBD->capacity + (unsigned char)pBD->inc_factor;
        }
        if (pBD->mode == MULTI){ /* Increase capacity from derived increment value based on available space */
            avail = MAXPOS - pBD->capacity;
            newInc = (avail * (unsigned char)pBD->inc_factor)/100;
            if (newInc == MIN_INC){ /* If newInc truncates to zero then assign any available space */ 
                newInc = avail;
            }
            newCap = pBD->capacity + newInc;
        }
        /* If the potential capacity overflowed then return NULL, otherwise make sure newCap is positive and update the Buffer
            capacity, if it's close to SHRT_MAX-1 then just assign that value */
        if (newCap < MIN_CAP){ 
            return NULL;
        } else if ((newCap > MIN_CAP) && (newCap < MAXPOS)){
            pBD->capacity = newCap;
        } else if ((MAXPOS - newInc) < newCap){
            newCap = MAXPOS;
            pBD->capacity = newCap;
        }
    }
    
    /* Make sure newCap has a positive value and assign memory, if its not mull then assign it to cb_head*/
    if (newCap){
        temp = (char*)realloc(pBD->cb_head, sizeof(char) * (newCap));
        if (temp == NULL){
            return NULL;
        }
        pBD->cb_head = temp;
        if (temp != pBD->cb_head){
            pBD->flags |= SET_R_FLAG;
        }
    }
    if (!pBD->cb_head){
        return NULL;
    }

    /* As long as there is room, add the character to the Buffer and increment the offset, if the offset gets
        too high then assign to the max ALLOWED value and return NULL to signal it's out of room */
    if (pBD->capacity >= pBD->addc_offset){
        pBD->cb_head[pBD->addc_offset] = symbol;
        pBD->addc_offset++;
        if (pBD->addc_offset == SHRT_MAX){
            pBD->addc_offset = MAXPOS;
            return NULL;
        }    
    }
    
    return pBD;
}
/*
* Purpose: Reset all update fields to defaults
* Author: Justin De Vries 040 495 834
* History/Versions: 2.3
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer*
*       value: Memory reference of Buffer
* Algorithm: 
*   Reset flag and offsets to zero, retains info in Buffer but will start at beginning
*   for any changes or reads.
*   Returns NULL if pBD is empty.
*/
int b_clear (Buffer* const pBD) {

    if (pBD == NULL){
		return RT_FAIL_1;
	}

	pBD->addc_offset = RESET_VALUE;
	pBD->getc_offset = RESET_VALUE;
	pBD->markc_offset = RESET_VALUE;
	pBD->flags = DEFAULT_FLAGS;

	return FALSE;
}
/*
* Purpose: Remove references of the Buffer from memory
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: free()
* Parameters: 
*   pBD
*       type: Buffer*
*       value: Memory reference of Buffer
* Algorithm: 
*   Free up memory of struct to avoid leaks, return if appropriate.
*/
void b_free (Buffer* const pBD) {
    
    if (pBD == NULL){
		return;
	}
	free(pBD->cb_head);
	free(pBD);
}
/*
* Purpose: Checks to see if the buffer is full
* Author: Justin De Vries 040 495 834
* History/Versions: 1.2
* Called functions:  none
* Parameters: 
*   pBD
*       type: Buffer*
*       value: Memory reference of Buffer
* Algorithm: 
*   Checks to see if the add offset is equal to the current capacity and returns TRUE, otherwise false
*   Returns -1 if Buffer is empty
*/
int b_isfull (Buffer* const pBD) {
    
    if (pBD == NULL){
		return RT_FAIL_1;
	}
	if (pBD->addc_offset == pBD->capacity){
		return TRUE;
	}
	return FALSE;
}
/*
* Purpose: Return the current value of the add offset
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer*
*       value: Memory reference to Buffer
* Algorithm: 
*   If pBD actually exists then return the offset.
*/
short b_limit (Buffer* const pBD) {
    
    if (pBD == NULL){
		return RT_FAIL_1;
	}
	return pBD->addc_offset;
}
/*
* Purpose: Return the current capacity of the buffer.
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer*
*       value: Memory reference to Buffer
* Algorithm: 
*   If pBD actually exists then return the current capacity.
*/
short b_capacity(Buffer* const pBD) {

    if (pBD == NULL){
        return RT_FAIL_1;
    }
    return pBD->capacity;
}
/*
* Purpose: Set the value of the mark offset
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Current state of pBD
*   mark
*       type: short
*       value: The range from 0 to the current add offset
* Algorithm: 
*   Checks to see if mark is a valid short, assigns it and returns that value.
*/
short b_mark (pBuffer const pBD, short mark) {

    if (pBD == NULL){
        return RT_FAIL_1;
    }
    /* Make sure it's 'hitting the mark' :) */
    if ((mark >= MIN_CAP) && (mark <= pBD->addc_offset)){
        pBD->markc_offset = mark;
        return pBD->markc_offset;
    }
    return RT_FAIL_1;
}
/*
* Purpose: Return the operating mode
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference for pBD
* Algorithm: 
*   Returns the current operating mode if the Buffer is active
*/
int b_mode (Buffer* const pBD) {
    
    if (pBD == NULL){
        return RT_FAIL_2;
    }
    return pBD->mode;
}
/*
* Purpose: Return te increment value
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference of Buffer
* Algorithm: 
*   If inc_factor is a positive value then return it, otherwise return false.
*/
size_t  b_incfactor(Buffer * const pBD) {
	if (pBD != NULL) {
		return (unsigned char)pBD->inc_factor;
	}
	else {
		return 0x100;
	}
}
/*
* Purpose: Load info from file into the Buffer
* Author: Justin De Vries
* History/Versions: 2.5
* Called functions: b_addc(), ungetc(), fgetc(), feof(), printf()
* Parameters: 
*   fi
*       type: FILE
*       value: file at runtime
*   pBD
*       type: Buffer
*       value: Memory reference of Buffer
* Algorithm: 
*   Read from file as long as there is data to get (not EOF)
*/
int b_load (FILE* const fi, Buffer * const pBD) {
	
    int num = MIN_VAL; /* Initialized variable for keeping track of what character has been read */
    char temp; /* storage for the currently read char */

    if ((pBD == NULL) || (fi == NULL)){
		return RT_FAIL_1;
	}
    /* while there are characters to get and EOF hasn't been reached, read from the file */
    while (temp = (char)fgetc(fi), !feof(fi)){

        if (b_addc(pBD, temp) == NULL){ /* if the value is null, ungetc and print out the character and return -1*/
            ungetc(num,fi);
            printf("The last character read from the file is: %c %d\n",temp, temp);
            return LOAD_FAIL;
        } else { /* increment counter if successful */
            num++;
        }
    }

    return num;
}
/*
* Purpose: Checks to see if the offset has moved
* Author: Justin De Vries 040 495 834
* History/Versions: 1 
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
* Algorithm: 
*   If the add offset is zero then true, otherwise false.  -1 on error
*/
int b_isempty(Buffer *const pBD) {
	if (pBD != NULL) {
		if (pBD->addc_offset == 0)
			return 1;
		else
			return 0;
	}
	else {
		return RT_FAIL_1;
	}

}
/*
* Purpose: Retrieve the character from a location in the char array
* Author: Justin De Vries 040 495 834
* History/Versions: 1.7
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
* Algorithm: 
*   Set end of buffer flag if getc and addc match.
*   Otherwise, reset end of buffer flag, get the character, increase the read offset 
*   and return the current character
*/
char b_getc (Buffer* const pBD) {
    
    char returnChar; /* char to return from cb_head */

    if (pBD == NULL){
        return RT_FAIL_2;
    }
    /* Are we done? Set flags and return if so */
    if (pBD->getc_offset == pBD->addc_offset){
        pBD->flags |= SET_EOB;
        return FALSE;
    }
    pBD->flags &= RESET_EOB;

    returnChar = pBD->cb_head[pBD->getc_offset]; /* Get the character from the proper position in cb_head */
    pBD->getc_offset++;

    return returnChar;
}
/*
* Purpose: Return the current state of the buffer
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
* Algorithm: 
*   Return the current state of the buffer using a bitwise operation
*/
int b_eob (Buffer* const pBD) {
    
    if (pBD == NULL){
        return RT_FAIL_1;
    }
    return pBD->flags & CHECK_EOB;
}
/*
* Purpose: Output the character buffer
* Author: Justin De Vries 040 495 834
* History/Versions: 1.8
* Called functions: b_isempty(), printf(), b_getc(), b_eob()
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
* Algorithm: 
*   Checks to see if the Buffer is empty and returns if it is.
*   Otherwise it resets the read offset to 0 and starts looping through cb_head via the 
*   getc function until there are no more characters left to read or the end of the buffer is hit.
*/
int b_print(Buffer * const pBD) {

	int count = 0; /*act as a counter to return number of characters printed*/
	char c = ' '; /*act as a temp var to hold character before printing*/

	if (pBD != NULL) {
		if (b_isempty(pBD)) {
			/*if empty notify user*/
			printf("Empty buffer!\n");
		}
		else {
			do {
				c = b_getc(pBD);
				count++;
			} while (!b_eob(pBD) && printf("%c", c)) ;
				/*if it isn't empty and end of bit use getc to get character and print it*/
				
			
			printf("\n");
		}
	}
	else {
		return RT_FAIL_1;
	}
	return count;
}
/*
* Purpose: Compact, or expand, the buffer.
* Author: Justin De Vries 040 495 834
* History/Versions: 2.1
* Called functions: realloc()
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
*   symbol
*       type: char
*       value: Current character in the buffer
* Algorithm: 
*   Performs a sanity check on current capacity by assigning it to be one more than the current add offset
*   
*/
Buffer* b_compact(Buffer* const pBD, char symbol) {
    
    char* temp; /* char array for temporary memory allocation */
    short newCap = pBD->addc_offset + 1; /* new capacity */

    pBD->flags &= RESET_R_FLAG;

    if (pBD == NULL){
		return NULL;
	}
    if (newCap < MIN_CAP){
        return NULL;
    }
    
    temp = (char*)realloc(pBD->cb_head, sizeof(char) * (newCap));

    if (temp == NULL){
        return NULL;
    }
    pBD->cb_head = temp;
    pBD->capacity = newCap;
    
    if (temp != pBD->cb_head){
        pBD->flags |= SET_R_FLAG;
    }
    *(pBD->cb_head + pBD->addc_offset) = symbol; /* different way of assigning this time to test out */
    pBD->addc_offset++;

    return pBD;
}
/*
* Purpose: Returns the current flag value
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
* Algorithm: 
*   Returns the flag value if the buffer exists
*/
char b_rflag(Buffer * const pBD) {
	if (pBD != NULL) {
		return pBD->flags & CHECK_R_FLAG;
	}
	else {
		return RT_FAIL_1;
	}
}
/*
* Purpose: Retract read offset
* Author: Justin De Vries 040 495 834
* History/Versions: 1.3
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
* Algorithm: 
*   If the buffer exists and the read offset isn't already 0 it post-decrements the read offset and returns
*/
short b_retract (Buffer* const pBD) {

    if (pBD == NULL){
		return RT_FAIL_1;
	}
	else if (pBD->getc_offset == MIN_VAL){
		return RT_FAIL_1;
	}
	pBD->getc_offset--;
	return pBD->getc_offset;
}
/*
* Purpose: Resets the read offset to the marked offset
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
* Algorithm: 
*   Sets getc_offset so the current value of markc_offset
*/
short b_reset (Buffer* const pBD) {

    if (pBD == NULL){
		return RT_FAIL_1;
	}
	pBD->getc_offset = pBD->markc_offset;
	return pBD->getc_offset;
}
/*
* Purpose: Gets the current poition of the read offset
* Author: Justin De Vries 040 495 834
* History/Versions: 1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
* Algorithm: 
*   Returns the getc_offset of the buffer exists
*/
short b_getcoffset (Buffer* const pBD) {
     
    if (pBD == NULL){
		return RT_FAIL_1;
	}
	return pBD->getc_offset;
}
/*
* Purpose: Reset the mark and read offsets to 0
* Author: Justin De Vries 040 495 834
* History/Versions: 1.1
* Called functions: none
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
* Algorithm: 
*   If the buffer exists then reset getc and mark offset and return zero
*/
int b_rewind(Buffer* const pBD) {

    if (pBD == NULL){
		return RT_FAIL_1;
	}
    pBD->getc_offset = MIN_VAL;
    pBD->markc_offset = MIN_VAL;

    return FALSE;
}
/*
* Purpose: Return the pointer to the location of a char based on loc_offset
* Author: Justin De Vries 040 495 834
* History/Versions: 1.4
* Called functions: b_getcoffset()
* Parameters: 
*   pBD
*       type: Buffer
*       value: Memory reference to Buffer
*   loc_offset
*       type: short
*       value: 
* Algorithm: 
*   Verifies that loc_offset matches the position of getcoffset and return
*   Not 100% sure it's correct mind you
*/
char * b_location(Buffer * const pBD, short loc_offset) {
	if (pBD == NULL) {
		return NULL;
	}
	return (char*)(pBD->cb_head + loc_offset);

}