# EcoWay — Monitoramento de Calor Urbano

O **EcoWay** é um projeto de tecnologia sustentável desenvolvido com Arduino Uno, voltado ao monitoramento de condições ambientais relacionadas ao calor urbano. O sistema utiliza sensores para acompanhar temperatura, umidade, luminosidade e um índice UV simulado, transformando as medições em informações que auxiliam na identificação de possíveis situações de risco.

O monitoramento é iniciado automaticamente quando o sensor ultrassônico HC-SR04 identifica a aproximação de uma pessoa. Após o início, o sistema realiza leituras a cada cinco segundos, classifica as condições ambientais e determina um estado geral: **OK, Alerta ou ALTO**.

As informações são apresentadas em um display LCD 16x2, sinalizadas por LEDs, transmitidas pela comunicação Serial em formato JSON e armazenadas na memória EEPROM do Arduino.

## Equipe
**Calm Tech Team**

- Caio Fernando De Deus Gomes
- Amanda Souza de Barros
- Luiza de Freitas Benevides
- Mariana do Nascimento
- Lirity Ribeiro de Paiva

## Objetivo do projeto
- Monitorar as condições ambientais por meio de sensores.
- Identificar possíveis situações de risco relacionadas ao calor urbano.
- Classificar temperatura, umidade e índice UV.
- Identificar a luminosidade do ambiente.
- Detectar a aproximação do usuário para iniciar o monitoramento.
- Apresentar informações de maneira acessível por meio de um display LCD.
- Emitir alertas visuais conforme as condições identificadas.
- Armazenar registros ambientais na memória EEPROM.
- Transmitir os dados para uma interface externa por meio da comunicação Serial.

## Funcionalidades
- **Monitoramento de temperatura:** utiliza o sensor DHT22 para medir a temperatura do ambiente.
- **Monitoramento de umidade:** acompanha a umidade relativa do ar por meio do DHT22.
- **Identificação de luminosidade:** utiliza o sensor LDR para classificar o ambiente como CLARO, LUZ AMBIENTE ou ESCURO.
- **Simulação do índice UV:** utiliza um potenciômetro para representar valores de índice UV entre 0 à 11.
- **Detecção de proximidade:** utiliza o sensor ultrassônico HC-SR04 para identificar a aproximação do usuário em até 200 cm.
- **Classificação ambiental:** determina o estado geral do ambiente em OK, Alerta ou ALTO.
- **Indicação por LEDs:** utiliza LEDs verde, amarelo e vermelho para representar as condições identificadas.
- **Display LCD:** apresenta as medições, classificações e recomendações relacionadas às condições ambientais.
- **Animação de inicialização:** apresenta uma animação personalizada com a identidade visual do EcoWay.
- **Comunicação Serial:** transmite as informações dos sensores em formato JSON.
- **Armazenamento de dados:** utiliza a memória EEPROM para preservar até 15 registros ambientais.

## Componentes utilizados
| Componente | Função |
|---|---|
| Arduino Uno | Controla o sistema e processa as informações |
| Sensor DHT22 | Mede temperatura e umidade |
| Sensor LDR | Identifica a luminosidade do ambiente |
| Potenciômetro | Simula o índice UV |
| Sensor ultrassônico HC-SR04 | Detecta a aproximação do usuário |
| Display LCD I2C 16x2 | Apresenta informações e recomendações |
| LED verde | Indica estado OK |
| LED amarelo | Indica estado de Alerta |
| LED vermelho | Indica estado ALTO |
| Memória EEPROM | Armazena os registros ambientais |
| Resistores | Auxiliam na proteção dos componentes |
| Protoboard | Permite a montagem do circuito |
| Jumpers | Realizam as conexões entre os componentes |

