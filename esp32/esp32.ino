//---Bibliotecas---
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "ESPAsyncWebServer.h"


//---Pinos de entrada e saída---
#define PIN_DS18B20 2
#define PIN_DHT11 4
#define PIN_NIPPLE 27  
#define PIN_FAN 19 
#define PIN_NEBULIZER 21

//---Configuração de bibliotecas---
#define DHTYPE DHT11
DHT dht(PIN_DHT11, DHTYPE);
OneWire oneWire(PIN_DS18B20);
DallasTemperature ds18b20(&oneWire);
AsyncWebServer server(80);

//---Conexão na rede local sem fio---
const char* ssid     = "Raul";
const char* password = "pederinoceronte";

//---Variáveis globais---
//Valores dos parâmetros
float deltat_nipple_parameter = 5.4;
float temperature_nebulizer_parameter = 20.0;
float temperature_fan_parameter = 22.2;
int humidity_nebulizer_parameter = 90;
long time_nipple_parameter = 14400000; // 14.400.000 ms = 4 horas
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
bool operation_mode_exchanger = false; //false = automático | true = manual


//PROGMEM para armazenar na memória flash | R"rawliteral" = trate tudo como uma "raw string"
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="pt-BR">

<head>
  <title>Controle Aviário</title>
  <meta charset="UTF-8">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
    }

    p {
      font-size: 3.0rem;
    }

    span {
      font-weight: bold;
    }

    button {
      border-radius: 8px;
    }

    table, td {
      margin: auto;
      border: 1px solid black;
      border-collapse: collapse;
    }

    a {
      color: black;
    }

    input {
      width: 20%;
      margin: 4px 0;
      box-sizing: border-box;
      border: 2px solid #ccc;
      -webkit-transition: 0.5s;
      transition: 0.5s;
      outline: none;
    }

    input:focus{
      border: 2px solid #555;
    }

    .units {
      font-size: 1.2rem;
    }

    .divPrincipal {
      border: 2px solid black;
      height: 300px;
      width: 1000px;
      margin: auto;
    }

    .divSensor {
      box-sizing: border-box;
      float: left;
      width: 25%;
      height: 100.2%;
    }

    .divAtuador {
      box-sizing: border-box;

      float: left;
      width: 33.33333%;
      height: 100.2%;
    }

    .divOperacao {
      float: left;
      width: 100%;
      height: 50%;
    }

    .divError {
      float: left;
      width: 33.33333%;
      color: red;
      display: none;
    }


    #div_param_nebulizer {
      display: block;
    }

    #div_act_nebulizer {
      display: none;
    }

    #div_sensor_temp {
      border-right: 1px solid black;
    }

    #div_sensor_umidity {
      border-right: 1px solid black;
      border-left: 1px solid black;
    }

    #div_sensor_nipple {
      border-right: 1px solid black;
      border-left: 1px solid black;
    }

    #div_sensor_box {
      border-left: 1px solid black;
    }

    #div_actuator_nebulizer {
      border-right: 1px solid black;
    }

    #div_actuator_exchanger {
      border-right: 1px solid black;
      border-left: 1px solid black;
    }

    #div_actuator_fan {
      border-left: 1px solid black;
    }

    


  </style>
</head>

