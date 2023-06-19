#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/shm.h>
#include<sys/wait.h>
/*--------------------------------*/
#define ITERACOES 1000
#define ERRO 0.0000000001
/*--------------------------------*/
struct retorno {
    double * resultado;
    int pid;
};
typedef struct retorno RETORNO;
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
double calcula_elemento(double **A, double *x, int lin, double b, int n){
    int i;
    double soma = 0;
    for(i=0; i<n ;i++){
        if(i != lin){
            soma += (A[lin][i]) * x[i];
        }
    }
    return (b-soma)/(A[lin][lin]);
}
/*--------------------------------*/
RETORNO* jacobi(double **A, double *b, int n, int np, int *iter, double *shm_id){
    RETORNO* retorno = (RETORNO*)malloc(sizeof(RETORNO));
    int i, j, l, contador=0;
    double soma;

    printf("tamanho da matriz: %d\n", n);

    double * x = (double*)malloc(n*sizeof(double));
    for(l=0; l<n; l++){
        x[l] = 0;
    }

    double shared_memory_id = shmget(IPC_PRIVATE, n * sizeof(double), IPC_CREAT | 0666);
    double * novo_x = (double *) shmat(shared_memory_id, NULL, 0);
    *shm_id = shared_memory_id;

    while (contador < ITERACOES){

        int k, pid = 1, li;
        li = 0;
        for(k=1; k<np; k++){
            pid = fork();
            if(pid == 0){
                li = k;
                break;
            }
        }
        retorno->pid = pid;

        for (i=li; i < n; i+=np){
            novo_x[i] = calcula_elemento(A, x, i, b[i], n);
        }

        if(pid > 0){
            for(i=1; i<np; i++){
                wait(NULL);
            }
            if(fabs(norma_vetor(x, n) - norma_vetor(novo_x, n)) < ERRO){
                *iter = contador;
                retorno->resultado = novo_x;
                return retorno;
            }
            else {
                for ( i = 0; i < n; i++){
                    x[i] = novo_x[i];
                }
            }
        }
        else {
            break;
        }
        contador++;
    }
    retorno->resultado = novo_x;
    return retorno;
}
/*--------------------------------*/
int main(int argc, char ** argv) {

    int i, iter = 0;
    double shm_id = 0;

    if(argc != 3){
        printf("Erro: Numero de argumentos invalido\n");
        printf("Uso: ./jacobi_linear <numero de processos> <tamanho da matriz>\n");
        exit(0);
    }

    int np = atoi(argv[1]);
    int n = atoi(argv[2]);

    double **A = aloca_matriz(n);
    inicializa_matriz(A, n);
    double *b = gera_vet_b(n);
    // escreve_matriz(A, n);


    RETORNO* retorno = jacobi(A, b, n, np, &iter, &shm_id);

    double* resultado = retorno->resultado;

    if (retorno->pid == 0) {
        return 0;
    }

    for(i=0; i<n; i++){
        printf("x[%d] = %f\n", i, resultado[i]);
    }
    printf("Iteracoes: %d\n", iter);

	desaloca_matriz(A, n);
    free(b);
    shmdt(resultado);
    shmctl(shm_id, IPC_RMID, 0);
    
    return 0;
}