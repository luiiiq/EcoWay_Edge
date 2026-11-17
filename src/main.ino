/*
Projeto: EcoWay - Monitoramento de Calor Urbano

Equipe:
Integrantes:
- Caio Fernando De Deus Gomes
- Amanda Souza de Barros
- Luiza de Freitas Benevides
- Mariana do Nascimento
- Lirity Ribeiro de Paiva

Placa: Arduino Uno

Descrição:
O EcoWay é um sistema desenvolvido com Arduino Uno para monitorar as 
condições ambientais por meio dos sensores DHT22, LDR e ultrassônico HC-SR04
, além de um potenciômetro que simula o índice UV. O sistema analisa a 
temperatura, a umidade e o índice UV, classificando o estado geral em OK, 
Alerta ou ALTO e acionando os LEDs indicadores conforme os resultados. 
O LDR identifica a luminosidade do ambiente (CLARO, LUZ AMBIENTE ou ESCURO),
sem interferir na classificação geral, enquanto o sensor ultrassônico 
identifica a aproximação do usuário para iniciar o monitoramento. 
As informações são exibidas no LCD, enviadas pela Serial em formato JSON 
e armazenadas na memória EEPROM.

Rev: 1 - Monitoramento do ambiente inicial

Componentes:
- DHT22: medição de temperatura e umidade
- LDR: identificação da luminosidade do ambiente
- Potenciômetro: simulação do índice UV
- Sensor ultrassônico HC-SR04: identificação da aproximação do usuário
- LCD I2C 16x2: exibição das informações
- LEDs verde, amarelo e vermelho: indicação do estado geral
- EEPROM interna do Arduino Uno: armazenamento dos registros
*/

// -------------------- BIBLIOTECAS --------------------

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <EEPROM.h>
#include <string.h>
#include <avr/pgmspace.h>

// -------------------- PINOS --------------------

#define DHT_PIN 2
#define DHT_TYPE DHT22
#define LDR A0

#define LED_VERDE 13
#define LED_AMARELO 10
#define LED_VERMELHO 9

#define POT_UV A2

#define TRIG 7
#define ECHO 8

const int DISTANCIA_MAXIMA = 200;

// -------------------- OBJETOS --------------------

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------------------- EEPROM --------------------

const int MAX_REGISTROS = 15;

const uint8_t ASSINATURA = 0xE8;

struct Registro {
  uint32_t tempoMillis;
  float temperatura;
  float umidade;
  char estado[12];
};

struct DadosSalvos {
  uint8_t assinatura;
  uint8_t proximoIndice;
  uint8_t quantidade;
  Registro registros[MAX_REGISTROS];
};

DadosSalvos dados;

// -------------------- LEITURA DO LDR --------------------

int valorLDR = 0;
int porcentagemLuz = 0;
int luminosidade = 0;
const int NUM_LEITURAS = 10;

// -------------------- LEITURA DO POTENCIÔMETRO --------------------
int valorPotUV = 0;
int indiceUV = 0;

// -------------------- STATUS DOS SENSORES --------------------

const char* statusTemperatura;
const char* statusUmidade;
const char* statusLuminosidade;
const char* estado;
const char* statusUV;

// -------------------- CONTROLE --------------------

unsigned long ultimaLeitura = 0;
const unsigned long INTERVALO_LEITURA = 5000;

unsigned long ultimoRegistro = 0;
const unsigned long INTERVALO_REGISTRO = 60000;

unsigned long ultimaTrocaTela = 0;

int telaAtual = 0;

const unsigned long INTERVALO_TELA = 3000;

bool monitoramentoAtivo = false;
bool animacaoExecutada = false;
bool mensagemAguardando = false;

// Animação
// FRAME 01 - Animação: Introdução da folha (Cena inicial de tamanho intermédio)
const byte frame01_0[8] PROGMEM = {
  B11110,
  B11110,
  B11111,
  B11111,
  B11110,
  B11100,
  B11100,
  B11100
};

const byte frame01_1[8] PROGMEM = {
  B11010,
  B11110,
  B11000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte frame01_2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00001,
  B00011,
  B00010,
  B00110,
  B11111
};

const byte frame01_3[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00011,
  B11111,
  B10000
};

const byte frame01_4[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B11000,
  B10110,
  B00001,
  B11001,
  B00111
};

const byte frame01_5[8] PROGMEM = {
  B11100,
  B11100,
  B11111,
  B11111,
  B11111,
  B11111,
  B11110,
  B11111
};