<body>
  <h1>MONITORAMENTO E CONTROLE DO AMBIENTE AVIÁRIO</h1>
  <h2>Sensores    <img src="https://i.ibb.co/bFLxfbg/sensores.png" width="35" height="25"></h2> 
  <div class="divPrincipal">
    <div class="divSensor" id="div_sensor_temp">
      <h3>Temperatura ambiente</h3> <br> <span id="climate_temperature"> --- </span> <br> <br>
      <img src="https://i.ibb.co/F62Cdvf/temperatura-ambiente.png" width="210" height="170">
    </div>
    <div class="divSensor" id="div_sensor_umidity">
      <h3>Umidade relativa do ar</h3> <br> <span id="climate_humidity"> --- </span> <br> <br>
      <img src="https://i.ibb.co/brRzT46/umidade.png" width="150" height="170">
    </div>
    <div class="divSensor" id="div_sensor_nipple">
      <h3>Temperatura da água do bebedouro</h3> <span id="nipple_temperature"> --- </span><br> <br>
      <img src="https://i.ibb.co/1bLQnq1/temperatura-bebedouro.png" width="130" height="170">
    </div>
    <div class="divSensor" id="div_sensor_box">
      <h3>Temperatura da água da caixa</h3> <span id="box_temperature"> --- </span><br> <br>
      <img src="https://i.ibb.co/nRSqZgW/temperatura-caixa.png" width="200" height="170">
    </div>
  </div>
  <h2>Atuadores  <img src="https://i.ibb.co/xzX95vw/atuadores.png" width="25" height="30"></h2>
  <div class="divPrincipal">
    <div class="divAtuador" id="div_actuator_nebulizer">
      <h3>Nebulizador</h3>
      Status: <span id="status_nebulizer"> --- </span> <br>
      <b> Modo de Operação  <img src="https://i.ibb.co/3hS4Xv1/modo-operacao.png" width="18" height="18"> </b>
      <table>
        <thead>
          <tr>
            <td id="manual1"> <a href='javascript:buttonSetOpModeNebulizer(1);'>Manual</a> </td>
            <td id="automatico1"> <a href='javascript:buttonSetOpModeNebulizer(0);'>Automático</a> </td>
          </tr>
        </thead>
      </table>
      <hr>

      <div id="div_param_nebulizer" class="divOperacao">

        <b>Parâmetro de acionamento <img src="https://i.ibb.co/fdLtDWH/parametro.png" width="18" height="18"></b><br><br>
        Umidade máxima: <br>
        <input type="text" id="minUmidity" value=""> % 
        <button type="button" onclick="buttonSetUmidity()">Reconfigurar</button> <br><br>
        Temperatura mínima: <br>
        <input type="text" id="maxTempNeb" value=""> °C 
        <button type="button" onclick="buttonSetTempNeb()">Reconfigurar</button>
      </div>

      <div id="div_act_nebulizer" class="divOperacao">
        <br><b>Acionamento <img src="https://i.ibb.co/1dJJJDr/acionamento.png" width="18" height="18"></b><br><br>
        <button type="button" id="button_nebulizer" onclick="buttonSetNebulizer()">---</button>
      </div>

      <div id="div_error_1" class="divError"> <b>Falha na conexão com o atuador!</b> </div>

    </div>
    <div class="divAtuador" id="div_actuator_exchanger">
      <h3>Trocador de água</h3>
      Status: <span id="status_exchanger"> --- </span> <br>
      <b> Modo de Operação  <img src="https://i.ibb.co/3hS4Xv1/modo-operacao.png" width="18" height="18"> </b>
      <table>
        <thead>
          <tr>
            <td id="manual2"> <a href='javascript:buttonSetOpModeExchanger(1);'>Manual</a> </td>
            <td id="automatico2"> <a href='javascript:buttonSetOpModeExchanger(0);'>Automático</a> </td>
          </tr>
        </thead>
      </table>
      <hr>

      <div id="div_param_exchanger" class="divOperacao">

        <b>Parâmetro de acionamento <img src="https://i.ibb.co/fdLtDWH/parametro.png" width="18" height="18"></b><br><br><br>
        Variação mínima: <br>
        <input type="text" id="deltaTemperature" value=""> °C 
        <button type="button" onclick="buttonSetVariation()">Reconfigurar</button>
      </div>

      <div id="div_act_exchanger" class="divOperacao">
        <br>
        <b>Acionamento <img src="https://i.ibb.co/1dJJJDr/acionamento.png" width="18" height="18"></b><br><br>
        <button type="button" id="button_exchanger" onclick="buttonSetExchanger()">---</button>
      </div>
      <div id="div_error_2" class="divError"> <b>Falha na conexão com o atuador!</b> </div>
    </div>

    <div class="divAtuador" id="div_actuator_fan">
      <h3>Ventilador</h3>
      Status: <span id="status_fan">---</span> <br>
      <b> Modo de Operação  <img src="https://i.ibb.co/3hS4Xv1/modo-operacao.png" width="18" height="18"> </b>
      <table>
        <thead>
          <tr>
            <td id="manual3"> <a href='javascript:buttonSetOpModeFan(1);'>Manual</a> </td>
            <td id="automatico3"> <a href='javascript:buttonSetOpModeFan(0);'>Automático</a> </td>
          </tr>
        </thead>
      </table>
      <hr>

      <div id="div_param_fan" class="divOperacao">
        <b>Parâmetro de acionamento  <img src="https://i.ibb.co/fdLtDWH/parametro.png" width="18" height="18"></b><br><br><br>
        Temperatura mínima: <br>
        <input type="text" id="maxTempFan"> °C 
        <button onclick="buttonSetTempFan()">Reconfigurar</button>

      </div>
      <div id="div_act_fan" class="divOperacao">
        <br>
        <b>Acionamento <img src="https://i.ibb.co/1dJJJDr/acionamento.png" width="18" height="18"></b><br><br>
        <button type="button" id="button_fan" onclick="buttonSetFan()">---</button>
      </div>
      <div id="div_error_3" class="divError"> <b>Falha na conexão com o atuador!</b> </div>
    </div>
  </div>
