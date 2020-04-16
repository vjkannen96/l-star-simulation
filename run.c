#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_LENGTH 10
#define TABLE_OVERFLOW 10
#define TABLE_MAX (MAX_LENGTH + TABLE_OVERFLOW)

#define ANALYSIS 1
#define DEBUG 0
#define SINGLE_RUN 0
/*
prints the observation table with row and column headers
blank spaces are "-1" i.e. unassigned
*/
int print_observation_table(int observation_table[TABLE_MAX + 1][TABLE_MAX + 1]){
    printf("   ");
    for (int i = 0; i <= TABLE_MAX; i++){
        printf("%2i ", i);
    }
    printf("\n");
    for (int i = 0; i <= TABLE_MAX; i++){
        printf("%2i ", i);
        for(int j = 0; j < TABLE_MAX; j++){
            if(observation_table[i][j] == -1){
                printf("   ");
            }
            else{
                printf("%2i ", observation_table[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n");
    return 0;
}

/*
calculates(arg)log2(arg) returns 0 if arg = 0;
*/ 
double mutual_info_log(double arg){
    //printf("mutual info log of %f = ", arg);
    if(arg == 0){
        //printf("0\n");
        return 0.0;
    }
    else{
        double result = arg * log2l(arg);
        //printf("%f\n", result);
        return result;
    }
}

/*
calculates the mutual info between the teacher and learner using the following equation where,
a = true negative probability
b = false positive probability
c = true positive probability
d = false negative probability
mutual_info = 
-(b+c)log2(b+c) - (a+d)log2(a+d) - (a+b)log2(a+b) - (c+d)log2(c+d) + alog2a + blog2b + clog2c + dlog2d
*/
double mutual_info(double true_negative, double false_positive, double true_positive, double false_negative, double total){
    double a = true_negative/total;
    double b = false_positive/total;
    double c = true_positive/total;
    double d = false_negative/total;
    double result = (0 - mutual_info_log(b+c) - mutual_info_log(a+d) - mutual_info_log(a+b) - mutual_info_log(c+d)
     + mutual_info_log(a) + mutual_info_log(b) + mutual_info_log(c) + mutual_info_log(d));
    return result;
}
/*
checks if the observation table is closed.
The table is considered closed if every row in S.A, there is a identical row in S
*/
int closed(int observation_table[TABLE_MAX + 1][TABLE_MAX + 1], int current_max_row, int current_max_col){
    //check whether each row in S is identical to the row in S.A
    // printf("current_max_row = %d\n", current_max_row);
    // printf("current_max_col = %d\n", current_max_col);
    for (int i = 0; i <= current_max_row; i++){
        int flag = 0;
        for(int j = 0; j <= current_max_col; j++){
            if(observation_table[i][j] != observation_table[current_max_row + 1][j]){
                flag = 1;
            }
        }
        //if it is identical, return true
        if(flag == 0){
            return 1;
        }
    }
    //else return true
    return 0;
}

/*
checks if the table is consistent.
The table is considered consistent if for every pair of identical rows in S, i and j, rows (i+i) and (j+1) are also identical
*/
int consistent(int observation_table[TABLE_MAX + 1][TABLE_MAX + 1], int current_max_row, int current_max_col){
    //for each row in S
    for(int i = 0; i < current_max_row; i++){
        //check all other rows in S
        for(int k = i + 1; k <= current_max_row; k++){
            int flag = 1;
            for(int j = 0; j <= current_max_col; j++){
                if(observation_table[i][j] != observation_table[k][j]){
                    flag = 0;
                }
            }
            //if rows are identical, ensure that the subsequent row for both rows is also identical
            if (flag){
                int flag = 1;
                for(int j = 0; j <= current_max_col; j++){
                    if(observation_table[i+1][j] != observation_table[k+1][j]){
                        flag = 0;
                    }
                }
                //if subsequent rows are not identical, return false
                if(!flag){
                    return 0;
                }
            }
        }
    }
    //else return true
    return 1;
}

/*
queries the teacher if strings of a certain length are accepted.
strings longer than MAX_LENGTH are rejected by default.
returns 1 if string is accepted, 0 if not.
*/
int membership_query(int* teacher, int query, int length){
    if(query > length){
        return 0;
    }
    else{
        return teacher[query];
    }
}

/*
checks  if the observation table produced by the learner matches the same set of strings as the teacher.
counts number of true positive, false positive, true negative and false positive.
returns 0 if it matches, returns a counterexample if it does not. 
counterexample can never be 0 since it is always queried at the beginning of the algorithm
*/
int check_equivalence(int observation_table[TABLE_MAX + 1][TABLE_MAX + 1], int current_max_row, int current_max_col, int teacher[MAX_LENGTH + 1], double* mutual_information){
    int table_row = 0;
    int false_positive = 0;
    int false_negative = 0;
    int true_positive = 0;
    int true_negative = 0;
    int counterexample = 0;
    printf("LEARNER ACCEPTS: ");
    for (int i = 0; i <= MAX_LENGTH; i++){
        // printf("table row = %i\n", table_row);
        if(observation_table[table_row][0] == teacher[i]){
            if(teacher[i] == 0){
                true_negative++;
            }
            else{
                true_positive++;
                printf("%2i ", i);
            }
        }
        else{
            if(teacher[i] == 0){
                false_positive++;
                printf("%2i ", i);
                if(counterexample == 0){
                    counterexample = i;
                }
            }
            else{
                false_negative++;
                if(counterexample == 0){
                    counterexample = i;
                }
            }
        }
        if(table_row == current_max_row){
            //move to appropriate row
            for(int a = 0; a <= current_max_row; a++){
                int row_match_flag = 1;
                for(int b = 0; b <= current_max_col; b++){
                    if(observation_table[a][b] != observation_table[current_max_row+1][b]){
                        row_match_flag = 0;
                    }
                }
                if(row_match_flag == 1){
                    table_row = a;
                    break;
                }
            }
        }
        else{
            table_row++;
        }
    }
    printf("\n");

    #if ANALYSIS
        printf("TRUE POSITIVE: %2i/%2i\n", true_positive, MAX_LENGTH+1);
        printf("FALSE POSITIVE: %2i/%2i\n", false_positive, MAX_LENGTH+1);
        printf("TRUE NEGATIVE: %2i/%2i\n", true_negative, MAX_LENGTH+1);
        printf("FALSE NEGATIVE: %2i/%2i\n", false_negative, MAX_LENGTH+1);
    #endif
    
    double new_mutual_information = 
        mutual_info(true_negative, false_positive, true_positive, false_negative, MAX_LENGTH + 1);
    printf("MUTUAL INFO = %f\n", new_mutual_information);
    printf("CHANGE IN MUTUAL INFO = %f\n", new_mutual_information - *mutual_information);
    *mutual_information = new_mutual_information;
    printf("bd-ac = %f\n", 
        (double)((false_positive * false_negative) - (true_negative * true_positive))/(MAX_LENGTH + 1));
    if(false_positive == 0 && false_negative == 0){
        return 0;
    }
    else{
        #if ANALYSIS
            printf("COUNTEREXAMPLE = %2i\n", counterexample);
        #endif

        return counterexample;
    }
}

void lstar(int* array, int length){
    printf("TEACHER ACCEPTS: ");
    for(int i = 0; i < length; i++){
        if(array[i] == 1){
            printf("%2d ", i);
        }
    }
    printf("\n");
    
    /*
    initialisation of observation table. 
    -1 for unassigned values
    0 for false
    1 for true
    */
    int observation_table[TABLE_MAX + 1][TABLE_MAX + 1];
    for(int i = 0; i <= TABLE_MAX; i++){
        for(int j = 0; j <= TABLE_MAX; j++){
            observation_table[i][j] = -1;
        }
    }
    observation_table[0][0] = membership_query(array, 0, length);
    int current_max_row = 0;
    int current_max_col = 0;
    observation_table[current_max_row + 1][0] = membership_query(array, current_max_row + 1 + 0, length);
    
    #if DEBUG
        print_observation_table(observation_table);
    #endif

    //main loop
    double mutual_information = 0.0;
    int matched_teacher= 0;
    while(!matched_teacher){
        int consistent_flag = consistent(observation_table, current_max_row, current_max_col);
        int closed_flag = closed(observation_table, current_max_row, current_max_col);
        while(!(closed_flag && consistent_flag)){
            consistent_flag = 0;
            closed_flag = 0;
            if(!consistent(observation_table, current_max_row, current_max_col)){
                // printf("NOT CONSISTENT\n");
                if(current_max_col < TABLE_MAX){
                    current_max_col++;
                }
                else{
                    printf("\n***\nEXCEEDED MAX COLUMNS\n***\n");
                    return;
                }
                for(int i=0; i <= current_max_row + 1; i++){
                    observation_table[i][current_max_col] = membership_query(array, i + current_max_col, length);
                }
                // #if ANALYSIS
                //     print_observation_table(observation_table);
                // #endif
            }
            else{
                consistent_flag = 1;
            }
            if(!closed(observation_table, current_max_row, current_max_col)){
                // printf("NOT CLOSED\n");
                if(current_max_row < TABLE_MAX){
                    current_max_row++;
                }
                else{
                    printf("\n***\nEXCEEDED MAX ROWS\n***\n");
                    return;
                }
                // printf("current_max_row = %d\n", current_max_row);
                for(int j = 0; j<= current_max_col; j++){
                    observation_table[current_max_row + 1][j] = membership_query(array, current_max_row + 1 + j, length);
                }
                // #if ANALYSIS
                //     print_observation_table(observation_table);
                // #endif
            }
            else{
                closed_flag = 1;
            }
        }
        #if DEBUG
            print_observation_table(observation_table);
        #endif
        int counterexample = check_equivalence(observation_table, current_max_row, current_max_col, array, &mutual_information);
        if(counterexample == 0){
            matched_teacher = 1;
            // printf("MATCHED TEACHER\n");
            // #if ANALYSIS
            //     print_observation_table(observation_table);
            // #endif
        }
        else{
            // printf("DID NOT MATCH TEACHER\n");
            if(counterexample < MAX_LENGTH - 1){
                current_max_row = counterexample;
            }
            else{
                current_max_row = MAX_LENGTH - 1;
            }
            for(int i = 0; i <= current_max_row + 1; i++){
                for(int j = 0; j <= current_max_col; j++){
                    observation_table[i][j] = membership_query(array, i+j, length);
                }
            }
            // #if ANALYSIS
            //     print_observation_table(observation_table);
            // #endif
        }

    }
    printf("\n");
}

void tryAllSets(int* array, int index, int length){
    if(index == length - 1){
        array[index] = 0;
        lstar(array, length);
        array[index] = 1;
        lstar(array, length);
        return;
    }
    else{
        array[index] = 0;
        tryAllSets(array, index + 1, length);
        array[index] = 1;
        tryAllSets(array, index + 1, length);
    }
}

int main(void){
     /*
    initialisation of teacher.
    0 for strings not in language
    1 for strings in language.
    */
    int* teacher;
    teacher = (int *)malloc((MAX_LENGTH + 1) * sizeof(int));
    
    /*
    use this if you want to try all possible sets
    */
    #if !SINGLE_RUN
        tryAllSets(teacher, 0, MAX_LENGTH + 1);
    #endif
    
    
    /*
    use this if you want to test a particular set
    */
    #if SINGLE_RUN
        for(int i = 0; i < MAX_LENGTH + 1; i++){
            teacher[i] = 0;
        }
        teacher[0] = 0;
        teacher[1] = 1;
        teacher[2] = 0;
        teacher[3] = 0;
        teacher[4] = 0;
        teacher[5] = 0;
        teacher[6] = 0;
        teacher[7] = 0;
        teacher[8] = 1;
        teacher[9] = 1;
        teacher[10] = 0;
        teacher[11] = 1;
        teacher[12] = 1;
        teacher[13] = 1;
        teacher[14] = 1;
        teacher[15] = 1;
        teacher[16] = 1;
        teacher[17] = 1;
        teacher[18] = 0;
        teacher[19] = 1;
        teacher[20] = 0;
        
        lstar(teacher, MAX_LENGTH + 1);
    #endif


    free(teacher);
    return 0;
}