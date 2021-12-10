#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

/* #define's -----------------------------------------------------------------*/

#define ASIZE 26

#define TRUE '1'    
#define FALSE '0'
#define NEW_LINE '\n'
#define END_SEC '#' // symbol that indicates the end of section
#define ASCII_a 97  // ascii value of a
#define ASCII_z 122 // ascii value of z
#define ASCII_A 65  // ascii value of A
#define ASCII_Z 90  // ascii value of Z
#define SCOLON ':'  // semi-colon
#define UNKNOWN '?' // symbol for variables that are involved in the action 
#define STG0_SEP "==STAGE 0==============================="
#define DELIM "----------------------------------------"
#define BVAR "abcdefghijklmnopqrstuvwxyz"   // all the boolean variables
#define STG1_SEP "==STAGE 1==============================="
#define STG2_SEP "==STAGE 2==============================="
#define END_SEP "==THE END==============================="

/* type definitions ----------------------------------------------------------*/

// state (values of the 26 Boolean variables)
typedef unsigned char state_t[ASIZE];

// action
typedef struct action action_t;
struct action {
    char name;        // action name
    state_t precon;   // precondition
    state_t effect;   // effect
};

// step in a trace
typedef struct step step_t;
struct step {
    action_t *action; // pointer to an action performed at this step
    step_t   *next;   // pointer to the next step in this trace
};

// trace (implemented as a linked list)
typedef struct {
    step_t *head;     // pointer to the step in the head of the trace
    step_t *tail;     // pointer to the step in the tail of the trace
} trace_t;

/* function prototypes -------------------------------------------------------*/
trace_t* make_empty_trace(void);
trace_t* insert_at_tail(trace_t*, action_t*);
void free_trace(trace_t*);

/* my function prototypes ----------------------------------------------------*/

// add your function prototypes here ...
void copy_state(state_t state, state_t copy);
void set_all(state_t state, char con);
void initial_state(state_t state);
int read_action(trace_t* trace);
int read_trace(trace_t* actions, trace_t* trace);
void apply_change(state_t state, state_t effect);
int trace_check(state_t state, trace_t* trace);
int action_check(state_t state, step_t* step);
void print_info(trace_t* actions, int nactions, int valid);
void print_state(state_t state);
void print_trace(state_t state, trace_t* trace);
int next_stage(void);
int read_croutine(trace_t* actions, trace_t* routine, int* stage2);
void cum_effect(trace_t* routine, state_t tmp);
int same_state(state_t s1, state_t s2);
void print_candidate(trace_t* routine);
void subsequence(trace_t* trace, state_t cumeffect, int stage);
step_t* print_subsequence(step_t* start, int* spos, int epos);
int stage2_check(state_t precon, state_t cumeffect, state_t tmp_state);

/* where it all happens ------------------------------------------------------*/
int
main(int argc, char *argv[]) {
    state_t state, cumeffect;
    trace_t* actions, *trace, *routine;
    int nactions, valid, first_candidate=1, stage2=0;

    /* stage 0 */
    initial_state(state);
    actions = make_empty_trace();
    trace = make_empty_trace();
    while (read_action(actions)) {
        // do nothing
    }
    nactions = read_trace(actions, trace);
    valid = trace_check(state, trace);
    print_info(actions, nactions, valid);
    print_trace(state, trace);

    /* stage 1 */
    if (next_stage()) {
        printf("%s\n", STG1_SEP);
        while ((routine = make_empty_trace()) && 
        read_croutine(actions, routine, &stage2)) { // read candidate routines
            if (first_candidate) {
                first_candidate=0;
            } else {
                printf("%s", DELIM);
                printf("\n");
            }
            /* print candidate routines and corresponding subsequence */
            print_candidate(routine);
            cum_effect(routine, cumeffect);
            subsequence(trace, cumeffect, stage2);            
            free_trace(routine);
        }
    }

    /* stage 2 */
    if (stage2) {
        first_candidate = 1;
        printf("%s\n", STG2_SEP);
        while ((routine = make_empty_trace()) && 
        read_croutine(actions, routine, &stage2)) { // read candidate routines
            if (first_candidate) {
                first_candidate=0;
            } else {
                printf("%s", DELIM);
                printf("\n");
            }
            /* print candidate routines and corresponding subsequence */
            print_candidate(routine);
            cum_effect(routine, cumeffect);
            subsequence(trace, cumeffect, stage2);            
            free_trace(routine);
        }
        printf("%s\n", END_SEP);
    }
    return EXIT_SUCCESS;        // we are done !!! algorithms are fun!!!
}

