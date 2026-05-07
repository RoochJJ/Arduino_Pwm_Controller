
This directory is intended for project specific (private) libraries.
PlatformIO will compile them to static libraries and link into the executable file.

The source code of each library should be placed in a separate directory
("lib/your_library_name/[Code]").

For example, see the structure of the following example libraries `Foo` and `Bar`:

|--lib
|  |
|  |--Bar
|  |  |--docs
|  |  |--examples
|  |  |--src
|  |     |- Bar.c
|  |     |- Bar.h
|  |  |- library.json (optional. for custom build options, etc) https://docs.platformio.org/page/librarymanager/config.html
|  |
|  |--Foo
|  |  |- Foo.c
|  |  |- Foo.h
|  |
|  |- README --> THIS FILE
|
|- platformio.ini
|--src
   |- main.c

Example contents of `src/main.c` using Foo and Bar:
```
#include <Foo.h>
#include <Bar.h>

int main (void)
{
  ...
}

```

The PlatformIO Library Dependency Finder will find automatically dependent
libraries by scanning project source files.

More information about PlatformIO Library Dependency Finder
- https://docs.platformio.org/page/librarymanager/ldf.html


# Arduino PWM Controller

Projeto de controle de velocidade de motor DC por PWM utilizando **NodeMCU V3 (ESP8266)** e driver **L293D**. A velocidade é ajustada em 4 níveis via botão físico.

---

## Índice

