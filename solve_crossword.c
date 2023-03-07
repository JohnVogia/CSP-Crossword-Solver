#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crossword.h" //crossword.h also has crossword_structs included in it

int add_to_set(int *set, int num, int level){ //If this function finds the element needed to be added in the
    int i;                                    //given set, it returns 0, else it adds the element and returns 1
    for(i = 0; i < num; i++){                 
        if (set[i] == level) return 0; 
    }
    set[num] = level;
    return 1;
}

int max_of_set(int *set, int num){ // Returns the max element of a set
    int i, pos, max;
    if (!num){
        printf("Didn't find a solution for the crossword\n");
        exit(-1); /*Since I only call this function to determine the next variable to backjump to, if no such variable exists,
                    it is impossible that there exists a solution for the crossword*/
    }
    max = set[0];
    pos = 0;
    for (i = 1; i < num; i++){
        if (set[i] > max){
            max = set[i];
            pos = i;
        }
    }
    set[pos] = set[num-1];
    set[num-1] = -1; //I am representing empty slots with the value -1
    return max;
}

void earliest_conf_set(int **set1, int *set2, int *emc_num, int num2, int limit){ //This function is important for determining the earliest constraint scope
    int *set1_copy, *set2_copy;                                     //(The constraint scope whose max level -out of the levels that are different between the two scopes- is the smallest out of
    int max1, max2, num1, num2_copy;                                //all the constraint scopes that conflict with a variable whose domain just got wiped out) 
    num1 = *emc_num; 
    num2_copy = num2;
    if (!num1 && num2){
        *emc_num = num2_copy;
        memcpy(*set1, set2, limit * sizeof(int)); 
        return;
    }
    else if (!num2){
        return;
    }
    if ((set1_copy = malloc(num1 * sizeof(int))) == NULL || (set2_copy = malloc(num2 * sizeof(int))) == NULL){ 
        exit(-1);
    }

    //I am making copies of the sets because I don't want the changes that help in my calculations to affect the original sets

    memcpy(set1_copy, *set1, num1 * sizeof(int)); 
    memcpy(set2_copy, set2, num2 * sizeof(int));
    while (num1 && num2){
        if ((max1 = max_of_set(set1_copy, num1--)) != (max2 = max_of_set(set2_copy, num2--))){ //When max1 != max2 I have finally found a max level that differs and I can stop checking
            break;
        }
    }
    if (!num1 && num2){
        *emc_num = num2_copy;
        memcpy(*set1, set2, limit*sizeof(int)); //if num1 = 0 I change to the conflict set that isn't empty
        free(set1_copy);
        free(set2_copy);
        return;
    }
    else if (!num2){
        free(set1_copy);
        free(set2_copy);
        return;  //If num2 = 0, I just keep the set1 untouched
    }
    if ((max1 < max2)){ //This means set1 must be left unchanged
        free(set1_copy);
        free(set2_copy);
        return;
    }

    //At this point, max2 >= max1 so I have to copy set2 to set1

    free(set1_copy);
    free(set2_copy);
    *emc_num = num2_copy;
    memcpy(*set1, set2, limit * sizeof(int));
    return;
}

int remove_conflicts(int *set, int num, int level){ //This function will take a conflict set and remove all values k for which k >= level
    int i, count = 0, a;
    for (i = 0; i < num; i++){
        if (set[i] >= level){
            set[i] = -1; //Empty slots are represented with the value -1
            count++;
        }
    }
    a = 0;
    for(i = 0; i < num; i++){
        if (set[i] > -1){
            set[a++] = set[i]; //Bringing the non empty slots behind the -1 values
        }
    }
    for(i = a; i < num; i++){
        set[i] = -1; 
    }
    return a;
}

int unite(int *set1, int *set2, int num1, int num2){
    int i;
    for(i = 0; i < num2; i++){
        num1 += add_to_set(set1, num1, set2[i]);
    }
    return num1;
}

void push_3d(stack_3d *domain_stack, char **domain) //This function pushes char** domains to a stack. That way I can know the current state of each variable's domain
{
    stack_3d newnode;
    newnode = *domain_stack;
    if ((*domain_stack = malloc(sizeof(node_3d))) == NULL)
    {
        exit(-1);
    }
    (*domain_stack)->domain_3d = domain;
    (*domain_stack)->next_3d = newnode;
    return; 
}