const byte frame01_6[8] PROGMEM = {
  B00011,
  B01111,
  B11111,
  B11100,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte frame01_7[8] PROGMEM = {
  B11000,
  B00000,
  B10000,
  B11000,
  B00000,
  B00000,
  B00000,
  B00000
};


// ZOOM A: Primeira fase da aproximação da folha à câmara

const byte zoomA0[8] PROGMEM = {
  B11100,
  B11110,
  B11111,
  B11111,
  B11110,
  B11100,
  B11000,
  B10000
};

const byte zoomA1[8] PROGMEM = {
  B11000,
  B11100,
  B11110,
  B11000,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte zoomA2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00011,
  B00111,
  B00110,
  B01100,
  B11100
};

const byte zoomA3[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00001,
  B00011,
  B00111,
  B11100,
  B10000
};

const byte zoomA4[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B10000,
  B11000,
  B01100,
  B00110,
  B00011
};

const byte zoomA5[8] PROGMEM = {
  B11000,
  B11100,
  B11110,
  B11111,
  B11111,
  B11110,
  B11100,
  B11000
};

const byte zoomA6[8] PROGMEM = {
  B00001,
  B00111,
  B01111,
  B11110,
  B11000,
  B10000,
  B00000,
  B00000
};


// ZOOM B: Transição intermédia de escala

const byte zoomB0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00011,
  B00111,
  B00100
};

const byte zoomB1[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B11000,
  B01000
};

const byte zoomB2[8] PROGMEM = {
  B00000,
  B00000,
  B00001,
  B01111,
  B11111,
  B11110,
  B11000,
  B00000
};

const byte zoomB3[8] PROGMEM = {
  B00000,
  B00000,
  B00011,
  B11110,
  B11000,
  B10000,
  B00000,
  B00000
};

const byte zoomB4[8] PROGMEM = {
  B00000,
  B10000,
  B11000,
  B11100,
  B01110,
  B00110,
  B00000,
  B00000
};

const byte zoomB5[8] PROGMEM = {
  B00000,
  B00100,
  B01100,
  B11010,
  B01110,
  B00100,
  B00000,
  B00000
};


// FRAME 02: Estabilização da trajetória da folha

const byte frame02_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B00100
};

const byte frame02_1[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B01000
};

const byte frame02_2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00111,
  B11111,
  B11111,
  B11100,
  B00000
};

const byte frame02_3[8] PROGMEM = {
  B00000,
  B00000,
  B00111,
  B11100,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte frame02_4[8] PROGMEM = {
  B01000,
  B10001,
  B11110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte frame02_5[8] PROGMEM = {
  B00100,
  B10010,
  B11001,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000
};


// FRAME 03 / 04: Estrutura estática da folha no ponto médio

const byte folhaBase0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00111,
  B11111,
  B11111,
  B11100,
  B00000
};

const byte folhaBase1[8] PROGMEM = {
  B00000,
  B00000,
  B00111,
  B11100,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte frame03_2[8] PROGMEM = {
  B00000,
  B00001,
  B11010,
  B00101,
  B00010,
  B00001,
  B00000,
  B00000
};

const byte frame03_3[8] PROGMEM = {
  B01100,
  B10011,
  B00001,
  B10100,
  B00001,
  B00010,
  B11100,
  B00000
};

const byte frame03_4[8] PROGMEM = {
  B00000,
  B11010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};


// Folha Voando - FRAME A: Oscilação (Fase superior)

const byte vooA0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B01100,
  B10000
};

const byte vooA1[8] PROGMEM = {
  B00000,
  B00000,
  B01000,
  B11000,
  B01000,
  B01000,
  B10000,
  B10000
};

const byte vooA2[8] PROGMEM = {
  B00000,
  B00001,
  B00010,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte vooA3[8] PROGMEM = {
  B11001,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};


// Folha Voando - FRAME B: Oscilação aerodinâmica (Fase inferior)

const byte vooB0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00100,
  B01000
};

const byte vooB1[8] PROGMEM = {
  B00000,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte vooB2[8] PROGMEM = {
  B11111,
  B00011,
  B11110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte vooB3[8] PROGMEM = {
  B11000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

// Arrays para inversão
// Guardam temporariamente versões invertidas dos desenhos da folha.

byte invertido0[8];
byte invertido1[8];
byte invertido2[8];
byte invertido3[8];

// Transição Final - ENTRADA 1: Início do fecho da cena 
const byte entrada1_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00110
};

const byte entrada1_1[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00110,
  B11000
};

// Transição Final - ENTRADA 2: Avanço do preenchimento geométrico
const byte entrada2_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B11000
};

const byte entrada2_1[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00001,
  B00110,
  B01000,
  B10000,
  B00000
};

const byte entrada2_2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B01000,
  B00100,
  B00010
};


// Transição Final - ENTRADA 3: Quase oclusão do espaço

const byte entrada3_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B01000
};

const byte entrada3_1[8] PROGMEM = {
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000
};

const byte entrada3_2[8] PROGMEM = {
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000
};

const byte entrada3_3[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};

// Frame Central - Folha mais Próxima: O pico do efeito "Macro/Zoom"
const byte close0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B01100,
  B10000
};

