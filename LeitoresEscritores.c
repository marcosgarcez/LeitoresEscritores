#include<pthread.h>
#include<semaphore.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>

#define NUM_LEITORES 10
#define NUM_ESCRITORES 3
#define MAX_LOGS 5

int leitores_dentro = 0;
int escritores_aguardando = 0;

pthread_mutex_t mutex;
pthread_mutex_t mutex_dashboard;
sem_t porta_leitores;
sem_t porta_escritores;
sem_t sala;
sem_t vagas_sala;

// Estrutura para logs
typedef struct {
    char mensagem[100];
    char timestamp[20];
} Log;

// Estrutura do dashboard
typedef struct {
    int leitores_dentro;
    int escritores_aguardando;
    Log logs[MAX_LOGS];
    int log_index;
} Dashboard;

Dashboard dashboard;

// Inicializa o dashboard
void inicializa_dashboard() {
    dashboard.leitores_dentro = 0;
    dashboard.escritores_aguardando = 0;
    dashboard.log_index = 0;
    
    for(int i = 0; i < MAX_LOGS; i++) {
        strcpy(dashboard.logs[i].mensagem, "");
        strcpy(dashboard.logs[i].timestamp, "");
    }
}

// Adiciona log ao dashboard
void adiciona_log(const char* mensagem) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    pthread_mutex_lock(&mutex_dashboard);
    
    sprintf(dashboard.logs[dashboard.log_index].timestamp, 
            "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    
    strncpy(dashboard.logs[dashboard.log_index].mensagem, mensagem, 99);
    
    dashboard.log_index = (dashboard.log_index + 1) % MAX_LOGS;
    
    pthread_mutex_unlock(&mutex_dashboard);
}

// Atualiza contadores no dashboard
void atualiza_contadores() {
    pthread_mutex_lock(&mutex_dashboard);
    dashboard.leitores_dentro = leitores_dentro;
    dashboard.escritores_aguardando = escritores_aguardando;
    pthread_mutex_unlock(&mutex_dashboard);
}

// Thread monitor que imprime o dashboard
void* monitor(void* arg) {
    while(1) {
        system("clear");
        
        pthread_mutex_lock(&mutex_dashboard);
        
        printf("╔═══════════════════════════════════════════════════════╗\n");
        printf("║              🚪 MONITOR DA SALA 🚪                    ║\n");
        printf("╠═══════════════════════════════════════════════════════╣\n");
        printf("║                                                       ║\n");
        printf("║  📖 Leitores dentro:       %-27d║\n", dashboard.leitores_dentro);
        printf("║  ✍️  Escritores aguardando: %-27d║\n", dashboard.escritores_aguardando);
        printf("║                                                       ║\n");
        printf("╠═══════════════════════════════════════════════════════╣\n");
        printf("║  📋 ÚLTIMOS EVENTOS:                                  ║\n");
        printf("╠═══════════════════════════════════════════════════════╣\n");
        
        for(int i = 0; i < MAX_LOGS; i++) {
            int idx = (dashboard.log_index + i) % MAX_LOGS;
            
            if(strlen(dashboard.logs[idx].mensagem) > 0) {
                printf("║ [%s] %-43s║\n", dashboard.logs[idx].timestamp, dashboard.logs[idx].mensagem);
            } else {
                printf("║                                                       ║\n");
            }
        }
        
        printf("╚═══════════════════════════════════════════════════════╝\n");
        
        pthread_mutex_unlock(&mutex_dashboard);
        
        sleep(1);
    }
    return NULL;
}

void* leitor(void* arg){
    int id = *(int*)arg;
    char log_msg[100];

    while(1){
        sleep(rand() % 5 + 1);

        sem_wait(&porta_leitores);
        sem_post(&porta_leitores);

        sem_wait(&vagas_sala);

        pthread_mutex_lock(&mutex);
        leitores_dentro++;

        if(leitores_dentro == 1){
            sem_wait(&porta_escritores);
        }

        pthread_mutex_unlock(&mutex);
        
        atualiza_contadores();
        sprintf(log_msg, "Leitor %d entrou na sala", id);
        adiciona_log(log_msg);
        
        sleep(rand() % 6 + 3);
        
        pthread_mutex_lock(&mutex);
        leitores_dentro--;

        if(leitores_dentro == 0){
            sem_post(&porta_escritores);
        }

        pthread_mutex_unlock(&mutex);

        atualiza_contadores();
        sprintf(log_msg, "Leitor %d saiu da sala", id);
        adiciona_log(log_msg);

        sem_post(&vagas_sala);
    }
    
    return NULL;
}

void* escritor(void* arg){
    int id = *(int*)arg;
    char log_msg[100];

    while (1){
        sleep(rand() % 15 + 10); 

        pthread_mutex_lock(&mutex);
        escritores_aguardando++;
        
        if(escritores_aguardando == 1) {
            sem_wait(&porta_leitores);
        }
        
        pthread_mutex_unlock(&mutex);

        atualiza_contadores();
        sprintf(log_msg, "Escritor %d entrou na fila", id);
        adiciona_log(log_msg);

        sem_wait(&porta_escritores);
        sem_wait(&sala);

        sprintf(log_msg, "Escritor %d escrevendo!", id);
        adiciona_log(log_msg);
        
        sleep(3);

        sem_post(&sala);

        pthread_mutex_lock(&mutex);
        escritores_aguardando--;
        
        if(escritores_aguardando == 0) {
            sem_post(&porta_leitores);
        }
        
        pthread_mutex_unlock(&mutex);

        atualiza_contadores();
        sprintf(log_msg, "Escritor %d terminou", id);
        adiciona_log(log_msg);
        
        sem_post(&porta_escritores);
    }

    return NULL;
}

int main(){
    srand(time(NULL));

    inicializa_dashboard();

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_dashboard, NULL);
    sem_init(&porta_leitores, 0, 1);
    sem_init(&porta_escritores, 0, 1);
    sem_init(&sala, 0, 1);
    sem_init(&vagas_sala, 0, 15);

    pthread_t leitores[NUM_LEITORES];
    pthread_t escritores[NUM_ESCRITORES];
    pthread_t thread_monitor;
    
    int ids_leitores[NUM_LEITORES];
    int ids_escritores[NUM_ESCRITORES];

    printf("=== Simulação iniciada! ===\n\n");
    sleep(2);

    // Criar thread monitor
    pthread_create(&thread_monitor, NULL, monitor, NULL);

    // Criar threads dos leitores
    for (int i = 0; i < NUM_LEITORES; i++){
        ids_leitores[i] = i + 1;
        pthread_create(&leitores[i], NULL, leitor, &ids_leitores[i]);
    }

    // Criar threads dos escritores
    for (int i = 0; i < NUM_ESCRITORES; i++){
        ids_escritores[i] = i + 1;
        pthread_create(&escritores[i], NULL, escritor, &ids_escritores[i]);
    }

    // Deixa rodar por 60 segundos
    sleep(60);
    
    printf("\n=== Encerrando simulação ===\n");

    // Limpar recursos
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_dashboard);
    sem_destroy(&porta_leitores);
    sem_destroy(&porta_escritores);
    sem_destroy(&sala);
    sem_destroy(&vagas_sala);

    return 0;
}