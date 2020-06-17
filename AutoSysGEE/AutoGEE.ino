/*Codigo para Executação de um monitorador de gases GEE em aviário usando a placa lógica arduino e alguns componentes
  Esse trabalho é para vim de pesquisa de Iniciação Cientifíca da UFMT-Cuiabá
  
**Autor: Ronei Lopes dos Santos -> Graduando em Agronomia
  Componentes utilizados:
  • Microcontrolador Arduino UNO R3;
  • Sensor MQ4 -> Conexão: **1º Perna -> GND, 2º -> pin 2, 3° -> A2, 4º -> 5v;
  • Sensor MQ-135 -> 
  • Módulo Micro SD -> Conexão: ** MOSI -> pin 11, ** MISO -> pin 12,  ** CLK -> pin 13, ** CS -> pin 10;
  • Módulo RTC DS3231 -> Conexão: **GND -> GND, ** VCC -> 5v,  ** SDA -> A4, ** SCL -> A5;
  • protoboard 400 furos.
  ____________________________________________________________________________________________________*/

#include <SPI.h>      // Biblioteca de comunicação SPI Nativa
#include <SD.h>       // Biblioteca para manipulação SD nativa
#include <Wire.h>     // Biblioteca para manipulação do protocolo I2C
#include <DS3231.h>   // Biblioteca para manipulação do DS3231

#define MQ4_analog A2
#define MQ4_dig 7

int valor_analog;
int valor_dig;
int i = 1; 

const int chipSelect = 10; // declarando o pino responsável por ativar o módulo SD
File arquivo;              //Declarando o objeto responsavel para escrever/ler o arquivo no Cartão SD

DS3231 rtc;                // Declarando o objeto do tipo DS3231
RTCDateTime dataehora;     // Declarando o objeto do tipo RTCDateTime

DHT dht(DHTPIN, DHTTYPE);  // Declarando o objeto responsável pela comunicação/sensor

void setup()
{
  Serial.begin(9600);     // Iniciando a comunicação Serial
  pinMode(MQ4_analog, INPUT);
  pinMode(MQ4_dig, INPUT);

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
  delay (1500);
}

void loop() 
{
   valor_analog = analogRead(MQ4_analog); 
   valor_dig = digitalRead(MQ4_dig);
   Serial.print(valor_analog);
   Serial.print(" || ");
   if(valor_dig == 0)
     Serial.println("GAS DETECTADO !!!");
   else 
     Serial.println("GAS AUSENTE !!!");
   delay(500);
}