const byte close1[8] PROGMEM = {
  B00100,
  B01000,
  B10000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte close2[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00001,
  B00110,
  B01000,
  B10000
};

const byte close3[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11110,
  B00001,
  B00000
};

const byte close4[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B11000,
  B00000,
  B00000,
  B11000,
  B00110
};

const byte close5[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B01111,
  B00000,
  B00000,
  B00000
};

const byte close6[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00011,
  B00010,
  B00100,
  B01000
};

const byte close7[8] PROGMEM = {
  B00000,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000
};

// Transição Final - SAÍDA 1: Limpeza da tela por varrimento diagonal
const byte saida1_0[8] PROGMEM = {
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000
};

const byte saida1_1[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};

const byte saida1_2[8] PROGMEM = {
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000
};

const byte saida1_3[8] PROGMEM = {
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

// Transição Final - SAÍDA 2: Conclusão do corte
const byte saida2_0[8] PROGMEM = {
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000
};

const byte saida2_1[8] PROGMEM = {
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000
};

const byte saida2_2[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};

// Transição Final - SAÍDA 3
// Descrição: Frame final de corte geométrico antes da revelação do logótipo.
const byte saida3_0[8] PROGMEM = {
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B00100,
  B01000,
  B10000
};

const byte saida3_1[8] PROGMEM = {
  B00001,
  B00010,
  B00100,
  B01000,
  B10000,
  B00000,
  B00000,
  B00000
};

// Logo
char logoLinha1[] = "      ECO    ";
char logoLinha2[] = "      WAY      ";

// Carrega um desenho armazenado na memória Flash para o LCD.
void criarCaractere(byte posicao, const byte desenho[]) {

  byte buffer[8];

  memcpy_P(buffer, desenho, 8);

  lcd.createChar(posicao, buffer);
};


// Limpa as Duas linhas LCD, evitando que partes de uma imagem antiga permaneçam visíveis.
void limpaTela() {
  lcd.setCursor(0, 0);
  lcd.print("                "); // Limpa tela superior

  lcd.setCursor(0, 1);
  lcd.print("                "); // Limpa tela inferior
}

// Cada bitmap tem 8 linhas. Copiamos as linhas de baixo para cima
// para produzir uma versão verticalmente invertida do desenho.
// Recebe um bitmap de 8 linhas e grava no destino a ordem inversa das linhas.
// Inverte verticalmente um desenho armazenado na memória Flash.
void inverterVertical(const byte original[8], byte destino[8]) {
  for (int i = 0; i < 8; i++) {

    destino[i] = pgm_read_byte(&original[7 - i]);
  }
}

// FRAME 01 -- posiciona cada parte da folha:
void mostrarFrame01() {
  criarCaractere(0, frame01_0);
  criarCaractere(1, frame01_1);
  criarCaractere(2, frame01_2);
  criarCaractere(3, frame01_3);
  criarCaractere(4, frame01_4);
  criarCaractere(5, frame01_5);
  criarCaractere(6, frame01_6);
  criarCaractere(7, frame01_7);

  limpaTela();

  lcd.setCursor(0, 0); lcd.write(byte(0));
  lcd.setCursor(1, 0); lcd.write(byte(1));
  lcd.setCursor(2, 0); lcd.write(byte(2));
  lcd.setCursor(3, 0); lcd.write(byte(3));
  lcd.setCursor(4, 0); lcd.write(byte(4));

  lcd.setCursor(0, 1); lcd.write(byte(5));
  lcd.setCursor(1, 1); lcd.write(byte(6));
  lcd.setCursor(2, 1); lcd.write(byte(7));
}
// ZOOM A
void mostrarZoomA() {
  criarCaractere(0, zoomA0);
  criarCaractere(1, zoomA1);
  criarCaractere(2, zoomA2);
  criarCaractere(3, zoomA3);
  criarCaractere(4, zoomA4);
  criarCaractere(5, zoomA5);
  criarCaractere(6, zoomA6);

  limpaTela();

  lcd.setCursor(0, 0); lcd.write(byte(0));
  lcd.setCursor(1, 0); lcd.write(byte(1));
  lcd.setCursor(2, 0); lcd.write(byte(2));
  lcd.setCursor(3, 0); lcd.write(byte(3));
  lcd.setCursor(4, 0); lcd.write(byte(4));

  lcd.setCursor(0, 1); lcd.write(byte(5));
  lcd.setCursor(1, 1); lcd.write(byte(6));
}

// ZOOM B
void mostrarZoomB() {
  criarCaractere(0, zoomB0);
  criarCaractere(1, zoomB1);
  criarCaractere(2, zoomB2);
  criarCaractere(3, zoomB3);
  criarCaractere(4, zoomB4);
  criarCaractere(5, zoomB5);

  limpaTela();

  lcd.setCursor(2, 0); lcd.write(byte(0));
  lcd.setCursor(3, 0); lcd.write(byte(1));

  lcd.setCursor(0, 1); lcd.write(byte(2));
  lcd.setCursor(1, 1); lcd.write(byte(3));
  lcd.setCursor(2, 1); lcd.write(byte(4));
  lcd.setCursor(3, 1); lcd.write(byte(5));
}

// FRAME 02
void mostrarFrame02() {
  criarCaractere(0, frame02_0);
  criarCaractere(1, frame02_1);
  criarCaractere(2, frame02_2);
  criarCaractere(3, frame02_3);
  criarCaractere(4, frame02_4);
  criarCaractere(5, frame02_5);

  limpaTela();

  lcd.setCursor(2, 0); lcd.write(byte(0));
  lcd.setCursor(3, 0); lcd.write(byte(1));

  lcd.setCursor(0, 1); lcd.write(byte(2));
  lcd.setCursor(1, 1); lcd.write(byte(3));
  lcd.setCursor(2, 1); lcd.write(byte(4));
  lcd.setCursor(3, 1); lcd.write(byte(5));
}

// FRAME 03
void mostrarFrame03() {
  criarCaractere(0, folhaBase0);
  criarCaractere(1, folhaBase1);
  criarCaractere(2, frame03_2);
  criarCaractere(3, frame03_3);
  criarCaractere(4, frame03_4);

  limpaTela();

  lcd.setCursor(0, 1); lcd.write(byte(0));
  lcd.setCursor(1, 1); lcd.write(byte(1));
  lcd.setCursor(2, 1); lcd.write(byte(2));
  lcd.setCursor(3, 1); lcd.write(byte(3));
  lcd.setCursor(4, 1); lcd.write(byte(4));
}

// FRAME 04
void mostrarFrame04() {
  criarCaractere(0, folhaBase0);
  criarCaractere(1, folhaBase1);
  criarCaractere(2, frame03_2);
  criarCaractere(3, frame03_3);
  criarCaractere(4, frame03_4);

  limpaTela();

  lcd.setCursor(0, 1); lcd.write(byte(0));
  lcd.setCursor(1, 1); lcd.write(byte(1));
  lcd.setCursor(2, 1); lcd.write(byte(2));
  lcd.setCursor(3, 1); lcd.write(byte(3));
  lcd.setCursor(4, 1); lcd.write(byte(4));
}

// FOLHA VOANDO A
void desenhaVooA(int deslocamento) {
  criarCaractere(0, vooA0);
  criarCaractere(1, vooA1);
  criarCaractere(2, vooA2);
  criarCaractere(3, vooA3);

  limpaTela();

  int c;

  c = 4 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 0);
    lcd.write(byte(0));
  }

  c = 5 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 0);
    lcd.write(byte(1));
  }

  c = 3 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 1);
    lcd.write(byte(2));
  }

  c = 4 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 1);
    lcd.write(byte(3));
  }
}

