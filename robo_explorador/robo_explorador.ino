#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>                       // Biblioteca DHT Sensor Adafruit 
#include <DHT.h>
#include <DHT_U.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

extern "C" {
  #include "predict_chancevida.h"
}
/* Definição de constantes utilizadas */

#define DHTTYPE  DHT11  
#define DHTPIN 12     // entrada digital
#define LDR 13        // entrada analógica
#define POTA 2        // entrada analógica
#define BT_PARADA 15  // entrada digital via interrupção 16 trocar
#define DELAY_DB  100 // 100 ms de tempo para o debouncer
/*   Definição de prototipos de funções*/
void conectar_wifi(const char *ssid ,const char *password,bool rede_segura);
void conectar_mqtt();
void publicar_dados();
void conectar_NTP();
void callback(char *topic, byte *payload, unsigned int length);
void leitura_sensores();
void leitura_dht11();
uint16_t leitura_LDR();
uint16_t leitura_gas();
void IRAM_ATTR isr();
void message_zap(String message);
String formatar_string();
void get_time_stamp();
String gerarJson();
/*
| Constantes definidas para as configurações de Wifi e do cliente MQTT 
*/

// rede wifi
const char *ssid = "Geane";
const char *password = "21050719";

// dados para api do whatssapp
const char* phoneNumber = "+557581080457"; 
const char* apiKey = "8233157";

// cliente mqtt
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "uefsleds/esp32";
const char *mqtt_username = "NAlisson";
const char *mqtt_password = "Nathiele";
const int mqtt_port = 1883;

const char *topico_temperatura =      "UEFS_TEC_470/temperatura";
const char *topico_umidade_relativa = "UEFS_TEC_470/umidade_relativa";
const char *topico_luz =              "UEFS_TEC_470/luz";
const char *topico_gas =              "UEFS_TEC_470/gas";
const char *topico_chance_vida =      "UEFS_TEC_470/chance_vida";
const char *topico_data_hora =        "UEFS_TEC_470/data_hora";
const char *topico_operacao =         "UEFS_TEC_470/operacao";

const uint8_t led_gpio = 12;
// variaveis globais definidas
float    temperatura =0.0;
float    umidade_relativa=0.0;
double   chance_vida = 0.0;
uint16_t luz=0;
uint16_t gas=0;

// Variaveis para timestamp
String formattedDate;
String dayStamp;
String timeStamp;

// Flag para temporização
static volatile bool flag_temp_100ms = false;

// Variaveis para debouncer via software
unsigned long tempo_atual = 0;  
unsigned long tempo_anterior = 0;

// variaveis para temporização
unsigned long tempo_decorrido =0;
unsigned long tempo_loop=0;

uint8_t ct_500ms=0;
uint8_t ct_1s = 0;

// Cria clientes de conexão de rede
WiFiClient espClient;
PubSubClient client(espClient);

// variavies para conexão com servidor de cliente de data
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Cria objeto DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);                  // configurando o Sensor DHT - pino e tipo
uint32_t delayMS;                                  // variável para atraso no tempo

// Cliente HTTP
HTTPClient http;
static volatile bool flag_msg =false;

