#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct {
    char nome[10];
    int tempo_execucao;
    int periodo;
    int deadline;
} Task;

typedef struct {
    int numero;
    int num_tarefas;
    Task tarefas[10];
} Workload;

int ciclo_maior;
int ciclo_menor;
int num_divisores;
int divisores[20];

// ------- Funcoes auxiliares ---------

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}

int maior_tempo_execucao(Workload w){
	
	int maior = w.tarefas[0].tempo_execucao;
	
	for(int i = 0; i < w.num_tarefas; i++){
		if( maior < w.tarefas[i].tempo_execucao)
			maior = w.tarefas[i].tempo_execucao;	
	}
	return maior;
}

int menor_periodo(Workload w){
	
	int menor = w.tarefas[0].periodo;
	
	for(int i = 0; i < w.num_tarefas; i++){
		if( menor > w.tarefas[i].periodo)
			menor = w.tarefas[i].periodo;	
	}
	return menor;
}

bool teste_exato(Workload workload){
	
	for(int i = 0; i < workload.num_tarefas; i++){
		if(workload.tarefas[i].deadline != workload.tarefas[i].periodo)
			return false;
	}
	return true;
}

double calc_taxa_utl(Workload workload, bool periodo_igual_deadline){
	
	double taxa_utl = 0;
	double divisor;
	
	if(periodo_igual_deadline){
		for(int i = 0; i < workload.num_tarefas; i++){
			taxa_utl += (double) workload.tarefas[i].tempo_execucao / workload.tarefas[i].periodo;
		}
	}
	else{
		for(int i = 0; i < workload.num_tarefas; i++){
			if(workload.tarefas[i].periodo < workload.tarefas[i].deadline)
				divisor = workload.tarefas[i].periodo;
			else
				divisor = workload.tarefas[i].deadline;
			taxa_utl += (double) workload.tarefas[i].tempo_execucao / divisor;
		}
	}
	
	return taxa_utl;
}

void load_workloads(const char* filename, Workload workloads[], int* num_workloads) {
   
    FILE *file = fopen(filename, "r");
   
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(1);
    }

    int workload_index = 0;
    while (fscanf(file, "%d", &workloads[workload_index].numero) != EOF) {
        
		int task_index = 0;
        fscanf(file, "%s", workloads[workload_index].tarefas[task_index].nome);
        
		if(isalpha(workloads[workload_index].tarefas[task_index].nome[0])){
			while (true) {
				fscanf(file, "%d %d %d", &workloads[workload_index].tarefas[task_index].tempo_execucao,
	                                                &workloads[workload_index].tarefas[task_index].periodo,
	                                                &workloads[workload_index].tarefas[task_index].deadline);
	                    
	            task_index++;
	            
	            fscanf(file, "%s", workloads[workload_index].tarefas[task_index].nome);
	            
	            if( feof(file) || isdigit(workloads[workload_index].tarefas[task_index].nome[0])){
	            	workloads[workload_index].num_tarefas = task_index;
	            	break;
	            }
	        }
	    }
        workload_index++;
    }

    *num_workloads = workload_index;
    fclose(file);
}

bool escalonavel_executivo_ciclico(Workload workload) {
    ciclo_maior = 1;
    ciclo_menor = 0;
    num_divisores = 0;
    
    int menor_p = menor_periodo(workload);
    int maior_te = maior_tempo_execucao(workload);
    bool e_divisor;
    bool escalonavel;
    
	for (int i = 0; i < workload.num_tarefas; i++) {
        ciclo_maior = lcm(ciclo_maior, workload.tarefas[i].periodo);
    }
    
    for(int i = maior_te; i < menor_p; i++){
    	if( (ciclo_maior % i) == 0){
    		divisores[num_divisores] = i;
			num_divisores++;  
		}
	}
	
	escalonavel = false;
	
	for(int i = 0; i < num_divisores; i++){
		
		e_divisor = true;
		
		for(int j = 0; j < workload.num_tarefas; j++){
			if( (2*divisores[i] - gcd(divisores[i], workload.tarefas[j].periodo)) > workload.tarefas[j].periodo){
				e_divisor = false;
				break;
			}	
		}
		if(e_divisor){
			escalonavel = true;
			ciclo_menor = divisores[i];
			//return true;
		}
	}
    
	if(!escalonavel){
		ciclo_maior = 0;
		return false;
	} else return true;
}

int escalonavel_rm(Workload workload) {

	bool tst_exato = teste_exato(workload);
	double taxa_utilizacao = calc_taxa_utl(workload, tst_exato);
	
	if(taxa_utilizacao <= (workload.num_tarefas*( pow(2, 1.0/workload.num_tarefas) - 1)) )
		return 1;
	else if(taxa_utilizacao <= 1)
		return 2;
	else
		return 0;
}

int escalonavel_edf(Workload workload) {
	
	bool tst_exato = teste_exato(workload);
	double taxa_utilizacao = calc_taxa_utl(workload, tst_exato);
	
	if(tst_exato){
		if(taxa_utilizacao <= 1)
			return 1;
		else
			return 0;
	}
	else{
		if(taxa_utilizacao <= 1)
			return 1;
		else
			return 2;
	}
}


int main(int argc, char *argv[]) {
	
	//const char* filename = "carga_de_teste.txt";
	
    if (argc != 2) {
        printf("Uso: %s arquivo_com_cargas.txt\n", argv[0]);
        return 1;
    }

    Workload workloads[15];  // Assuming a maximum of 10 workloads
    int num_workloads;

    load_workloads(argv[0], workloads, &num_workloads);

    for (int i = 0; i < num_workloads; i++) {
        printf("Carga %d\n", i + 1);
        printf("  Executivo: %s\n", escalonavel_executivo_ciclico(workloads[i]) ? "SIM" : "NAO");
        printf("    Ciclo maior: %d\n", ciclo_maior);
        printf("    Ciclo menor: %d\n", ciclo_menor);
        
        if(escalonavel_edf(workloads[i]) == 0)
        	printf("  EDF: NAO\n");
        else if(escalonavel_edf(workloads[i]) == 1)
        	printf("  EDF: SIM\n");
        else
        	printf("  EDF: INCONCLUSIVO\n");
        
        if(escalonavel_rm(workloads[i]) == 0)
        	printf("  RM: NAO\n");
        else if(escalonavel_rm(workloads[i]) == 1)
        	printf("  RM: SIM\n");
        else
        	printf("  RM: INCONCLUSIVO\n");
        	
    }

    return 0;
}
