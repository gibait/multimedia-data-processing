/*
 * Created on Fri Mar 03 2023
 *
 * by Giulio Barabino
 */


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_SIZE 100

int cmpfunc(const void* a, const void* b) {
    int32_t x = *(int32_t*)a;
    int32_t y = *(int32_t*)b;
    
    if (x < y) {
        return -1;
    } else if (x > y) {
        return 1;
    } else {
        return 0;
    }
}


typedef struct vector_struct {
    int32_t *nums;
    int n;
    int capacity;
} vector;

void vector_constructor(vector* ptr) {
    ptr->nums = NULL;
    ptr->n = 0;
    ptr->capacity = 0;
}

void vector_destructor(vector* ptr) {
    free(ptr->nums);
    ptr->nums = NULL;
    ptr->n = 0;
    ptr->capacity = 0;
}

void vector_push_back(vector* ptr, int32_t num) {
    if (ptr->capacity == ptr->n) {
        ptr->capacity = ptr->capacity == 0 ? 1 : ptr->capacity * 2;
        ptr->nums = (int32_t*) realloc(ptr->nums, ptr->capacity * sizeof(int));
        if (ptr->nums == NULL) {
            perror("errore ri-allocazione v.nums");
            exit(1);
        }
    }
    ptr->nums[ptr->n] = num;
    ptr->n++;
}

void vector_sort(vector* ptr) {
    qsort(ptr->nums, ptr->n, sizeof(int), cmpfunc);
}

// const assicura che il puntatore non venga modificato
int vector_get_n(const vector* ptr) {
    return ptr->n;
}

uint32_t vector_at(vector* ptr, int i) {
    return ptr->nums[i];
}


void usage() {
    printf("sort_int filein fileout");
    exit(1);
}


int main(int argc, char** argv) {
    FILE* filein;
    FILE* fileout;
    vector v;

    if (argc != 3) {
        perror("numero di parametri errato");
        usage();
    }

    filein = fopen(argv[1], "r");
    if (filein == NULL) {
        perror("errore apertura file in");
        usage();
    }
    
    fileout = fopen(argv[2], "w");
    if (fileout == NULL) {
        perror("errore apertura file out");
        usage();
    }

    vector_constructor(&v);


    int32_t num;
    while (1) {
        if (fscanf(filein, "%d", &num) == 1) {
            vector_push_back(&v, num);
        } else if (feof(filein)) {
            break;
        } else {
            printf("Warning: non-integer value\n");
            continue;
        }
    }

    vector_sort(&v);

    for (int i = 0; i < vector_get_n(&v); i++) {
        fprintf(fileout, "%d\n", vector_at(&v, i));
    }

    fclose(filein);
    fclose(fileout);
    vector_destructor(&v);

    exit(0);
}