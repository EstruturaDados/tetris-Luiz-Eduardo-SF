/*
 * CÓDIGO FEITO POR: LUIZ EDUARDO DA SILVA FERNANDES
 * TODOS OS COMENTÁRIOS DOS OUTROS CÓDIGOS SÃO APLICADOS A ESSE PROJETO TAMBÉM
 * UM ADENDO É QUE EU NÃO SEI DIREITO SE ENTENDI TUDO O QUE FOI PROPOSTO, MAS OKAY, É ISSO.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Definições Globais ---
#define TAMANHO_FILA 5
#define CAPACIDADE_PILHA 3
#define TAMANHO_HISTORICO 1 // Apenas a última jogada pode ser desfeita (simplificação)

// --- Estrutura para representar uma peça ---
typedef struct {
    int id;
    char nome[2]; // 'I', 'O', 'T', 'L', etc. (max 1 char + '\0')
} Peca;

// --- Estrutura de Controle da Fila Circular ---
typedef struct {
    Peca pecas[TAMANHO_FILA];
    int frente;
    int tras;
} FilaCircular;

// --- Estrutura de Controle da Pilha ---
typedef struct {
    Peca pecas[CAPACIDADE_PILHA];
    int topo;
} Pilha;

// --- Estrutura de Histórico para "Desfazer" (Pilha Simples) ---
// Usaremos esta pilha para armazenar a última peça jogada/reservada
typedef struct {
    Peca peca;
    int estadoAnterior; // 1: Jogada da Fila | 2: Reservada (Push)
} Acao;

typedef struct {
    Acao historico[TAMANHO_HISTORICO];
    int topo;
} Historico;


// Variável global para gerar IDs únicos de peças
static int proximoIdPeca = 1;

// =========================================================
// FUNÇÕES AUXILIARES E DE INICIALIZAÇÃO
// =========================================================

// Gera uma nova peça com ID único e nome aleatório
Peca gerarPeca() {
    Peca novaPeca;
    novaPeca.id = proximoIdPeca++;
    
    // Tipos de peças Tetris: I, O, T, J, L, S, Z
    const char *tipos[] = {"I", "O", "T", "J", "L", "S", "Z"};
    int indiceAleatorio = rand() % 7;
    
    strcpy(novaPeca.nome, tipos[indiceAleatorio]);
    
    return novaPeca;
}

// Inicializa a Fila Circular
void inicializarFila(FilaCircular *f) {
    f->frente = 0;
    f->tras = TAMANHO_FILA - 1; 
    
    for (int i = 0; i < TAMANHO_FILA; i++) {
        f->tras = (f->tras + 1) % TAMANHO_FILA;
        f->pecas[f->tras] = gerarPeca();
    }
}

// Inicializa a Pilha
void inicializarPilha(Pilha *p) {
    p->topo = -1; // Pilha vazia
}

// Inicializa o Histórico
void inicializarHistorico(Historico *h) {
    h->topo = -1;
}

// =========================================================
// FUNÇÕES DA FILA CIRCULAR
// =========================================================

// Remove a peça da frente da fila (JOGAR)
Peca dequeue(FilaCircular *f) {
    Peca pecaJogada = f->pecas[f->frente];
    f->frente = (f->frente + 1) % TAMANHO_FILA;
    return pecaJogada;
}

// Insere uma peça na traseira da fila
void enqueue(FilaCircular *f, Peca novaPeca) {
    f->tras = (f->tras + 1) % TAMANHO_FILA;
    f->pecas[f->tras] = novaPeca;
}

// Visualiza o estado da Fila
void visualizarFila(FilaCircular *f) {
    printf("|| FILA DE FUTURAS (%d/%d) ||\n", TAMANHO_FILA, TAMANHO_FILA);
    printf("[Frente] -> ");
    int i = f->frente;
    int count = 0;

    while (count < TAMANHO_FILA) {
        printf("| %s#%d ", f->pecas[i].nome, f->pecas[i].id);
        i = (i + 1) % TAMANHO_FILA;
        count++;
    }
    printf("<- [Trás]\n");
}

// =========================================================
// FUNÇÕES DA PILHA (RESERVA)
// =========================================================

// Adiciona uma peça ao topo da pilha (RESERVAR)
int push(Pilha *p, Peca peca) {
    if (p->topo < CAPACIDADE_PILHA - 1) {
        p->topo++;
        p->pecas[p->topo] = peca;
        return 1; // Sucesso
    }
    return 0; // Falha (cheia)
}

// Remove e retorna a peça do topo da pilha (USAR RESERVADA)
Peca pop(Pilha *p, int *sucesso) {
    if (p->topo >= 0) {
        Peca pecaReservada = p->pecas[p->topo];
        p->topo--;
        *sucesso = 1; // Sucesso
        return pecaReservada;
    }
    Peca pecaVazia = {-1, ""};
    *sucesso = 0; // Falha (vazia)
    return pecaVazia;
}

// Visualiza o estado da Pilha
void visualizarPilha(Pilha *p) {
    printf("|| PILHA DE RESERVA (%d/%d) ||\n", p->topo + 1, CAPACIDADE_PILHA);
    if (p->topo == -1) {
        printf("[Vazia]\n");
    } else {
        for (int i = p->topo; i >= 0; i--) { // Visualiza do topo para a base
            printf(" | %s#%d %s", p->pecas[i].nome, p->pecas[i].id, (i == p->topo) ? "<- TOPO\n" : "\n");
        }
    }
}

// =========================================================
// FUNÇÕES DO HISTÓRICO (DESFAZER)
// =========================================================

// Salva a ação no histórico
void registrarAcao(Historico *h, Peca peca, int tipo) {
    // Apenas armazena a última ação, limpando a anterior
    h->topo = 0;
    h->historico[h->topo].peca = peca;
    h->historico[h->topo].estadoAnterior = tipo;
}

// =========================================================
// FUNÇÕES ESTRATÉGICAS (NÍVEL MESTRE)
// =========================================================

/**
 * 4 - Trocar peça do topo da pilha com a da frente da fila
 */