void push(stack *mystack, int value) //This function just pushes a normal int value to a stack of ints
{                                    //I use this function for 2 purposes: 1) To know the current state of a variable's domain's cardinality and
    stack newnode;                   // 2) To know the levels at which, each of the variables that have reduced the domain of a specific (common) neighbor, had been assigned
    newnode = *mystack;
    if ((*mystack = malloc(sizeof(node))) == NULL)
    {
        exit(-1);
    }
    (*mystack)->value = value;
    (*mystack)->next = newnode;
    return;
}

int pop_3d(stack_3d *domain_stack){ //Pops the latest domain of a variable (and returns 1 if it succeeds in finding a domain to pop or 0 if it doesnt)
    node_3d *temp;
    if (*domain_stack == NULL) 
    {
        return 0;
    }
    temp = *domain_stack;
    *domain_stack = (*domain_stack)->next_3d; 
    free(temp->domain_3d);
    free(temp);
    return 1;
}

int pop(stack *mystack){ // This function pops the top element of a regular stack of ints
    stack temp;
    if (*mystack == NULL)
    {
        return 0;
    }
    temp = *mystack;
    *mystack = (*mystack)->next;
    free(temp);
    return 1;
}

var* MRV(var *var_h, var* var_v, int amount_var_h, int amount_var_v) 
{                                         //This function implements the MRV heuristic which stands for "Minimum Remaining Values".
    int i, min, found_unassigned;         //It compares the cardinalities of the domains of all the variables that are not assigned in order to find the 
    var *MRV_var;                         //the variable with the smallest cardinality that isn't assigned. Then, it returns an address to that variable,
                                          //So that it can be assigned a value
    min = var_h[0].cardinality_stack->value; 
    MRV_var = var_h;
    found_unassigned = !(var_h[0].is_assigned); //if the variable is assigned I want found_unassigned to be 0, else I want it to be 1

    for (i = 1; i < amount_var_h; i++) //I first find the variable with the minimum cardinality out of the horizontal variables
    {
        if (!var_h[i].is_assigned && (!found_unassigned || (var_h[i].cardinality_stack->value < min))) 
        {
            min = var_h[i].cardinality_stack->value;
            MRV_var = var_h + i;
            found_unassigned = 1;
        }
    }
    for (i = 0; i < amount_var_v; i++) //Now I find whether there exists a vertical variable with less cardinality than the horizontal one I searched for earlier
    {
        if (!var_v[i].is_assigned && (!found_unassigned || (var_v[i].cardinality_stack->value < min)))
        {
            min = var_v[i].cardinality_stack->value;
            MRV_var = var_v + i;
            found_unassigned = 1;
        }
    }
    if (!min){
        printf("dictionary does not allow for a solution\n"); //This could only happen if there was a variable that didn't have any values in its domain since the start of the program
                                                              //which obviously means the crossword is unsolvable and I can stop trying to solve it right now
        exit(-1);
    }
    return MRV_var;
}


void fix_variables_info(var *variables, int amount_var, int level) //This is the function that undoes any changes that currently unassigned variables made to the domain of any variable
{                                                                   //I call this function twice, once for the horizontal variables and once for the vertical variables
    int i;
    for (i = 0; i < amount_var; i++){
        if (variables[i].is_assigned) continue; //Saving time by checking if the variable is assigned
        while (variables[i].pastfc != NULL){ 
            if (variables[i].pastfc->value < level){ //I don't want to undo any changes that ASSIGNED variables (with level lower than the 1 of the variable I backtracked to)
                break;                               //made to other variables, so I break; when this condition is met
            }

            //I am resetting the pastfc, cardinality_stack and domain_stack of a variable to the state where they were affected only by ASSIGNED variables by
            //popping these stacks as many times as there are currently unassigned variables in  that variable's pastfc
            //(the unassigned variables are obviously the last ones to make those changes).

            pop(&(variables[i].pastfc)); 
            pop(&(variables[i].cardinality_stack));
            pop_3d(&(variables[i].domain_stack));
        }
        variables[i].conf_num = remove_conflicts(variables[i].conf_set, variables[i].conf_num, level); //Removing all the levels in the conflict set
                                                                                                       //that correspond to unassigned variables
        if (variables[i].level > level){
            variables[i].backjump_count = 0;  //If the variable is unassiged I reset its backjump_count to 0 so it can start from the first word 
                                              // in its domain again when it is its time to be assigned
        }
    }
}