// FOLHA VOANDO B
void desenhaVooB(int deslocamento) {
  criarCaractere(0, vooB0);
  criarCaractere(1, vooB1);
  criarCaractere(2, vooB2);
  criarCaractere(3, vooB3);

  limpaTela();

  int c;

  c = 6 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 0);
    lcd.write(byte(0));
  }

  c = 4 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 1);
    lcd.write(byte(1));
  }

  c = 5 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 1);
    lcd.write(byte(2));
  }

  c = 6 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 1);
    lcd.write(byte(3));
  }
}

// FOLHA VOANDO A INVERTIDA
void desenhaVooAInvertido(int deslocamento) {
  inverterVertical(vooA0, invertido0);
  inverterVertical(vooA1, invertido1);
  inverterVertical(vooA2, invertido2);
  inverterVertical(vooA3, invertido3);

  lcd.createChar(0, invertido0);
  lcd.createChar(1, invertido1);
  lcd.createChar(2, invertido2);
  lcd.createChar(3, invertido3);

  limpaTela();

  int c;

  c = 4 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 1);
    lcd.write(byte(0));
  }

  c = 5 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 1);
    lcd.write(byte(1));
  }

  c = 3 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 0);
    lcd.write(byte(2));
  }

  c = 4 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 0);
    lcd.write(byte(3));
  }
}

// FOLHA VOANDO B INVERTIDA
void desenhaVooBInvertido(int deslocamento) {
  inverterVertical(vooB0, invertido0);
  inverterVertical(vooB1, invertido1);
  inverterVertical(vooB2, invertido2);
  inverterVertical(vooB3, invertido3);

  lcd.createChar(0, invertido0);
  lcd.createChar(1, invertido1);
  lcd.createChar(2, invertido2);
  lcd.createChar(3, invertido3);

  limpaTela();

  int c;

  c = 6 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 1);
    lcd.write(byte(0));
  }

  c = 4 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 0);
    lcd.write(byte(1));
  }

  c = 5 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 0);
    lcd.write(byte(2));
  }

  c = 6 + deslocamento;
  if (c >= 0 && c < 16) {
    lcd.setCursor(c, 0);
    lcd.write(byte(3));
  }
}


