#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// ==================== CONFIGURAÇÕES ====================
#define NUM_LEITORES 5      // Quantidade de leitores
#define NUM_ESCRITORES 2    // Quantidade de escritores
#define TEMPO_LEITURA 3     // Tempo que leva para ler
#define TEMPO_ESCRITA 4     // Tempo que leva para escrever

// ==================== CORES ANSI ====================
#define RESET   "\033[0m"
#define VERDE   "\033[32m"
#define VERMELHO "\033[31m"
#define AMARELO "\033[33m"
#define AZUL    "\033[34m"
#define MAGENTA "\033[35m"
#define CIANO   "\033[36m"
#define NEGRITO "\033[1m"

// ==================== VARIÁVEIS DE SINCRONIZAÇÃO ====================
sem_t recurso;              // Semáforo: controla acesso ao recurso compartilhado
sem_t mutex_leitores;       // Semáforo: protege a variável 'leitores_ativos'
sem_t fila_ordem;           // Semáforo: implementa fila FIFO (evita starvation)

int leitores_ativos = 0;    // Contador: quantos leitores estão lendo agora

// ==================== FUNÇÃO LEITOR ====================
void* leitor(void* arg) {
    int id = *((int*)arg);
    free(arg);
    
    while (1) {
        // Simula tempo entre leituras
        sleep(rand() % 3 + 2);
        
        printf(CIANO "[LEITOR %d] Quer ler...\n" RESET, id);
        fflush(stdout);
        sleep(1);  // Pausa para dar tempo de ler
        
        // === PROTOCOLO DE ENTRADA ===
        sem_wait(&fila_ordem);          // Entra na fila de espera
        sem_wait(&mutex_leitores);      // Protege leitores_ativos
        
        leitores_ativos++;
        if (leitores_ativos == 1) {
            // Primeiro leitor bloqueia escritores
            printf(AMARELO "    -> Primeiro leitor bloqueando escritores\n" RESET);
            fflush(stdout);
            sleep(1);
            sem_wait(&recurso);
        }
        
        sem_post(&mutex_leitores);      // Libera leitores_ativos
        sem_post(&fila_ordem);          // Libera a fila
        
        // === SEÇÃO CRÍTICA: LENDO ===
        printf(VERDE NEGRITO "    [LEITOR %d] *** LENDO *** " RESET VERDE "(%d leitores no total)\n" RESET, 
               id, leitores_ativos);
        fflush(stdout);
        sleep(TEMPO_LEITURA);
        
        // === PROTOCOLO DE SAÍDA ===
        sem_wait(&mutex_leitores);
        leitores_ativos--;
        printf(CIANO "    [LEITOR %d] Terminou de ler.\n" RESET, id);
        fflush(stdout);
        sleep(1);  // Pausa para dar tempo de ler
        
        if (leitores_ativos == 0) {
            // Último leitor libera escritores
            printf(AMARELO "    -> Último leitor liberando escritores\n" RESET);
            fflush(stdout);
            sleep(1);
            sem_post(&recurso);
        }
        sem_post(&mutex_leitores);
        
        printf("\n");
        fflush(stdout);
    }
    
    return NULL;
}

// ==================== FUNÇÃO ESCRITOR ====================
void* escritor(void* arg) {
    int id = *((int*)arg);
    free(arg);
    
    while (1) {
        // Escritores esperam mais tempo antes de escrever novamente
        sleep(rand() % 4 + 4);
        
        printf(MAGENTA "[ESCRITOR %d] Quer escrever...\n" RESET, id);
        fflush(stdout);
        sleep(1);  // Pausa para dar tempo de ler
        
        // === PROTOCOLO DE ENTRADA ===
        sem_wait(&fila_ordem);          // Entra na fila de espera
        printf(AMARELO "    [ESCRITOR %d] Aguardando acesso exclusivo...\n" RESET, id);
        fflush(stdout);
        sleep(1);  // Pausa para dar tempo de ler
        
        sem_wait(&recurso);             // Pega acesso EXCLUSIVO
        sem_post(&fila_ordem);          // Libera a fila
        
        // === SEÇÃO CRÍTICA: ESCREVENDO ===
        printf(VERMELHO NEGRITO "    [ESCRITOR %d] >>> ESCREVENDO (EXCLUSIVO) <<<\n" RESET, id);
        fflush(stdout);
        sleep(TEMPO_ESCRITA);
        
        printf(MAGENTA "    [ESCRITOR %d] Terminou de escrever.\n" RESET, id);
        fflush(stdout);
        sleep(1);  // Pausa para dar tempo de ler
        
        // === PROTOCOLO DE SAÍDA ===
        sem_post(&recurso);             // Libera o recurso
        
        printf("\n");
        fflush(stdout);
    }
    
    return NULL;
}

// ==================== FUNÇÃO PRINCIPAL ====================
int main() {
    srand(time(NULL));
    
    pthread_t threads_leitores[NUM_LEITORES];
    pthread_t threads_escritores[NUM_ESCRITORES];
    
    // Inicializa semáforos
    sem_init(&recurso, 0, 1);           // 1 = livre inicialmente
    sem_init(&mutex_leitores, 0, 1);    // 1 = livre inicialmente
    sem_init(&fila_ordem, 0, 1);        // 1 = livre inicialmente
    
    printf(AZUL NEGRITO "=== SIMULADOR LEITORES-ESCRITORES ===\n" RESET);
    printf(AZUL "Leitores: %d | Escritores: %d\n\n" RESET, NUM_LEITORES, NUM_ESCRITORES);
    printf(AMARELO "Iniciando em 2 segundos...\n\n" RESET);
    sleep(2);
    
    // Cria threads de leitores
    for (int i = 0; i < NUM_LEITORES; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&threads_leitores[i], NULL, leitor, id);
    }
    
    // Cria threads de escritores
    for (int i = 0; i < NUM_ESCRITORES; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&threads_escritores[i], NULL, escritor, id);
    }
    
    // Aguarda threads (programa roda infinitamente)
    for (int i = 0; i < NUM_LEITORES; i++) {
        pthread_join(threads_leitores[i], NULL);
    }
    for (int i = 0; i < NUM_ESCRITORES; i++) {
        pthread_join(threads_escritores[i], NULL);
    }
    
    // Limpa semáforos
    sem_destroy(&recurso);
    sem_destroy(&mutex_leitores);
    sem_destroy(&fila_ordem);
    
    return 0;
}