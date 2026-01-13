#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// ==================== CONFIGURAÇÕES ====================
#define NUM_LEITORES 15     // Quantidade de leitores
#define NUM_ESCRITORES 3    // Quantidade de escritores
#define TEMPO_LEITURA 2     // Tempo que leva para ler
#define TEMPO_ESCRITA 3     // Tempo que leva para escrever
#define INTERVALO_CICLO 1   // Intervalo para atualizar a tela

// ==================== CORES ANSI ====================
#define RESET   "\033[0m"
#define VERDE   "\033[32m"
#define VERMELHO "\033[31m"
#define AMARELO "\033[33m"
#define AZUL    "\033[34m"
#define AZUL_CLARO "\033[96m"
#define MAGENTA "\033[35m"
#define CIANO   "\033[36m"
#define NEGRITO "\033[1m"
#define CLEAR_SCREEN "\033[2J\033[H"

// ==================== VARIÁVEIS DE SINCRONIZAÇÃO ====================
sem_t recurso;              // Semáforo: controla acesso ao recurso compartilhado
sem_t mutex_leitores;       // Semáforo: protege a variável 'leitores_ativos'
sem_t fila_ordem;           // Semáforo: implementa fila FIFO (evita starvation)
pthread_mutex_t mutex_display = PTHREAD_MUTEX_INITIALIZER;  // Para atualização da tela

int leitores_ativos = 0;    // Contador: quantos leitores estão lendo agora
int escritores_aguardando = 0;  // Contador: quantos escritores estão aguardando
int leitores_aguardando = 0;    // Contador: quantos leitores estão aguardando

// Arrays para rastrear estado de cada thread
int estado_leitores[20];    // 0=idle, 1=aguardando, 2=acessando
int estado_escritores[20];  // 0=idle, 1=aguardando, 2=acessando

// ==================== FUNÇÃO DE DISPLAY ====================
void atualizar_display() {
    pthread_mutex_lock(&mutex_display);
    
    printf(CLEAR_SCREEN);
    
    // Cabeçalho
    printf(AZUL_CLARO "╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║              " NEGRITO "SIMULADOR - LEITORES/ESCRITORES" RESET AZUL_CLARO "                               ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n" RESET);
    printf("\n");
    
    // Status de aguardando
    printf(VERMELHO "Escritores aguardando: %2d\n" RESET, escritores_aguardando);
    printf(AMARELO "Leitores aguardando:   %2d\n" RESET, leitores_aguardando);
    printf("\n");
    
    // Box do recurso
    if (leitores_ativos > 0) {
        printf(AZUL_CLARO "┌─────────────────────── " RESET VERDE NEGRITO "RECURSO" RESET AZUL_CLARO " ───────────────────────────┐\n" RESET);
        printf(VERDE NEGRITO "│  %2d leitor(es) acessando                                       │\n" RESET, leitores_ativos);
        printf(AZUL_CLARO "└──────────────────────────────────────────────────────────────┘\n" RESET);
    } else {
        // Verifica se algum escritor está acessando
        int escritor_ativo = 0;
        for (int i = 0; i < NUM_ESCRITORES; i++) {
            if (estado_escritores[i] == 2) {
                escritor_ativo = i + 1;
                break;
            }
        }
        
        if (escritor_ativo > 0) {
            printf(AZUL_CLARO "┌─────────────────────── " RESET VERMELHO NEGRITO "RECURSO" RESET AZUL_CLARO " ───────────────────────────┐\n" RESET);
            printf(VERMELHO NEGRITO "│  Escritor %2d acessando (EXCLUSIVO)                          │\n" RESET, escritor_ativo);
            printf(AZUL_CLARO "└──────────────────────────────────────────────────────────────┘\n" RESET);
        } else {
            printf(AZUL_CLARO "┌─────────────────────── " RESET "RECURSO" AZUL_CLARO " ───────────────────────────┐\n" RESET);
            printf("│  Recurso livre                                               │\n");
            printf(AZUL_CLARO "└──────────────────────────────────────────────────────────────┘\n" RESET);
        }
    }
    
    printf("\n");
    
    pthread_mutex_unlock(&mutex_display);
    fflush(stdout);
}

