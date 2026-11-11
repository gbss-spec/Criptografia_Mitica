#include "raylib.h"
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned long long int forge_elusive_key(int n, int m) { //funcao para gerar chave secreta
    int m0 = m; int y = 0, x = 1;
    if (m == 1) return 0; //caso base
    while (n > 1) { //enquanto n>1, realizar simplificacao
        int q = n / m; int t = m;
        m = n % m; n = t; t = y;
        y = x - q * y; x = t;
    }
    if (x < 0) x += m0;
    return x;
}

bool primo_check(unsigned long long int n) {
    if (n < 2) return 0;
    if (n % 2 == 0) return 0;
    for (int i = 3; i <= sqrt(n); i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

unsigned long long int mdc(unsigned long long int a, unsigned long long int b) {
    if (b > a) { unsigned long long int t = a; a = b; b = t; }
    if (b == 0) return a;
    if (a == b) return a;
    return mdc(b, a % b);
}

unsigned long long int mod_exp(unsigned long long int base, unsigned long long int exp, unsigned long long int mod) {
    unsigned long long int res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return res;
}

// ==========================================
// FUNÇÃO PARA CARREGAR E REDIMENSIONAR IMAGENS
// ==========================================
Texture2D LoadAndResizeTexture(const char* fileName, int targetHeight) {
    // 1. Carrega a imagem gigante na memória RAM
    Image img = LoadImage(fileName);

    // 2. Calcula a nova largura para não distorcer a imagem (manter proporção)
    float scale = (float)targetHeight / (float)img.height;
    int targetWidth = (int)(img.width * scale);

    // 3. Redimensiona a imagem na RAM
    ImageResize(&img, targetWidth, targetHeight);

    // 4. Converte a Imagem em Textura (que a Placa de Vídeo consegue desenhar)
    Texture2D tex = LoadTextureFromImage(img);

    // 5. Apaga a imagem pesada da RAM, pois já está na Textura
    UnloadImage(img);

    return tex;
}

// ==========================================
// ESTRUTURAS DE ESTADO
// ==========================================
typedef enum { STATE_MENU, STATE_NICHOLAS, STATE_UZUR, STATE_KIARA } GameState;
typedef enum { N_PRIME1, N_PRIME2, N_EXP, N_DONE } NicholasState;
typedef enum { U_ASK_N, U_ASK_E, U_MSG, U_DONE } UzurState;
typedef enum { K_ASK_N, K_ASK_D, K_DONE } KiaraState;

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Cryptotale");
    SetTargetFPS(60);

    InitAudioDevice();

    // Carregando as imagens usando nossa nova função!
    // Todas terão 200 pixels de altura, e a largura será ajustada proporcionalmente.
    Texture2D texNicholas = LoadAndResizeTexture("nicholas.png", 200);
    Texture2D texUzur = LoadAndResizeTexture("uzur.png", 200);
    Texture2D texKiara = LoadAndResizeTexture("kiara.png", 200);

    Sound sndNicholas = LoadSound("snd_nicholas.wav");
    Sound sndUzur = LoadSound("snd_uzur.wav");
    Sound sndKiara = LoadSound("snd_kiara.wav");

    // Variáveis Globais de Estado
    GameState currentState = STATE_MENU;
    int selectedOption = 0; // 0 = Nicholas, 1 = Uzur, 2 = Kiara

    // Sub-estados e variáveis de cada personagem
    NicholasState nicholasState = N_PRIME1;
    UzurState uzurState = U_ASK_N;
    KiaraState kiaraState = K_ASK_N;

    // Chaves de memoria temporária para Nicholas, Uzur e Kiara
    unsigned long long int n_entrada1 = 0, n_entrada2 = 0, n_expoente = 0, n_modulus = 0, n_private_key = 0;
    unsigned long long int uzur_n = 0, uzur_e = 0;
    unsigned long long int kiara_n = 0, kiara_d = 0;

    char currentDialog[512] = "* Escolha com quem deseja falar.";
    int visibleChars = 0;
    float charTimer = 0.0f;
    const float textSpeed = 0.04f;

    char inputText[64] = "\0";
    int letterCount = 0;

    #define SET_DIALOG(text) { strcpy(currentDialog, text); visibleChars = 0; charTimer = 0.0f; }

    while (!WindowShouldClose()) {
        // ==========================================
        // UPDATE: Máquina de Escrever (Typewriter)
        // ==========================================
        int totalChars = strlen(currentDialog);
        if (visibleChars < totalChars) {
            charTimer += GetFrameTime();
            if (charTimer >= textSpeed) {
                visibleChars++;
                charTimer = 0.0f;

                char justRevealed = currentDialog[visibleChars - 1];
                if (justRevealed != ' ' && justRevealed != '\n') {
                    if (currentState == STATE_NICHOLAS) PlaySound(sndNicholas);
                    else if (currentState == STATE_UZUR) PlaySound(sndUzur);
                    else if (currentState == STATE_KIARA) PlaySound(sndKiara);
                }
            }
        }

        // ==========================================
        // UPDATE: Lógica de Inputs e Telas
        // ==========================================

        // ---------------- MENU ----------------
        if (currentState == STATE_MENU) {
            if (IsKeyPressed(KEY_LEFT)) selectedOption--;
            if (IsKeyPressed(KEY_RIGHT)) selectedOption++;
            if (selectedOption < 0) selectedOption = 2;
            if (selectedOption > 2) selectedOption = 0;

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_Z)) {
                letterCount = 0;
                inputText[0] = '\0';

                if (selectedOption == 0) {
                    currentState = STATE_NICHOLAS;
                    nicholasState = N_PRIME1;
                    SET_DIALOG("* Diga-me um grande numero\n  primo (min 6 digitos)...");
                }
                else if (selectedOption == 1) {
                    currentState = STATE_UZUR;
                    uzurState = U_ASK_N;
                    SET_DIALOG("* Para ocultar tua mensagem,\n  preciso do modulo 'n'. Qual e?");
                }
                else if (selectedOption == 2) {
                    currentState = STATE_KIARA;
                    kiaraState = K_ASK_N;
                    SET_DIALOG("* Para revelar o segredo,\n  qual e o modulo 'n'?");
                }
            }
        }

        // ---------------- NICHOLAS (GERAÇÃO) ----------------
        else if (currentState == STATE_NICHOLAS) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 48) && (key <= 57) && (letterCount < 20)) {
                    inputText[letterCount] = (char)key;
                    inputText[letterCount + 1] = '\0';
                    letterCount++;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0) {
                letterCount--;
                inputText[letterCount] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER) && letterCount > 0 && nicholasState != N_DONE && visibleChars == totalChars) {
                unsigned long long int inputNum = strtoull(inputText, NULL, 10);

                if (nicholasState == N_PRIME1) {
                    if (inputNum < 100000) SET_DIALOG("* Muito pequeno... \n  Tentas um numero maior.")
                    else if (!primo_check(inputNum)) SET_DIALOG("* Este numero nao e primo...\n  Tentas mais uma vez.")
                    else {
                        n_entrada1 = inputNum;
                        nicholasState = N_PRIME2;
                        SET_DIALOG("* Perfeito. Diga-me outro para\n  completar o par...");
                    }
                }
                else if (nicholasState == N_PRIME2) {
                    if (inputNum < 100000) SET_DIALOG("* Muito pequeno... \n  Tentas um numero maior.")
                    else if (!primo_check(inputNum)) SET_DIALOG("* Este numero nao e primo...\n  Tentas mais uma vez.")
                    else {
                        n_entrada2 = inputNum;
                        nicholasState = N_EXP;
                        SET_DIALOG("* Agora, um numero primo 'e'\n  em relacao a (p-1)(q-1).");
                    }
                }
                else if (nicholasState == N_EXP) {
                    if (mdc(inputNum, (n_entrada1 - 1) * (n_entrada2 - 1)) != 1) {
                        SET_DIALOG("* Nao serve... Tente outro.");
                    } else {
                        n_expoente = inputNum;
                        n_modulus = n_entrada1 * n_entrada2;
                        n_private_key = forge_elusive_key(n_expoente, (n_entrada1 - 1) * (n_entrada2 - 1));

                        nicholasState = N_DONE;
                        char tempStr[256];
                        sprintf(tempStr, "* ANOTE ISSO!\nModulo (n): %llu\nExpoente (e): %llu\nChave Privada (d): %llu\n(Aperte X)", n_modulus, n_expoente, n_private_key);
                        SET_DIALOG(tempStr);
                    }
                }
                letterCount = 0;
                inputText[0] = '\0';
            }

            if (IsKeyPressed(KEY_X)) {
                currentState = STATE_MENU;
                SET_DIALOG("* Escolha com quem deseja falar.");
            }
        }

        // ---------------- UZUR (ENCRIPTAÇÃO) ----------------
        else if (currentState == STATE_UZUR) {
            int key = GetCharPressed();
            while (key > 0) {
                if (uzurState != U_MSG) {
                    if ((key >= 48) && (key <= 57) && (letterCount < 20)) {
                        inputText[letterCount] = (char)key;
                        inputText[letterCount + 1] = '\0';
                        letterCount++;
                    }
                } else {
                    if ((key >= 32) && (key <= 126) && (letterCount < 40)) {
                        inputText[letterCount] = (char)key;
                        inputText[letterCount + 1] = '\0';
                        letterCount++;
                    }
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0) {
                letterCount--;
                inputText[letterCount] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER) && letterCount > 0 && uzurState != U_DONE && visibleChars == totalChars) {
                if (uzurState == U_ASK_N) {
                    uzur_n = strtoull(inputText, NULL, 10);
                    uzurState = U_ASK_E;
                    SET_DIALOG("* Compreendo... E qual e o\n  expoente 'e' publico?");
                }
                else if (uzurState == U_ASK_E) {
                    uzur_e = strtoull(inputText, NULL, 10);
                    uzurState = U_MSG;
                    SET_DIALOG("* As trancas estao prontas.\n  Escreva seu segredo (Ate 40 let).");
                }
                else if (uzurState == U_MSG) {
                    FILE* cryptPtr = fopen("Secret.txt", "w");
                    for (int i = 0; i < letterCount; i++) {
                        unsigned long long int encrypted_char = mod_exp(inputText[i], uzur_e, uzur_n);
                        fprintf(cryptPtr, "%llu ", encrypted_char);
                    }
                    fclose(cryptPtr);

                    SET_DIALOG("* As sombras ocultaram tuas\n  palavras no arquivo Secret.txt.\n  (Aperte X para voltar)");
                    uzurState = U_DONE;
                }
                letterCount = 0;
                inputText[0] = '\0';
            }
            if (IsKeyPressed(KEY_X)) {
                currentState = STATE_MENU;
                SET_DIALOG("* Escolha com quem deseja falar.");
            }
        }

        // ---------------- KIARA (DECRIPTAÇÃO) ----------------
        else if (currentState == STATE_KIARA) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 48) && (key <= 57) && (letterCount < 20)) {
                    inputText[letterCount] = (char)key;
                    inputText[letterCount + 1] = '\0';
                    letterCount++;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0) {
                letterCount--;
                inputText[letterCount] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER) && letterCount > 0 && kiaraState != K_DONE && visibleChars == totalChars) {
                if (kiaraState == K_ASK_N) {
                    kiara_n = strtoull(inputText, NULL, 10);
                    kiaraState = K_ASK_D;
                    SET_DIALOG("* Entendido... E qual e a tua\n  Chave Privada (d)?");
                }
                else if (kiaraState == K_ASK_D) {
                    kiara_d = strtoull(inputText, NULL, 10);

                    FILE* fSecret = fopen("Secret.txt", "r");
                    if (fSecret == NULL) {
                        SET_DIALOG("* Nao sinto segredos no ar...\n  O arquivo Secret.txt sumiu.");
                    } else {
                        char decryptedMessage[256] = "";
                        unsigned long long int cryptChar;
                        int i = 0;
                        while (fscanf(fSecret, "%llu", &cryptChar) != EOF) {
                            unsigned long long int plainChar = mod_exp(cryptChar, kiara_d, kiara_n);
                            decryptedMessage[i] = (char)plainChar;
                            i++;
                        }
                        decryptedMessage[i] = '\0';
                        fclose(fSecret);

                        char tempStr[256];
                        sprintf(tempStr, "* A luz revela a verdade:\n  \"%s\"\n(Aperte X)", decryptedMessage);
                        SET_DIALOG(tempStr);
                    }
                    kiaraState = K_DONE;
                }
                letterCount = 0;
                inputText[0] = '\0';
            }
            if (IsKeyPressed(KEY_X)) {
                currentState = STATE_MENU;
                SET_DIALOG("* Escolha com quem deseja falar.");
            }
        }

        // ==========================================
        // DRAWING (Lógica Visual)
        // ==========================================
        BeginDrawing();
        ClearBackground(BLACK);

        // Caixa de Diálogo Base
        DrawRectangleLines(50, 420, 700, 150, WHITE);
        DrawRectangleLines(52, 422, 696, 146, WHITE);

        // Texto desenhado pela Máquina de Escrever
        DrawText(TextSubtext(currentDialog, 0, visibleChars), 80, 450, 25, WHITE);

        // ---------------- DESENHANDO A TELA DO MENU ----------------
        if (currentState == STATE_MENU) {
            DrawText("SELECIONE O PERSONAGEM (Use as Setas <- e ->)", 150, 30, 20, GRAY);

            // NICHOLAS (Índice 0)
            Color colorNic = (selectedOption == 0) ? WHITE : DARKGRAY;
            DrawTexture(texNicholas, 133 - texNicholas.width/2, 100, colorNic);
            DrawText("Nicholas", 133 - MeasureText("Nicholas", 20)/2, 330, 20, colorNic);
            DrawText("O Forjador", 133 - MeasureText("O Forjador", 15)/2, 350, 15, colorNic);
            if(selectedOption == 0) DrawRectangleLines(133 - texNicholas.width/2 - 10, 90, texNicholas.width + 20, texNicholas.height + 20, WHITE);

            // UZUR (Índice 1)
            Color colorUz = (selectedOption == 1) ? WHITE : DARKGRAY;
            DrawTexture(texUzur, 400 - texUzur.width/2, 100, colorUz);
            DrawText("Uzur", 400 - MeasureText("Uzur", 20)/2, 330, 20, colorUz);
            DrawText("O Ocultista", 400 - MeasureText("O Ocultista", 15)/2, 350, 15, colorUz);
            if(selectedOption == 1) DrawRectangleLines(400 - texUzur.width/2 - 10, 90, texUzur.width + 20, texUzur.height + 20, WHITE);

            // KIARA (Índice 2)
            Color colorKia = (selectedOption == 2) ? WHITE : DARKGRAY;
            DrawTexture(texKiara, 666 - texKiara.width/2, 100, colorKia);
            DrawText("Kiara", 666 - MeasureText("Kiara", 20)/2, 330, 20, colorKia);
            DrawText("A Reveladora", 666 - MeasureText("A Reveladora", 15)/2, 350, 15, colorKia);
            if(selectedOption == 2) DrawRectangleLines(666 - texKiara.width/2 - 10, 90, texKiara.width + 20, texKiara.height + 20, WHITE);
        }

        // ---------------- TELAS INDIVIDUAIS DOS PERSONAGENS ----------------
        else {
            int centerX = screenWidth / 2;

            if (currentState == STATE_NICHOLAS) {
                DrawText("--- NICHOLAS ---", centerX - MeasureText("--- NICHOLAS ---", 20)/2, 20, 20, GRAY);
                DrawTexture(texNicholas, centerX - texNicholas.width/2, 50, WHITE);

                if (nicholasState != N_DONE && visibleChars == totalChars) {
                    DrawText("DIGITE AQUI:", centerX - 100, 310, 20, LIGHTGRAY);
                    DrawRectangleLines(centerX - 110, 340, 220, 40, WHITE);
                    DrawText(inputText, centerX - 100, 350, 20, WHITE);
                    if ((int)(GetTime() * 2) % 2 == 0) DrawText("_", centerX - 100 + MeasureText(inputText, 20), 350, 20, WHITE);
                }
            }
            else if (currentState == STATE_UZUR) {
                DrawText("--- UZUR ---", centerX - MeasureText("--- UZUR ---", 20)/2, 20, 20, GRAY);
                DrawTexture(texUzur, centerX - texUzur.width/2, 50, WHITE);

                if (uzurState != U_DONE && visibleChars == totalChars) {
                    int boxWidth = (uzurState == U_MSG) ? 420 : 220;
                    DrawText("DIGITE AQUI:", centerX - boxWidth/2 + 10, 310, 20, LIGHTGRAY);
                    DrawRectangleLines(centerX - boxWidth/2, 340, boxWidth, 40, WHITE);
                    DrawText(inputText, centerX - boxWidth/2 + 10, 350, 20, WHITE);
                    if ((int)(GetTime() * 2) % 2 == 0) DrawText("_", centerX - boxWidth/2 + 10 + MeasureText(inputText, 20), 350, 20, WHITE);
                }
            }
            else if (currentState == STATE_KIARA) {
                DrawText("--- KIARA ---", centerX - MeasureText("--- KIARA ---", 20)/2, 20, 20, GRAY);
                DrawTexture(texKiara, centerX - texKiara.width/2, 50, WHITE);

                if (kiaraState != K_DONE && visibleChars == totalChars) {
                    DrawText("DIGITE AQUI:", centerX - 100, 310, 20, LIGHTGRAY);
                    DrawRectangleLines(centerX - 110, 340, 220, 40, WHITE);
                    DrawText(inputText, centerX - 100, 350, 20, WHITE);
                    if ((int)(GetTime() * 2) % 2 == 0) DrawText("_", centerX - 100 + MeasureText(inputText, 20), 350, 20, WHITE);
                }
            }
        }

        EndDrawing();
    }

    UnloadTexture(texNicholas);
    UnloadTexture(texUzur);
    UnloadTexture(texKiara);
    UnloadSound(sndNicholas);
    UnloadSound(sndUzur);
    UnloadSound(sndKiara);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}