// Animação do voo da folha
void folhaVoando() {
  desenhaVooA(0);
  delay(170);

  desenhaVooB(0);
  delay(140);

  desenhaVooAInvertido(2);
  delay(150);

  desenhaVooBInvertido(3);
  delay(130);

  desenhaVooA(5);
  delay(150);

  desenhaVooB(6);
  delay(130);

  desenhaVooAInvertido(8);
  delay(140);

  desenhaVooBInvertido(9);
  delay(120);

  desenhaVooA(11);
  delay(130);

  desenhaVooB(13);
  delay(100);

  limpaTela();
}

// Desenha uma parte do Logo
// Mostra somente a quantidade de colunas solicitada, permitindo revelar
// o texto gradualmente durante a animação de saída da folha.
void desenhaLogoAte(int quantidadeColunas) {
  if (quantidadeColunas < 0) {
    quantidadeColunas = 0;
  }

  if (quantidadeColunas > 16) {
    quantidadeColunas = 16;
  }

  for (int coluna = 0; coluna < quantidadeColunas; coluna++) {

    lcd.setCursor(coluna, 0);
    lcd.print(logoLinha1[coluna]);

    lcd.setCursor(coluna, 1);
    lcd.print(logoLinha2[coluna]);
  }
}

// ENTRADA 1
void mostrarEntrada1() {
  criarCaractere(0, entrada1_0);
  criarCaractere(1, entrada1_1);

  limpaTela();

  lcd.setCursor(0, 1);
  lcd.write(byte(0));

  lcd.setCursor(1, 1);
  lcd.write(byte(1));
}

// ENTRADA 2
void mostrarEntrada2() {
  criarCaractere(0, entrada2_0);
  criarCaractere(1, entrada2_1);
  criarCaractere(2, entrada2_2);

  limpaTela();

  lcd.setCursor(0, 1);
  lcd.write(byte(0));

  lcd.setCursor(1, 1);
  lcd.write(byte(1));

  lcd.setCursor(2, 1);
  lcd.write(byte(2));
}

// ENTRADA 3
void mostrarEntrada3() {
  criarCaractere(0, entrada3_0);
  criarCaractere(1, entrada3_1);
  criarCaractere(2, entrada3_2);
  criarCaractere(3, entrada3_3);

  limpaTela();

  lcd.setCursor(1, 1);
  lcd.write(byte(0));

  lcd.setCursor(2, 1);
  lcd.write(byte(1));

  lcd.setCursor(2, 0);
  lcd.write(byte(2));

  lcd.setCursor(3, 0);
  lcd.write(byte(3));
}

// FRAME Central da folha - Ela representa o momento em que a folha aparece em destaque, como se estivesse mais próxima da tela.
void mostrarFolhaCentral() {
  criarCaractere(0, close0);
  criarCaractere(1, close1);
  criarCaractere(2, close2);
  criarCaractere(3, close3);
  criarCaractere(4, close4);
  criarCaractere(5, close5);
  criarCaractere(6, close6);
  criarCaractere(7, close7);

  limpaTela();

  lcd.setCursor(2, 0);
  lcd.write(byte(0));

  lcd.setCursor(3, 0);
  lcd.write(byte(1));

  lcd.setCursor(7, 0);
  lcd.write(byte(2));

  lcd.setCursor(0, 1);
  lcd.write(byte(3));

  lcd.setCursor(1, 1);
  lcd.write(byte(4));

  lcd.setCursor(3, 1);
  lcd.write(byte(5));

  lcd.setCursor(10, 1);
  lcd.write(byte(6));

  lcd.setCursor(11, 1);
  lcd.write(byte(7));
}

// SAÍDA 1
void mostrarSaida1() {
  criarCaractere(0, saida1_0);
  criarCaractere(1, saida1_1);
  criarCaractere(2, saida1_2);
  criarCaractere(3, saida1_3);

  limpaTela();

  desenhaLogoAte(5);

  lcd.setCursor(8, 1);
  lcd.write(byte(0));

  lcd.setCursor(9, 1);
  lcd.write(byte(1));

  lcd.setCursor(10, 0);
  lcd.write(byte(2));

  lcd.setCursor(11, 0);
  lcd.write(byte(3));
}

// SAÍDA 2
void mostrarSaida2() {
  criarCaractere(0, saida2_0);
  criarCaractere(1, saida2_1);
  criarCaractere(2, saida2_2);

  limpaTela();

  desenhaLogoAte(10);

  lcd.setCursor(11, 0);
  lcd.write(byte(0));

  lcd.setCursor(12, 0);
  lcd.write(byte(1));

  lcd.setCursor(13, 0);
  lcd.write(byte(2));
}

// SAÍDA 3
void mostrarSaida3() {
  criarCaractere(0, saida3_0);
  criarCaractere(1, saida3_1);

  limpaTela();

  desenhaLogoAte(14);

  lcd.setCursor(14, 0);
  lcd.write(byte(0));

  lcd.setCursor(15, 0);
  lcd.write(byte(1));
}