void setup() {
  // Inicia o monitor serial
  Serial.begin(115200);
  //inicia conexão wifi
  conectar_wifi(ssid,password,true);
  // inicia cliente mqtt
  conectar_mqtt();
  // inscreve-se nos topicos
  client.subscribe(topico_temperatura);
  client.subscribe(topico_umidade_relativa);
  client.subscribe(topico_gas);
  client.subscribe(topico_luz);
  client.subscribe(topico_chance_vida);
  client.subscribe(topico_temperatura);
  client.subscribe(topico_operacao);

  //Inicia sensor DHT11
  dht.begin();
  // configura botão de parada como entrada em modo pull-up e interrupção por borda de descida
  pinMode(BT_PARADA,INPUT_PULLUP);
  attachInterrupt(BT_PARADA, isr, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(flag_msg){
    client.publish(topico_operacao,"teste");
    flag_msg = false;
  }
  tempo_loop = millis();
  // caso tenham se passado 100ms
  if(tempo_loop - tempo_anterior >= DELAY_DB){
    // incrementa o contador de 500ms
    ct_500ms++;
    tempo_loop = tempo_anterior;
    client.loop();
    publicar_dados();
    //Serial.println("hora de ler");
  }
  // caso tenham se passado 500ms
  if(ct_500ms == 5){
    // limpa contador
    ct_500ms=0;
    // incrementa o contador de segundos
    ct_1s++;
    // realiza leitura dos sensores
    leitura_sensores();
    // calcula chance de vida
    double entrada[4] = {temperatura,umidade_relativa,gas,luz};
    chance_vida = predict_chancevida(entrada);

    if(chance_vida >80){
      Serial.println("Da pra abrir uma drogasil!!");

      // envia menssagem para o whatssapp
      message_zap(formatar_string());
    }
    else if(chance_vida <80 && chance_vida >=50){
      Serial.println("Tão moravel quanto o Brasil(parte dele)!!");
    }
    else{
       Serial.println("Muito ruim, pior que isso só morar em Feira");
    }
    Serial.print("Chance de vida:");
    Serial.println(chance_vida);

  }

  //caso tenham se passado
  if(ct_1s ==2){
    // limpa contador
    ct_1s=0;
    // envia dados ao broker
    publicar_dados();
  }
}

/*
| Função conectar_wifi
| Conecta a esp32 a rede wifi presente
| Caso seja uma rede sem proteção o argumento password deve ser 0
*/
void conectar_wifi(const char *ssid ,const char *password,bool rede_segura){
  // configura a conexão
  if(rede_segura){
    WiFi.begin(ssid,password);
  }
  else{
    WiFi.begin(ssid);
  }

  // Tenta realizar a conexão com a rede
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Conectando-se ao wifi..");
  }

  Serial.println("Conectado a rede wifi!!");
}


void conectar_mqtt(){
  // Configura o endereço e porta do broker
  client.setServer(mqtt_broker, mqtt_port);
  // define a função de callback
  client.setCallback(callback);

  // Tenta realizar a conexão com o brocker
  while (!client.connected()) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("O cliente %s esta conectado ao broker publico\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Conectado ao brocker");
      } else {
          Serial.print("Falha em estabelecer conexão ");
          Serial.print(client.state());
          delay(2000);
      }
    }
}

/*
| Função publicar_dados
| Publica os dados dos sensores nos topicos inscritos como pulblicador
*/
void publicar_dados(){
  char buffer[10];
    
    // dado de temperatura
    sprintf(buffer,"%.2f",temperatura);
    client.publish(topico_temperatura,buffer);
    // dado de umidade_relativa
    sprintf(buffer,"%d",umidade_relativa);
    client.publish(topico_umidade_relativa,buffer);
    // dado de luz
    sprintf(buffer,"%d",luz);
    client.publish(topico_luz,buffer);
    // dado de gas
    sprintf(buffer,"%d",gas);
    client.publish(topico_gas,buffer);
    // dado de chance de vida
    sprintf(buffer,"%d",chance_vida);
    client.publish(topico_chance_vida,buffer);
    // time_stamp da medição
    /*
    String jsonParaEnviar = gerarJson();
    client.publish("UEFS_TEC_470/medidas", jsonParaEnviar.c_str());
    */
}
/*
| Função callback
| É chamada quando uma mensagem é postada em um tópico no qual o cliente
| esta inscrito como assinante
*/
void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    String message= "";
    for (int i = 0; i < length; i++) {
        message += (char) payload[i];
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
    /*
    if(message == "ligar"){
        digitalWrite(led_gpio,HIGH);
    }
    else if(message == "desligar"){
         digitalWrite(led_gpio,LOW);
    }
    */
}

