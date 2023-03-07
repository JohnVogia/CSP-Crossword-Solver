#include "crossword_structs.h"

/*All functions from here until the function check have to only with the part of the program
  that solves the crossword */

void earliest_conf_set(int **set1, int *set2, int *emc_num, int num2, int limit);
int add_to_set(int *set, int num, int level);
int remove_conflicts(int *set, int num, int level);
int unite(int *set1, int *set2, int num1, int num2);
int max_of_set(int *set, int num);
void push(stack *mystack, int value);
void push_3d(stack_3d *domain_stack, char **domain);
int pop_3d(stack_3d *domain_stack);
int pop(stack *mystack);
var* MRV(var *var_h, var* var_v, int amount_var_h, int amount_var_v);
void reset_domain(var *variable);
void fix_variables_info(var *variables, int amount_var, int level);
int neighbour_update(var *assignment, char* assignment_value, var *variables, char **dict, int **emc, int *emc_num, int limit);
int check_forward(var *assigned_var, var *variables, char **dict, int **emc, int *emc_num, int limit);
char** CSP(var **assignments, var* var_v, var* var_h, int amount_var_h, int amount_var_v, char **grid, char **dict);
void find_neighbours(var* variables, var* other_var, int var_amount, int other_var_amount);
void find_variables(var **var_h, var **var_v, char **grid, int dimension, int *count_h_var, int *count_v_var);
void find_domain_of_variables(var *variables, int *length, int amount_var, int count_total_words, char **dict);


int check(char **grid, char **dict, int count_words, int dimension);  /*This function checks whether a solution for a crossword is valid
                                                                        when -check is read an argument*/

solutions solve_crossword(char **grid, char **dict, int *lengths, int count_words, int dimension); /*This is the function that solves the program and transfers the solutions 
                                                                                                    it finds to the main part of the program so they can be printed*/