int neighbour_update(var *assignment, char* assignment_value, var *variables, char **dict, int **emc, int *emc_num, int limit){ 
                                                                    //This function is the way I make necessary changes to variables' info when forward checking
                                                                    //It also updates conflict sets of each variable that has changes performed on it
                                                                    //And it helps me calculate the emc set (which stands for Earliest Minimal Conflict set) 
                                                                    //in order to perform CBJ (Conflict-Directed Backjumping) 
    int i, j, cardinality, new_cardinality, *temp_domain_index; 
    int neighbours_amount, reset, last_neighbour_pos = -2;
    char **domain, **new_domain;
    neighbours_amount = assignment->neighbours_amount;
    reset = 0;
    for (i = 0; i < neighbours_amount; i++){
        if(!variables[assignment->neighbours[i]].is_assigned){
            last_neighbour_pos = i;      //Firstly I find the index (in relation to the other variables of the same type -Horizontal/Vertical-) of 
                                         // the last unassigned neighbor of the variable that is performing the forward check 
        }
    }
    for (i = 0; i <= last_neighbour_pos + 1; i++) //I search until last_neighbour_pos + 1 because it helps me perform the undoing of changes when
                                                  //the forward check causes a domain wipeout.
    {
        if (i <= last_neighbour_pos){
            if (variables[assignment->neighbours[i]].is_assigned) //If the ith neighbor is assigned and isn't last_neighbour_pos + 1, I can't change anything so i use continue;
            {
                continue;
            }
            domain = variables[assignment->neighbours[i]].domain_stack->domain_3d;
            cardinality = variables[assignment->neighbours[i]].cardinality_stack->value;
            if ((temp_domain_index = malloc(cardinality * sizeof(int))) == NULL)
            {
                exit(-1);
            }
            new_cardinality = 0;
            for (j = 0; j < cardinality; j++)
            {
                if (domain[j][assignment->intersections[i][1]] == assignment_value[assignment->intersections[i][0]]) //filtering the neighbour's domain by only keeping the words                                                                                                                    
                {                                                                                                    //that have the same letter as the variable that performs the forward check
                    temp_domain_index[new_cardinality++] = j;                                                        //in the position that the neighbor intersects the variable
                }
            } 
            if (!new_cardinality){
                reset = 1; //Since I now know there is at least 1 neighbour whose domain got wiped out by the variable's forward check
                           //I set reset to 1, in order to revert the changes made to the neighbours up to this point
                           //but the reset won't necessarily happen immediately because I first need to extract some info for the CBJ
            }
        }
        if (reset && (i == last_neighbour_pos+1)) //The reset of the neighbours' info will happen only when reset = 1 and i has reached its final value
        {
            for (j = 0; j <= last_neighbour_pos; j++) //for every neighbour that the variable forward check I must revert the changes made
            {
                if (variables[assignment->neighbours[j]].is_assigned) continue; //I didn't touch the info of the assigned neighbours, so I use continue;
                variables[assignment->neighbours[j]].conf_set[--(variables[assignment->neighbours[j]].conf_num)] = -1; //Deleting the last variable in the conflict set
                                                                                                                        //(it is obviously the variable that is performing the forward check)

                if (!variables[assignment->neighbours[j]].cardinality_stack->value){ //If this condition is met, I know that the jth neighbor had its domain wiped out

                    earliest_conf_set(emc, variables[assignment->neighbours[j]].conf_set, emc_num, //This means that the jth neighbour is an important variable for deciding 
                    variables[assignment->neighbours[j]].conf_num, limit);                   //which variable I will backjump to in a bit. To make this choice, I try to find the
                }                                                                           //earliest minimal conflict set out of all the neighbours for this current word
                pop_3d(&(variables[assignment->neighbours[j]].domain_stack));    
                pop(&(variables[assignment->neighbours[j]].cardinality_stack));
                pop(&(variables[assignment->neighbours[j]].pastfc));              
            }
            return 0;
        }
        if (i <= last_neighbour_pos){ 
            if (new_cardinality < cardinality){ //If the variable reduced the i'th neighbour's domain I need to update the info for the i'th neighbour
                variables[assignment->neighbours[i]].conf_set[(variables[assignment->neighbours[i]].conf_num)++] = assignment->level; //updating neighbour's conflict set
                if (new_cardinality){ //If new_cardinality isn't 0 I make a char** array that kees the words that are left after the reduction
                                      //and then I push it to the domain stack of the variable
                    if ((new_domain = malloc(new_cardinality * sizeof(char *))) == NULL) 
                    {
                        exit(-1);
                    }
                    for (j = 0; j < new_cardinality; j++)
                    {
                        new_domain[j] = domain[temp_domain_index[j]]; 
                    }
                }
                else{
                    new_domain = NULL; //If the domain of the i'th neighbour got wiped out I push the NULL pointer
                                       //to the stack. It doesn't matter what I push to the stack actually because I will 
                                       //end up popping it in a bit anyways 
                }
                push_3d(&(variables[assignment->neighbours[i]].domain_stack), new_domain); 
                push(&(variables[assignment->neighbours[i]].cardinality_stack), new_cardinality);
                push(&(variables[assignment->neighbours[i]].pastfc), assignment->level); //Pushing the level of the variable that performs the forward check
                                                                                         //to the i'th neighbour's pastfc stack
            } 
            free(temp_domain_index); 
        }
    }
    return 1;
}
int check_forward(var *assigned_var, var *variables, char **dict, int **emc, int *emc_num, int limit) //This function performs forward check
{                                                                                       //and it returns the index of the first word that succeeded
    int i, cardinality, start;                                                          //in not resulting in any domain wipeout for the variables' neighbours.
    char *word;                                                                         //Otherwise it returns -1.
    cardinality = assigned_var->cardinality_stack->value;
    start = assigned_var->backjump_count;
    *emc_num = 0; //Initializing the number of elements in emc to 0
    for (i = start; i < cardinality; i++){
        word = assigned_var->domain_stack->domain_3d[i]; //Trying the i'th word of the domain
        if (neighbour_update(assigned_var, word, variables, dict, emc, emc_num, limit)) //Updating the info of the neighbours
        {     
            assigned_var->backjump_count = i;
            return i;
        }
    }
    assigned_var->backjump_count = i;
    return -1;
}

