/*Codigo para Executação de um monitorador de gases GEE (Gases do Efeito Estufa) em aviário usando a placa lógica arduino e alguns componentes
  Esse trabalho é para vim de pesquisa de Iniciação Cientifíca da UFMT-Cuiabá

**Autor: Ronei Lopes dos Santos -> Graduando em Agronomia
  Componentes utilizados:
  • Microcontrolador Arduino UNO R3;
  • Sensor MQ4 -> Conexão: **1º Perna -> GND, 2º -> pin 2, 3° -> A2, 4º -> 5v;
  • Sensor MQ-135 -> Conexão: **1º Perna -> GND, 2º -> pin 4, 3° -> A0, 4º -> 5v;
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

#define VRL_VALOR 5 //resistência de carga
#define RO_FATOR_AR_LIMPO 9.83 //resistência do sensor em ar limpo 9.83 de acordo com o datasheet
                                                     
#define ITERACOES_CALIBRACAO 50    //numero de leituras para calibracao
#define ITERACOES_LEITURA 5     //numero de leituras para analise

#define GasCH4 0
#define GasCO2 1
#define GasN2O 2

int valor_analog;
int valor_dig;
int i = 1;
float Ro4 = 10;
float Ro135 = 10;

float CH4Curve[3]  =  {2.34,0.25,-0.35};  //curva CH4 aproximada baseada na sensibilidade descrita no datasheet {x,y,deslocamento} baseada em dois pontos 
                                          //p1: (log220.2466, log1.7793), p2: (log10409,37, log 0,4466)
                                          //inclinacao = (Y2-Y1)/(X2-X1)
                                          //vetor={x, y, inclinacao}

float CO2Curve[3]  =  {1,0.36,-0.35};  //curva CO2 aproximada baseada na sensibilidade descrita no datasheet {x,y,deslocamento} baseada em dois pontos 
                                          //p1: (log10, log2.3), p2: (log200, log0,8)
                                          //inclinacao = (Y2-Y1)/(X2-X1)
                                          //vetor={x, y, inclinacao}

/*float N2OCurve[3]  =  {x,x.x,-x.xx};  //???curva N2O aproximada baseada na sensibilidade descrita no datasheet {x,y,deslocamento} baseada em dois pontos 
                                          //p1: (log10, log2.3), p2: (log200, log0,8)
                                          //inclinacao = (Y2-Y1)/(X2-X1)
                                          //vetor={x, y, inclinacao}*/

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
  Serial.print("Calibrando o sensores MQ4 e MQ135...\n");                
  Ro4 = MQCalibration(MQ4_analog);      //calibra o sensor MQ4
  Serial.print("Valor de Ro-MQ4=");
  Serial.print(Ro4);
  Serial.println("kohm");
  Ro135 = MQCalibration(MQ135_analog);      //calibra o sensor MQ135
  Serial.print("Valor de Ro-MQ135=");
  Serial.print(Ro135);
  Serial.println("kohm");
  
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
  delay (1500);
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
  Serial.print("\t");
  Serial.print(getQuantidadeGasMQ(leitura_MQ(MQ4_analog)/Ro4,GasCH4));   //Imprimindo o valor de CH4 com o MQ4
  Serial.print("\t");
  Serial.print(getQuantidadeGasMQ(leitura_MQ(MQ135_analog)/Ro135,GasCO2));   //Imprimindo o valor de CO2 lido com MQ135
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
    arquivo.print("\t\t");
    arquivo.print(getQuantidadeGasMQ((leitura_MQ(MQ4_analog)/Ro4),GasCH4));   //Armazena o valor de CH4 captado pelo MQ4
    arquivo.print("\t");
    arquivo.print(getQuantidadeGasMQ((leitura_MQ(MQ135_analog)/Ro135),GasCO2));   //Armazena o valor de CO2 captado pelo MQ135
    arquivo.print("\t");
    arquivo.print(analogRead(MQ135_analog));   //Armazenando o valor bruto do MQ-135
    arquivo.print("\t");
    arquivo.println("");
    arquivo.close();           // Fechamos o arquivo
  }

  delay (3000); // Intervalo de 15 minutos para a proxima leitura e gravação no arquivo
  i += 1;

}

float calcularResistencia(int tensao)   //funcao que recebe o tensao (dado cru) e calcula a resistencia efetuada pelo sensor. O sensor e a resistência de carga forma um divisor de tensão. 
{
  return (((float)VRL_VALOR*(1023-tensao)/tensao));
}

float MQCalibration(int mq_pin)   //funcao que calibra o sensor em um ambiente limpo utilizando a resistencia do sensor em ar limpo 9.83
{
  int i;
  float valor=0;

  for (i=0;i<ITERACOES_CALIBRACAO;i++) {    //sao adquiridas diversas amostras e calculada a media para diminuir o efeito de possiveis oscilacoes durante a calibracao
    valor += calcularResistencia(analogRead(mq_pin));
    delay(500);
  }
  valor = valor/ITERACOES_CALIBRACAO;        

  valor = valor/RO_FATOR_AR_LIMPO; //o valor lido dividido pelo R0 do ar limpo resulta no R0 do ambiente

  return valor; 
}

float leitura_MQ(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<ITERACOES_LEITURA;i++) {
    rs += calcularResistencia(analogRead(mq_pin));
    delay(50);
  }

  rs = rs/ITERACOES_LEITURA;

  return rs;  
}

int calculaGasPPM(float rs_ro, float *pcurve) //Rs/R0 é fornecido para calcular a concentracao em PPM do gas em questao. O calculo eh em potencia de 10 para sair da logaritmica
{
  return (pow(10,( ((log(rs_ro)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

int getQuantidadeGasMQ(float rs_ro, int gas_id)
{
  if ( gas_id == 0 ) {
     return calculaGasPPM(rs_ro,CH4Curve);
  } else if ( gas_id == 1 ) {
     return calculaGasPPM(rs_ro,CO2Curve);
  }/*else if ( gas_id == 2 ) {
     return calculaGasPPM(rs_ro,N2OCurve);
  } */   

  return 0;
}
