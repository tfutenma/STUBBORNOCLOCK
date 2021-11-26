#include <LiquidCrystal_I2C.h>
#include <virtuabotixRTC.h>
#include <Wire.h>
#include <stdlib.h>

//Constantes dos componentes
#define RTC_CLK 2
#define RTC_DAT 3
#define RTC_RST 4
#define LCD_linhas 2
#define LCD_colunas 16
#define LCD_END 0x3F
#define BOTAO_HORA_DEZ 8
#define BOTAO_MIN_UNI 9
#define BOTAO_RESET_ENTER 10
#define BUZZER 13
#define SENSOR_ECHO 52
#define SENSOR_TRIGGER 53

//Variaveis para setar o horario no RTC
#define segL 00
#define minL 58
#define horL 16
#define d_semL 5
#define d_mesL 25
#define mesL 11
#define anoL 2021

//Variavel para setar a distancia maxima
#define DISTANCIA_MAXIMA 20

//Variavel para setar a freq em milisegundos do relogio
#define FREQ_CLOCK 1000
//Variavel que guarda o tempo de execução do relogio
unsigned long clockTimer;

//Variavel para setar a freq em milisegundos do relogio
#define FREQ_DIST 30000
//Variavel que guarda o tempo de execução do relogio
unsigned long distTimer;

//Variavel para setar a freq em milisegundos do relogio
#define FREQ_ALARME 700
//Variavel que guarda o tempo de execução do relogio
unsigned long alarmeTimer;

//Setar Variaveis
int hours, minutes, seconds; //Variaveis do relogio
int ahours = 00, aminutes = 00; //Variaveis do alarme

long duration = 0, distancia; //variaveis de distancia
int breset_count = 0; //variavel pra limitar a configuração do alarme 1 vez por dia
bool alarme_set = true, alarme_tocando = false, switch_alarme = false; //variaveis para alarme
String opcao = "relogio"; //variavel para definir qual opcao aparecer na tela

int a = 0, b = 0, c = 0, result = 0, prova = 0; //variaveis para calculadora

//Declarando Objeto do RTC
virtuabotixRTC myRTC(RTC_CLK, RTC_DAT, RTC_RST);
//Declarando Objeto LCD
LiquidCrystal_I2C lcd(LCD_END, LCD_colunas, LCD_linhas);

//Declarando funções
void printClock(int hours, int minutes, int seconds);
void printResult(int i);
void printProblem(int a, int b, int c);
void despertador();
long microsecondsToCentimeters(long microseconds);
void geraRandomicos(int a, int b, int c);

void setup() {
  //myRTC.setDS1302Time(segL, minL, horL, d_semL, d_mesL, mesL, anoL);
  Serial.begin(9600);
  lcd.init(); //inicia o lcd
  lcd.backlight(); //seta o backlight do lcd
  lcd.clear(); //limpa o lcd
  pinMode(BOTAO_RESET_ENTER, INPUT);
  pinMode(BOTAO_HORA_DEZ, INPUT);
  pinMode(BOTAO_MIN_UNI, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SENSOR_TRIGGER, OUTPUT); 
  pinMode(SENSOR_ECHO, INPUT); 
  clockTimer = millis(); 
  distTimer = millis();
  alarmeTimer = millis();
}

