# Problema dos leitores-escritores

Implementação do clássico problema de sincronização de processos **Leitores-Escritores** em C, utilizando semáforos POSIX para garantir acesso seguro a recursos compartilhados e evitar condições de corrida.

## 📋 Sobre o projeto

Este projeto foi desenvolvido como trabalho prático da disciplina de **Sistemas Operacionais**, com o objetivo de implementar uma solução para o problema dos leitores-escritores utilizando mecanismos de sincronização (semáforos, mutexes) para garantir:

- **Exclusão mútua**: escritores têm acesso exclusivo ao recurso
- **Leitura concorrente**: múltiplos leitores podem ler simultaneamente
- **Prevenção de starvation**: implementação de fila FIFO para garantir justiça no acesso

## 🎯 Características da implementação

### Solução adotada

A implementação utiliza **três semáforos**:

- `recurso`: controla o acesso exclusivo ao recurso compartilhado
- `mutex_leitores`: protege a variável `leitores_ativos` (região crítica)
- `fila_ordem`: implementa uma fila FIFO para evitar starvation de escritores

### Regras de sincronização

1. **Leitores**:
   - Múltiplos leitores podem ler simultaneamente
   - O primeiro leitor bloqueia escritores
   - O último leitor libera escritores

2. **Escritores**:
   - Têm acesso exclusivo ao recurso
   - Nenhum leitor ou escritor pode acessar durante a escrita

3. **Justiça**:
   - Fila FIFO garante que nenhum processo sofra starvation
   - Todos entram na mesma fila de espera

## 🛠️ Tecnologias utilizadas

- **Linguagem**: C
- **Threads**: POSIX Threads (pthread)
- **Sincronização**: Semáforos POSIX (semaphore.h)
- **Sistema**: Linux/Unix

## 📦 Requisitos

- Compilador GCC
- Bibliotecas POSIX (pthread, semaphore)
- Sistema operacional Linux/Unix

## 🚀 Como compilar e executar

### Compilação

```bash
gcc -o leitores_escritores LeitoresEscritores.c -lpthread
```

### Execução

```bash
./leitores_escritores
```

## ⚙️ Configurações

Você pode ajustar os parâmetros no início do arquivo `LeitoresEscritores.c`:

```c
#define NUM_LEITORES 5      // Quantidade de threads leitoras
#define NUM_ESCRITORES 2    // Quantidade de threads escritoras
#define TEMPO_LEITURA 3     // Tempo de leitura (segundos)
#define TEMPO_ESCRITA 4     // Tempo de escrita (segundos)
```

## 📊 Saída do programa

O programa exibe uma saída colorida e detalhada mostrando:

- Quando cada processo quer acessar o recurso
- Quando obtém acesso (leitura ou escrita)
- Quantos leitores estão lendo simultaneamente
- Quando escritores obtêm acesso exclusivo
- Quando processos terminam suas operações

### Exemplo de saída:

```
=== SIMULADOR LEITORES-ESCRITORES ===
Leitores: 5 | Escritores: 2

[LEITOR 1] Quer ler...
    -> Primeiro leitor bloqueando escritores
    [LEITOR 1] *** LENDO *** (1 leitores no total)
[LEITOR 2] Quer ler...
    [LEITOR 2] *** LENDO *** (2 leitores no total)
[ESCRITOR 1] Quer escrever...
    [ESCRITOR 1] Aguardando acesso exclusivo...
    [LEITOR 1] Terminou de ler.
    [LEITOR 2] Terminou de ler.
    -> Último leitor liberando escritores
    [ESCRITOR 1] >>> ESCREVENDO (EXCLUSIVO) <<<
```

## 🔍 Detalhes da implementação

### Protocolo do leitor

1. Entra na fila de ordem (`sem_wait(&fila_ordem)`)
2. Protege o contador de leitores ativos (`sem_wait(&mutex_leitores)`)
3. Incrementa `leitores_ativos`
4. Se for o primeiro leitor, bloqueia escritores (`sem_wait(&recurso)`)
5. Libera a fila e o mutex
6. **Lê o recurso**
7. Decrementa `leitores_ativos`
8. Se for o último leitor, libera escritores (`sem_post(&recurso)`)

### Protocolo do escritor

1. Entra na fila de ordem (`sem_wait(&fila_ordem)`)
2. Aguarda acesso exclusivo ao recurso (`sem_wait(&recurso)`)
3. Libera a fila
4. **Escreve no recurso (exclusivo)**
5. Libera o recurso (`sem_post(&recurso)`)

## 🎓 Conceitos de sistemas operacionais aplicados

- **Sincronização de processos**
- **Exclusão mútua**
- **Semáforos**
- **Condições de corrida**
- **Deadlock** (prevenção)
- **Starvation** (prevenção via FIFO)
- **Threads POSIX**
- **Seções críticas**

## 📝 Observações

- O programa executa indefinidamente (loop infinito nas threads)
- Use `Ctrl+C` para encerrar a execução
- Os tempos de espera entre operações são aleatórios para simular comportamento real
- A saída inclui pausas intencionais (`sleep(1)`) para facilitar a leitura do log

## 👨‍💻 Autores

Trabalho desenvolvido para a disciplina de Sistemas Operacionais do curso de Análise e Desenvolvimento de Sistemas do IFPI Campus Parnaíba.
**Discentes:** Marcos, Guilherme, Luana, Luiza e Ludmyla

## 📄 Licença
Este projeto foi desenvolvido para fins educacionais.
