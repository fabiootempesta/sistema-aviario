//---Bibliotecas---
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPIFFS.h>
#include "DHT.h"
#include "ESPAsyncWebServer.h"


//---Pinos de entrada e saída---
#define PIN_DS18B20 2
#define PIN_DHT11 4

#define PIN_NIPPLE 5  
#define PIN_FAN 18 
#define PIN_NEBULIZER 19  

//---Configuração de bibliotecas---
#define DHTYPE DHT11
DHT dht(PIN_DHT11, DHTYPE);
OneWire oneWire(PIN_DS18B20);
DallasTemperature ds18b20(&oneWire);
AsyncWebServer server(80);

//---Conexão na rede local sem fio---
const char* ssid     = "Bethpsi";
const char* password = "raulfabio21";

//---Variáveis globais---
//Valores dos parâmetros

int parameter_nebulizer_on_humidity = 60;
int parameter_nebulizer_off_humidity = 75;
float parameter_nebulizer_on_temperature = 26;
float parameter_nebulizer_off_temperature = 22;

float parameter_nipple_on_deltat = 5.4;
int parameter_nipple_off_time = 8000;

float parameter_fan_on_temperature = 23.5;
float parameter_fan_off_temperature = 22;


//Valores atuais rebido dos sensores
float current_nipple_temp = 0;
float current_box_temp = 0;
float current_climate_temp = 0;
float current_climate_humidity = 0;

//Variáveis de tempo para função millis()
long update_time = 0;
long exchanger_time = 0;

//Variável para auxiliar desligar ou manter o ventilador ligado após desligar o nebulizador
bool fan_on_directly = 0;
 
//Modos de operação
bool operation_mode_nebulizer = false; //false = automático | true = manual
bool operation_mode_fan = false; //false = automático | true = manual
bool operation_mode_exchanger = true; //false = automático | true = manual




bool stringToBool(String n){ //qualquer valor diferente de 1 retornará false
  return (n == "1");
}

String getStatusActuator(char n){
  //'n' para nebulizador; 'e' para o trocador de agua; 'f' para o ventilador
  int pin;
  switch (n) {
    case 'n':
      pin = PIN_NEBULIZER;
      break;
    case 'e':
      pin = PIN_NIPPLE;
      break;
    case 'f':
      pin = PIN_FAN;
      break;
  }

  if(digitalRead(pin) == HIGH)
    return "1";
  else
    return "0";
  
}

void printActuators(){
  Serial.println("---------");
  Serial.print("N: " );
  Serial.print(getStatusActuator('n'));
  Serial.print(" ");
  Serial.println(operation_mode_nebulizer);
  Serial.print("F: ");
  Serial.print(getStatusActuator('f'));
  Serial.print(" ");
  Serial.println(operation_mode_fan);
  Serial.print("E: ");
  Serial.print(getStatusActuator('e'));
  Serial.print(" ");
  Serial.println(operation_mode_exchanger);
  Serial.println("---------");

}

void setNebulizer(bool state){
  if (state){
    Serial.println("ligou");
    digitalWrite(PIN_NEBULIZER, HIGH);
    if (digitalRead(PIN_FAN)==HIGH)
      fan_on_directly = 1;
    else{
      fan_on_directly=0;
      digitalWrite(PIN_FAN, HIGH);
    }
  }else{
    Serial.println("desligou nebulizador");
    digitalWrite(PIN_NEBULIZER, LOW);
    Serial.println("desligou nebulizador");
    if(fan_on_directly == 0)
      digitalWrite(PIN_FAN, LOW);
  }
  printActuators();
}

void setExchanger(bool state, int time_exchange){
  //Liga trocador de água
  //timeExchange = 0 quando a duração da troca for da variavel parameter_nipple_off_time
	if (state){
    digitalWrite(PIN_NIPPLE, HIGH);
    if (time_exchange == 0)
      exchanger_time = millis() + parameter_nipple_off_time;
    else
      exchanger_time = millis() + time_exchange;
	}else{ 
    digitalWrite(PIN_NIPPLE, LOW);
	}
}

