#include <stack>
#include <ctype.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <string>

//#define DEBUG

typedef std::vector<int> ivec;

std::vector<std::string> listanomes;
std::vector<int> tamregras;
std::vector<int> regraesq;

#define MAX_ESTADOS 256
#define MAX_COLUNAS 256

int tabela_n[MAX_ESTADOS][MAX_COLUNAS];
char tabela_a[MAX_ESTADOS][MAX_COLUNAS];

void poetab_n(int estado, int coluna, int valor) {
    tabela_n[estado][coluna] = valor;
}

void poetab_a(int estado, int coluna, char valor) {
    tabela_a[estado][coluna] = valor;
}

int pegarnomenum(const char *nome) {
    for (int i = 0; i < (int)listanomes.size(); i++) {
        if (listanomes[i] == nome) {
            return i;
        }
    }
    return -1;
}

int pegar_a(int estado, int coluna) {
    char nome[25];

    if (coluna != 0) {
        sprintf(nome, "'%d'", coluna);
        coluna = pegarnomenum(nome);
    }
    if (estado < 0 || estado >= MAX_ESTADOS) {
        return '?';
    }
    if (coluna < 0 || coluna >= MAX_COLUNAS) {
        return '?';
    }
    return tabela_a[estado][coluna];
}

int pegar_n(int estado, int coluna) {
    char nome[25];

    if (coluna != 0) {
        sprintf(nome, "'%d'", coluna);
        coluna = pegarnomenum(nome);
    }
    if (estado < 0 || estado >= MAX_ESTADOS) {
        return -1;
    }
    if (coluna < 0 || coluna >= MAX_COLUNAS) {
        return -1;
    }
    return tabela_n[estado][coluna];
}


void abrarq(void) {
    {
        for (int i = 0; i < MAX_ESTADOS; i++) {
            for (int j = 0; j < MAX_COLUNAS; j++) {
                tabela_n[i][j] = -1;
            }
        }
    }
    memset(tabela_a, 0, sizeof(tabela_a));

    char linha[256];
    enum {
        NENHUM,
        LENDO_TERMINAIS,
        DEPOIS_TERMINAIS,
        LENDO_NAOTERMINAIS,
        DEPOIS_NAOTERMINAIS,
        LENDO_REGRAS,
        DEPOIS_REGRAS,
        LENDO_ESTADOS,
        LER_INI_ESTADO,
        LENDO_MEIO_ESTADO,
        LENDO_FIM_ESTADO,
        ACABOU
    } estado;

    estado = NENHUM;
    //FILE *f = fopen("/media/sf_vshared/ptgrila.txt", "rt");
    FILE *f = fopen("data/ptgrila.txt", "rt");
    int pular_linhas = 0;
    int state = -1;
    while (fgets(linha, sizeof(linha), f)) {
        std::string l = linha;

        switch(estado) {
            case NENHUM:
                if (l == "Terminals\r\n") {
                    estado = LENDO_TERMINAIS;
                    pular_linhas = 2;
                }
                break;
            case LENDO_TERMINAIS:
                if (pular_linhas > 0) {
                    pular_linhas--;
                } else {
                    if (l == "\r\n") {
                        estado = DEPOIS_TERMINAIS;
                    } else {
                        int id;
                        char nome[100];
                        sscanf(linha, "%d %s\r", &id, nome);
                        #ifdef DEBUG
                        printf("terminal <%d> <%s>\n", id, nome);
                        #endif

                        listanomes.push_back(nome);
                    }
                }
                break;
            case DEPOIS_TERMINAIS:
                if (l == "Nonterminals\r\n") {
                    estado = LENDO_NAOTERMINAIS;
                    pular_linhas = 2;
                }
                break;
            case LENDO_NAOTERMINAIS:
                if (pular_linhas > 0) {
                    pular_linhas--;
                } else {
                    if (l == "\r\n") {
                        estado = DEPOIS_NAOTERMINAIS;
                    } else {
                        int id;
                        char nome[100];
                        sscanf(linha, "%d %s\r", &id, nome);
                        #ifdef DEBUG
                        printf("naoterminal <%d> <%s>\n", id, nome);
                        #endif

                        listanomes.push_back(nome);
                    }
                }
                break;
            case DEPOIS_NAOTERMINAIS:
                if (l == "Rules\r\n") {
                    estado = LENDO_REGRAS;
                    pular_linhas = 2;
                }
                break;
            case LENDO_REGRAS:
                if (pular_linhas > 0) {
                    pular_linhas--;
                } else {
                    if (l == "\r\n") {
                        estado = DEPOIS_REGRAS;
                    } else {
                        bool tentar = false;
                        int tamanho  = 0;
                        int numregra;
                        sscanf(linha, "%d", &numregra);
                        const char *s = linha;
                        while(!isspace(*s)) {
                            s++;
                        }
                        while(isspace(*s)) {
                            s++;
                        }
                        char nome[30];
                        int i= 0;
                        while(!isspace(*s)) {
                            nome[i++] = *s;
                            s++;
                        }
                        nome[i] = '\0';
                        int ladoesq = pegarnomenum(nome);

                        for (s = linha; *s != '\r'; s++) {
                            if (*s == ':') {
                                tentar = true;
                            }
                            if (!tentar) {
                                continue;
                            }
                            if (*s == '-') {
                                tamanho++;
                            } else if (*s == '<') {
                                tamanho++;
                            }
                        }
                        #ifdef DEBUG
                        printf("REGRA %d tamanho=%d ladoesq=%d <%s>\n", numregra, tamanho, ladoesq, nome);
                        #endif
                        tamregras.push_back(tamanho);
                        regraesq.push_back(ladoesq);
                    }
                }
                break;
            case DEPOIS_REGRAS:
                if (l == "LALR States\r\n") {
                    estado = LENDO_ESTADOS;
                    pular_linhas = 2;
                }
                break;


            case LENDO_ESTADOS:
                if (pular_linhas > 0) {
                    pular_linhas--;
                } else {
                    if (sscanf(linha, "State %d\r\n", &state) == 1) {
                        #ifdef DEBUG
                        printf("estado <%d>\n", state);
                        #endif
                        estado = LER_INI_ESTADO;
                    }
                    if (strncmp(linha, "====", 4) == 0) {
                        estado = ACABOU;
                    }
                }
                break;
            case LER_INI_ESTADO:
                if (strncmp(linha, "        Prior States:", 21) == 0) {
                    //printf("AAA [%s]\n", linha);
                    pular_linhas = 2;
                } else {
                    pular_linhas = 0;
                }
                estado = LENDO_MEIO_ESTADO;
                break;
            case LENDO_MEIO_ESTADO:
                if (pular_linhas > 0) {
                    pular_linhas--;
                } else {
                    if (strncmp(linha, "\r\n",2) == 0) {
                        estado = LENDO_FIM_ESTADO;
                    }
                }
                break;
            case LENDO_FIM_ESTADO:
                if (strncmp(linha, "\r\n",2) == 0) {
                    estado = LENDO_ESTADOS;
                } else {
                    char nome[100];
                    char comando;
                    int prox;

                    if (sscanf(linha, "%s %c %d", nome, &comando, &prox) == 3) {
                        int nnum = pegarnomenum(nome);
                        #ifdef DEBUG
                        printf("normal %s %c %d :: (%d,%d)=%c\n", nome, comando, prox, state, nnum, comando);
                        #endif
                        poetab_n(state, nnum, prox);
                        poetab_a(state, nnum, comando);
                    } else if (sscanf(linha, "%s %c", nome, &comando) == 2) {
                        int nnum = pegarnomenum(nome);
                        #ifdef DEBUG
                        printf("aceite %s %c\n", nome, comando);
                        #endif
                        poetab_a(state, nnum, comando);
                    } else {
                        printf("erro\n");
                    }
                }
                break;
            case ACABOU:
                break;
        }
        //printf("%s", linha);
    }
    

    fclose(f);
}