/*
| Função leitura_dht11
| Realiza a leitura da temperatura e umidade com o sensor dht11 e atualiza os valores das variaveis
*/
void leitura_dht11(){
  // cria o evento de captura do sensor
  sensors_event_t event;
  // captura evento de leitura da temperatura
  dht.temperature().getEvent(&event);

  if(isnan(event.temperature)){
    Serial.println("Erro na leitura da temperatura!!");
  }
  else{
    temperatura = event.temperature;
    Serial.print("Temperatura:");
    Serial.print(temperatura);
    Serial.println(" *C");
  }
  // cria o evento de leitura da umidade
  dht.humidity().getEvent(&event);           
  if (isnan(event.relative_humidity)){
    Serial.println("Erro na leitura da Umidade!!!");
  }
  else                                          
  {
    umidade_relativa = event.relative_humidity;
    Serial.print("Umidade: ");                  // imprime a Umidade
    Serial.print(umidade_relativa);
    Serial.println("%");
  }
}
/*
| Função de leitura dos sensores
|
*/
void leitura_sensores(){
  leitura_dht11();
  luz = leitura_LDR();
  gas = leitura_gas();

}
/*
| Função leitura_LDR
| Realiza a leitura anlógica do sensor de luz e converte os valores para faixa de operação do sensor
| Retorna o valor da luz medida de 0 até 100
*/
uint16_t leitura_LDR(){
  uint16_t aux;
  // faz a leitura no pino do sensor LDR
  aux = analogRead(LDR);
  aux = map(aux,0,1800,1090,150);
  return aux;
}
uint16_t leitura_gas(){
  uint16_t aux;
  // faz a leitura no pino do sensor de gás(potenciometro)
  aux = analogRead(POTA);
  aux = map(aux,100,4095,9,220);
  return aux;
}

/*
| Função de tratamento da interrupção externa do GPIO
|
*/
void IRAM_ATTR isr() {
  tempo_atual = millis();
  if(tempo_atual-tempo_anterior > DELAY_DB){
    //Serial.println("bt pressionado");
    // atualiza contador de tempo
    tempo_anterior = tempo_atual;
    flag_msg = true;
  }
	
}

void conectar_NTP(){
  timeClient.begin();
  timeClient.setTimeOffset(-10800);
}

void get_time_stamp(){
  // Inicia conexão e tenta buscar servidor
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  // extrai a data e hora formatada
  formattedDate = timeClient.getFormattedTime();
  Serial.println(formattedDate);
  // extrai a data
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // extrai o tempo
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
}

/*
| Função message_zap
|
*/
void message_zap(String message){
  if (WiFi.status() == WL_CONNECTED) {
    String url = "https://api.callmebot.com/whatsapp.php?phone=" + String(phoneNumber) + 
                 "&text=" + message + 
                 "&apikey=" + String(apiKey);
    
    //String url = formatar_string();
    Serial.println(url);
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Resposta do CallMeBot:");
      Serial.println(response);
    } else {
      Serial.print("Erro HTTP: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}

String formatar_string(){
  // formata mensagem a ser enviada
  String mensagem = "Vida%20encontrada%20no%20planeta!!%0A";
  mensagem += "Chance%20de%20vida:%20" + String(chance_vida) + "%0A";
  mensagem += "Temperatura:%20" + String(temperatura) + "%0A";
  mensagem += "Umidade%20relativa:%20" + String(umidade_relativa) + "%0A";
  mensagem += "Gas:%20" + String(gas) + "%0A";
  mensagem += "Luz:%20" + String(luz) + "%0A";
 
  return mensagem;
}

String gerarJson(){
  StaticJsonDocument<256> doc;

  // Preenche o documento JSON
  doc["temperatura"] = temperatura;
  doc["umidade_relativa"] = umidade_relativa;
  doc["luz"] = luz;
  doc["gas"] = gas;
  doc["chance_vida"] = chance_vida;
  doc["operacao"] = operacao;
  doc["data_hora"] = data_hora;

  // Serializa para uma string e retorna
  String output;
  serializeJson(doc, output);
  return output;
}