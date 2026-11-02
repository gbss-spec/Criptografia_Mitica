#include <stdio.h>
#include <stdbool.h>
#include <math.h>

/*Nicholas tem como objetivo:
- Ler um par de números primos a1 e a2;

- Ler um expoente e que seja relativamente primo a (p-1)(q-1);

- Produzir um arquivo txt com os dois números que formam a chave pública;

*/

//Forge Elusive Key (funcao para gerar chave privada)
unsigned long long int forge_elusive_key(int n, int m) //optei por unsigned long long por duas razoes: noa receberei numero negativo e usarei grandes numeros nas operacoes seguintes.
{
    int m0 = m;
    int y = 0, x = 1;

    if (m == 1)
        return 0;

    while (n > 1) {
        int q = n / m;
        int t = m;

        m = n % m;
        n = t;
        t = y;

        y = x - q * y;
        x = t;
    }

    if (x < 0)
        x += m0;

    return x;
}

//Função para checar se um número n é primo
bool primo_check(unsigned long long int n) {
    if (n < 2) return 0; //Caso base: Se n é menor que 2, ele não é primo. Responder "falso".
    if (n % 2 == 0) return 0; //Caso n seja par, ele não é primo. Responder "falso".

    for (int i = 3; i <= sqrt(n); i += 2) { //O loop checa a divisão de n por todos os númemros ímpares até sqrt(n)
        if (n % i == 0) return 0; //Se alguma divisão retornar resto 0, n não é primo.
    }

    return 1; //Se nenhum dos anteriores aconteceu, retornar 1 (Verdadeiro, n é primo).
}

//Função para obter o mdc entre dois números
unsigned long long int mdc(unsigned long long int a, unsigned long long int b) {
    //Antes de qualquer coisa, ordernar as variáveis
    if (b > a) {
        //Criação de variável efêmera para me auxiliar nesta tarefa
        unsigned long long int t = a;
        a = b;
        b = t;
    }

    //Casos base
    if (b == 0) return a;
    if (a == b) return a;

    return mdc(b, a % b); // chamada recursiva para o algoritimo de euclides
}

//Função Main
int main() {
    unsigned long long int entrada1, entrada2, expoente, modulus, pritave_key; //Ao usar unsigned, esta variável expande sua capacidade de representar grandes números positivos, em detrimento de sua capacidade de representar números negativos.

    //Diálogo de Nicholas.
    printf("Bem vindo, filho... Há algo que queiras proteger? Contai-me sobre números interessantes e dar-te-ei trancas em que podes confiar...\n");
    printf("Me dizes dois grandes números primos com, pelo menos, 6 dígitos...\n");

    for (int i = 0; i <= 1; i++) { //Loop para ler entradas

        //Primeira leitura (entrada1)
        if (i == 0) {
            scanf("%llu", &entrada1);

            //Caso entrada1 tenha menos que 6 dígitos...
            if (entrada1 < 100000) {
                //Nicholas nega a entrada e pede que o usuário tente novamente.
                printf("hmmphmm... Este número é muito pequeno... Não daria uma chave boa... Tentas um número maior...\n");
                i--; //diminuir index para repetir entrada
            }
            //Invoca função primo_check para dizer se o número é primo ou não
            else if (primo_check(entrada1) == 0) {
                //Subtrai 1 de i para que a leitura de entrada1 seja feita novamente.
                printf("Hmmphm.. Este número não é primo... Tentas mais uma vez...\n");
                i--;
            }
            else {
                printf("Gostei desse numero... Me fala outro para completar um par...\n");
            }
        }

        //Segunda leitura (Completamente análoga à primeira).
        else if (i == 1) {
            scanf("%llu", &entrada2);

            if (entrada1 < 100000) {
                printf("Hmmphmm... Este número é muito pequeno... Não daria uma chave boa... Tentas um número maior...\n");
                i--;
            }
            else if (primo_check(entrada2) == 0) {
                printf("Hmmphm.. Este número não é primo... Tentas mais uma vez...\n");
                i--;
            }
        }
    } //Fim do loop de leitura para entrada1 e entrada2

    printf("Muito bom... Agora, eu gostaria de um número que fosse primo em relação aos produto dos dois números anteriores subtraídos de 1...\n");
    scanf("%llu", &expoente);

    //Invocar função mdc para verificar se o expoente é primo em relação a (entrada1-1)(entrada2-1)
    while (mdc(expoente, (entrada1 - 1) * (entrada2 - 1)) != 1) {
        //Try again
        printf("Não não... este não satisfaz meu pedido... Tente outro...\n");
        scanf("%llu", &expoente);
    }

    /** A chave publica é o par (n, e)
     * n é o produto p*q
     * 'e' é o expoente relativamente primo a (p-1)(q-1) que já foi dado
     * fonte: aplicação a teoria do numero - slide do professor de MD / pag. 55
     */

    //Nomeei a variável "n" como modulus para que fique mais claro do que se trata.
    modulus = entrada1 * entrada2;

    //Invocar função para calcular d
    unsigned long long int private_key = forge_elusive_key(expoente, (entrada1 - 1) * (entrada2 - 1)); //Aqui esta a chave privada

    //Escrever documento com chave pública
    char* filename = "The_Keys.txt"; //criacao da variavel para armazenar o nome do documento

    //abre o documento especificado em write mode
    FILE* fptr = fopen(filename, "w");

    //retornar erro caso o documento nao possa ser aberto
    if (fptr == NULL) {
        printf("Error: could not open file '%s'.\n", filename);
        return 1;
    }

    //escrever chaves dentro do documento
    fprintf(fptr, "publica: (%llu, %llu)\nprivada: (%llu)", modulus, expoente, private_key);

    //fechar documento
    fclose(fptr);

    printf("Ah sim, sim... Bom! Esse numero satisfaz a condição que te pedi...\n");
    printf("Tua chave está no pergaminho '%s'... Leve-a a Uzur e ele usará esta chave para ocultar teus segredos.  \nAdeus, meu amigo!\n", filename);

    return 0;
}
