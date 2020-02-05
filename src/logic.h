#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef int* olist_t ;

enum e_no_stim  { s_idle, s_random, s_lrandom, SE_NUMBER };
enum e_food     { f_idle, f_random, f_lrandom, f_avoid, f_seek, FE_NUMBER };
enum e_big      { b_idle, b_random, b_lrandom, b_avoid, b_seek, BE_NUMBER };
enum e_pray     { p_idle, p_random, p_lrandom, p_avoid, p_seek, PE_NUMBER };

/* this type represents selected action's enum number */
typedef struct {
    enum e_no_stim  nostim_o;
    enum e_food     food_o;
    enum e_big      big_o;
    enum e_pray     pray_o;
} actions_opt_t;

typedef struct {

} traits_opt_t;