char** CSP(var **assignments, var* var_v, var* var_h, int amount_var_h, int amount_var_v, char **grid, char **dict)
{
    int i, j, cardinality, word_index, limit;
    int cbj, first_letter_index, force_change = 0;
    int i_prev, *emc, emc_num;
    char **word_assignments;
    var *other_variables, *current_var, *force_var;
    i = 0;
    limit = amount_var_h + amount_var_v; 
    if ((word_assignments = malloc(limit * sizeof(char *))) == NULL) 
    {
        exit(-1);
    }
    force_var = MRV(var_h, var_v, amount_var_h, amount_var_v); //The variable that first gets assigned will be chosen by the MRV heuristic
    force_change = 1; //This flag determines whether I am forced to change a variable without implementing MRV to choose
                      //Usually using this when backtracking
    cardinality = force_var->cardinality_stack->value;
    cbj = 0; //Flag that has the value 1 when I need to backjump and 0 when I don't need to backjump
    if ((emc = malloc(limit * sizeof(int))) == NULL){
        exit(-1);
    }
    do  
    {
        emc_num = 0; //This variable keeps the amount of elements that are stored in the 
                     //earliest minimal conflict set (emc) of a variable
                     //I initialize it to 0 now but I only use it when backjumping                     
        for (j=0; j < limit; j++){
            emc[j] = -1;
        }
        if (force_change){
            current_var = force_var; 
            force_change = 0;
        }
        else{
            current_var = MRV(var_h, var_v, amount_var_h, amount_var_v); //Choosing the next variable to assign based on the MRV Heuristic
        }
        current_var->level = i;
        other_variables = (current_var->type) ? var_v : var_h;
        if ((word_index = check_forward(current_var, other_variables, dict, &emc, &emc_num, limit)) > -1) //Performing forward check
        {
            current_var->is_assigned = 1;
            assignments[i] = current_var;
            word_assignments[i] = current_var->domain_stack->domain_3d[word_index];           
            i++;            
        }
        else{ //If word_index == -1, I need to backjump because there weren't words 
            i_prev = i;
            current_var->conf_num = unite(current_var->conf_set, emc, current_var->conf_num, emc_num);  //Uniting the earliest minimal conflict set (emc) and
                                                                                                        //the conflict set of the variable whose forward check failed
            if (!current_var->conf_num){
                printf("Didn't find a solution for the crossword\n");
                exit(-1);
            }
            else{ 
                i = max_of_set(current_var->conf_set, current_var->conf_num); //Finding the max element of the new conflict set for the variable whose forward check failed
                (current_var->conf_num)--; //Updating conf_num
            }
            assignments[i]->conf_num = unite(assignments[i]->conf_set, current_var->conf_set,
            assignments[i]->conf_num, current_var->conf_num);   //Uniting the conflict set of the variable I will backjump to and the
                                                                //conflict set of the variable whose forward check failed
            cbj = 1;
        }
        if (cbj){ 
            for (j = i_prev - 1; j > i; j--){ 
                assignments[j]->is_assigned = 0; //Changing the is_assigned status to 0 for every variable
                                                 //that was assigned after the variable I backjumped to
                                                 //This change is enough because the rest of the resets will happen in fix_variables_info
            }  
            fix_variables_info(var_h, amount_var_h, i); //Firstly calling the function to reset the horizontal variables
            fix_variables_info(var_v, amount_var_v, i); //And then to reset the vertical variables
            assignments[i]->is_assigned = 0; //Now I am changing the the is_assigned status to 0 for the variable I backjumped to
                                             //since I have to have it forward check its neighbours again with a new word
            (assignments[i]->backjump_count)++; //Incrementing the backjump_count. This is important because it decides the word 
                                                //that will attempt to be assigned to the variable
            cbj = 0; 
            force_change = 1; //I want to force the program to change the variable to be assigned to 1 wiht
            force_var = assignments[i];  
        }
    }
    while ((assignments[0]->backjump_count < cardinality) && (i < limit)); 
    free(emc);
    if ((i < limit) || (assignments[0]->backjump_count == cardinality)){ 
        printf("Didn't find a solution for the crossword\n");
        return NULL; //When the program doesn't find a solution I return NULL so I can know it failed
    }
    for (i = 0; i < limit; i++)
    {
        first_letter_index = assignments[i]->last_different_line_covered - assignments[i]->length + 1;

        //Initializing first_letter_index to the position at which the i'th word begins with

        for (j = first_letter_index; j <= assignments[i]->last_different_line_covered; j++){ 
            if (assignments[i]->type) grid[assignments[i]->line][j] = word_assignments[i][j - first_letter_index]; 
            else grid[j][assignments[i]->line] = word_assignments[i][j - first_letter_index];
        }
    }
    return word_assignments; //When the program succeeds in solving the crossword I return the words assigned
}

