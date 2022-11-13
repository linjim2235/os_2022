#include <stdio.h>
#include<malloc.h>

int thread_num = 0;
char *f1;
char *f2;
int **matrix1, **matrix2;
int row1, row2, col1, col2;

int main(int argc, char *argv[]) {
    //scanf("%d", &thread_num);
    f1 = malloc(sizeof(char)*20);
    scanf("%s", f1);
    //scanf("%c", f2);

    FILE *file1;
    file1 = fopen(f1, "r");
    if(file1 == 0){
        printf("Can't open the file\n");
    }
    
    fscanf(file1, "%d %d", &row1, &col1);
    matrix1 = malloc(row1*sizeof(int*));
    for(int i = 0; i < row1; i++){
        matrix1[i] = malloc(sizeof(int)*col1);
    }

    for(int i=0; i < row1; i++){
        for(int j=0; j < col1; j++){
            fscanf(file1, "%d", &matrix1[i][j]);
        }
    }

    for(int i=0; i < row1; i++){
        for(int j=0; j < col1; j++){
            printf("%d ", matrix1[i][j]);
        }
        printf("\n");
    }


    return 0;
}