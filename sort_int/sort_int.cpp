/*
 * Created on Fri Mar 03 2023
 *
 * by Giulio Barabino
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

int cmpfunc(const void *a, const void *b)
{
	int32_t x = *(int32_t *)a;
	int32_t y = *(int32_t *)b;

	if (x < y) {
		return -1;
	} else if (x > y) {
		return 1;
	} else {
		return 0;
	}
}

struct vector {
	int32_t *nums_;
	int n_;
	int capacity_;

	vector()
	{
		nums_ = NULL;
		n_ = 0;
		capacity_ = 0;
	}

	vector(int initial_size)
	{
		n_ = initial_size;
		capacity_ = initial_size;
		nums_ = new int32_t[capacity_];
	}

	/*
        Costruttore di copia
        DEEP COPY using REFERENCE
        */
	vector(const vector &other)
	{
		// costruttore che riceve una REFERENCE a un oggetto const vector
		n_ = other.n_;
		capacity_ = other.capacity_;
		nums_ = new int32_t[capacity_];
		if (nums_ == NULL) {
			perror("errore allocazione v.nums_");
			exit(1);
		}
		for (int i = 0; i < n_; i++) {
			nums_[i] = other.nums_[i];
		}
	}

	~vector()
	{
		delete[] nums_;
		nums_ = NULL;
		n_ = 0;
		capacity_ = 0;
	}

	void push_back(int32_t num)
	{
		if (capacity_ == n_) {
			capacity_ = capacity_ == 0 ? 1 : capacity_ * 2;
			nums_ = (int32_t *)realloc(nums_,
						   capacity_ * sizeof(int));
			if (nums_ == NULL) {
				perror("errore ri-allocazione v.nums_");
				exit(1);
			}
		}
		nums_[n_] = num;
		n_++;
	}

	void sort()
	{
		qsort(nums_, n_, sizeof(int), cmpfunc);
	}

	void raddoppia(int &val)
	{
		// val è una REFERENCE a un intero
		val = val * 2;
	}

	int size() const
	{
		// const indica che il metodo non modifica l'oggetto
		return n_;
	}

	// Metodo per testing che presenta un controllo di errore
	const int32_t &at(int i) const
	{
		assert(i >= 0 && i < n_);
		return nums_[i];
	}

	// OVERLOADING dell'operatore [] che non permette la modifica
	const int32_t &operator[](int i) const
	{
		return nums_[i];
	}

	// OVERLOADING dell'operatore [] che permette la modifica
	int32_t &operator[](int i)
	{
		return nums_[i];
	}

	// OVERLOADING dell'operatore assegnamento
	vector &operator=(const vector &other)
	{
		// controllo che non si stia assegnando l'oggetto a se stesso
		if (this != &other) {
			// distrugge il vecchio vettore
			delete[] nums_;
			// assegna i nuovi valori
			n_ = other.n_;
			capacity_ = other.capacity_;
			// alloca la memoria
			nums_ = new int32_t[capacity_];
			if (nums_ == NULL) {
				perror("errore allocazione v.nums_");
				exit(1);
			}
			// copia i valori
			for (int i = 0; i < n_; i++) {
				nums_[i] = other.nums_[i];
			}
		}
		// restituisce l'oggetto puntato da this
		return *this;
	}
};

void usage()
{
	printf("usage: sort_int filein fileout\n");
	exit(1);
}

int main(int argc, char **argv)
{
	FILE *filein;
	FILE *fileout;
	vector v;

	if (argc != 3) {
		perror("numero di parametri errato");
		usage();
	}

	filein = fopen(argv[1], "r");
	if (filein == NULL) {
		perror("errore apertura file in");
		printf("argv[1] = %s\n", argv[1]);
		usage();
	}

	fileout = fopen(argv[2], "w");
	if (fileout == NULL) {
		perror("errore apertura file out");
		usage();
	}

	int32_t num;
	while (1) {
		if (fscanf(filein, "%d", &num) == 1) {
			v.push_back(num);
		} else if (feof(filein)) {
			break;
		} else {
			printf("Warning: non-integer value\n_");
			continue;
		}
	}

	v.sort();

	for (int i = 0; i < v.size(); i++) {
		fprintf(fileout, "%d\n", v[i]);
	}

	// Shallow Copy
	vector v1 = v;
	// Deep Copy
	vector v2(v);

	// Distinzione tra operatore [] che permette la modifica
	// e l'operatore [] che non permette la modifica
	v2[0] = 5;

	// Costruttore con parametro
	vector v3(10);
	// Assegnazione successiva alla inizializzione
	v3 = v2 = v1;

	fclose(filein);
	fclose(fileout);

	exit(0);
}