void find_neighbours(var* variables, var* other_var, int var_amount, int other_var_amount){

    /*This function finds the neighbours of every variable. As neighbours of a variable I define the 
      all variables that intersect with that variable at some position (that holds a letter
      when the variable is assigned) of the slots that the variable covers*/

    int neighbours_amount, *temp_indexes, i, j, *temp_intersections;
    for (i=0; i < var_amount; i++)
    {
        neighbours_amount = 0;
        if ((temp_indexes = malloc(other_var_amount * sizeof(int))) == NULL ||
            (temp_intersections = malloc(other_var_amount * sizeof(int))) == NULL)
        {
            exit(-2);
        } 
        for (j = 0; j < other_var_amount; j++)
        { 
            if ((other_var[j].line < variables[i].last_different_line_covered - variables[i].length + 1) ||
                (other_var[j].line  > variables[i].last_different_line_covered)) //I only care about the variables that lie between the position of the
                                                                                // first letter of the variables[i] and that of its last letter
            {
                continue; 
            }
            if ((other_var[j].last_different_line_covered + 1 - other_var[j].length <= variables[i].line) &&
            (other_var[j].last_different_line_covered >= variables[i].line))
            {
                temp_indexes[neighbours_amount] = j;
                temp_intersections[neighbours_amount++] = other_var[j].line - (variables[i].last_different_line_covered - variables[i].length + 1);
            }
        }
        if ((variables[i].neighbours = malloc(neighbours_amount * sizeof(int))) == NULL)
        {
            exit(-2);
        } 
        if ((variables[i].intersections = malloc(neighbours_amount * sizeof(int *))) == NULL){
            exit(-2);
        }
        for (j=0; j < neighbours_amount; j++)
        {
            variables[i].neighbours[j] = temp_indexes[j];
            if ((variables[i].intersections[j] = malloc(2 * sizeof(int))) == NULL){ //Each variable keeps the position (from the variables point of view) in                                                                                                  
                exit(-2);                                                           //which it intersects withits neighbour, as well as the position from
            }                                                                       //the neighbour's point of view
            variables[i].intersections[j][0] = temp_intersections[j];                                                
        }
        variables[i].neighbours_amount = neighbours_amount;
        free(temp_indexes);
        free(temp_intersections);
    }
}

