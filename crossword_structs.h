/* All structs from node_3d down to var have to do with the function 
   that solves the crossword. Meaning they are useless to the main 
   part of the program and the function that checks whether a solution
   to a crossword is correct*/ 

typedef struct node_3d{
    char **domain_3d;
    struct node_3d *next_3d;
} node_3d;

typedef struct node{
    int value;
    struct node *next;
} node;

typedef node_3d *stack_3d;
typedef node *stack;

typedef struct var{
    int index;
    int type;  // 1 means horizontal and 0 means vertical
    int length;
    stack_3d domain_stack;
    stack cardinality_stack;
    stack pastfc;
    int line;  
    int last_different_line_covered;  
    int neighbours_amount;
    int *neighbours;
    int is_assigned;   //This is a flag that tells me whether a variable is assigned or not
    int **intersections;
    int *conf_set;
    int conf_num;
    int level;
    int backjump_count;
} var;

/*The struct solutions is used to transfer all the necessary info
  about the solution to the function that called the 
  function that solves the crossword */

typedef struct solutions{
    int amount;
    char **words;
    int *types;
} solutions;


/* This struct is used in the function that
   checks whether a given solution is correct
   (when given -check as an argument) */

typedef struct check_var{ 
    int length;
    int line;
    int last_different_line_covered;
} check_var;