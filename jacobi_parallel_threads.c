#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
/*--------------------------------*/
#define ITERACOES 1000
#define ERRO 0.0000000001
/*--------------------------------*/
struct args {
	int numero_threads;
	int linha_inicial;
	int tamanho_matriz;
	double **A;
	double *b;
	double *x;
	double *novo_x;
};
typedef struct args ARGS;
/*--------------------------------*/
double **aloca_matriz(int n){
	int i;
	double **mat = (double**)malloc(n*sizeof(double*));
	for(i=0; i<n; i++){
		mat[i] = (double*)malloc(n*sizeof(double));
	}
	return mat;
}
/*--------------------------------*/
void desaloca_matriz(double **mat, int n){
	int i;
	for(i=0; i<n; i++){
		free(mat[i]);
	}
	free(mat);
}
/*--------------------------------*/
void inicializa_matriz(double **mat, int n){
	int i, j;
	for(i=0; i<n; i++){
		for(j=0; j<n; j++){
            if(j == i) {
                mat[i][j] = 4;
            }
            else if(j == i+1 || j == i-1){
                mat[i][j] = 1;
            }
            else{
			    mat[i][j] = 0;
            }
		}
	}
}
/*--------------------------------*/
void escreve_matriz(double **mat, int n){
	int i, j;
	for(i=0; i<n; i++){
		for(j=0; j<n; j++){
			printf("%5.2f", mat[i][j]);
		}
		printf("\n");
	}
}
/*--------------------------------*/
double * gera_vet_b(int n) {
    int i;

    double *b = (double*)malloc(n*sizeof(double));
    for(i=0; i<n; i++){
        b[i] = 4;
    }
    return b;
}
/*--------------------------------*/
double norma_vetor(double *x, int n){
    int i;
    double soma = 0;
    for(i=0; i<n; i++){
        soma +=(x[i]*x[i]);
    }
    return sqrt(soma);
}
/*--------------------------------*/
double calcula_elemento(double **A, double *x, int lin, double b, int tamanho_matriz){
	int i;
    double soma = 0;
    for(i=0; i<tamanho_matriz ;i++){
        if(i != lin){
            soma += (A[lin][i]) * x[i];
        }
    }
    return (b-soma)/(A[lin][lin]);
}
/*--------------------------------*/
void * thread_func(void * args) {
	ARGS *argumentos = (ARGS *)args;
	int i;
	double temp[argumentos->tamanho_matriz];
	for (i=argumentos->linha_inicial; i < argumentos->tamanho_matriz; i+=argumentos->numero_threads){
            temp[argumentos->linha_inicial] = calcula_elemento(
												argumentos->A,
												argumentos->x, 
												argumentos->linha_inicial, 
												argumentos->b[argumentos->linha_inicial],
												argumentos->tamanho_matriz
												);
	// printf("thread %d calculou o elemento %d\n", argumentos->linha_inicial, i);
	// printf("resultado: %f\n", argumentos->novo_x[argumentos->linha_inicial]);
	}
	for(i=0; i<argumentos->tamanho_matriz; i++){
		printf("temp[%d] = %f\n", i, temp[i]);	
	}
	printf("---------------------\n");
	for(i=0; i<argumentos->tamanho_matriz; i++){
		argumentos->novo_x[i] = temp[i];
	}
}
/*--------------------------------*/
void jacobi(double **A, double *b, int tamanho_matriz, int numero_threads, int *iter){

	int i, j, l, contador = 0;
	ARGS *argumentos = NULL;

	double * novo_x = (double*)malloc(tamanho_matriz*sizeof(double));
	double * x = (double*)malloc(tamanho_matriz*sizeof(double));
    for(l=0; l<tamanho_matriz; l++){
        x[l] = 0;
    }

	pthread_t *tid = (pthread_t*)malloc(numero_threads*sizeof(pthread_t));
	argumentos = (ARGS *)malloc(numero_threads * sizeof(ARGS));

	while (contador < ITERACOES){
		
		for(i=0; i<numero_threads; i++){
			argumentos[i].numero_threads = numero_threads;
			argumentos[i].tamanho_matriz = tamanho_matriz;
			argumentos[i].linha_inicial = i;
			argumentos[i].A = A;
			argumentos[i].b = b;
			argumentos[i].x = x;
			argumentos[i].novo_x = novo_x;
			pthread_create(&tid[i], NULL, thread_func, (void*)&argumentos[i]);
			// pthread_create(&tid[i], NULL, calcula_elemento, (void*)&argumentos[i]);
		}

		double teste[tamanho_matriz];
		for(i=0; i<numero_threads; i++){
			pthread_join(tid[i], NULL);
			for(j=0; j<tamanho_matriz; j++){
				x[j] = argumentos[i].novo_x[j];
			}
			teste[i] = argumentos[i].novo_x[i];
		}

		// printf("---------------------\n");
		// for(i=0; i<tamanho_matriz; i++){
			// printf("[%d] = %f\n",i, teste[i]);
		// }
		// printf("----------- NOVO X ---------\n");
		// for(i=0; i<tamanho_matriz; i++){
			// printf("[%d] = %f\n",i, novo_x[i]);
		// }
		// printf("---------------------\n");
		// printf("x= %f\n", norma_vetor(x, tamanho_matriz));
		// printf("novo_x= %f\n", norma_vetor(novo_x, tamanho_matriz));

		if(fabs(norma_vetor(x, tamanho_matriz) - norma_vetor(teste, tamanho_matriz)) < ERRO){
			*iter = contador;
			printf("convergiu\n");
			exit(0);
			// retorno->resultado = novo_x;
			// return retorno;
		}
		else {
			for ( i = 0; i < tamanho_matriz; i++){
				x[i] = novo_x[i];
			}
		}
		contador++;
		printf("### contador: %d ###\n", contador);
	}

}
/*--------------------------------*/
int main(int argc, char ** argv) {


    if(argc != 3){
        printf("Erro: Numero de argumentos invalido\n");
        printf("Uso: ./jacobi_linear <numero de threads> <tamanho da matriz>\n");
        exit(0);
    }

    int numero_threads = atoi(argv[1]);
    int tamanho_matriz = atoi(argv[2]);
	int iter = 0;
	
    double **A = aloca_matriz(tamanho_matriz);
	double *b = gera_vet_b(tamanho_matriz);
    inicializa_matriz(A, tamanho_matriz);
	escreve_matriz(A, tamanho_matriz);

	jacobi(A, b, tamanho_matriz, numero_threads, &iter);

    desaloca_matriz(A, tamanho_matriz);
}