void find_variables(var **var_h, var **var_v, char **grid, int dimension, int *count_h_var, int *count_v_var){
    int i, j, total, count_h, count_v;
    int temp_amount_h, temp_amount_v;
    var *temp_var_h, *temp_var_v;
    *count_h_var = 0;
    *count_v_var = 0;
    if ((temp_var_h = malloc(dimension * dimension * sizeof(var))) == NULL ||
           (temp_var_v = malloc(dimension * dimension * sizeof(var))) == NULL)  //The amount of variables is for sure less than dimension*dimension because 
    {                                                                           // variables have length > 1 
        exit(-1);
    } 
    count_v = 0;
    count_h = 0;
    temp_amount_h = 0;
    temp_amount_v = 0;
    for (i = 0; i < dimension; i++)
    {
        for (j = 0; j < dimension; j++)
        {
            if (grid[i][j] != '#')
            {
                count_h++; 
            }
            if (grid[j][i] != '#')
            {
                count_v++;
            }
            if (j == dimension - 1 || grid[i][j] == '#')
            {
                if (count_h > 1) 
                {               
                    temp_var_h[temp_amount_h].index = temp_amount_h;
                    temp_var_h[temp_amount_h].length = count_h;  
                    temp_var_h[temp_amount_h].line = i;    /*This represents the row or column the variable covers. Obviously for horizontal 
                                                            variables line is the row it covers and for vertical variables line is the column it covers*/
                    temp_var_h[temp_amount_h].last_different_line_covered = (grid[i][j] == '#') ? j - 1 : j;  /* I need this
                                                             information to know the exact positon of the variable, if the variable is
                                                            horizontal for example, I need the row (line) it covers as well as the last column (different line) 
                                                            it occupies. I can find the exact position each variable because I scan the grid from left to right
                                                            and from top to bottom */ 
                    temp_var_h[temp_amount_h].is_assigned = 0;
                    temp_var_h[temp_amount_h].type = 1;   //Type = 1 => Variable is horizontal
                    temp_var_h[temp_amount_h].backjump_count = 0; 
                    temp_var_h[temp_amount_h].conf_num = 0;
                    temp_var_h[temp_amount_h].level = 0;   //level corresponds to the order in which the variable was assigned a value
                    temp_var_h[temp_amount_h].pastfc = NULL;
                    temp_var_h[temp_amount_h].domain_stack = NULL;
                    temp_var_h[temp_amount_h].cardinality_stack = NULL;
                    temp_amount_h++;
                }
                count_h = 0;
            }
            if (j == dimension - 1 || grid[j][i] == '#')     
            {   
                if (count_v > 1)
                {
                    temp_var_v[temp_amount_v].index = temp_amount_v;
                    temp_var_v[temp_amount_v].length = count_v;
                    temp_var_v[temp_amount_v].line = i;
                    temp_var_v[temp_amount_v].last_different_line_covered = (grid[j][i] == '#') ? j - 1 : j;
                    temp_var_v[temp_amount_v].is_assigned = 0;
                    temp_var_v[temp_amount_v].type = 0;   //Type = 0 => Variable is vertical
                    temp_var_v[temp_amount_v].backjump_count = 0;
                    temp_var_v[temp_amount_v].conf_num = 0;
                    temp_var_v[temp_amount_v].level = 0;  
                    temp_var_v[temp_amount_v].pastfc = NULL;
                    temp_var_v[temp_amount_v].domain_stack = NULL;
                    temp_var_v[temp_amount_v].cardinality_stack = NULL;
                    temp_amount_v++;
                } 
                count_v = 0;
            }   
        }   
    }
    if ((*var_h = malloc(temp_amount_h * sizeof(var))) == NULL || (*var_v = malloc(temp_amount_v * sizeof(var))) == NULL)  //The amount of variables is for sure less than dimension*dimension
    {
        exit(-1);
    }
    memcpy(*var_h, temp_var_h, temp_amount_h * sizeof(var));
    memcpy(*var_v, temp_var_v, temp_amount_v * sizeof(var));
    total = temp_amount_h + temp_amount_v; 

    /* I need to allocate memory for the conflict sets of the variables.
       The conflict set of a variable cannot possibly contain more
       than total - 1 levels, so allocating total * sizeof(int) bytes is safe */
    

    for (i = 0; i < temp_amount_h; i++){
        if ((var_h[0][i].conf_set = malloc(total * sizeof(int))) == NULL) 
        {
            exit(-1);
        }
    } 
    for (i = 0; i < temp_amount_v; i++){
        if ((var_v[0][i].conf_set = malloc(total * sizeof(int))) == NULL) 
        {
            exit(-1);
        }
    }
    *count_h_var = temp_amount_h;
    *count_v_var = temp_amount_v;
    free(temp_var_h);
    free(temp_var_v);
}