// Transição final da folha
// Apresenta quadros de entrada, aproximação e saída da folha.
// Ao final, revela o nome da equipe no LCD.
void transicaoFolhaGigante() {
  mostrarEntrada1();
  delay(80);

  mostrarEntrada2();
  delay(80);

  mostrarEntrada3();
  delay(100);

  mostrarFolhaCentral();
  delay(150);

  mostrarSaida1();
  delay(100);

  mostrarSaida2();
  delay(80);

  mostrarSaida3();
  delay(80);

  // Logo final
  limpaTela();

  lcd.setCursor(3, 0);
  lcd.print("   ECO");

  lcd.setCursor(6, 1);
  lcd.print("WAY");
  delay(2000);

  limpaTela();

  lcd.setCursor(0, 0);
  lcd.print(F("   PENSANDO EM"));

  lcd.setCursor(0, 1);
  lcd.print(F("      VOCE"));
  delay(2000);
}

// Animação de Abertura
// Executa os quadros de introdução, aproximação e voo da folha,
// antes de iniciar a tela de boas-vindas e o monitoramento.
void animacaoIntroducao() {
  mostrarFrame01();
  delay(700);

  mostrarZoomA();
  delay(180);

  mostrarZoomB();
  delay(180);

  mostrarFrame02();
  delay(350);

  mostrarFrame03();
  delay(250);

  mostrarFrame04();
  delay(350);

  folhaVoando();

  delay(80);

  transicaoFolhaGigante();
};

// -------------------- FUNÇÕES --------------------

// Desliga todos os LEDs.
void apagarLeds() {

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, LOW);
};

// Carrega os registros armazenados na EEPROM.
void carregarEEPROM() {

  EEPROM.get(0, dados);

  if (dados.assinatura != ASSINATURA ||
      dados.proximoIndice >= MAX_REGISTROS ||
      dados.quantidade > MAX_REGISTROS) {

    memset(&dados, 0, sizeof(dados));

    dados.assinatura = ASSINATURA;
    dados.proximoIndice = 0;
    dados.quantidade = 0;

    EEPROM.put(0, dados);
  }
}

// Salva uma leitura na EEPROM.
void salvarRegistro(float temperatura, float umidade,
                    const char* estado) {

  Registro novoRegistro;

  novoRegistro.tempoMillis = millis();
  novoRegistro.temperatura = temperatura;
  novoRegistro.umidade = umidade;

  strncpy(novoRegistro.estado, estado,
          sizeof(novoRegistro.estado) - 1);

  novoRegistro.estado[sizeof(novoRegistro.estado) - 1] = '\0';

  dados.registros[dados.proximoIndice] = novoRegistro;

  dados.proximoIndice =
    (dados.proximoIndice + 1) % MAX_REGISTROS;

  if (dados.quantidade < MAX_REGISTROS) {
    dados.quantidade++;
  }

  EEPROM.put(0, dados);
};

// Atualiza os LEDs conforme o estado geral.
void atualizarLeds(const char* estado) {

  apagarLeds();

  if (strcmp(estado, "ALTO") == 0) {

    digitalWrite(LED_VERMELHO, HIGH);

  } else if (strcmp(estado, "Alerta") == 0) {

    digitalWrite(LED_AMARELO, HIGH);

  } else {

    digitalWrite(LED_VERDE, HIGH);
  }
};

// Atualiza as informações do LCD.
void atualizarLCD(float temperatura,
                  float umidade,
                  int luminosidade,
                  const char* statusTemperatura,
                  const char* statusUmidade,
                  const char* statusLuminosidade,
                  const char* estado,
                  int indiceUV,
                  const char* statusUV) {

  lcd.clear();

  // Tela 1: Temperatura e status.
  if (telaAtual == 0) {

    lcd.setCursor(0, 0);
    lcd.print(F("Temp: "));
    lcd.print(temperatura, 1);
    lcd.print((char)223);
    lcd.print(F("C"));

    lcd.setCursor(0, 1);
    lcd.print(F("Status: "));
    lcd.print(statusTemperatura);
  }

  // Tela 2: Umidade e status.
  else if (telaAtual == 1) {

    lcd.setCursor(0, 0);
    lcd.print(F("Umidade: "));
    lcd.print(umidade, 1);
    lcd.print(F("%"));

    lcd.setCursor(0, 1);
    lcd.print(F("Status: "));
    lcd.print(statusUmidade);
  }

  // Tela 3: Ambiente (somente informativo).
  else if (telaAtual == 2) {

    lcd.setCursor(0, 0);
    lcd.print(F("Ambiente:"));

    lcd.setCursor(0, 1);
    lcd.print(statusLuminosidade);
  }

  // Tela 4: Indice UV.
  else if (telaAtual == 3) {

    lcd.setCursor(0, 0);
    lcd.print(F("Indice UV: "));
    lcd.print(indiceUV);

    lcd.setCursor(0, 1);
    lcd.print(F("Nivel: "));
    lcd.print(statusUV);
  }

    // Tela 5: Recomendacoes.
  else if (telaAtual == 4) {
    lcd.setCursor(0, 0);

    if (strcmp(statusUV, "ALTO") == 0 ||strcmp(statusUV, "EXTREMO") == 0) {
      lcd.print(F("Use protetor!"));

      lcd.setCursor(0, 1);
      lcd.print(F("Evite sol forte"));

    }
    else if (strcmp(estado, "ALTO") == 0) {
      lcd.print(F("Hidrate-se!"));

      lcd.setCursor(0, 1);
      lcd.print(F("Use protetor"));

    }
    else if (strcmp(estado, "Alerta") == 0) {
      lcd.print(F("Busque rota"));

      lcd.setCursor(0, 1);
      lcd.print(F("mais fresca"));

    }
    else {
      lcd.print(F("Condicoes OK"));

      lcd.setCursor(0, 1);
      lcd.print(F("Aproveite!"));

    }
  }
};