void loop() {
  myRTC.updateTime(); //atualiza a hora baseado no RTC
  hours = myRTC.hours;
  minutes = myRTC.minutes;
  seconds = myRTC.seconds;

  if (alarme_tocando) {
    if (alarme_set) { //se o alarme_set e o alarme_tocando estiverem como true, o alarme começa a tocar e é gerado os numeros para fazer a conta aritmetica
      if (a == 0 && b == 0 && c == 0) {
        a = random(1, 10);
        b = random(1, 10);
        c = random(1, 10);
      }
      lcd.setCursor(4, 0);
      lcd.print("RESOLVA: ");
      printProblem(a, b, c);
      despertador();
      alarme_tocando == true;
    }

    if (digitalRead(BOTAO_MIN_UNI) == HIGH) { //Se o botao da unidade foi clicado, adiciona 1 ao resultado
      result = result + 1;
      printResult(result);
    }

    if (digitalRead(BOTAO_HORA_DEZ) == HIGH) {//Se o botao da dezena foi clicado, adiciona 10 ao resultado
      result = result + 10;
      printResult(result);
    }

    if(millis() >= distTimer){ //Mantém o sensor funcionando paralelamente e lendo de 30 em 30 segundos (setado pelo FREQ_DIST)
      distTimer += FREQ_DIST;
      digitalWrite(SENSOR_TRIGGER, LOW);
      delayMicroseconds(2);
      digitalWrite(SENSOR_TRIGGER, HIGH);
      delayMicroseconds(5);
      digitalWrite(SENSOR_TRIGGER, LOW);   
      duration = pulseIn(SENSOR_ECHO, HIGH);
      distancia = microsecondsToCentimeters(duration);
      Serial.println(distancia);
    }

    if (digitalRead(BOTAO_RESET_ENTER) == HIGH) { //Se o botao de enter for clicado, verifica se o resultado está certo e o sensor está detectando a presença
      prova = (a * b) + c;
      Serial.println(prova);
      if (result == prova) {
        if (distancia <= DISTANCIA_MAXIMA){
          alarme_set = false;
          alarme_tocando = false;
          a = 0;
          b = 0;
          c = 0;
          result = 0;
        }
      } else { // se o resultado nao estiver certo, é gerado outra conta aritmetica
        result = 0;
        a = random(1, 10);
        b = random(1, 10);
        c = random(1, 10);
        printProblem(a, b, c);
        lcd.setCursor(10, 1);
        lcd.print("    ");
      }
      delay(500);
    }

  } else {
    if (switch_alarme) { //se o switch alarme estiver setado, mostra no visor o alarme
      lcd.setCursor(0, 0);
      lcd.print("    ");
      lcd.setCursor(12, 0);
      lcd.print("    ");
      lcd.setCursor(0, 1);
      lcd.print("    ");
      lcd.setCursor(10, 1);
      lcd.print("    ");
      lcd.setCursor(4, 1);
      lcd.print("Alarme");
      if (digitalRead(BOTAO_MIN_UNI) == HIGH) aminutes++;
      else if (digitalRead(BOTAO_HORA_DEZ) == HIGH) ahours++;
      lcd.setCursor(4, 0);
      if (ahours < 10) lcd.print("0");
      lcd.print(ahours);
      lcd.print(":");
      if (aminutes < 10) lcd.print("0");
      lcd.print(aminutes);
      if (aminutes > 59) {
        ahours++;
        aminutes = 0;
      }
      if (ahours > 23) ahours = 0;
      lcd.print(":00");
      delay(300);
      if (digitalRead(BOTAO_RESET_ENTER) == HIGH) { //se o botao de enter for clicado, volta pro relogio
        if (breset_count < 2) {
          delay(500);
          Serial.println(breset_count);
          switch_alarme = !switch_alarme;
          breset_count++;
        }
      }
    } else { //se o switch nao estiver setado, mostra o relogio
      if (digitalRead(BOTAO_RESET_ENTER) == HIGH) { //se o botao de enter for clicado, volta pro alarme (se nao tiver sido setado no mesmo dia)
        if (breset_count < 2) {
          delay(500);
          Serial.println(breset_count);
          switch_alarme = !switch_alarme;
          breset_count++;
        }
      }
      if (millis() >= clockTimer) { //seta a hora na tela de maneira paralela
        clockTimer += FREQ_CLOCK;
        printClock(hours, minutes, seconds);
      }
      alarme_set = true;
    }
  }

  if (ahours == hours && aminutes == minutes && seconds == 0) { //se der a hora setada no alarme, o alarme_tocando é setado
    alarme_tocando = true;
  } else {
    if (alarme_tocando == true) { //se o alarme tiver tocando, ele continua tocando
      alarme_tocando = true;
    }
  }

  if (hours == 0 && minutes == 0 && seconds == 0) { //reseta o dia para setar o alarme
    breset_count = 0;
  }

}

void printClock(int hours, int minutes, int seconds) { //FUNÇÃO QUE PRINTA O HORÁRIO NO LCD
  lcd.setCursor(0, 0);
  lcd.print("    ");
  lcd.setCursor(12, 0);
  lcd.print("    ");
  lcd.setCursor(0, 1);
  lcd.print("    ");
  lcd.setCursor(11, 1);
  lcd.print("    ");
  lcd.setCursor(4, 0);
  if (hours < 10) lcd.print("0");
  lcd.print(hours);
  lcd.print(":");
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
  lcd.setCursor(4, 1);
  lcd.print("Relogio");
}

void printResult(int i) { //FUNÇAO PRA PRINTAR O RESULTADO DA CONTA ARITMETICA
  lcd.setCursor(10, 1);
  if (i < 10) lcd.print("00");
  else if (i < 100) lcd.print("0");
  lcd.print(i);
}

void printProblem(int a, int b, int c) { //FUNCAO PRA PRINTAR A CONTA ARITMETICA
  lcd.setCursor(3, 1);
  lcd.print("(");
  lcd.print(a);
  lcd.print("x");
  lcd.print(b);
  lcd.print(")");
  lcd.print("+");
  lcd.print(c);
  lcd.print("=");
}

void despertador() { //FUNCAO PRA FAZER O DESPERTADOR TOCAR EM PARALELO
  if (millis() >= alarmeTimer) {
    alarmeTimer += FREQ_ALARME;
    tone(BUZZER, 1000);
    delay(700);
    noTone(BUZZER);
  }
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

void geraRandomicos(int a, int b, int c) {
  a = rand() % 10;
  b = rand() % 10;
  c = rand() % 10;
}