void automaticModeNebulizer(){
  if(!operation_mode_nebulizer){
    
    //Liga Nebulizador
    if(digitalRead(PIN_NEBULIZER) == LOW){
      if( (current_climate_humidity < parameter_nebulizer_on_humidity) && (current_climate_temp > parameter_nebulizer_on_temperature) ){
        setNebulizer(1);
        Serial.println("Nebulizador ligado via modo automático");
      }

    //Desliga Nebulizador
    }else{
      if( (current_climate_humidity > parameter_nebulizer_off_humidity) || (current_climate_temp < parameter_nebulizer_off_temperature) ){
        setNebulizer(0);
        Serial.println("Nebulizador desligado via modo automático");
      }
    }
  }
}

void automaticModeExchanger(){
  if(!operation_mode_exchanger){
    if( (current_nipple_temp - current_box_temp) > parameter_nipple_on_deltat){
      Serial.println("Água trocada via modo automático");
      setExchanger(1, 0);
    }
  }
}

void automaticModeFan(){
  if(!operation_mode_fan)  { //Liga Ventilador
    if (digitalRead(PIN_FAN) == LOW){
      if (current_climate_temp > parameter_fan_on_temperature){
        digitalWrite(PIN_FAN, HIGH);
        Serial.println("Ventilaador ligado via modo automático");
      }
    }else{ //Desliga Ventilador
      if ((current_climate_temp < parameter_fan_off_temperature) && (digitalRead(PIN_NEBULIZER) == LOW)){
        digitalWrite(PIN_FAN, LOW);
        Serial.println("Ventilador ligado via modo automático");
      }
    }
  }
}

void changeWaterOff(){
  if((digitalRead(PIN_NIPPLE) == HIGH) && (exchanger_time < millis())){
    digitalWrite(PIN_NIPPLE, LOW);
    Serial.println("Trocador de água terminou sua ação!");
	}
}

void printSensorsValue(){
	Serial.println("VALORES DOS SENSORES");
	// Sensores DS18B20
	Serial.print("Temperatura do bebedouro: ");
	Serial.print(current_nipple_temp);
	Serial.println("ºC");

	Serial.print("Temperatura da água da caixa: ");
	Serial.print(current_box_temp);
	Serial.println("ºC");


  //Sensor DHT11
  
	if (isnan(current_climate_temp) || isnan(current_climate_humidity)) 
    	Serial.println("Falha em ler o sensor DHT11!");
  	else{
		Serial.print("Temperatura do aviário: ");
		Serial.print(current_climate_temp);
		Serial.print(" ºC\n");
		Serial.print("Umidade do aviário: ");
		Serial.print(current_climate_humidity);
		Serial.println(" % ");
		Serial.println();
  	}
  
  
}

void updateSensorsValue(){

  Serial.println("Atualizando valores dos sensores.");
	ds18b20.requestTemperatures(); 
  if (!(isnan( dht.readTemperature()) || isnan(dht.readHumidity()))) {
    current_climate_temp = dht.readTemperature();
    current_climate_humidity = dht.readHumidity();
  }
  if (ds18b20.getTempCByIndex(0) != -127.00)
	  current_nipple_temp = ds18b20.getTempCByIndex(0);

  if (ds18b20.getTempCByIndex(1) != -127.00) 
	  current_box_temp = ds18b20.getTempCByIndex(1);
	
}