</body>
<script>
  function buttonSetTempFan() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/actuators/parameter/fan", true);
    xhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4) {
        if (this.status == 200) {
          alert("Temperatura reconfigurada para " + document.getElementById("maxTempFan").value + "°C");
        }
        if (this.status == 304) {
          alert("Erro em reconfigurar o parâmetro de acionamento do ventilador!");
        }
      }

    };
    xhttp.send("value=" + document.getElementById("maxTempFan").value);
  }
  function buttonSetTempNeb() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/actuators/parameter/nebTemp", true);
    xhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4) {
        if (this.status == 200) {
          alert("Temperatura reconfigurada para " + document.getElementById("maxTempNeb").value + "°C");
        }
        if (this.status == 304) {
          alert("Erro em reconfigurar o parâmetro de acionamento do nebulizador!");
        }
      }

    };
    xhttp.send("value=" + document.getElementById("maxTempNeb").value);
  }
  function buttonSetVariation() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/actuators/parameter/exchanger", true);
    xhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4) {
        if (this.status == 200) {
          alert("Temperatura reconfigurada para " + document.getElementById("deltaTemperature").value + "°C");
        }
        if (this.status == 304) {
          alert("Erro em reconfigurar o parâmetro de acionamento do trocador de água!");
        }
      }
    };
    xhttp.send("value=" + document.getElementById("deltaTemperature").value);
  }
  function buttonSetUmidity() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/actuators/parameter/nebulizer", true);
    xhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4) {
        if (this.status == 200) {
          alert("Umidade reconfigurada para " + document.getElementById("minUmidity").value + "%");
        }
        if (this.status == 304) {
          alert("Erro em reconfigurar o parâmetro de acionamento do nebulizador!");
        }
      }
    };
    xhttp.send("value=" + document.getElementById("minUmidity").value);
  }

  function setOperationMode(actuator, mode, actuator_name) {
    //actuator= |1: nebulizador|  |2: trocador de água|   |3: ventilador|
    // true = modo manual | false = modo automático
    document.getElementById("div_error_" + actuator).style.display = "none";
    if (mode == true) {
      document.getElementById("manual" + actuator).innerHTML = "<b>" + document.getElementById("manual" + actuator).innerHTML + "</b>";
      document.getElementById("automatico" + actuator).innerHTML = " <a href='javascript:buttonSetOpMode" + actuator_name + "(0);'>Automático</a> ";

      if (actuator == 1) { //tratando o modo de operação do nebulizador
        document.getElementById("div_param_nebulizer").style.display = "none";
        document.getElementById("div_act_nebulizer").style.display = "block";
      }

      if (actuator == 2) { //tratando o modo de operação do trocador de água
        document.getElementById("div_param_exchanger").style.display = "none";
        document.getElementById("div_act_exchanger").style.display = "block";
      }

      if (actuator == 3) { //tratando o modo de operação do ventilador
        document.getElementById("div_param_fan").style.display = "none";
        document.getElementById("div_act_fan").style.display = "block";
      }

    } else {
      document.getElementById("automatico" + actuator).innerHTML = "<b>" + document.getElementById("automatico" + actuator).innerHTML + "</b>";
      document.getElementById("manual" + actuator).innerHTML = " <a href='javascript:buttonSetOpMode" + actuator_name + "(1);'>Manual</a> ";

      if (actuator == 1) { //tratando o modo de operação do nebulizador
        document.getElementById("div_param_nebulizer").style.display = "block";
        document.getElementById("div_act_nebulizer").style.display = "none";
      }

      if (actuator == 2) { //tratando o modo de operação do trocador de água
        document.getElementById("div_param_exchanger").style.display = "block";
        document.getElementById("div_act_exchanger").style.display = "none";
      }

      if (actuator == 3) { //tratando o modo de operação do ventilador
        document.getElementById("div_param_fan").style.display = "block";
        document.getElementById("div_act_fan").style.display = "none";
      }

    }
  }

  function buttonSetOpModeNebulizer(opmode) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/actuators/opmode/setnebulizer" + opmode, true);

    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        setOperationMode(1, opmode, "Nebulizer");
      }
    };
    xhttp.send();

  }

  function buttonSetOpModeExchanger(opmode) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/actuators/opmode/setexchanger" + opmode, true);

    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        setOperationMode(2, opmode, "Exchanger");
      }
    };
    xhttp.send();

  }

  function buttonSetOpModeFan(opmode) {
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/actuators/opmode/setfan" + opmode, true);

    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        setOperationMode(3, opmode, "Fan");
      }
    };
    xhttp.send();

  }

  function buttonSetFan() {
    var xhttp0 = new XMLHttpRequest();
    xhttp0.open("POST", "/actuators/setfan0", true);
    var xhttp1 = new XMLHttpRequest();
    xhttp1.open("POST", "/actuators/setfan1", true);

    xhttp1.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        setFan(1);
      }
    };
    xhttp0.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        setFan(0);
      }
    };

    if (document.getElementById("button_fan").innerHTML == "Ligar")
      xhttp1.send();
    else
      xhttp0.send();



  }

  function buttonSetNebulizer() {
    var xhttp0 = new XMLHttpRequest();
    xhttp0.open("POST", "/actuators/setnebulizer0", true);
    var xhttp1 = new XMLHttpRequest();
    xhttp1.open("POST", "/actuators/setnebulizer1", true);

    xhttp1.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        setNebulizer(1);
        setFan(1);
      }
    };

    xhttp0.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        setNebulizer(0);
        updateFanState();
      }
    };

    if (document.getElementById("button_nebulizer").innerHTML == "Ligar")
      xhttp1.send();
    else
      xhttp0.send();
  }

  function buttonSetExchanger() {
    var xhttp1 = new XMLHttpRequest();
    xhttp1.open("POST", "/actuators/setexchanger1", true);
    var xhttp0 = new XMLHttpRequest();
    xhttp0.open("POST", "/actuators/setexchanger0", true);
    xhttp1.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        setExchanger(1);
      }
    };
    xhttp0.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        setExchanger(0);
      }
    };
    if (document.getElementById("button_exchanger").innerHTML == "Ligar")
      xhttp1.send();
    else
      xhttp0.send();
  }

  function updateActuatorsParam() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        //console.log("response text: "+this.responseText);
        var array = this.responseText.split(" ");
        //console.log(array[0]);

        //index = |0: nebulizador umidade| |1: nebulizador temperatura|  |2: trocador de água variação| |3: ventilador temperatura|
        document.getElementById("minUmidity").placeholder = array[0];
        document.getElementById("maxTempNeb").placeholder = array[1];
        document.getElementById("deltaTemperature").placeholder = array[2];
        document.getElementById("maxTempFan").placeholder = array[3];
      }
    };
    xhttp.open("GET", "/actuators/parameter/getall", true);
    xhttp.send();
  }

  function updateActuatorsOpMode() { //atualizar o modo de operação dos atuadores
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        var array = this.responseText.split(" ");

        //array index = |0: nebulizador|  |1: trocador de água|   |2: ventilador|
        setOperationMode(1, array[0], "Nebulizer");
        setOperationMode(2, array[1], "Exchanger");
        setOperationMode(3, array[2], "Fan");
      }
    };
    xhttp.open("GET", "/actuators/opmode/getall", true);
    xhttp.send();
  }

  function setNebulizer(state) {

    if (state == true) {
      document.getElementById("button_nebulizer").innerHTML = "Desligar";
      document.getElementById("status_nebulizer").innerHTML = "Ligado";
      document.getElementById("status_nebulizer").style.color = "green";
    } else {
      document.getElementById("button_nebulizer").innerHTML = "Ligar";
      document.getElementById("status_nebulizer").innerHTML = "Desligado";
      document.getElementById("status_nebulizer").style.color = "red";
    }
  }

  function setFan(state) {

    if (state == true) {
      document.getElementById("button_fan").innerHTML = "Desligar";
      document.getElementById("status_fan").innerHTML = "Ligado";
      document.getElementById("status_fan").style.color = "green";
    } else {
      document.getElementById("button_fan").innerHTML = "Ligar";
      document.getElementById("status_fan").innerHTML = "Desligado";
      document.getElementById("status_fan").style.color = "red";
    }
  }

  function setExchanger(state) {

    if (state == true) {
      document.getElementById("status_exchanger").innerHTML = "Ligado";
      document.getElementById("button_exchanger").innerHTML = "Interromper";
      document.getElementById("status_exchanger").style.color = "green";
    } else {
      document.getElementById("status_exchanger").innerHTML = "Desligado";
      document.getElementById("button_exchanger").innerHTML = "Ligar";
      document.getElementById("status_exchanger").style.color = "red";
    }
  }

  function updateFanState() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        if (this.responseText == "Ligado")
          setFan(1);
        else
          setFan(0);
      }
    };
    xhttp.open("GET", "/actuators/getfan", true);
    xhttp.send();
  }

  function updateExchangerState() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        if (this.responseText == "Ligado")
          setExchanger(1);
        else
          setExchanger(0);
      }
    };
    xhttp.open("GET", "/actuators/getexchanger", true);
    xhttp.send();
  }

  function updateNebulizerState() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        if (this.responseText == "Ligado")
          setNebulizer(1);
        else
          setNebulizer(0);
      }
    };
    xhttp.open("GET", "/actuators/getnebulizer", true);
    xhttp.send();
  }

  function updateSensor(sensor_url, sensor_span, unit_measurement, text_error) {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        if (this.responseText == text_error) {
          document.getElementById(sensor_span).innerHTML = "Falha na leitura do sensor!";
          document.getElementById(sensor_span).style.color = "red";
        } else {
          document.getElementById(sensor_span).style.color = "black";
          document.getElementById(sensor_span).innerHTML = this.responseText + unit_measurement;
        }

      }
    };
    xhttp.open("GET", "/sensors/get" + sensor_url, true);
    xhttp.send();
  }

  //taxa de atualização de 5 segundos
  setInterval(function () {
    //atualizando o valor dos sensores
    updateSensor("nippletemp", "nipple_temperature", "°C", "-127.00");
    updateSensor("boxtemp", "box_temperature", "°C", "-127.00");
    updateSensor("climatehumdt", "climate_humidity", "%", "nan");
    updateSensor("climatetemp", "climate_temperature", "°C", "nan");

    //parametro de acionamento
    updateActuatorsParam();

    //status do modo de operação dos atuadores
    updateActuatorsOpMode();
  }, 5000);

  //taxa de atualização de 1 segundo
  setInterval(function () {
    //status dos atuadores
    updateFanState();
    updateExchangerState();
    updateNebulizerState();
  }, 1000);

