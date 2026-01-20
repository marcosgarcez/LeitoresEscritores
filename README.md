# Problema dos leitores e escritores

Uma implementação em C do clássico problema de sincronização de threads usando semáforos e mutexes POSIX, com prioridade para escritores e monitoramento visual em tempo real.

## 📋 Descrição do problema

Imagine uma sala com um painel de informações e duas portas: uma para leitores e outra para escritores. A sala comporta até 15 leitores simultaneamente que podem ler as informações do painel, mas precisa garantir exclusão mútua com os escritores.

### Regras de sincronização

- **Exclusão mútua**: Leitores e escritores não podem estar na sala ao mesmo tempo
- **Escritor único**: Apenas um escritor pode estar na sala por vez
- **Capacidade limitada**: Máximo de 15 leitores simultâneos
- **Prioridade de escritores**: Quando um escritor chega, novos leitores são bloqueados

### Dinâmica dos leitores

1. O primeiro leitor a entrar fecha a porta dos escritores
2. Múltiplos leitores podem ler simultaneamente (até 15)
3. Cada leitura demora entre 3 e 8 segundos
4. O último leitor a sair libera a porta dos escritores

### Dinâmica dos escritores

1. Ao chegar, o escritor bloqueia a entrada de novos leitores
2. Aguarda todos os leitores atuais saírem
3. Entra na sala e atualiza o painel (3 segundos)
4. Se houver fila de escritores, eles entram sequencialmente
5. O último escritor libera a porta dos leitores

## 🚀 Funcionalidades

- ✅ Sincronização completa usando semáforos e mutexes
- ✅ Prioridade para escritores (evita starvation)
- ✅ Dashboard visual em tempo real
- ✅ Log dos últimos 5 eventos com timestamp
- ✅ Contadores de leitores ativos e escritores aguardando
- ✅ Interface com bordas e emojis para melhor visualização

## 🛠️ Compilação e execução

### Pré-requisitos

- GCC (GNU Compiler Collection)
- Biblioteca pthread (geralmente já incluída no Linux)
- Terminal com suporte a Unicode para visualização correta dos caracteres

### Compilar

```bash
gcc -o leitores_escritores LeitoresEscritores.c -lpthread
```

### Executar

```bash
./leitores_escritores
```

O programa roda por 60 segundos e exibe um dashboard atualizado a cada segundo.

## 📊 Dashboard de monitoramento

```
╔═══════════════════════════════════════════════════════╗
║              🚪 MONITOR DA SALA 🚪                    ║
╠═══════════════════════════════════════════════════════╣
║                                                       ║
║  📖 Leitores dentro:       3                          ║
║  ✏️  Escritores aguardando: 1                         ║
║                                                       ║
╠═══════════════════════════════════════════════════════╣
║  📋 ÚLTIMOS EVENTOS:                                  ║
╠═══════════════════════════════════════════════════════╣
║ [14:23:45] Leitor 5 entrou na sala                    ║
║ [14:23:46] Escritor 2 entrou na fila                  ║
║ [14:23:47] Leitor 3 saiu da sala                      ║
║ [14:23:50] Leitor 5 saiu da sala                      ║
║ [14:23:51] Escritor 2 escrevendo!                     ║
╚═══════════════════════════════════════════════════════╝
```

## 🔧 Estrutura do código

### Variáveis globais

- `leitores_dentro`: Contador de leitores ativos
- `escritores_aguardando`: Contador de escritores na fila

### Semáforos

- `porta_leitores`: Controla o acesso de novos leitores
- `porta_escritores`: Controla o acesso de escritores
- `sala`: Garante exclusão mútua na sala
- `vagas_sala`: Limita a capacidade a 15 leitores

### Mutexes

- `mutex`: Protege variáveis compartilhadas
- `mutex_dashboard`: Protege o acesso ao dashboard

### Threads

- **Leitores** (10 threads): Tentam ler periodicamente
- **Escritores** (3 threads): Tentam escrever com menor frequência
- **Monitor** (1 thread): Atualiza o dashboard visual

## 📝 Parâmetros configuráveis

```c
#define NUM_LEITORES 10      // Número de threads leitoras
#define NUM_ESCRITORES 3     // Número de threads escritoras
#define MAX_LOGS 5           // Quantidade de eventos no log
```

Você pode ajustar esses valores no início do arquivo para testar diferentes cenários.

## 🎯 Solução para starvation

A implementação garante que escritores não sofram starvation através do mecanismo de prioridade:

1. Quando um escritor chega, a porta dos leitores é fechada
2. Novos leitores ficam bloqueados até todos os escritores terminarem
3. Escritores em fila são processados sequencialmente
4. Apenas após o último escritor sair, os leitores podem entrar novamente

## 📚 Conceitos aplicados

- **Threads POSIX**: Programação concorrente
- **Semáforos**: Sincronização e contagem
- **Mutexes**: Exclusão mútua para regiões críticas
- **Problema clássico**: Leitores e escritores com prioridade
- **Deadlock prevention**: Ordem consistente de aquisição de recursos

## 🐛 Troubleshooting

**Caracteres não aparecem corretamente:**
- Certifique-se de que seu terminal suporta UTF-8
- No Linux, geralmente já é o padrão

**Erro de compilação com pthread:**
- Verifique se a flag `-lpthread` está sendo usada
- Em alguns sistemas, pode ser necessário `-pthread`

**Tela não limpa corretamente:**
- O comando `clear` precisa estar disponível
- Em Windows, substitua por `system("cls")`

## 📄 Licença

Este projeto é de código aberto e está disponível para fins educacionais.

## 👨‍💻 Autor

Desenvolvido como material didático para estudo de sincronização de threads e problemas clássicos de sistemas operacionais.