## Mapeamento dos pinos
| Componente | Pino do Arduino |
|---|---|
| Sensor DHT22 | Digital 2 |
| Sensor LDR | Analógico A0 |
| Potenciômetro (índice UV) | Analógico A2 |
| Sensor ultrassônico TRIG | Digital 7 |
| Sensor ultrassônico ECHO | Digital 8 |
| LED vermelho | Digital 9 |
| LED amarelo | Digital 10 |
| LED verde | Digital 13 |
| LCD I2C | Comunicação I2C |

## Critérios de monitoramento
O sistema utiliza condições programadas para classificar individualmente cada variável ambiental.

### Temperatura
| Faixa | Classificação |
|---|---|
| Abaixo de 24 °C | OK |
| De 24 °C até abaixo de 28 °C | Alerta |
| A partir de 28 °C | ALTO |

### Umidade relativa
| Faixa | Classificação |
|---|---|
| Abaixo ou igual a 60% | BAIXA |
| Acima de 60% e abaixo de 70% | Alerta |
| A partir de 70% | OK |

### Índice UV
| Índice UV | Classificação |
|---|---|
| De 0 a 2 | BAIXO |
| De 3 a 5 | MODERADO |
| De 6 a 7 | ALTO |
| De 8 a 11 | EXTREMO |

**Importante:** o índice UV é simulado por um potenciômetro para fins demonstrativos. Ele não realiza a medição real da radiação ultravioleta.

### Luminosidade
| Porcentagem | Classificação |
|---|---|
| A partir de 80% | CLARO |
| De 40% até abaixo de 80% | LUZ AMBIENTE |
| Abaixo de 40% | ESCURO |

A luminosidade é utilizada exclusivamente para identificar as condições de iluminação do ambiente, não interferindo na classificação geral.

### Classificação geral
O estado geral é determinado pela combinação das classificações individuais:

- **OK:** quando as condições não apresentam critérios de alerta ou nível alto.
- **Alerta:** quando a temperatura está em Alerta, a umidade está em Alerta ou o índice UV está Moderado.
- **ALTO:** quando a temperatura está ALTA, a umidade está BAIXA ou o índice UV está ALTO ou EXTREMO.

### Indicação por LEDs
| LED | Estado geral |
|---|---|
| Verde | OK |
| Amarelo | Alerta |
| Vermelho | ALTO |

Os LEDs são acionados de acordo com o estado geral identificado pelo sistema.

## Funcionamento do monitoramento
1. O Arduino inicia os componentes e apresenta a mensagem de aproximação no LCD.
2. O sensor ultrassônico HC-SR04 identifica a presença de uma pessoa em até 200 cm.
3. Quando a aproximação é detectada, o monitoramento é iniciado.
4. Uma animação de apresentação do EcoWay é executada na primeira aproximação.
5. O DHT22 realiza a leitura da temperatura e da umidade.
6. O LDR identifica a luminosidade, enquanto o potenciômetro simula o índice UV.
7. O sistema classifica individualmente as variáveis e determina o estado geral.
8. Os LEDs são acionados conforme a classificação.
9. As informações são apresentadas no LCD e transmitidas pela Serial.
10. Os registros de temperatura, umidade e estado geral são armazenados na EEPROM.

As leituras são realizadas em intervalos de cinco segundos. Caso o usuário se afaste para além da distância configurada, o sistema retorna à tela de espera.

## Armazenamento de dados
O projeto utiliza a memória EEPROM interna do Arduino Uno para armazenar até **15 registros ambientais**.

Cada registro contém:

- Tempo da ocorrência em milissegundos (`millis()`).
- Temperatura medida.
- Umidade relativa do ar.
- Estado geral identificado.

Os registros são armazenados a cada 60 segundos. Quando a capacidade máxima é atingida, os registros mais antigos são sobrescritos conforme a organização circular implementada no código.

A EEPROM preserva as informações mesmo após o desligamento do Arduino.

**Observação:** o armazenamento está implementado no firmware. A consulta do histórico não é apresentada como uma funcionalidade de interface nesta versão Rev 1.

## Comunicação Serial
O sistema transmite as medições em formato JSON, permitindo a integração com uma interface externa.