/* function definitions ------------------------------------------------------*/

// Adapted version of the make_empty_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
trace_t
*make_empty_trace(void) {
    trace_t *R;
    R = (trace_t*)malloc(sizeof(*R));
    assert(R!=NULL);
    R->head = R->tail = NULL;
    return R;
}

// Adapted version of the insert_at_foot function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
trace_t 
*insert_at_tail(trace_t* R, action_t* addr) {
    step_t *new;
    new = (step_t*)malloc(sizeof(*new));
    assert(R!=NULL && new!=NULL);
    new->action = addr;
    new->next = NULL;
    if (R->tail==NULL) { /* this is the first insertion into the trace */
        R->head = R->tail = new; 
    } else {
        R->tail->next = new;
        R->tail = new;
    }
    return R;
}

// Adapted version of the free_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
void
free_trace(trace_t* R) {
    step_t *curr, *prev;
    assert(R!=NULL);
    curr = R->head;
    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }
    free(R);
}

/* my function definitions ---------------------------------------------------*/

// add your function definitions here ...

/* sets all variables to the specified state (either true or false) */

/* make a copy of the original state */
void copy_state(state_t state, state_t copy) {
    int i;
    for (i=0; i<ASIZE; i++) {
        copy[i] = state[i];
    }
}

/* set all variables to the specified condition */
void set_all(state_t state, char con) {
    int i;
    for (i=0; i<ASIZE; i++) {
        state[i] = con;
    }
}

/* read in the inital state */
void initial_state(state_t state) {
    char var;
    int last_new_line = 0;
    set_all(state, FALSE);
    while (scanf("%c", &var)==1) {
        if (var==END_SEC) {
            last_new_line = 1;
        }  
        if (last_new_line && var==NEW_LINE) {
            return ; 
        } 
        /* only extract the boolean variables in the input */
        if (var>=ASCII_a && var<=ASCII_z) {    
            state[var-ASCII_a] = TRUE;
        }
    }
}

/* read a single action, returns 1 if an action is successfully read, and 
returns 0 otherwise */
int read_action(trace_t* actions) {
    char ch;
    int i=0, ndelim=0, end_sec=0;
    action_t* action;
    action = (action_t*)malloc(sizeof(*action));
    set_all(action->precon, UNKNOWN);
    set_all(action->effect, UNKNOWN);

    while (scanf("%c", &ch)==1) { 
        if (ch==NEW_LINE) { // reached the end of the action
            if (end_sec) {
                free(action);
                return 0;
            }
            break;
        } else if (ch==END_SEC) {  // reached the end of the section
            end_sec = 1;
        } else if (ch>=ASCII_A && ch<=ASCII_Z) {   // action name detected
            action->name = ch;
        } else if (ch==SCOLON) {   // semi-colon detected
            ndelim++;
        } else if (ch>=ASCII_a && ch<=ASCII_z) {   
            if (ndelim==0) {    // var that must be true 
                action->precon[ch-ASCII_a] = TRUE;
            } else if (ndelim==1) { // var that must be false
                action->precon[ch-ASCII_a] = FALSE;
            } else if (ndelim==3) { // var to be set to true
                action->effect[ch-ASCII_a] = TRUE;
            } else if (ndelim==4) { // var to be set to false
                action->effect[ch-ASCII_a] = FALSE;
            }
        }
        i++;
    }
    insert_at_tail(actions, action);
    return 1;
}