void setup() {

	Serial.begin(9600);
	pinMode(PIN_NIPPLE, OUTPUT);
	pinMode(PIN_FAN, OUTPUT);
	pinMode(PIN_NEBULIZER, OUTPUT);
	digitalWrite(PIN_NIPPLE, LOW);
	digitalWrite(PIN_FAN, LOW);
	digitalWrite(PIN_NEBULIZER, LOW);
	
  ds18b20.begin();
	dht.begin();
  
  Serial.print("Conectando em ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  long time_out_connect = 30000 + millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (time_out_connect > millis()){
      WiFi.reconnect();
      long time_out_connect = 30000 + millis();
    }
  }
  if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // URL para raiz (index)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/html", index_html, processor);
    //Send index.htm with default content type
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // HTTP basic authentication
  /*server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request->authenticate("user", "pass"))
        return request->requestAuthentication();
    request->send(200, "text/plain", "Login Success!");
  });*/

  // --- URL para arquivos estáticos ---
  
  //CSS e JS pessoais
  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles.css", "text/css");
  });
  server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/scripts.js", "text/javascript");
  });

  /*//Bootstrap
  server.on("/bootstrap/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bootstrap/bootstrap.min.css", "text/css");
  });
  server.on("/bootstrap/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bootstrap/bootstrap.min.js", "text/javascript");
  });*/
  
  //Imagens
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/favicon.ico", "image/png");
  });

  server.on("/img/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/logo.png", "image/png");
  });

  server.on("/img/air_temperature.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/air_temperature.png", "image/png");
  });

  server.on("/img/exchanger.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/exchanger.png", "image/png");
  });

  server.on("/img/box_temperature.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/box_temperature.png", "image/png");
  });

  server.on("/img/fan.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/fan.png", "image/png");
  });

  server.on("/img/humidity.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/humidity.png", "image/png");
  });

  server.on("/img/nebulizer.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/nebulizer.png", "image/png");
  });

  server.on("/img/nipple_temperature.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/nipple_temperature.png", "image/png");
  });




  //---Rotas para requisições de origem da Página Web---
  //URL para requisitar a temperatura   temperatura em c_str, porque deve ser mandado em vetor de char   request é um ponteiro
  //request é um ponteiro que aponta pra qual tipo de requisição vai ser feita  send_P é para enviar uma pagina web grande  send é para respostas simples

  //URL para requisitar o valor atual de cada sensor
  server.on("/sensors/getall", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = String(current_climate_temp) + " " + String(current_climate_humidity) + " " + String(current_box_temp) + " " + String(current_nipple_temp);
    request->send(200, "text/plain", response.c_str());
  });

  //URL para acionar/desligar o nebulizador
  server.on(
    "/actuators/status/set/nebulizer",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          setNebulizer(stringToBool(p->value()));
          request->send(200);
          Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
          Serial.println(stringToBool(p->value()));
            
        }
        
    }
  });

  //URL para ligar/desligar o trocador de agua
  server.on(
    "/actuators/status/set/exchanger", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    bool flagState, flagTime, currentState;
    int currentTime = 0;
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name() == "value"){
          flagState = true;
          currentState = (stringToBool(p->value()));
        }
        if(p->name() == "time"){
          if (currentTime = p->value().toInt())
            flagTime = true;
        }
        
    }
    if (flagState){
      if (currentState){
        if (flagState){
          setExchanger(currentState, currentTime * 1000);
          request->send(200);
        }else{
          request->send(304);
        }
      }else{
        setExchanger(currentState, 0);
        request->send(200);
      }
    }else{
      request->send(304);
    }
  });

  //URL para ligar/desligar o ventilador
  server.on(
    "/actuators/status/set/fan", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
      AsyncWebParameter *p = request->getParam(i);
      
      if(p->name()=="value"){
        if (stringToBool(p->value())){
          digitalWrite(PIN_FAN,HIGH);
          request->send(200);
          Serial.println("Status do ventilador modificado via interface Web!");
        }else{
          if(digitalRead(PIN_NEBULIZER)==HIGH){
            request->send(409, "text/plain", String("Ventilador não pode ser desligado enquanto o nebulizador estiver em funcionamento!").c_str());
          }else{
            digitalWrite(PIN_FAN,LOW);
            request->send(200);
            Serial.println("Status do ventilador modificado via interface Web!");
          }
        }
      }
    }
  });

  //URL para capturar o status atual de cada atuador
  server.on("/actuators/status/getall", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = String(getStatusActuator('e')) +" " + String(getStatusActuator('f')) + " " + String( getStatusActuator('n'));
    request->send(200, "text/plain", response.c_str());
  });

  //URL para capturar o valor atual de cada parâmetro de acionamento
  server.on("/actuators/parameter/getall", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = String(parameter_nebulizer_on_humidity) +" " + String(parameter_nebulizer_off_humidity) + " " + String(parameter_nebulizer_on_temperature)
    + " " + String(parameter_nebulizer_off_temperature) + " " + String(parameter_nipple_on_deltat) +
    " " + String(parameter_nipple_off_time / 1000) + " " + String(parameter_fan_on_temperature) +" " + String(parameter_fan_off_temperature);
    
    request->send(200, "text/plain", response.c_str());
  });

  //URL para capturar o modo de operação de cada atuador
  server.on("/actuators/opmode/getall", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = String(operation_mode_nebulizer) + " " + String(operation_mode_exchanger) + " " + String(operation_mode_fan);
    request->send(200, "text/plain", response.c_str());
  });

  //URL para modificar o modo de operação do nebulizador
  server.on(
    "/actuators/opmode/set/nebulizer", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
      AsyncWebParameter *p = request->getParam(i);
      
      if(p->name()=="value"){
        operation_mode_nebulizer = stringToBool(p->value());
        Serial.println(stringToBool(p->value()));
        request->send(200);
        Serial.println("Nebulizador trocou seu modo de operação!");
      }
    }
  });

  //URL para modificar o modo de operação do trocador de água
  server.on(
    "/actuators/opmode/set/exchanger", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          operation_mode_exchanger = stringToBool(p->value());
          request->send(200);
          Serial.println("Trocador trocou seu modo de operação!");
        }
    }
  });

  //URL para modificar o modo de operação do ventilador
  server.on(
    "/actuators/opmode/set/fan", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          operation_mode_fan = stringToBool(p->value());
          request->send(200);
          Serial.println("Ventilador trocou seu modo de operação!");
        }
    }
  });

  //URL para modificar o parâmetro de temperatura do ventilador
  server.on(
    "/actuators/parameter/fan/on/temp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          
          if(float current_value = (p->value().toFloat())){
            parameter_fan_on_temperature = current_value;
            request->send(200);
            Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
    }
  });

  server.on(
    "/actuators/parameter/fan/off/temp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          
          if(float current_value = (p->value().toFloat())){
            parameter_fan_off_temperature = current_value;
            request->send(200);
            Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
    }
  });

  //URL para modificar o parâmetro de variação da temperatura do trocador de água
  server.on(
    "/actuators/parameter/exchanger/on/deltatemp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name() == "value"){
          if(float current_value = (p->value().toFloat())){
            parameter_nipple_on_deltat = current_value;
            request->send(200);
            Serial.println("Parâmetro de acionamento do trocador modificado via interface Web!");
          }else{
            request->send(304);
            Serial.println("Erro em modificar o parâmetro de acionamento do trocador via interface Web!");
          }
        }
    }
  });

  //URL para modificar o parâmetro de variação da temperatura do trocador de água
  server.on(
    "/actuators/parameter/exchanger/off/time",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name() == "value"){
          if(float current_value = (p->value().toFloat())){
            parameter_nipple_off_time = current_value * 1000;
            request->send(200);
            Serial.println("Parâmetro de acionamento do trocador modificado via interface Web!");
          }else{
            request->send(304);
            Serial.println("Erro em modificar o parâmetro de acionamento do trocador via interface Web!");
          }
        }
    }
  });

  //URL para modificar o parâmetro de umidade do nebulizador
  server.on(
    "/actuators/parameter/nebulizer/on/humidity",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(float current_value = (p->value().toFloat())){
            parameter_nebulizer_on_humidity = current_value;
            request->send(200);
            Serial.println("Parâmetro de umidade de acionamento do nebulizador modificado via interface Web!");
          }else{
            request->send(304);
            Serial.println("Erro em modificar o parâmetro de acionamento do nebulizador via interface Web!");
          }
        }
    }
  });

  //URL para modificar o parâmetro de umidade do nebulizador
  server.on(
    "/actuators/parameter/nebulizer/off/humidity",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(float current_value = (p->value().toFloat())){
            parameter_nebulizer_off_humidity = current_value;
            request->send(200);
            Serial.println("Parâmetro de umidade de acionamento do nebulizador modificado via interface Web!");
          }else{
            request->send(304);
            Serial.println("Erro em modificar o parâmetro de acionamento do nebulizador via interface Web!");
          }
        }
    }
  });
  
  //URL para modificar o parâmetro de temperatura do nebulizador
  server.on(
    "/actuators/parameter/nebulizer/on/temp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        if(p->name()=="value"){
          if(float current_value = (p->value().toFloat())){
            parameter_nebulizer_on_temperature = current_value;
            request->send(200);
            Serial.println("Parâmetro de temperatura de acionamento do nebulizador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
    }
  });

  //URL para modificar o parâmetro de temperatura do nebulizador
  server.on(
    "/actuators/parameter/nebulizer/off/temp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        if(p->name()=="value"){
          if(float current_value = (p->value().toFloat())){
            parameter_nebulizer_off_temperature = current_value;
            request->send(200);
            Serial.println("Parâmetro de temperatura de acionamento do nebulizador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
    }
  });
  
  server.begin();

}

void loop() {

	if (update_time < millis()){
    printActuators();
    updateSensorsValue();
    printSensorsValue();
    
    automaticModeExchanger();
    automaticModeFan();
    automaticModeNebulizer();

    update_time = millis() + 10000;
  }

  changeWaterOff();
	
}