void trocarPeca(FilaCircular *f, Pilha *p) {
    if (p->topo == -1) {
        printf("⚠️ Não é possível trocar: A pilha de reserva está VAZIA.\n");
        return;
    }

    // 1. Obter a peça da frente da fila
    Peca pecaFila = f->pecas[f->frente];

    // 2. Obter a peça do topo da pilha
    Peca pecaPilha = p->pecas[p->topo];

    // 3. Trocar na memória
    f->pecas[f->frente] = pecaPilha;
    p->pecas[p->topo] = pecaFila;

    printf("🔄 TROCA REALIZADA:\n");
    printf("   - Peça %s#%d (Fila) trocada com Peça %s#%d (Pilha).\n", 
           pecaFila.nome, pecaFila.id, pecaPilha.nome, pecaPilha.id);
}

/**
 * 5 - Desfazer última jogada (Simplificado)
 * Reverte a última operação de JOGAR ou RESERVAR.
 */
void desfazerJogada(FilaCircular *f, Pilha *p, Historico *h) {
    if (h->topo == -1) {
        printf("⚠️ Não há ações para desfazer.\n");
        return;
    }

    Acao ultimaAcao = h->historico[h->topo];

    if (ultimaAcao.estadoAnterior == 1) { // Peça JOGADA (Dequeue)
        // A peça jogada deve ser reinserida na frente da fila.
        // O elemento inserido no final (o gerado automaticamente) deve ser removido.
        
        // 1. Remove a peça gerada no final (Encontrando a peça 'anterior' à frente)
        f->tras = (f->tras - 1 + TAMANHO_FILA) % TAMANHO_FILA; // Move tras para trás
        
        // 2. A peça desfeita é inserida na posição que era a frente
        f->frente = (f->frente - 1 + TAMANHO_FILA) % TAMANHO_FILA; // Move frente para trás
        f->pecas[f->frente] = ultimaAcao.peca;

        printf("🔙 AÇÃO DESFEITA: Peça %s#%d retornou para a frente da fila.\n", 
               ultimaAcao.peca.nome, ultimaAcao.peca.id);

    } else if (ultimaAcao.estadoAnterior == 2) { // Peça RESERVADA (Push)
        // A peça reservada deve ser removida do topo da pilha (POP) e colocada na frente da fila.
        
        // 1. A peça reservada (que já foi para o histórico) é removida do topo da pilha.
        p->topo--; // Assume que a peça do histórico estava no topo da pilha.

        // 2. A peça deve ser colocada na frente da fila, e a peça gerada removida do final (igual ao caso 1)
        f->tras = (f->tras - 1 + TAMANHO_FILA) % TAMANHO_FILA; 
        f->frente = (f->frente - 1 + TAMANHO_FILA) % TAMANHO_FILA;
        f->pecas[f->frente] = ultimaAcao.peca;
        
        printf("🔙 AÇÃO DESFEITA: Peça %s#%d removida da reserva e retornou para a frente da fila.\n", 
               ultimaAcao.peca.nome, ultimaAcao.peca.id);
    }

    // Remove a ação do histórico
    h->topo = -1;
}

/**
 * 6 - Inverter fila com pilha
 * Move todas as peças da Fila para a Pilha e vice-versa.
 */
