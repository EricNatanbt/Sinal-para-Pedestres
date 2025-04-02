#include <string.h>
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "hardware/i2c.h"

//biblioteca da matriz de led
#include "ws2818b.pio.h"

// ============================ Bloco relacionado a Matriz de LED ============================
// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7

//Definição dos sprites (imagens na matriz de led)
    int spriteVerde1[5][5][3] = {
        {{0, 0, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 0, 0}},
        {{0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 255, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 255, 0}},
        {{0, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 255, 0}}};

    int spriteVerde2[5][5][3] = {
        {{0, 0, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 0, 0}},
        {{0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 255, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}}};

    int spriteVermelho[5][5][3] = {
        {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
        {{255, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {255, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {255, 0, 0}, {0, 0, 0}, {255, 0, 0}, {0, 0, 0}}};


// Definição de pixel GRB
typedef struct {
    uint8_t G, R, B; 
} npLED_t;

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

// Função para inicializar os LEDs
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true);
    }
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
    for (uint i = 0; i < LED_COUNT; ++i) {
        leds[i] = (npLED_t){0, 0, 0};
    }
}

// Função para reduzir a claridade dos LEDs em 50%
void reduceBrightness(int sprite[5][5][3]) {
    for (int linha = 0; linha < 5; linha++) {
        for (int coluna = 0; coluna < 5; coluna++) {
            // Reduz cada componente RGB pela metade
            sprite[linha][coluna][0] = sprite[linha][coluna][0] / 7; // Red
            sprite[linha][coluna][1] = sprite[linha][coluna][1] / 7; // Green
            sprite[linha][coluna][2] = sprite[linha][coluna][2] / 7; // Blue
        }
    }
}

// Função para atribuir cor a um LED
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b) {
    leds[index] = (npLED_t){g, r, b};
}

// Função para limpar o buffer de LEDs
void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        npSetLED(i, 0, 0, 0);
    }
}

// Função para escrever os dados nos LEDs
void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}

// Função para calcular índice do LED na matriz
int getIndex(int x, int y) {
    return (y % 2 == 0) ? 24 - (y * 5 + x) : 24 - (y * 5 + (4 - x));
}

// Função para desenhar sprite
void drawSprite(int sprite[5][5][3]) {
    for (int linha = 0; linha < 5; linha++) {
        for (int coluna = 0; coluna < 5; coluna++) {
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, sprite[coluna][linha][0], sprite[coluna][linha][1], sprite[coluna][linha][2]);
        }
    }
    npWrite();
}
//=================================================================================================

//============================Bloco relacionado ao semaforo de pedreste============================
//Definição das entradas do display, leds e botões
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
const uint BUTTON_PIN_A = 5;
const uint BUTTON_PIN_B = 6;
const uint LED_VERDE = 11;
const uint LED_VERMELHO = 13;

// Protótipos de funções
void MensagemDisplay(const char *text[], int lines);
void SinalAberto(int segundos);
void SinalAtencao(void);
void SinalFechado(void);
int EsperandoLeitura(int seg);

// Inicialização do display e área de renderização
struct render_area frame_area;
uint8_t ssd[ssd1306_buffer_length];

