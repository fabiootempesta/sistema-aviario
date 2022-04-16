//cv = current value (valor atual)
var cv_climate_temp;
var cv_climate_humidity;
var cv_nipple_temp;
var cv_box_temp;
var cv_actuator_status_nebulizer;
var cv_actuator_status_exchanger;
var cv_actuator_status_fan;
var cv_actuator_opmode_nebulizer_humidity_on;
var cv_actuator_opmode_nebulizer_humidity_off;
var cv_actuator_opmode_nebulizer_temp_on;
var cv_actuator_opmode_nebulizer_temp_off;
var cv_actuator_opmode_exchanger_time;
var cv_actuator_opmode_exchanger_deltat;
var cv_actuator_opmode_fan_temp_on;
var cv_actuator_opmode_fan_temp_off;

var cv_card_nebulizer = 1;
var cv_card_exchanger = 1;
var cv_card_fan = 1;



function buttonSetOpModeActuator(checkboxId, actuator_name) {
  var checkbox = document.getElementById(checkboxId);
  var automatic_div = document.getElementById("automatic_" + actuator_name);
  var manual_div = document.getElementById("manual_" + actuator_name);

  if(checkbox.checked){
    automatic_div.style.setProperty('display', 'none', 'important');
    manual_div.style.setProperty('display', 'flex', 'important');
  }else{
    automatic_div.style.setProperty('display', 'flex', 'important');
    manual_div.style.setProperty('display', 'none', 'important');
  }
}

function ShowHideDivParam(radioCheck, actuator_name) {
  var divParam = document.getElementById("card_param_" + actuator_name);
  if (radioCheck) {
    divParam.classList.remove('hover');
    document.getElementById("card_front_" + actuator_name).classList.remove('hidden-face');
    document.getElementById("card_back_" + actuator_name).classList.add('hidden-face');
  }else{
    divParam.classList.add('hover');
    document.getElementById("card_back_" + actuator_name).classList.remove('hidden-face');
    document.getElementById("card_front_" + actuator_name).classList.add('hidden-face');
  }
}



function updateActuatorsState() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var array = this.responseText.split(" ");
      //array index = |0: trocador|  |1: ventilador|   |2: nebulizador|
      cv_actuator_status_exchanger = array[0];
      cv_actuator_status_fan = array[1];
      cv_actuator_status_nebulizer = array[2];
      document.getElementById("status_exchanger").innerHTML = cv_actuator_status_exchanger;
      document.getElementById("status_fan").innerHTML = cv_actuator_status_fan;
      document.getElementById("status_nebulizer").innerHTML = cv_actuator_status_nebulizer;
    }
  };
  xhttp.open("GET", "/actuators/status/getall", true);
  xhttp.send();
}
function updateActuatorsOpMode() { //atualizar o modo de operação dos atuadores
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var array = this.responseText.split(" ");

      //array index = |0: nebulizador|  |1: trocador de água|   |2: ventilador|
      document.getElementById("opmode_nebulizer").innerHTML = array[0];
      document.getElementById("opmode_exchanger").innerHTML = array[1];
      document.getElementById("opmode_fan").innerHTML = array[2];
      
    }
  };
  xhttp.open("GET", "/actuators/opmode/getall", true);
  xhttp.send();
}
function updateSensors() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var array = this.responseText.split(" ");

      //array index = |0: temperatura ambiente| |1: umidade ambiente relativa|  |2: temperatura da caixa| |3: temperatura do bebedouro|

      updateSensor(array[3], "nipple_temperature", "°C", "-127.00");
      updateSensor(array[2], "box_temperature", "°C", "-127.00");
      updateSensor(array[1], "climate_humidity", "%", "nan");
      updateSensor(array[0], "climate_temperature", "°C", "nan");


    }
  };
  xhttp.open("GET", "/sensors/getall", true);
  xhttp.send();
}
function updateSensor(value, sensor_span, unit_measurement, text_error) {
  if (value == text_error) {
    document.getElementById(sensor_span).innerHTML = "Falha na leitura do sensor!";
    document.getElementById(sensor_span).style.color = "red";
  } else {
    document.getElementById(sensor_span).style.color = "black";
    document.getElementById(sensor_span).innerHTML = value + unit_measurement;
  }
}

//taxa de atualização de 5 segundos
setInterval(function () {
  //atualizando o valor dos sensores
  updateSensors();
  updateActuatorsState();
  updateActuatorsOpMode();
}, 5000);