void inverterFilaComPilha(FilaCircular *f, Pilha *p) {
    // Verifica se a operação é possível: a Pilha e a Fila têm tamanhos diferentes (5 vs 3)
    // Vamos transferir o máximo possível e garantir que a Fila fique cheia (5) e a Pilha fique cheia (3).

    Peca tempFila[TAMANHO_FILA];
    Peca tempPilha[CAPACIDADE_PILHA];
    int countFila = 0;
    int countPilha = p->topo + 1; // Número atual de peças na pilha

    // 1. Esvaziar Fila para um temporário
    for (int i = 0; i < TAMANHO_FILA; i++) {
        tempFila[i] = dequeue(f); // O ponteiro frente avança até dar a volta
    }
    f->frente = 0;
    f->tras = TAMANHO_FILA - 1; // Reseta a fila para o estado vazio/pronto

    // 2. Esvaziar Pilha para um temporário
    int sucessoPop;
    for (int i = 0; i < CAPACIDADE_PILHA; i++) {
        tempPilha[i] = pop(p, &sucessoPop); // Pop remove do topo (inverte a ordem)
    }
    // A pilha já está resetada (p->topo = -1)

    // 3. Re-inserir: Pilha (3 peças) -> Fila (5 posições)
    for (int i = 0; i < countPilha; i++) {
        enqueue(f, tempPilha[i]);
    }

    // 4. Completar a Fila (5) com o que sobrou da Fila (5 - 3 = 2) ou gerar novas
    // Simplificação: Vamos gerar novas peças para garantir 5 na fila.
    for (int i = countPilha; i < TAMANHO_FILA; i++) {
        enqueue(f, gerarPeca());
    }

    // 5. Re-inserir: Fila (5 peças) -> Pilha (3 posições)
    // A pilha só aceita 3 peças (LIFO). Pegamos as 3 primeiras do tempFila.
    for (int i = 0; i < CAPACIDADE_PILHA; i++) {
        push(p, tempFila[i]);
    }

    printf("🔥 INVERSÃO TOTAL REALIZADA!\n");
    printf("   - Fila agora contém 3 peças da pilha original + 2 peças novas.\n");
    printf("   - Pilha agora contém as 3 primeiras peças da fila original.\n");
}


// =========================================================
// MENU PRINCIPAL
// =========================================================

void exibirMenuMestre() {
    printf("\n--- Sistema de Gerenciamento de Peças (Nível Mestre) ---\n");
    printf("1 - Jogar peça (Dequeue na Fila)\n");
    printf("2 - Reservar peça (Dequeue da Fila e Push na Pilha)\n");
    printf("3 - Usar peça reservada (Pop da Pilha)\n");
    printf("4 - Trocar peça do topo da pilha com a da frente da fila\n");
    printf("5 - Desfazer última jogada\n");
    printf("6 - Inverter Fila com Pilha\n");
    printf("0 - Sair\n");
    printf("Escolha uma opção: ");
}

int main() {
    // Inicialização
    srand(time(NULL));
    FilaCircular fila;
    Pilha pilha;
    Historico historico;

    inicializarFila(&fila);
    inicializarPilha(&pilha);
    inicializarHistorico(&historico);

    int opcao;
    Peca pecaEmUso;
    int sucessoPop; 

    do {
        // Exibe o estado atual das estruturas
        printf("\n=======================================\n");
        visualizarFila(&fila);
        visualizarPilha(&pilha);
        printf("=======================================\n");

        exibirMenuMestre();
        if (scanf("%d", &opcao) != 1) {
            while (getchar() != '\n');
            opcao = -1;
        }

        // Limpa o histórico em ações que não são de "jogar" ou "reservar"
        if (opcao != 1 && opcao != 2 && opcao != 5) {
            inicializarHistorico(&historico);
        }

        switch (opcao) {
            case 1: // Jogar peça
                pecaEmUso = dequeue(&fila);
                registrarAcao(&historico, pecaEmUso, 1); // Registra jogada (Tipo 1)
                printf("✅ PEÇA JOGADA: Tipo %s, ID #%d.\n", pecaEmUso.nome, pecaEmUso.id);
                enqueue(&fila, gerarPeca());
                printf("✅ Nova peça gerada e inserida no final da fila.\n");
                break;

            case 2: { // Reservar peça
                Peca pecaParaReservar = dequeue(&fila);
                if (push(&pilha, pecaParaReservar)) {
                    registrarAcao(&historico, pecaParaReservar, 2); // Registra reserva (Tipo 2)
                    printf("✅ PEÇA RESERVADA: Tipo %s, ID #%d. Movida para a reserva.\n", pecaParaReservar.nome, pecaParaReservar.id);
                    enqueue(&fila, gerarPeca());
                    printf("✅ Nova peça gerada e inserida no final da fila.\n");
                } else {
                    printf("⚠️ A pilha de reserva está CHEIA. A peça %s#%d foi JOGADA/DESCARTADA para manter o fluxo da fila.\n", 
                           pecaParaReservar.nome, pecaParaReservar.id);
                    enqueue(&fila, gerarPeca());
                }
                break;
            }

            case 3: // Usar peça reservada
                pecaEmUso = pop(&pilha, &sucessoPop);
                if (sucessoPop) {
                    printf("✅ PEÇA USADA DA RESERVA: Tipo %s, ID #%d.\n", pecaEmUso.nome, pecaEmUso.id);
                }
                break;
            
            case 4: // Trocar
                trocarPeca(&fila, &pilha);
                break;

            case 5: // Desfazer
                desfazerJogada(&fila, &pilha, &historico);
                break;

            case 6: // Inverter
                inverterFilaComPilha(&fila, &pilha);
                break;

            case 0:
                printf("Encerrando o Tetris Stack. Desafio Mestre concluído!\n");
                break;

            default:
                printf("❌ Opção inválida. Tente novamente.\n");
                break;
        }
    } while (opcao != 0);

    return 0;
}