1. [Introdução ao PWM](#introdução-ao-pwm)
2. [Componentes necessários](#componentes-necessários)
3. [Esquemático](#esquemático)
4. [Código-fonte](#código-fonte)
5. [Funcionamento do projeto](#funcionamento-do-projeto)

---

## Introdução ao PWM

**PWM** (Pulse Width Modulation — Modulação por Largura de Pulso) é uma técnica que simula uma saída analógica variando o tempo em que o sinal permanece em nível alto (`HIGH`) dentro de um período fixo.

### Conceitos fundamentais

| Conceito | Descrição |
|---|---|
| **Período** | Tempo total de um ciclo do sinal |
| **Frequência** | Número de ciclos por segundo (Hz) |
| **Duty Cycle** | Porcentagem do tempo em nível alto no período |

### Exemplos de Duty Cycle

```
25%  → ▓░░░▓░░░▓░░░  (motor lento)
50%  → ▓▓░░▓▓░░▓▓░░  (motor médio)
75%  → ▓▓▓░▓▓▓░▓▓▓░  (motor rápido)
100% → ▓▓▓▓▓▓▓▓▓▓▓▓  (motor máximo)
```

Neste projeto, o PWM é gerado pelo pino **D9** do NodeMCU e entregue ao driver L293D, que amplifica a corrente para acionar o motor DC. O `analogWrite()` aceita valores de **0 a 255**, mapeando diretamente o duty cycle de 0% a 100%.

### Níveis de velocidade utilizados

| Nível | Valor PWM | Duty Cycle aproximado |
|---|---|---|
| 0 | 64  | 25% |
| 1 | 128 | 50% |
| 2 | 192 | 75% |
| 3 | 254 | ~100% |

---

## Componentes necessários

| Quantidade | Componente | Especificação |
|---|---|---|
| 1 | NodeMCU V3 | ESP8266, 80/160 MHz |
| 1 | Driver de motor | L293D |
| 1 | Motor DC | 5V |
| 1 | Botão (push button) | NA (normalmente aberto) |
| 1 | Resistor pull-down | 10 kΩ (R1) |
| 1 | Fonte de alimentação | Bateria 5V (BAT1) |
| 1 | Protoboard | — |
| — | Jumpers | Macho-macho |

---

## Esquemático

![Esquemático do circuito](schematics/pwm_controller_schematic.png)

### Descrição do circuito

O circuito é composto por três blocos principais:

**NodeMCU V3 (ESP8266)**
Responsável por gerar o sinal PWM e ler o estado do botão. Opera como cérebro do projeto.

**Driver L293D**
Recebe o sinal PWM do NodeMCU e entrega corrente suficiente para acionar o motor DC. As entradas de controle (IN1/IN2) definem o sentido de rotação; EN1 recebe o sinal PWM para controle de velocidade.

**Botão + Resistor Pull-down (R1 = 10kΩ)**
O botão é conectado com resistor pull-down de 10 kΩ, garantindo nível LOW estável quando não pressionado e HIGH ao pressionar, evitando leituras flutuantes.

### Tabela de conexões

| Componente | Pino/Terminal | Conectado em |
|---|---|---|
| NodeMCU | D9 (PWM_PIN) | L293D — EN1 (pino 1) |
| NodeMCU | D2 (BUTTON_PIN) | Botão + R1 pull-down |
| NodeMCU | GND | GND comum |
| L293D | VSS (pino 16) | 5V (BAT1) |
| L293D | VS (pino 8) | 5V (BAT1) |
| L293D | IN1 (pino 2) | NodeMCU D7 |
| L293D | IN2 (pino 7) | NodeMCU D1 |
| L293D | OUT1 (pino 3) | Motor DC terminal A |
| L293D | OUT2 (pino 6) | Motor DC terminal B |
| L293D | GND (pinos 4,5,12,13) | GND comum |
| Botão | Terminal 1 | 3.3V |
| Botão | Terminal 2 | D2 + R1 → GND |
| BAT1 | + | VIN / 5V do circuito |
| BAT1 | - | GND comum |

---

## Código-fonte

```cpp
#include <Arduino.h>

#define PWM_PIN    9
#define BUTTON_PIN 2

int velocidades[] = {64, 128, 192, 254};
int nivel = 0;

void setup() {
  pinMode(PWM_PIN, OUTPUT);    // motor saída
  pinMode(BUTTON_PIN, INPUT);  // botão entrada
  analogWrite(PWM_PIN, velocidades[0]); // inicia o motor na primeira velocidade
}

void loop() {
  // aguarda apertar, verifica se apertamos
  if (digitalRead(BUTTON_PIN) == HIGH) {

    // avança nível
    nivel++;
    if (nivel > 3) nivel = 0;
    analogWrite(PWM_PIN, velocidades[nivel]); // aplica nova velocidade

    // aguarda soltar antes de continuar para não bugar
    while (digitalRead(BUTTON_PIN) == HIGH);
    delay(100);
  }
}
```

### Explicação das funções principais

**`analogWrite(PWM_PIN, valor)`**
Gera o sinal PWM no pino definido com duty cycle proporcional ao valor (0–255). O L293D usa esse sinal para controlar a tensão média entregue ao motor.

**`digitalRead(BUTTON_PIN)`**
Lê o estado do botão. Retorna `HIGH` quando pressionado (pull-down garante `LOW` em repouso).

**`while (digitalRead(BUTTON_PIN) == HIGH)`**
Trava o loop enquanto o botão estiver pressionado — evita que o nível avance múltiplas vezes em um único pressionamento.

**`delay(100)`**
Debounce por software: aguarda 100 ms após soltar o botão para ignorar ruídos de contato.

### Como carregar no NodeMCU

1. Instale a [Arduino IDE](https://www.arduino.cc/en/software)
2. Adicione o suporte ao ESP8266 em **File → Preferences → Additional Boards Manager URLs:**
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Instale via **Tools → Board → Boards Manager → ESP8266**
4. Selecione a placa: **Tools → Board → NodeMCU 1.0 (ESP-12E Module)**
5. Selecione a porta: **Tools → Port → COMx** (Windows) ou `/dev/ttyUSB0` (Linux)
6. Clique em **Upload** (→)

---

## Funcionamento do projeto

### Fluxo de operação

```
[Botão pressionado]
        │
        ▼
  nivel++ (0→1→2→3→0)
        │
        ▼
  analogWrite(PWM_PIN, velocidades[nivel])
        │
        ▼
  L293D amplifica o sinal
        │
        ▼
  Motor DC gira na nova velocidade
        │
        ▼
  Aguarda soltar o botão + debounce 100ms
```

### Comportamento esperado

| Pressionamentos | Nível | PWM | Velocidade do motor |
|---|---|---|---|
| 0 (inicial) | 0 | 64  | Baixa (~25%) |
| 1× | 1 | 128 | Média (50%) |
| 2× | 2 | 192 | Alta (75%) |
| 3× | 3 | 254 | Máxima (~100%) |
| 4× | 0 | 64  | Volta ao início |

### Detalhes importantes

- O motor **já inicia ligado** no nível 0 (25% de velocidade) ao energizar o circuito.
- O ciclo de velocidades é **circular**: após o nível 3, retorna automaticamente ao nível 0.
- O resistor pull-down de **10 kΩ** é essencial para estabilizar a leitura do botão.
- O **L293D** protege o NodeMCU de correntes altas, pois o ESP8266 não fornece corrente suficiente para acionar motores diretamente.

---

## Estrutura do repositório

```
Arduino_PWM_Controller/
├── src/
│   └── main.cpp
├── schematics/
│   └── pwm_controller_schematic.png
└── README.md
```
 Test
 