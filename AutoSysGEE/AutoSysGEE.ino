/*Codigo para Executação de um monitorador de gases GEE (Gases do Efeito Estufa) em aviário usando a placa lógica arduino e alguns componentes
  Esse trabalho é para vim de pesquisa de Iniciação Cientifíca da UFMT-Cuiabá

**Autor: Ronei Lopes dos Santos -> Graduando em Agronomia
  Componentes utilizados:
  • Microcontrolador Arduino UNO R3;
  • Sensor MQ4 -> Conexão: **1º Perna -> GND, 2º -> pin 2, 3° -> A2, 4º -> 5v;
  • Sensor MQ-135 -> Conexão: **1º Perna -> VCC (5v), 2º -> GND, 3° -> A1, 4º -> pin 8;
  • Módulo Micro SD -> Conexão: * MOSI -> pin 11, * MISO -> pin 12,  * CLK -> pin 13, * CS -> pin 10;
  • Módulo RTC DS3231 -> Conexão: *GND -> GND, * VCC -> 5v,  * SDA -> A4, * SCL -> A5;
  • protoboard 400 furos.
  __________________________________*/

#include <SPI.h>      // Biblioteca de comunicação SPI Nativa
#include <SD.h>       // Biblioteca para manipulação SD nativa
#include <Wire.h>     // Biblioteca para manipulação do protocolo I2C
#include <DS3231.h>   // Biblioteca para manipulação do DS3231

#define MQ4_analog A2
#define MQ4_dig 2
#define MQ135_analog A1
#define MQ135_dig 8

int valor_analog;
int valor_dig;
int i = 1;

const int chipSelect = 10; // declarando o pino responsável por ativar o módulo SD
File arquivo;              //Declarando o objeto responsavel para escrever/ler o arquivo no Cartão SD

DS3231 rtc;                // Declarando o objeto do tipo DS3231
RTCDateTime dataehora;     // Declarando o objeto do tipo RTCDateTime

void setup()
{
  Serial.begin(9600);     // Iniciando a comunicação Serial
  pinMode(MQ4_analog, INPUT);
  pinMode(MQ4_dig, INPUT);
  pinMode(MQ135_analog, INPUT);
  pinMode(MQ135_dig, INPUT);
    
  rtc.begin();            // Iniciando o RTC DS3231
  //rtc.setDateTime(_DATE, __TIME_);   // Configurando valores iniciais do RTC 3231 (carregar o codigo 2x para o Arduino, sendo que na segunda comente esse comando)

  Serial.println ("Verificando conexão com o cartão SD");
  Serial.println ("-----------------------------------");
  delay (1500);
  while (!SD.begin(chipSelect)) {       // Validação da conexão do cartão SD, Se inserido ele pode operar
    Serial.println("Falha no cartao ou não está inserido");   // se o cartão não estiver inserido um aviso aparecerar na Monitor serial;
  }

  // Se o cartão estiver inserido....
  Serial.println("Cartão SD Inserido com sucesso "); // Mensagem de aviso de cartão SD conectado
  Serial.println ("-------------------------------");
  delay (1500);
  Serial.println("Cartão SD Inicializado e pronto para escrita de dados!!! ");
  Serial.println ("----------------------------------------------------");

  // Bloco para o cabeçalho da Tabela no Arquivo texto
  String RotuloTabela = ""; //Declarando e limpando a variavel que recebera o cabeçalho da tabela
  RotuloTabela = "  Data\t\t Hora\t   MQ4(CH4)\t MQ-135(CO2)\t MQ-135(N2O)";     // atribuindo valores do Cabeçalho da Tabela

  File arquivo = SD.open("DadosGEE.txt", FILE_WRITE); // Declarando e abrindo o arquivo onde será armazenado os dados da leitura do DTH e RTC

  if (arquivo) {
    arquivo.println(RotuloTabela);  // Escreve no arquivo o valor da string RotuloTabela e pulamos uma linha
    arquivo.close();           // Fechamento do arquivo
  }

  Serial.println("Dados contidos no arquivo");
  Serial.println("---------------------------");
  delay (1500);
  arquivo = SD.open("DadosGEE.txt", FILE_READ); // Abre o Arquivo
  if (arquivo) {
    while (arquivo.available()) { // Verifica existencia de dados no arquivo
      Serial.write(arquivo.read()); // Exibe o conteúdo do Arquivo enquanto encontrar dados
    }
    arquivo.close(); // Fecha o Arquivo após ler
  }
  else {
    Serial.println("Erro ao Abrir Arquivo .txt"); // Imprime na tela caso ocorra erro ao abrir o arquivo
  }
  delay (18000); //3min de espera para esquentar o sensor
}

void loop()
{
  dataehora = rtc.getDateTime();     //Atribuindo valores instantâneos de data e hora à instancia dataehora
  if (i == 1) {
    Serial.println ("____Dados Lidos do RTC 3231, MQ-4 & MQ-135____ \n");      //Imprimindo o Dia
    Serial.println ("  Data\t\t Hora\t   MQ4(CH4)\t MQ-135(CO2)\t MQ-135(N2O)");
  }

  Serial.print(dataehora.day);      //Imprimindo o Dia
  Serial.print("/");
  Serial.print(dataehora.month);    //Imprimindo o Mês
  Serial.print("/");
  Serial.print(dataehora.year);     //Imprimindo o Ano
  Serial.print("\t");
  Serial.print(dataehora.hour);     //Imprimindo a Hora
  Serial.print(":");
  Serial.print(dataehora.minute);   //Imprimindo o Minuto
  Serial.print(":");
  Serial.print(dataehora.second);   //Imprimindo o Segundo
  Serial.print("\t  ");
  Serial.print(analogRead(MQ4_analog));   //Imprime o valor bruto captado pelo MQ4
  Serial.print("\t");
  Serial.print(analogRead(MQ135_analog));   //Imprime o valor bruto captado pelo MQ135
  Serial.print("\t");
  Serial.print(analogRead(MQ135_analog));   //Imprimindo o valor bruto do MQ-135
  Serial.print("\t");
  Serial.println("");

//Bloco para gravar os dados lidos pelo RTC, MQ4 e MQ135

  File arquivo = SD.open("DadosGEE.txt", FILE_WRITE); // Abre o Arquivo
  if (arquivo) {
    arquivo.print(dataehora.day);      //Armazena no arquivo o Dia
    arquivo.print("/");
    arquivo.print(dataehora.month);    //Armazena no arquivo o Mês
    arquivo.print("/");
    arquivo.print(dataehora.year);     //Armazena no arquivo o Ano
    arquivo.print("\t");
    arquivo.print(dataehora.hour);     //Armazena no arquivo a Hora
    arquivo.print(":");
    arquivo.print(dataehora.minute);   //Armazena no arquivo o Minuto
    arquivo.print(":");
    arquivo.print(dataehora.second);   //Armazena no arquivo o Segundo
    arquivo.print("\t  ");
    arquivo.print(analogRead(MQ4_analog));   //Armazena o valor bruto captado pelo MQ4 para CH4
    arquivo.print("\t");
    arquivo.print(analogRead(MQ135_analog));   //Armazena o valor bruto captado pelo MQ135 para CO2
    arquivo.print("\t");
    arquivo.print(analogRead(MQ135_analog));   //Armazenando o valor bruto do MQ-135 para N2O
    arquivo.print("\t");
    arquivo.println("");
    arquivo.close();           // Fechamos o arquivo
  }

  delay (300000); // Intervalo de 5 minutos para a proxima leitura e gravação no arquivo
  i += 1;

}