São enviados os seguintes dados:

- Temperatura e respectiva classificação.
- Umidade e respectiva classificação.
- Luminosidade e sua identificação.
- Índice UV simulado e respectiva classificação.
- Estado geral do ambiente.

Exemplo ilustrativo:

```json
{
  "projeto": "EcoWay",
  "temperatura": 30.00,
  "statusTemperatura": "ALTO",
  "umidade": 55.00,
  "statusUmidade": "BAIXA",
  "luminosidade": 85,
  "statusLuminosidade": "CLARO",
  "indiceUV": 9,
  "statusUV": "EXTREMO",
  "estadoGeral": "ALTO"
}
```

A comunicação serial está configurada para **115200 bauds**.

## Dependências e bibliotecas
- `Arduino.h` — recursos básicos do ambiente Arduino.
- `Wire.h` — comunicação I2C.
- `LiquidCrystal_I2C.h` — controle do display LCD.
- `DHT.h` — leitura do sensor DHT22.
- `EEPROM.h` — armazenamento de dados na memória EEPROM.
- `string.h` — manipulação de textos e blocos de memória.
- `avr/pgmspace.h` — utilização da memória Flash para armazenar os desenhos da animação.

### Configuração do ambiente (PlatformIO)
O firmware do projeto foi desenvolvido em C++ para o Arduino Uno, utilizando o PlatformIO IDE, integrado ao Visual Studio Code.

O arquivo `platformio.ini`, localizado na raiz do projeto, contém as configurações da plataforma, da placa, do framework, da velocidade de comunicação Serial e das bibliotecas utilizadas.

```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino
monitor_speed = 115200

lib_deps =
    marcoschwartz/LiquidCrystal_I2C@^1.1.4
    adafruit/DHT sensor library@^1.4.7
```

O arquivo principal `main.cpp` está localizado na pasta `src/`.

As dependências externas estão declaradas no arquivo `platformio.ini`.

## Linguagem utilizada
<img
  align="left" 
  alt="c++" 
  title="c++"
  width="30px" 
  style="padding-right: 10px;" 
  src="https://cdn.jsdelivr.net/gh/devicons/devicon@latest/icons/cplusplus/cplusplus-original.svg" 
/>
<br>
<br>

## Como executar o projeto

### Rev. 1 – Simulação no Wokwi
1. Acesse o link da simulação disponibilizado neste README.
2. Clique em **Start the Simulation** para iniciar o projeto.
3. Aguarde a mensagem de aproximação no display LCD.
4. Aproxime um objeto do sensor HC-SR04 para iniciar o monitoramento.
5. Altere os valores dos sensores virtuais para observar as classificações e os indicadores luminosos.
6. Abra o Monitor Serial para acompanhar os dados enviados pelo Arduino (115200 bauds).

**Observação:** A Rev. 1 corresponde à simulação do circuito no Wokwi.

## Versão do projeto
**Revisão 1 — Monitoramento de Calor Urbano.**

Versão inicial do protótipo EcoWay, com monitoramento de temperatura, umidade, luminosidade e índice UV simulado, identificação de proximidade, classificação ambiental, sinalização visual, comunicação Serial e armazenamento de registros.

## Contexto acadêmico
Projeto desenvolvido no contexto acadêmico da FIAP, com foco na aplicação de conceitos de Edge Computing, programação, eletrônica e sistemas embarcados para propor soluções relacionadas à sustentabilidade e ao bem-estar urbano.

## Simulação no Wokwi
O projeto pode ser testado em ambiente de simulação por meio do Wokwi, permitindo observar o funcionamento dos sensores, do display LCD, dos LEDs e da lógica de monitoramento.
**Observação:** a simulação representa um protótipo acadêmico.

## Links do projeto
- **Repositório GitHub:** https://github.com/luiiiq/EcoWay_Edge
- **Simulação Wokwi:** [Acessar simulação](https://wokwi.com/projects/480306692485186561)
