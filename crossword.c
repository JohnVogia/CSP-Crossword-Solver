#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crossword.h" //crossword.h also has crossword_structs included in it
int main(int argc, char *argv[]){
    FILE *input_grid, *input_dict;
    char **grid, *word, **dict, lastchar, *dict_name;
    int dimension, length, *lengths, size, count_words;
    int i, j, x, y;
    int check_true = 0, custom_dict = 0, draw = 0, check_outcome;  
    solutions solution;
    for (i = 2; i < argc; i++){
        if (!strcmp(argv[i], "-dict")){ 
            custom_dict = 1;
            if (i == argc - 1){ //Making sure argc is big enough for the dictionary's name to fit
                printf("You didn't give me a dictionary after -dict\n");
                return 1; //The program can't proceed without a dictionary when -dict was an argument
            }
            dict_name = argv[i+1];
            i++; //This (extra incrementation) will skip argv[i+1] since it is just the name of the dictionary and I have already stored it
        }
        else if (!strcmp(argv[i], "-check")) check_true = 1;
        else if (!strcmp(argv[i], "-draw")) draw = 1;
    }
    if ((input_grid = fopen(argv[1], "r")) == NULL){
        return 1;
    } 
    if (!custom_dict){
        if ((input_dict = fopen("Words.txt", "r")) == NULL)
        {
            perror("fopen source-file");
            return 1;
        }
    }
    else if ((input_dict = fopen(dict_name, "r")) == NULL)
    {
        perror("fopen source-file");
        return 1;
    }
    if (fscanf(input_grid, "%d", &dimension) != 1)
    {
        return -1;
    }
    if ((grid = malloc(dimension * sizeof(char *))) == NULL)
    {
        exit(-1);
    }
    for (i = 0; i < dimension; i++)
    {
        if ((grid[i] = malloc(dimension * sizeof(char))) == NULL)
        {
            return 1;
        }
        for (j = 0; j < dimension; j++)
        {
            grid[i][j] = '0';   //character '0' is used to represents non black spots that are currently unassigned
        }
    }
    i=0;
    while (fscanf(input_grid, "%d %d", &x, &y) == 2)
    {
        grid[x-1][y-1] = '#'; //Character '#' represents the black boxes
        i++;
    }
    count_words = 0;
    while(!feof(input_dict))   
    {
        lastchar = fgetc(input_dict);
        if (lastchar == '\n')
        {
            count_words++; //Counting how many lines (which translates to words) exist
        }
    }    
    if ((size = ftell(input_dict)) == -1)
    {
        return 1;
    }
    fseek(input_dict, 0, SEEK_SET);
    if (((dict = malloc(count_words * sizeof(char *))) == NULL) || ((lengths = malloc(count_words * sizeof(int))) == NULL))
    {
        exit(-1);
    }
    if ((word = malloc(82 * sizeof(char))) == NULL)
    {
        exit(-1);
    }
    i = 0;
    while (fscanf(input_dict, "%81s", word) == 1){
        if (((length = strlen(word)) <= dimension) && (strlen(word) > 1))
        {
            if ((dict[i] = malloc((length + 1) * sizeof(char))) == NULL) 
            {
                exit(-1);
            }
            lengths[i] = length;
            strcpy(dict[i], word);
            i++;
        }
    }
    count_words = i;
    free(word);
    if (check_true){
        check_outcome = check(grid, dict, count_words, dimension);
    }
    if (!check_true) solution = solve_crossword(grid, dict, lengths, count_words, dimension);
    if (draw && (!check_true || check_outcome)){
        for (i = 0; i < dimension; i++)
        {
            for (j = 0; j < dimension; j++)
            {
                putchar(grid[i][j] == '#' ? '#' : ' ');
                putchar(grid[i][j]); 
                putchar(grid[i][j] == '#' ? '#' : ' ');
            }
            putchar('\n');
        }
        if (!check_true)
        {
            for (i = 0; i < solution.amount; i++){
                free(solution.words[i]);
            }
            free(solution.types);
            free(solution.words);
        }
    }
    else if (!check_true){
        for (i = 0; i < solution.amount; i++){
            if (solution.types[i]){
                printf("%s\n", solution.words[i]);
            }
        }
        for (i = 0; i < solution.amount; i++){
            if (!solution.types[i]){
                printf("%s\n", solution.words[i]);
            }
        }
        for (i = 0; i < solution.amount; i++){
            free(solution.words[i]);
        }
        free(solution.types);
        free(solution.words);
    }
    for(i = 0; i < count_words; i++){
        free(dict[i]); 
    }
    for (i = 0; i < dimension; i++){
        free(grid[i]);
    }
    free(dict);
    free(grid);
    free(lengths);
    fclose(input_grid);
    fclose(input_dict);
    return 0;
}