/* reads the trace, constructs the trace linked list, and returns the number of
actions in trace */
int read_trace(trace_t* actions, trace_t* trace) {
    char ch;
    int i=0, nactions=0;
    step_t* act;
    while (scanf("%c", &ch)==1 && ch!=NEW_LINE) {
        if (ch>=ASCII_A && ch<=ASCII_Z) {   
            act = actions->head;
            nactions++;
            while (act) {
                if (act->action->name==ch) { // corresponding action is found
                    insert_at_tail(trace, act->action);
                    break;
                }
                act = act->next;
            }
        }
        i++;
    }
    return nactions;
}

/* apply changes to the state */
void apply_change(state_t state, state_t effect) {
    int i;
    for (i=0; i<ASIZE; i++) {
        if (effect[i]!=UNKNOWN) {
            state[i] = effect[i];
        } 
    }
}

/* returns 1 if trace is valid, returns 0 otherwise */
int trace_check(state_t state, trace_t* trace) {
    step_t* step;
    state_t copy;
    copy_state(state, copy);
    step = trace->head;
    while (step) {
        if (!action_check(copy, step)) {
            return 0;
        }
        apply_change(copy, step->action->effect);
        step = step->next;
    }
    return 1;
}

/* returns 1 if action is valid, returns 0 otherwise */
int action_check(state_t state, step_t* step) {
    int i;
    for (i=0; i<ASIZE; i++) {
        if (step->action->precon[i]!=UNKNOWN && 
        step->action->precon[i]!=state[i]) {
            return 0;
        }
    }
    return 1;
}

/* prints basic information, including the number of distinct actions, length
of the trace, and the status of the trace read from the input */
void print_info(trace_t* actions, int nactions, int valid) {
    int ndistinct=0;
    step_t* action;
    action = actions->head;
    // counts the number of total actions and unique actions
    while (action) {
        ndistinct++;
        action = action->next;
    }
    /* prints basic info */
    printf("%s\n", STG0_SEP);
    printf("Number of distinct actions: %d\n", ndistinct);
    printf("Length of the input trace: %d\n", nactions);
    printf("Trace status: ");
    if (valid) {
        printf("valid\n");
    } else {
        printf("invalid\n");
    }
    printf("%s\n", DELIM);
}

/* prints the state of all the boolean variable */
void print_state(state_t state) {
    int i;
    for (i=0; i<ASIZE; i++) {
        printf("%c", state[i]);
    }
}

/* print the state after each action in the trace */
void print_trace(state_t state, trace_t* trace) {
    step_t* step;
    printf("  %s\n", BVAR); // print the intial state
    printf("> ");
    print_state(state);
    printf("\n");
    step = trace->head;
    /* print the valid part of the trace */
    while (step && action_check(state, step)) { 
        apply_change(state, step->action->effect);
        printf("%c ", step->action->name);
        print_state(state);
        printf("\n");
        step = step->next;
    }
}

/* returns 1 if stage 1/2 output should be generated, and returns 0 otherwise */
int next_stage(void) {
    char ch;
    int proceed=0;
    while (scanf("%c", &ch)==1 && ch!=NEW_LINE) {
        if (ch==END_SEC) {
            proceed=1;
        }
    }  
    return proceed;
}

/* reads a candidate routine and returns the pointer to the temperory trace */
int read_croutine(trace_t* actions, trace_t* routine, int* stage2) {
    step_t* step;
    char ch;
    int ncroutine=0;
    while (scanf("%c", &ch)==1 && ch!=NEW_LINE) {
        if (ch>=ASCII_A && ch<=ASCII_Z) {   // an action is detected 
            /* retrieve details on each action in the candidate routine and add 
            to the temperory trace */
            step = actions->head;
            while (step) {  
                if (step->action->name==ch) {
                    insert_at_tail(routine, step->action);
                    break;
                }
                step = step->next;
            }
            ncroutine++;
        } else if (ch==END_SEC) {
            *stage2 = 1;
        }
    }
    return ncroutine;
}