</script>

</html>)rawliteral";


String getStatusActuator(char n){
  //'n' para nebulizador; 'e' para o trocador de agua; 'f' para o ventilador
  int pin;
  switch (n) {
    case 'n':
      pin=PIN_NEBULIZER;
      break;
    case 'e':
      pin=PIN_NIPPLE;
      break;
    case 'f':
      pin=PIN_FAN;
      break;
  }

  if(digitalRead(pin)==HIGH)
    return "0";
  else
    return "1";
  
}

void setNebulizer(bool state){
  if (state==1){
    digitalWrite(PIN_NEBULIZER, HIGH);
    if (digitalRead(PIN_FAN)==HIGH)
      fan_on_directly=1;
    else{
      fan_on_directly=0;
      digitalWrite(PIN_FAN, HIGH);
    }
  }
  if (state==0){
    digitalWrite(PIN_NEBULIZER, LOW);
    if(fan_on_directly==0)
      digitalWrite(PIN_FAN, LOW);
  }
}

void setExchanger(bool state){
  //Liga trocador de água
	if (state){
    digitalWrite(PIN_NIPPLE, HIGH);
    exchanger_time = millis() + 20000;
	}else{ 
    digitalWrite(PIN_NIPPLE, LOW);
	}
}

void automaticModeNebulizer(){
  if(!operation_mode_nebulizer){
    //Liga Nebulizador
    if(digitalRead(PIN_NEBULIZER) == LOW){
      if((current_climate_humidity < humidity_nebulizer_parameter) && (current_climate_temp > temperature_nebulizer_parameter)){
        setNebulizer(1);
        Serial.println("Nebulizador ligado via modo automático");
      }

    //Desliga Nebulizador
    }else{
      if((current_climate_humidity > (humidity_nebulizer_parameter + 3)) || (current_climate_temp < (temperature_nebulizer_parameter + 0.5)) ){
        setNebulizer(0);
        Serial.println("Nebulizador desligado via modo automático");
      }
    }
  }
}