// ==================== FUNÇÃO LEITOR ====================
void* leitor(void* arg) {
    int id = *((int*)arg);
    free(arg);
    
    while (1) {
        // Estado: IDLE
        estado_leitores[id-1] = 0;
        atualizar_display();
        sleep(rand() % 3 + 2);
        
        // Estado: AGUARDANDO
        estado_leitores[id-1] = 1;
        pthread_mutex_lock(&mutex_display);
        leitores_aguardando++;
        pthread_mutex_unlock(&mutex_display);
        atualizar_display();
        
        // === PROTOCOLO DE ENTRADA ===
        sem_wait(&fila_ordem);
        sem_wait(&mutex_leitores);
        
        leitores_ativos++;
        if (leitores_ativos == 1) {
            sem_wait(&recurso);
        }
        
        pthread_mutex_lock(&mutex_display);
        leitores_aguardando--;
        pthread_mutex_unlock(&mutex_display);
        
        sem_post(&mutex_leitores);
        sem_post(&fila_ordem);
        
        // Estado: ACESSANDO
        estado_leitores[id-1] = 2;
        atualizar_display();
        sleep(TEMPO_LEITURA);
        
        // === PROTOCOLO DE SAÍDA ===
        sem_wait(&mutex_leitores);
        leitores_ativos--;
        
        if (leitores_ativos == 0) {
            sem_post(&recurso);
        }
        sem_post(&mutex_leitores);
        
        estado_leitores[id-1] = 0;
        atualizar_display();
        sleep(INTERVALO_CICLO);
    }
    
    return NULL;
}

// ==================== FUNÇÃO ESCRITOR ====================
void* escritor(void* arg) {
    int id = *((int*)arg);
    free(arg);
    
    while (1) {
        // Estado: IDLE
        estado_escritores[id-1] = 0;
        atualizar_display();
        sleep(rand() % 4 + 4);
        
        // Estado: AGUARDANDO
        estado_escritores[id-1] = 1;
        pthread_mutex_lock(&mutex_display);
        escritores_aguardando++;
        pthread_mutex_unlock(&mutex_display);
        atualizar_display();
        
        // === PROTOCOLO DE ENTRADA ===
        sem_wait(&fila_ordem);
        sem_wait(&recurso);
        
        pthread_mutex_lock(&mutex_display);
        escritores_aguardando--;
        pthread_mutex_unlock(&mutex_display);
        
        sem_post(&fila_ordem);
        
        // Estado: ACESSANDO
        estado_escritores[id-1] = 2;
        atualizar_display();
        sleep(TEMPO_ESCRITA);
        
        // === PROTOCOLO DE SAÍDA ===
        sem_post(&recurso);
        
        estado_escritores[id-1] = 0;
        atualizar_display();
        sleep(INTERVALO_CICLO);
    }
    
    return NULL;
}

// ==================== FUNÇÃO PRINCIPAL ====================
int main() {
    srand(time(NULL));
    
    pthread_t threads_leitores[NUM_LEITORES];
    pthread_t threads_escritores[NUM_ESCRITORES];
    
    // Inicializa arrays de estado
    for (int i = 0; i < NUM_LEITORES; i++) {
        estado_leitores[i] = 0;
    }
    for (int i = 0; i < NUM_ESCRITORES; i++) {
        estado_escritores[i] = 0;
    }
    
    // Inicializa semáforos
    sem_init(&recurso, 0, 1);
    sem_init(&mutex_leitores, 0, 1);
    sem_init(&fila_ordem, 0, 1);
    
    printf(CLEAR_SCREEN);
    printf(AZUL NEGRITO "\n  Iniciando simulador...\n" RESET);
    printf(AZUL "  Leitores: %d | Escritores: %d\n\n" RESET, NUM_LEITORES, NUM_ESCRITORES);
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
    
    // Aguarda threads
    for (int i = 0; i < NUM_LEITORES; i++) {
        pthread_join(threads_leitores[i], NULL);
    }
    for (int i = 0; i < NUM_ESCRITORES; i++) {
        pthread_join(threads_escritores[i], NULL);
    }
    
    // Limpa recursos
    sem_destroy(&recurso);
    sem_destroy(&mutex_leitores);
    sem_destroy(&fila_ordem);
    pthread_mutex_destroy(&mutex_display);
    
    return 0;
}