//Função de limpar o display
void LimpaDisplay(void) { 
 memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

//Função para aparecer as mensagens no display
void MensagemDisplay(const char *text[], int lines) {
    int y = 0;
    for (int i = 0; i < lines; i++) {
        ssd1306_draw_string(ssd, 5, y, (char *)text[i]);

        y += 8; // Avança para a próxima linha
    }
    render_on_display(ssd, &frame_area);
}

//Sinal verde
void SinalAberto(int segundos) {
    gpio_put(LED_VERDE, 1);
    gpio_put(LED_VERMELHO, 0);
    LimpaDisplay();
    const char *text[] = {
        "SINAL ABERTO",
        " ",
        "PODE",
        " ",
        "ATRAVESSAR"
    };
    MensagemDisplay(text, 5);
    for (int i = 0; i < (segundos/2); i++) { 
            drawSprite(spriteVerde1);
            sleep_ms(1000); // 1 segundo
            npClear();

            drawSprite(spriteVerde2);
            sleep_ms(1000); // 1 segundo
            npClear();
        }
}

//Sinal amarelo
void SinalAtencao(void) {
    gpio_put(LED_VERDE, 1);
    gpio_put(LED_VERMELHO, 1);
    LimpaDisplay();
    const char *text[] = {
        "SINAL DE",
        " ",
        "ATENCAO",
        " ",
        "FECHANDO SINAL",
        " ",
        "EM 3 SEGUNDOS"
    };
    MensagemDisplay(text, 7);
}

//Sinal vermelho
void SinalFechado(void) {
    gpio_put(LED_VERDE, 0);
    gpio_put(LED_VERMELHO, 1);
    LimpaDisplay();
    const char *text[] = {
        "SINAL FECHADO",
        " ",
        "AGUARDE"
    };
    drawSprite(spriteVermelho);
    MensagemDisplay(text, 3);
}


//fica lendo se o botão for pressionado durante sinal vermelho
int EsperandoLeitura(int seg) {
    for (int i = 0; i < seg *5; i++) { // Incrementos de 50ms
        if (gpio_get(BUTTON_PIN_A) == 0) { // Botão A pressionado
         LimpaDisplay();
        const char *text[] = {
        "BOTAO A",
        " ",
        "APERTADO",
        " ",
        "ABRINDO SINAL",
        " ",
        "EM 5 SEGUNDOS"
        };
            MensagemDisplay(text, 7);// Mensagem avisando que Botão A foi pressionado
            return 1;
        }
        else if (gpio_get(BUTTON_PIN_B) == 0) { // Botão B pressionado
         LimpaDisplay();
        const char *text[] = {
        "BOTAO B",
        " ",
        "APERTADO",
        " ",
        "ABRINDO SINAL",
        " ",
        "EM 5 SEGUNDOS"
        };
            MensagemDisplay(text, 7);// Mensagem avisando que Botão B foi pressionado
            return 1;
        }

        sleep_ms(100);
    }
    return 0; // Nennhum Botão foi pressionado
}

int main() {
    // Inicializações padrão dos botões e dos leds
    stdio_init_all();

    npInit(LED_PIN);
    npClear();

    gpio_init(BUTTON_PIN_A);
    gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_A);

    gpio_init(BUTTON_PIN_B);
    gpio_set_dir(BUTTON_PIN_B, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_B);

    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);

    // Inicialização do I2C e display
    i2c_init(i2c1, 400 * 1000); // 400 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();

    // Configuração da área de renderização
    frame_area.start_column = 0;
    frame_area.end_column = ssd1306_width - 1;
    frame_area.start_page = 0;
    frame_area.end_page = ssd1306_n_pages - 1;

    calculate_render_area_buffer_length(&frame_area);

    //Diminuição de claridade da matriz de led
    reduceBrightness(spriteVermelho);
    reduceBrightness(spriteVerde1);
    reduceBrightness(spriteVerde2);

    // Loop principal
   while (true) {
        // Semáforo para pedrestes:
        SinalFechado(); //Vermelho por 15 segundos
        int A_state =  EsperandoLeitura(15); // Verifica o botão A por 15 segundos
        int B_state =  EsperandoLeitura(15); // Verifica o botão B por 15 segundos

        if (A_state == 1 || B_state == 1) {
            sleep_ms(5000); //Botão pressionado, abre sinal em 5 segundos
            SinalAberto(10); //Botão pressionado, fica verde por 10 segundos
            SinalAtencao(); //Sinal amarelo por 3 segundos antes de fechar o sinal
            sleep_ms(3000); 
        } else {
            SinalAberto(8); //Sinal verde por 8 segundos
            SinalAtencao(); //Sinal amarelo por 3 segundos antes de  fechar o sinal
            sleep_ms(3000); 
        }
    }

    return 0;
}