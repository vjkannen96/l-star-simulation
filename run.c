#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_LENGTH 10
#define TABLE_OVERFLOW (MAX_LENGTH + 1) //TABLE_OVERFLOW must be this value to detect all FP and FN.
#define TABLE_MAX (MAX_LENGTH + TABLE_OVERFLOW)

#define ANALYSIS 0
#define DEBUG 0
#define SINGLE_RUN 0

int total_runs = 0;
int failed_runs = 0;
int total_steps = 0;
int total_decrease_in_mutual_info = 0;
int decrease_in_mutual_info_good = 0;
int decrease_in_mutual_info_bad = 0;
int good_oracles = 0;
int bad_oracles = 0;
int steps_with_increase_in_fp_and_fn = 0;

FILE *fp;

/*
prints the observation table with row and column headers
blank spaces are "-1" i.e. unassigned
*/
int print_observation_table(int observation_table[TABLE_MAX + 1][TABLE_MAX + 1]){
    fprintf(fp, "   ");
    for (int i = 0; i <= TABLE_MAX; i++){
        fprintf(fp, "%2i ", i);
    }
    fprintf(fp, "\n");
    for (int i = 0; i <= TABLE_MAX; i++){
        fprintf(fp, "%2i ", i);
        for(int j = 0; j < TABLE_MAX; j++){
            if(observation_table[i][j] == -1){
                fprintf(fp, "   ");
            }
            else{
                fprintf(fp, "%2i ", observation_table[i][j]);
            }
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
    return 0;
}

/*
calculates(arg)log2(arg) returns 0 if arg = 0;
*/ 
double mutual_info_log(double arg){
    if(arg == 0){
        return 0.0;
    }
    else{
        double result = arg * log2l(arg);
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
double mutual_info(double a, double b, double c, double d){
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
    //else return false
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
    if(query > length - 1){
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
int check_equivalence(int observation_table[TABLE_MAX + 1][TABLE_MAX + 1], int current_max_row, int current_max_col, int teacher[MAX_LENGTH + 1], double* mutual_information, double* oracle, int* old_false_positives,  int* old_false_negatives){
    int table_row = 0;
    int false_positive = 0;
    int false_negative = 0;
    int true_positive = 0;
    int true_negative = 0;
    int counterexample = 0;
    fprintf(fp, "LEARNER ACCEPTS: ");
    for (int i = 0; i <= TABLE_MAX; i++){
        int teacher_value = membership_query(teacher, i, MAX_LENGTH + 1);
        if(observation_table[table_row][0] == teacher_value){
            if(teacher_value == 0){
                true_negative++;
            }
            else{
                true_positive++;
                fprintf(fp, "%2i ", i);
            }
        }
        else{
            if(teacher_value == 0){
                false_positive++;
                fprintf(fp, "%2i ", i);
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
    fprintf(fp, "\n");

    double a = (double)true_negative/(TABLE_MAX + 1);
    double b = (double)false_positive/(TABLE_MAX + 1);
    double c = (double)true_positive/(TABLE_MAX + 1);
    double d = (double)false_negative/(TABLE_MAX + 1);

    #if ANALYSIS
        fprintf(fp, "TRUE NEGATIVE: %2i/%2i a = %f\n", true_negative, TABLE_MAX + 1, a);
        fprintf(fp, "FALSE POSITIVE: %2i/%2i b = %f\n", false_positive, TABLE_MAX + 1, b);
        fprintf(fp, "TRUE POSITIVE: %2i/%2i c = %f\n", true_positive, TABLE_MAX + 1, c);
        fprintf(fp, "FALSE NEGATIVE: %2i/%2i d = %f\n", false_negative, TABLE_MAX + 1, d);
    #endif
    if(*old_false_positives > false_positive && *old_false_negatives > false_negative){
        steps_with_increase_in_fp_and_fn++;
        #if ANALYSIS
            fprintf(fp, "BOTH FALSE POSITIVES AND NEGATIVES INCREASED FROM PREVIOUS STEP\n");
        #endif
    }

    
    double new_mutual_information = 
        mutual_info(a, b, c, d);
    double new_oracle = (b*d) - (a*c);
    fprintf(fp, "MUTUAL INFO = %f\n", new_mutual_information);
    fprintf(fp, "CHANGE IN MUTUAL INFO = %f\n", new_mutual_information - *mutual_information);
    if(*oracle >= 0){
        bad_oracles++;
    }
    else if(*oracle < 0){
        good_oracles++;
    }
    if(new_mutual_information - *mutual_information < -0.000001){
        total_decrease_in_mutual_info++;
        if(*oracle < 0){
            decrease_in_mutual_info_good++;
            #if ANALYSIS
                fprintf(fp, "DECREASE IN MUTUAL INFO WITH GOOD ORACLE\n");
            #endif
        }
        else{
            decrease_in_mutual_info_bad++;
            #if ANALYSIS
                fprintf(fp, "DECREASE IN MUTUAL INFO WITH BAD ORACLE\n");
            #endif
        }
    }
    *mutual_information = new_mutual_information;
    fprintf(fp, "bd-ac = %f\n", 
        new_oracle);
    *oracle = new_oracle;
    if(false_positive == 0 && false_negative == 0){
        return 0;
    }
    else{
        #if ANALYSIS
            fprintf(fp, "COUNTEREXAMPLE = %2i\n", counterexample);
        #endif

        return counterexample;
    }
}

void lstar(int* array, int length){
    total_runs++;
    fprintf(fp, "TEACHER ACCEPTS: ");
    for(int i = 0; i < length; i++){
        if(array[i] == 1){
            fprintf(fp, "%2d ", i);
        }
    }
    fprintf(fp, "\n");
    
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
    double oracle = 0.0;
    int old_false_positives = 0;
    int old_false_negatives = 0;

    int matched_teacher= 0;
    while(!matched_teacher){
        int consistent_flag = consistent(observation_table, current_max_row, current_max_col);
        int closed_flag = closed(observation_table, current_max_row, current_max_col);
        while(!(closed_flag && consistent_flag)){
            consistent_flag = 0;
            closed_flag = 0;
            if(!consistent(observation_table, current_max_row, current_max_col)){
                current_max_col++;
                if(current_max_col > TABLE_MAX){
                    fprintf(fp, "\n***\nEXCEEDED MAX COLUMNS\n***\n");
                    failed_runs++;
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
                current_max_row++;
                if(current_max_row >= TABLE_MAX){
                    fprintf(fp, "\n***\nEXCEEDED MAX ROWS\n***\n");
                    failed_runs++;
                    return;
                }
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
        total_steps++;
        int counterexample = check_equivalence(observation_table, current_max_row, current_max_col, array, &mutual_information, &oracle, &old_false_positives, &old_false_negatives);
        if(counterexample == 0){
            matched_teacher = 1;
            // #if ANALYSIS
            //     print_observation_table(observation_table);
            // #endif
        }
        else{
            if(counterexample < TABLE_MAX){
                current_max_row = counterexample;
            }
            else{
                current_max_row = TABLE_MAX - 1;
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
    fprintf(fp, "\n");
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

    fp = fopen("analysis.txt", "w+");
    int* teacher;
    teacher = (int *)malloc((MAX_LENGTH + 1) * sizeof(int));
    
    /*
    use this if you want to try all possible sets
    */
    #if !(SINGLE_RUN)
        tryAllSets(teacher, 0, MAX_LENGTH + 1);
    #endif
    
    
    /*
    use this if you want to test a particular set
    */
    #if SINGLE_RUN
        for(int i = 0; i < MAX_LENGTH + 1; i++){
            teacher[i] = 0;
        }
        teacher[0] = 1;
        teacher[1] = 0;
        teacher[2] = 0;
        teacher[3] = 1;
        teacher[4] = 0;
        teacher[5] = 0;
        teacher[6] = 0;
        teacher[7] = 1;
        teacher[8] = 0;
        teacher[9] = 0;
        teacher[10] = 1;
        
        lstar(teacher, MAX_LENGTH + 1);
    #endif

    printf("Total Runs = %d\n", total_runs);
    printf("Failed Runs = %d\n", failed_runs);
    printf("Total Number of Steps = %d\n", total_steps);
    printf("Average Number of Steps = %f\n", (double)total_steps/total_runs);
    printf("Number of Steps with Good Oracle = %d\n", good_oracles);
    printf("Number of Steps with Bad Oracle = %d\n", bad_oracles - total_runs); //we need to exclude the initial steps
    printf("Number of Steps with Decrease in Mutual Info = %d\n", total_decrease_in_mutual_info);
    printf("Number of Steps with Decrease in Mutual Info & Good Oracle = %d\n",decrease_in_mutual_info_good);
    printf("Number of Steps with Decrease in Mutual Info & Bad Oracle = %d\n", decrease_in_mutual_info_bad);
    printf("Number of Steps with Increase in Both False Positives and False Negatives = %d\n", steps_with_increase_in_fp_and_fn);
    free(teacher);
    fclose(fp);
    return 0;
}