// Envia as medições pela Serial em formato JSON.
void enviarSerial(float temperatura,
                  float umidade,
                  int luminosidade,
                  const char* statusTemperatura,
                  const char* statusUmidade,
                  const char* statusLuminosidade,
                  int indiceUV,
                  const char* statusUV,
                  const char* estado) {

  Serial.print(F("{"));

  Serial.print(F("\"projeto\":\"EcoWay\","));

  Serial.print(F("\"temperatura\":"));
  Serial.print(temperatura, 2);

  Serial.print(F(",\"statusTemperatura\":\""));
  Serial.print(statusTemperatura);

  Serial.print(F("\",\"umidade\":"));
  Serial.print(umidade, 2);

  Serial.print(F(",\"statusUmidade\":\""));
  Serial.print(statusUmidade);

  Serial.print(F("\",\"luminosidade\":"));
  Serial.print(luminosidade);

  Serial.print(F(",\"statusLuminosidade\":\""));
  Serial.print(statusLuminosidade);

  Serial.print(F(",\"indiceUV\":"));
  Serial.print(indiceUV);

  Serial.print(F(",\"statusUV\":\""));
  Serial.print(statusUV);

  Serial.print(F("\",\"estadoGeral\":\""));
  Serial.print(estado);

  Serial.println(F("\"}"));
};

// Envia mensagem de erro pela Serial.
void enviarErroSerial(const char* mensagem) {

  Serial.print(F("{\"projeto\":\"EcoWay\",\"erro\":\""));
  Serial.print(mensagem);
  Serial.println(F("\"}"));
};

// Sensor de Proximidade.
float medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracao = pulseIn(ECHO, HIGH, 30000);

  // Se não houver retorno do sensor, considera fora do alcance.
  if (duracao == 0) {
    return 999;
  }

  float distancia = duracao * 0.0343 / 2;

  return distancia;
}

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LDR, INPUT);
  pinMode(POT_UV, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  apagarLeds();

  // Arduino Uno: SDA = A4 e SCL = A5.
  Wire.begin();

  lcd.init();
  lcd.backlight();

  dht.begin();

  carregarEEPROM();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(F("Aproxime-se"));

  lcd.setCursor(0, 1);
  lcd.print(F("para comecar!"));
}


// -------------------- LOOP --------------------

