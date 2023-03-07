#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crossword_structs.h"
int check(char **grid, char **dict, int count_total_words, int dimension){
    char **solutions;
    int count, i, j, count_h, count_v, count_h_var, count_v_var, limit, found, start;
    check_var *temp_var_h, *temp_var_v, *var_h, *var_v; 

    if ((temp_var_h = malloc(dimension * dimension * sizeof(check_var))) == NULL ||
          (temp_var_v = malloc(dimension * dimension * sizeof(check_var))) == NULL) 
    {                                                   //The amount of variables is surely less than dimension*dimension due
        exit(-1);                                       //to the fact that every variable is of length > 1
    } 
    count = 0;
    count_h = 0;
    count_v = 0;
    count_h_var = 0;
    count_v_var = 0;   
    
    /* I need to find all the variables of the crossword to
       use some important info. These variables are not the same as
       the variables used in the function that solves the crossword,
       they are of type check_var because I need them to store less
       info than I do when solving the crossword */ 
    
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
                    temp_var_h[count_h_var].length = count_h;  
                    temp_var_h[count_h_var].line = i;
                    temp_var_h[count_h_var].last_different_line_covered = (grid[i][j] == '#') ? j - 1 : j;
                    (count_h_var)++;
                }
                count_h = 0;
            }
            if (j == dimension - 1 || grid[j][i] == '#')     
            {   
                if (count_v > 1)
                {
                    temp_var_v[count_v_var].length = count_v;
                    temp_var_v[count_v_var].line = i;
                    temp_var_v[count_v_var].last_different_line_covered = (grid[j][i] == '#') ? j - 1 : j;
                    (count_v_var)++;
                } 
                count_v = 0;
            }   
        }   
    }
    limit = count_h_var + count_v_var;
    if ((var_h = malloc(count_h_var * sizeof(check_var))) == NULL || (var_v = malloc(count_v_var * sizeof(check_var))) == NULL){
        exit(-1);
    }
    if ((solutions = malloc((limit + 1) * sizeof(char *))) == NULL){  /*This char** array will store the words 
                                                                  that are read from stdin. I givit size of
                                                                  I gave it size of (limit + 1) * size of char* because
                                                                  I need to be able to store at least one more word
                                                                  in the case of the user giving me more words than needed*/
        exit(-1);
    }
    for (i = 0; i < count_h_var; i++){
        var_h[i] = temp_var_h[i];
    }
    for (i=0; i < count_v_var; i++){
        var_v[i] = temp_var_v[i];
    }
    free(temp_var_h);
    free(temp_var_v);
    count = 0;
    if ((solutions[count] = malloc(82 * sizeof(char))) == NULL){
        exit(-1);
    }
    while ((scanf("%81s", solutions[count]) == 1)){
        if (count == limit){    /*If count was limit - 1 and the program didn't exit
                                  the crossword had a solution but since the program read 
                                  one more word, now there are more words than needed */
            printf("More words than needed\n");  
                free(var_h);
                free(var_v);
                for (i = 0; i <= count; i++){ //I always allocate memory for solutions[count] so thats why my for condition uses <= instead of <
                    free(solutions[i]);  
                }
                free(solutions);
                return 0;
        }
        if (count < count_h_var){
            if (strlen(solutions[count]) != var_h[count].length){  //If the word I read isn't of the same length as the variable
                printf("Word \"%s\" cannot be placed\n", solutions[count]);  //it obviously cannot be placed in the crossword
                free(var_h);
                free(var_v);
                for (i = 0; i <= count; i++){
                    free(solutions[i]);
                }
                free(solutions);
                return 0;
            }
        }
        else{
            if (strlen(solutions[count]) != var_v[count - count_h_var].length){
                printf("Word \"%s\" cannot be placed\n", solutions[count]);
                free(var_h);
                free(var_v);
                for (i = 0; i <= count; i++){
                    free(solutions[i]);
                }
                free(solutions);
                return 0;
            }
        }
        found = 0;
        for (i = 0; i < count_total_words; i++){
            if (!strcmp(dict[i], solutions[count])){
                found = 1;
                break;
            }
        }
        if (!found){ //The program didn't find the word in the dictionary
            printf("Word \"%s\" not in dictionary\n", solutions[count]);  
            free(var_h);                                                  
            free(var_v);
            for (i = 0; i <= count; i++){
                free(solutions[i]);
            }
            free(solutions);
            return 0;
        }
        if (count < count_h_var)
        {
            for(j = start = var_h[count].last_different_line_covered - var_h[count].length + 1;
             j <= var_h[count].last_different_line_covered; j++){
                grid[var_h[count].line][j] = solutions[count][j - start];
            }
        }
        else{
            for(j = start = var_v[count - count_h_var].last_different_line_covered - var_v[count - count_h_var].length + 1;
             j <= var_v[count - count_h_var].last_different_line_covered; j++){ 
                if (grid[j][var_v[count - count_h_var].line] == '0'){   //If the slot is empty I can place the (j - start)th of letter of 
                    grid[j][var_v[count - count_h_var].line] = solutions[count][j - start];  //solutions[count] in the grid
                }
                else{   
                    if (grid[j][var_v[count - count_h_var].line] != solutions[count][j - start]){  //Since the slot is occupied I need to check if
                        printf("Word \"%s\" cannot be placed\n", solutions[count]);           //solutions[count] does not have the same letter at the position of the slot  
                        free(var_h);
                        free(var_v);
                        for (i = 0; i <= count; i++){
                            free(solutions[i]);
                        }
                        free(solutions);
                        return 0;
                    }
                }
            }
        }
        found = 0;
        count++;
        if ((solutions[count] = malloc(82 * sizeof(char))) == NULL){
            exit(-1);
        }
    }
    if (count < limit){
        printf("Not enough words\n");
        for (i = 0; i <= count; i++){
            free(solutions[i]);
        }
        free(solutions);
        free(var_h);
        free(var_v);
        return 0;
    }
    free(var_h);
    free(var_v);
    for (i = 0; i <= count; i++){
        free(solutions[i]);
    }
    free(solutions);
    return 1;
}
