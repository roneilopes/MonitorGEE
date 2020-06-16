/*Codigo para Executação de um monitorador de gases GEE em aviário usando a placa lógica arduino e alguns componentes
  Esse trabalho é para vim de pesquisa de Iniciação Cientifíca da UFMT-Cuiabá
  
**Autor: Ronei Lopes dos Santos -> Graduando em Agronomia
  Componentes utilizados:
  • Microcontrolador Arduino UNO R3;
  • Sensor MQ4 -> Conexão: **1º Perna -> 5v, 2º pin 2, "" 4º GNG; (3º perna não utilizada);
  • Sensor MQ-135 -> 
  • Módulo Micro SD -> Coneção: ** MOSI -> pin 11, ** MISO -> pin 12,  ** CLK -> pin 13, ** CS -> pin 10;
  • Módulo RTC DS3231 -> Coneção: **GND -> GND, ** VCC -> 5v,  ** SDA -> A4, ** SCL -> A5;
  • protoboard 400 furos.
  ____________________________________________________________________________________________________*/

#include <SPI.h>      // Biblioteca de comunicação SPI Nativa
#include <SD.h>       //Inclui a Biblioteca para manipulação SD nativa
#include <Wire.h>     //Biblioteca para manipulação do protocolo I2C
#include <DS3231.h>   //Inclui a Biblioteca para manipulação do DS3231

const int chipSelect = 10; // declarando o pino responsável por ativar o módulo SD
File arquivo;              //Declarando o objeto responsavel para escrever/ler o arquivo no Cartão SD

DS3231 rtc;                // Declarando o objeto do tipo DS3231
RTCDateTime dataehora;     // Declarando o objeto do tipo RTCDateTime

DHT dht(DHTPIN, DHTTYPE);  // Declarando o objeto responsável pela comunicação/sensor

int i = 1; 
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);     // Iniciando a comunicação Serial

  rtc.begin();            // Iniciando o RTC DS3231
  //rtc.setDateTime(__DATE__, __TIME__);   // Configurando valores iniciais do RTC 3231 (carregar o codigo 2x para o Arduino, sendo que na segunda comente esse comando)
  dht.begin();

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