void loop() {
  // -------------------- CONTROLE DE PROXIMIDADE --------------------

  float distancia = medirDistancia();

  // Se a pessoa estiver longe, aguarda a aproximação.
  if (distancia > DISTANCIA_MAXIMA) {

    if (monitoramentoAtivo || !mensagemAguardando) {

      monitoramentoAtivo = false;
      mensagemAguardando = true;

      apagarLeds();

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print(F("Aproxime-se"));

      lcd.setCursor(0, 1);
      lcd.print(F("para comecar!"));

      Serial.println(F(
        "{\"projeto\":\"EcoWay\",\"status\":\"aguardando_aproximacao\"}"
      ));
    }

    return;
  }

  // -------------------- INICIO DO MONITORAMENTO --------------------

  if (!monitoramentoAtivo) {

    monitoramentoAtivo = true;
    mensagemAguardando = false;

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(F("Ola! Seja bem-"));

    lcd.setCursor(0, 1);
    lcd.print(F("vindo ao EcoWay"));

    delay(2000);

    // Executa a animação somente na primeira aproximação.
    if (!animacaoExecutada) {

      animacaoIntroducao();

      animacaoExecutada = true;
    }

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(F("EcoWay iniciado"));

    lcd.setCursor(0, 1);
    lcd.print(F("Monitoramento"));

    Serial.println(F(
      "{\"projeto\":\"EcoWay\",\"status\":\"iniciado\"}"
    ));

    delay(1500);

    lcd.clear();

    // Permite realizar a primeira leitura imediatamente.
    ultimaLeitura = millis() - INTERVALO_LEITURA;
  }

  // -------------------- INTERVALO ENTRE AS LEITURAS --------------------

  if (millis() - ultimaLeitura < INTERVALO_LEITURA) {
    return;
  }

  ultimaLeitura = millis();

  // -------------------- LEITURA DO DHT22 --------------------

  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  if (isnan(temperatura) || isnan(umidade)) {

    apagarLeds();

    lcd.clear();
    lcd.print(F("Erro no DHT22"));

    enviarErroSerial("Falha na leitura do DHT22");

    return;
  }


  // -------------------- LEITURA DO LDR --------------------

  // Acumula as amostras do sensor antes de calcular a média.
  long somaLeituras = 0;

  // Repete a leitura para reduzir oscilações pontuais.
  for (int i = 0; i < NUM_LEITURAS; i++) {

    somaLeituras += analogRead(LDR);

    delay(10);
  }

  // Calcula a média das amostras.
  valorLDR = (somaLeituras + (NUM_LEITURAS / 2)) / NUM_LEITURAS;

  // Converte a leitura em porcentagem.
  porcentagemLuz = map(valorLDR, 0, 1023, 100, 0);

  luminosidade = porcentagemLuz;

  // -------------------- LEITURA DO INDICE UV --------------------
  //Obs: O potenciômetro simula um índice UV para demonstração. 
  //Ele não mede raios ultravioletas reais.

  valorPotUV = analogRead(POT_UV);

  // Converte o valor do potenciometro para um indice UV de 0 a 11.
  indiceUV = map(valorPotUV, 0, 1023, 0, 11);

  // Classificacao do indice UV.
  if (indiceUV >= 8) {
    statusUV = "EXTREMO";
  } else if (indiceUV >= 6) {
    statusUV = "ALTO";
  } else if (indiceUV >= 3) {
    statusUV = "MODERADO";
  } else {
    statusUV = "BAIXO";
  }


  // -------------------- CLASSIFICAÇÃO DA TEMPERATURA --------------------

  if (temperatura >= 28.0) {

    statusTemperatura = "ALTO";

  } else if (temperatura >= 24.0 && temperatura < 28) {

    statusTemperatura = "Alerta";

  } else {

    statusTemperatura = "OK";
  }


  // -------------------- CLASSIFICAÇÃO DA UMIDADE --------------------

  if (umidade >= 70.0) {

    statusUmidade = "OK";

  } else if (umidade > 60.0 && umidade < 70) {

    statusUmidade = "Alerta";

  } else {

    statusUmidade = "BAIXA";
  }


  // -------------------- CLASSIFICAÇÃO DO AMBIENTE --------------------

  // A luminosidade serve somente para identificar o ambiente.
  // Não participa da classificação geral.

  if (luminosidade >= 80) {

    statusLuminosidade = "CLARO";

  } else if (luminosidade >= 40) {

    statusLuminosidade = "LUZ AMBIENTE";

  } else {

    statusLuminosidade = "ESCURO";
  }


  // -------------------- CLASSIFICAÇÃO GERAL --------------------

  // ex: Temperatura alta OU umidade baixa = estado ALTO.

// -------------------- CLASSIFICAÇÃO GERAL --------------------

  if (strcmp(statusTemperatura, "ALTO") == 0 ||
      strcmp(statusUmidade, "BAIXA") == 0 ||
      strcmp(statusUV, "ALTO") == 0 ||
      strcmp(statusUV, "EXTREMO") == 0) {

    estado = "ALTO";

  } else if (strcmp(statusTemperatura, "Alerta") == 0 ||
            strcmp(statusUmidade, "Alerta") == 0 ||
            strcmp(statusUV, "MODERADO") == 0) {

    estado = "Alerta";

  } else {

    estado = "OK";
  }


  // -------------------- ATUALIZAÇÃO DOS LEDS --------------------

  // Os LEDs consideram somente o estado geral.
  atualizarLeds(estado);


  // -------------------- ALTERNÂNCIA DAS TELAS --------------------

  if (millis() - ultimaTrocaTela >= INTERVALO_TELA) {

    telaAtual++;

    if (telaAtual > 4) {
      telaAtual = 0;
    }

    ultimaTrocaTela = millis();
  }

  atualizarLCD(
    temperatura,
    umidade,
    luminosidade,
    statusTemperatura,
    statusUmidade,
    statusLuminosidade,
    estado,
    indiceUV,
    statusUV
  );


  // -------------------- ENVIO PELA SERIAL --------------------

  // A luminosidade continua sendo enviada apenas como informação.
  enviarSerial(
    temperatura,
    umidade,
    luminosidade,
    statusTemperatura,
    statusUmidade,
    statusLuminosidade,
    indiceUV,
    statusUV,
    estado
  );


  // -------------------- ARMAZENAMENTO NA EEPROM --------------------

  // Armazena somente temperatura, umidade e estado geral.
  if (millis() - ultimoRegistro >= INTERVALO_REGISTRO) {

    salvarRegistro(
      temperatura,
      umidade,
      estado
    );

    ultimoRegistro = millis();

    Serial.println(F(
      "{\"projeto\":\"EcoWay\",\"registro_eeprom\":\"salvo\"}"
    ));
  }
}