void automaticModeExchanger(){
  if(!operation_mode_exchanger){
    if( (current_nipple_temp - current_box_temp) > deltat_nipple_parameter){
      Serial.println("Água trocada via modo automático");
      setExchanger(1);
    }
  }
}

void automaticModeFan(){
  if(!operation_mode_fan)  {//Liga Ventilador
    if (digitalRead(PIN_FAN) == LOW){
      if (current_climate_temp>temperature_fan_parameter){
        digitalWrite(PIN_FAN, HIGH);
        Serial.println("Ventilaador ligado via modo automático");
      }
    //Desliga Ventilador
    }else{
      if ((current_climate_temp < (temperature_fan_parameter - 2)) && (digitalRead(PIN_NEBULIZER) == LOW)){
        digitalWrite(PIN_FAN, LOW);
        Serial.println("Ventilador ligado via modo automático");
      }
    }
  }
}

void changeWaterOff(){
  if((digitalRead(PIN_NIPPLE)==HIGH) && (exchanger_time < millis())){
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
	current_nipple_temp = ds18b20.getTempCByIndex(0);
	current_box_temp = ds18b20.getTempCByIndex(1);
	current_climate_temp = dht.readTemperature();
	current_climate_humidity = dht.readHumidity();
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

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());



  // URL para raiz (index)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/html", index_html, processor);
    request->send_P(200, "text/html", index_html);
  });

  //---Rotas para requisições de origem do JavaScript---

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
          if(bool current_value = (p->value().toInt())){
            setNebulizer(current_value);
            request->send(200);
            Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
    }
  });

  //URL para ligar o trocador de agua
  server.on(
    "/actuators/status/set/exchanger", 
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(bool current_value = (p->value().toInt())){
            setExchanger(current_value);
            request->send(200);
            Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304);
          }
        }
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
          if(bool current_value = (p->value().toInt())){
            if (current_value){
              digitalWrite(PIN_FAN,HIGH);
              request->send(200);
              Serial.println("Parâmetro de acionamento do ventilador modificado via interface Web!");
            }else{
              if(digitalRead(PIN_NEBULIZER)==HIGH)
                request->send(409, "text/plain", String("Ventilador não pode ser desligado quando o nebulizador estiver em funcionamento!").c_str());
            }
            
          }else{
            Serial.println("Erro em modificar o parâmetro de acionamento do ventilador via interface Web!");
            request->send(304, "text/plain", String("Erro interface!").c_str());
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
    String response = String(humidity_nebulizer_parameter) +" " + String(temperature_nebulizer_parameter) + " " + String(deltat_nipple_parameter) + " " + String(temperature_fan_parameter);
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
          if(bool current_value = (p->value().toInt())){
            operation_mode_nebulizer = current_value;
            request->send(200);
            Serial.println("Nebulizador mudou seu modo de operação para Manual!");
          }else{
            Serial.println("Erro em mudar o modo de operação do Nebulizador!");
            request->send(304);
          }
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
          if(bool current_value = (p->value().toInt())){
            operation_mode_exchanger = current_value;
            request->send(200);
            Serial.println("Trocador de água mudou seu modo de operação para Manual!");
          }else{
            Serial.println("Erro em mudar o modo de operação do Trocador de água!");
            request->send(304);
          }
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
          if(bool current_value = (p->value().toInt())){
            operation_mode_fan = current_value;
            request->send(200);
            Serial.println("Ventilador mudou seu modo de operação para Manual!");
          }else{
            Serial.println("Erro em mudar o modo de operação do Ventilador!");
            request->send(304);
          }
        }
    }
  });

  //URL para modificar o parâmetro de temperatura do ventilador
  server.on(
    "/actuators/parameter/set/fan/temp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          
          if(float current_value = (p->value().toFloat())){
            temperature_fan_parameter = current_value;
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
    "/actuators/parameter/set/exchanger/deltatemp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name() == "value"){
          if(float current_value = (p->value().toFloat())){
            deltat_nipple_parameter = current_value;
            request->send(200);
            Serial.println("Parâmetro de acionamento do trocador modificado via interface Web!");
          }else{
            request->send(304);
            Serial.println("Erro em modificar o parâmetro de acionamento do trocador via interface Web!");
          }
        }
    }
  });

  //URL para modificar o parâmetro de tempo de um novo acionamento automático do trocador de água
  server.on(
    "/actuators/parameter/set/exchanger/time",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(int current_value = (p->value().toInt())){
            time_nipple_parameter = current_value * 3600000;
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
    "/actuators/parameter/set/nebulizer/humidity",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        
        if(p->name()=="value"){
          if(float current_value = (p->value().toFloat())){
            humidity_nebulizer_parameter = current_value;
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
    "/actuators/parameter/nebulizer/temp",
    HTTP_POST,
    [](AsyncWebServerRequest * request){
    int params = request->params();
    for (int i = 0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        if(p->name()=="value"){
          if(float current_value = (p->value().toFloat())){
            temperature_nebulizer_parameter = current_value;
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
    
    updateSensorsValue();
    printSensorsValue();
    
    automaticModeExchanger();
    automaticModeFan();
    automaticModeNebulizer();

    update_time = millis()+5000;
  }

  changeWaterOff();
	
}