char entrada[] = "-77 -99 -77 -105 0";
const char *sentrada;
int look = -1;

int lerentrada() {
    if (look == 0) {
        return look;
    }
    if (scanf("%d", &look) != 1) {
        look = 0;
    }
    return look;
#if 0
    while(isspace(*sentrada)) {
        sentrada++;
    }
    if (*sentrada == '\0') {
        look = 0;
        return look;
    }
    int num;
    sscanf(sentrada, "%d", &num);
    while(*sentrada == '-' || (*sentrada >= '0' && *sentrada <= '9')) {
        sentrada++;
    }
    look = num;
    return look;
#endif
}

void vai() {
    std::stack<int> pilha;

    sentrada = entrada;
    int num;
    pilha.push(0);
    lerentrada();


    int vezes = 11;

    while(true) {
        #ifdef DEBUG
        printf("%d\n", look);
        #endif

        if (look == -1) {
            printf("erro no lex\n");
            break;
        }

        int topo = pilha.top();

        #ifdef DEBUG
        printf("tenho pilha = %d fita = %d\n", topo, look);
        #endif

        int comando = pegar_a(topo, look);
        int numtab = pegar_n(topo, look);

        #ifdef DEBUG
        printf("achei %c (%d)\n", comando, comando);
        printf("achei %d\n", numtab);
        #endif

        if (comando == 's') {
            pilha.push(look);
            lerentrada();
            pilha.push(numtab);
        } else if (comando == 'r') {
            int popcount = tamregras[numtab] * 2;
            #ifdef DEBUG
            printf("vou dar pop de %d\n", popcount);
            printf("tamanho pilha = %d\n", (int)pilha.size());
            #endif

            for (int i = 0; i < popcount; i++) {
                pilha.pop();
            }

            int estadoolhar = pilha.top();
            int ladoesq = regraesq[numtab];
            //pilha.push(
            int comando2 = tabela_a[estadoolhar][ladoesq];
            int numtab2 = tabela_n[estadoolhar][ladoesq];


            #ifdef DEBUG
            printf("vou olhar em linha=%d coluna=%d\n", estadoolhar, ladoesq);
            printf("2achei %c (%d)\n", comando2, comando2);
            printf("2achei %d\n", numtab2);
            #endif


            pilha.push(ladoesq);
            pilha.push(numtab2);
        } else {
            if (comando == 'a') {
                printf("aceitou\n");
            } else {
                printf("erro\n");
            }
            break;
        }
        //vezes--;
        //if (vezes <= 0) {
        //    break;
        //}
    }

}

int main(void) {
    abrarq();
    vai();
    
    return 0;
}
