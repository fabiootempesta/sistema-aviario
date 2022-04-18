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

function buttonSetOpModeActuator(id_checkbox, actuator_name) {
  let opmode = 0;
  let value = document.getElementById(id_checkbox).checked;
  if (value){
    opmode = 1;
  }
  document.getElementById(id_checkbox).checked = !value;
  var xhttp = new XMLHttpRequest();
  xhttp.open("POST", "/actuators/opmode/set/" + actuator_name, true);
  xhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      updateOpMode(opmode, actuator_name, id_checkbox);
    }
  };
  xhttp.send("value=" + opmode);
}

function updateOpMode(value, actuator_name, id_checkbox) {
  var automatic_div = document.getElementById("automatic_" + actuator_name);
  var manual_div = document.getElementById("manual_" + actuator_name);
  if (value) {
    document.getElementById(id_checkbox).checked = true;
    if (manual_div.classList.contains('d-none')) {
      manual_div.classList.remove('d-none');
      automatic_div.classList.add('d-none');
    }
    document.getElementById("opmode_" + actuator_name).innerHTML = "Manual";
  } else {
    document.getElementById(id_checkbox).checked = false;
    if(automatic_div.classList.contains('d-none')) {
      automatic_div.classList.remove('d-none');
      manual_div.classList.add('d-none');
    }
    document.getElementById("opmode_" + actuator_name).innerHTML = "Automático";
  }
}

function dNoneAllItemsClass(class_name) {
  var sensors = document.getElementsByClassName(class_name);
  for (var i = 0; i < sensors.length; i++) {
    sensors[i].classList.add('d-none');
  }
}

function dFlexAllItemsClass(class_name) {
  var sensors = document.getElementsByClassName(class_name);
  for (var i = 0; i < sensors.length; i++) {
    sensors[i].classList.remove('d-none');
  }
}

function expandDiv(div_id_expand, div_id_none) {
  var div = (document.getElementById(div_id_none));
  div.classList.add('d-none');
  dNoneAllItemsClass("item-mobile");
  var div_expand = document.getElementById(div_id_expand);
  var buttonRetract = document.getElementById(div_id_expand + "_button");
  buttonRetract.classList.remove('d-none');
  div_expand.classList.remove('d-none');
  div_expand.classList.remove('border-4');
  div_expand.classList.add('border-0');
}

function retractDiv(div_id_retract, div_id_block) {
  var div = (document.getElementById(div_id_block));
  div.classList.remove('d-none');
  dFlexAllItemsClass("item-mobile");
  var div_retract = document.getElementById(div_id_retract);
  var buttonRetract = document.getElementById(div_id_retract + "_button");
  buttonRetract.classList.add('d-none');
  div_retract.classList.add('d-none');
  div_retract.classList.add('border-4');
  div_retract.classList.remove('border-0');
}



function showHideDivParam(radioCheck, actuator_name) {
  var divParam = document.getElementById("card_param_" + actuator_name);
  if (radioCheck) {
    divParam.classList.remove('hover');
    document.getElementById("card_front_" + actuator_name).classList.remove('hidden-face');
    document.getElementById("card_back_" + actuator_name).classList.add('hidden-face');
  } else {
    divParam.classList.add('hover');
    document.getElementById("card_back_" + actuator_name).classList.remove('hidden-face');
    document.getElementById("card_front_" + actuator_name).classList.add('hidden-face');
  }
}

function updateActuatorState(state, actuator_name) {

  if (state == true) {
    
    if (actuator_name == "exchanger")
      document.getElementById("button_act_exchanger").innerHTML = "Interromper a troca";
    else
      document.getElementById("button_act_" + actuator_name).innerHTML = "Desligar";
    
    document.getElementById("status_" + actuator_name + "1").innerHTML = "Ligado";
    document.getElementById("status_" + actuator_name + "2").innerHTML = "Ligado";
    document.getElementById("status_" + actuator_name + "1").style.color = "green";
    document.getElementById("status_" + actuator_name + "2").style.color = "green";
  } else {
    if (actuator_name == "exchanger")
      document.getElementById("button_act_exchanger").innerHTML = "Realizar a troca";
    else
      document.getElementById("button_act_" + actuator_name).innerHTML = "Ligar";

    document.getElementById("status_" + actuator_name + "1").innerHTML = "Desligado";
    document.getElementById("status_" + actuator_name + "2").innerHTML = "Desligado";
    document.getElementById("status_" + actuator_name + "1").style.color = "red";
    document.getElementById("status_" + actuator_name + "2").style.color = "red";
  }
}

function updateActuatorsState() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var array = this.responseText.split(" ");
      //array index = |0: trocador|  |1: ventilador|   |2: nebulizador|
      updateActuatorState(array[0], "exchanger");
      updateActuatorState(array[1], "fan");
      updateActuatorState(array[2], "nebulizer");


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
      updateOpMode(array[0], "nebulizer", "op_mode_nebulizer");

      document.getElementById("opmode_exchanger").innerHTML = array[1];
      updateOpMode(array[1], "exchanger", "op_mode_exchanger");

      document.getElementById("opmode_fan").innerHTML = array[2];
      updateOpMode(array[2], "fan", "op_mode_fan");
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

      updateSensor(array[3], "nipple_temperature1", "°C", "-127.00");
      updateSensor(array[3], "nipple_temperature2", "°C", "-127.00");

      updateSensor(array[2], "box_temperature1", "°C", "-127.00");
      updateSensor(array[2], "box_temperature2", "°C", "-127.00");

      updateSensor(array[1], "climate_humidity1", "%", "nan");
      updateSensor(array[1], "climate_humidity2", "%", "nan");

      updateSensor(array[0], "climate_temperature1", "°C", "nan");
      updateSensor(array[0], "climate_temperature2", "°C", "nan");
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