void find_domain_of_variables(var *variables, int *length, int amount_var, int count_total_words, char **dict)
{   
    int *temp_words_index, i, j, count_words, cardinality; 
    char **domain;
    if ((temp_words_index = malloc(count_total_words * sizeof(int))) == NULL){
        exit(-1);
    }
    for (i = 0; i < amount_var; i++)
    {
        count_words = 0;
        for (j = 0; j < count_total_words; j++) 
        {
            if (variables[i].length == length[j]) //The variable's domain only can contain words of the same length as the variable
            {
                temp_words_index[count_words] = j; //Keeping the index of the words of the same length as the variable to 
                                                   //make the domain of the variable consist of only those words
                count_words++;
            }        
        }
        cardinality = count_words;
        if ((domain = malloc(cardinality * sizeof(char *))) == NULL)
        {
            exit(-1);
        }
        for (j = 0; j < cardinality; j++)
        { 
            domain[j] = dict[temp_words_index[j]];  //Copying the domain
        }
        push_3d(&(variables[i].domain_stack), domain); 
        push(&(variables[i].cardinality_stack), cardinality); 
    }
    free(temp_words_index);
}

solutions solve_crossword(char **grid, char **dict, int *length, int count_words, int dimension){
    int amount_var_h, amount_var_v, i, j, k, limit;
    var **assignments, *var_h, *var_v;
    char **word_assignments;
    solutions assigned_words;
    find_variables(&var_h, &var_v, grid, dimension, &amount_var_h, &amount_var_v); /*This function finds many of the basic info
                                                                                    of the variables*/
    find_domain_of_variables(var_h, length, amount_var_h, count_words, dict); /* This function finds the original domainand cardinality
                                                                                 of a variable and pushes it and I call it twice, once for
                                                                                 each type of variable */
    find_domain_of_variables(var_v, length, amount_var_v, count_words, dict);
    find_neighbours(var_h, var_v, amount_var_h, amount_var_v); /* This functions finds the amount of neighbours of every variable, the indexes
                                                                  of the neighbours of every variable as well as the positions
                                                                  in which it intersects its neighbours (in according to the variable's slots)
                                                                  as well as the position in which a neighbour intersects it (according to the
                                                                  neighbour's slots. Those intersections are stored in a 2d int array called intersecitons*/
    find_neighbours(var_v, var_h, amount_var_v, amount_var_h); //This function is called twice. Once for each type of variables (horizontal/vertical)
    for(i = 0; i < amount_var_h; i++){
        for(j = 0; j < var_h[i].neighbours_amount; j++){ 
            for (k=0; k < var_v[var_h[i].neighbours[j]].neighbours_amount; k++){
                if (var_v[var_h[i].neighbours[j]].neighbours[k] == var_h[i].index){
                    var_v[var_h[i].neighbours[j]].intersections[k][1] = var_h[i].intersections[j][0];
                    var_h[i].intersections[j][1] = var_v[var_h[i].neighbours[j]].intersections[k][0];
                    break;
                }
            }
        }
    }
    for(i = 0; i < amount_var_v; i++){
        for(j = 0; j < var_v[i].neighbours_amount; j++){
            for (k=0; k < var_h[var_v[i].neighbours[j]].neighbours_amount; k++){
                if (var_h[var_v[i].neighbours[j]].neighbours[k] == var_v[i].index){
                    var_h[var_v[i].neighbours[j]].intersections[k][1] = var_v[i].intersections[j][0];
                    var_v[i].intersections[j][1] = var_h[var_v[i].neighbours[j]].intersections[k][0];
                    break;
                }
            }
        }
    }
    if ((assignments = malloc((amount_var_h + amount_var_v) * sizeof(var *))) == NULL) //I need this array to store the variables that solve the crossword.
    {                                                                                 //The reason I use sizeof(var *) instead of sizeof(var) is because
        exit(-1);                                                                     //when solving the crossword, I want to use the array to make changes
    }                                                                                 //to the variables of the arrays var_h and var_v. This can be done only if
                                                                                      //i save the addresses of the variables instead of the variables

    word_assignments = CSP(assignments, var_v, var_h, amount_var_h, amount_var_v, grid, dict);  /* The function CSP is the function that solves the crossword 
                                                                                                   and it returns a char**. If the search for a solution succeeds
                                                                                                   it returns an array that contains the strings that have been 
                                                                                                   assigned to each variable to solve the crossword.
                                                                                                   If the search for a solution fails, it returns NULL.
                                                                                                   The function is named CSP because this program solves
                                                                                                   a Constraint Satisfaction Problem */

    for (i = 0; i < amount_var_h; i++){         //It is time to free the memory allocated for solving the crossword                                                           
        while (pop(&(var_h[i].cardinality_stack)));
        while (pop_3d(&(var_h[i].domain_stack)));
        while (pop(&(var_h[i].pastfc)));
        free(var_h[i].conf_set);
        for(j = 0; j < var_h[i].neighbours_amount; j++){
            free(var_h[i].intersections[j]);
        }
        free(var_h[i].intersections);
        free(var_h[i].neighbours);
    }
    for (i = 0; i < amount_var_v; i++){     
        while (pop(&(var_v[i].cardinality_stack)));
        while (pop_3d(&(var_v[i].domain_stack)));
        while (pop(&(var_v[i].pastfc)));
        free(var_v[i].conf_set);
        for(j = 0; j < var_v[i].neighbours_amount; j++){
            free(var_v[i].intersections[j]);
        }
        free(var_v[i].intersections);
        free(var_v[i].neighbours);
    }
    if (word_assignments == NULL) exit(-1); 
    limit = amount_var_h + amount_var_v;
    /* Now, i need to return the important info of the solution to the function that called this function.
       What I need to return is:
       1) The amount of variables the crossword has
       2) The words that have been assigned
       3) The type of each variable (1 or 0 based on whether it is horizontal or vertical) */
         
    if ((assigned_words.types = malloc(limit * sizeof(int))) == NULL){ 
        exit(-1);
    }
    if ((assigned_words.words = malloc(limit * sizeof(char *))) == NULL){
        exit(-1);
    }
    for (i = 0; i < amount_var_h; i++){
        if ((assigned_words.words[i] = malloc((var_h[i].length + 1) * sizeof(char))) == NULL){
            exit(-1);
        }
        strcpy(assigned_words.words[i], word_assignments[var_h[i].level]);
        assigned_words.types[i] = var_h[i].type;
    }
    for (i = 0; i < amount_var_v; i++){
        if ((assigned_words.words[amount_var_h + i] = malloc((var_v[i].length + 1) * sizeof(char))) == NULL){
            exit(-1);
        }
        strcpy(assigned_words.words[amount_var_h + i], word_assignments[var_v[i].level]);
        assigned_words.types[amount_var_h + i] = var_v[i].type;
    }
    assigned_words.amount = limit;
    free(word_assignments);  /* I do not need to free each string in word_assignments before freeing the original 
                                pointer because the strings will be freed when I free all the strings of the dictionary
                                in the function that called this function */

    free(assignments);     //I can now free assignments, var_h and var_v
    free(var_h);
    free(var_v);
    return assigned_words;
}