/* generate the cumulative effect of a list of actions in a candidate routine */
void cum_effect(trace_t* routine, state_t tmp) {
    step_t* step;
    step = routine->head;
    set_all(tmp, UNKNOWN);
    while (step) {  // go through each action in the routine
        apply_change(tmp, step->action->effect);
        step = step->next;
    }
}

/* returns 1 if input states are the same, and 0 otherwise */
int same_state(state_t s1, state_t s2) {
    int i;
    for (i=0; i<ASIZE; i++) {
        if (s1[i]!=s2[i]) {
            return 0;
        }
    }
    return 1;
}

/* prints header for each candidate routine */
void print_candidate(trace_t* routine) {
    step_t* step=routine->head;
    printf("Candidate Routine: ");
    while(step) {
        printf("%c", step->action->name);
        step = step->next;
    }
    printf("\n");
}

/* prints all the subsequences and the corresponding index that produces the 
same cumulative effect as the candidate routine */
void subsequence(trace_t* trace, state_t cumeffect, int stage2) {
    step_t* start, *end, *step;
    trace_t* tmp_trace;
    state_t tmp_state;
    int spos=0, epos, found;

    start = trace -> head;
    /* test out every possible subsequence */
    while (start) {
        end = start;
        tmp_trace = make_empty_trace(); 
        epos = spos;
        found = 0; 
        while (end) { // subsequence are generated from left to right
            insert_at_tail(tmp_trace, end->action);
            cum_effect(tmp_trace, tmp_state);   // cum-effect of temporary trace
            if (!stage2) {  // stage 1 output required
                found=same_state(tmp_state, cumeffect);
            } else if (stage2) {    // stage 2 output required 
                step = tmp_trace->head;
                while (step) {  // var not set by c-routine needs to be checked
                    if((found=stage2_check(step->action->precon, cumeffect, 
                    tmp_state))) {
                        break;
                    }
                    step = step->next;
                }
            } 
            if (found) {    // valid subsequence is found and to be printed
                start = print_subsequence(start, &spos, epos);
                break;
            }
            end = end->next;
            epos++;
        }
        if (!found) {  // increment position by 1 if there's no valid subseq
            start = start->next;
            spos++;
        }
        free_trace(tmp_trace);
    }
}

/* returns 1 if the cumulative effect of the subsequence is the same as the 
cumulative effect of the candidate routine, with values of variables not set by
candidate routine also set to the values they had before executing the 
subsequence. returns 0 other wise */
int stage2_check(state_t precon, state_t cumeffect, state_t tmp_state) {
    int i;
    for (i=0; i<ASIZE; i++) {
        // variable not set by the candidate routine is detected 
        if (cumeffect[i]!=UNKNOWN && tmp_state[i]!=cumeffect[i]) {
            return 0;
        } else if (cumeffect[i]==UNKNOWN && tmp_state[i]!=cumeffect[i] && 
        tmp_state[i]!=precon[i]) {  // common variable with c-routine is found
            return 0;
        }
    }
    return 1;
}

/* print the actions names of the subsequence and returns the pointer at index 
end-position + 1 */
step_t* print_subsequence(step_t* start, int* spos, int epos) {
    int nskips=epos-*spos;
    printf("%5d: ", *spos); // index of starting position
    while (start && nskips>=0) {   
        printf("%c", start->action->name);
        start = start->next;
        *spos = *spos + 1;
        nskips--;
    }
    printf("\n");
    return start;
}

/* ta-da-da-daa!!! -----------------------------------------------------------*/
