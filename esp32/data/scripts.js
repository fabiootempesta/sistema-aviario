
updateSensors();
updateActuatorsState();
updateActuatorsOpMode();
updateActuatorsParam();

function buttonSetOpModeActuator(id_checkbox, actuator_name) {
  let opmode = 0;
  console.log(document.getElementById(id_checkbox).checked);
  let value = document.getElementById(id_checkbox).checked;
  if (value)
    opmode = 1;
  
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
  if (value == "1") {
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

function updateActuatorsParam() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var array = this.responseText.split(" ");

      //index = |0: ligar nebulizador umidade| |1: desligar nebulizador umidade| |2: ligar nebulizador temperatura| 
      //|3: desligar nebulizador temperatura| |4: ligar trocador de água variação temperatura| |5: desligar trocador de água tempo|
      //|6: ligar ventilador temperatura| |7: desligar ventilador temperatura|
      document.getElementById("nebulizer_parameter_on_humidity").placeholder = array[0].replace(".", ",");
      document.getElementById("nebulizer_parameter_off_humidity").placeholder = array[1].replace(".", ",");
      document.getElementById("nebulizer_parameter_on_temperature").placeholder = array[2].replace(".", ",");
      document.getElementById("nebulizer_parameter_off_temperature").placeholder = array[3].replace(".", ",");
      document.getElementById("exchanger_parameter_on_temperature").placeholder = array[4].replace(".", ",");
      document.getElementById("exchanger_parameter_off_time").placeholder = array[5].replace(".", ",");
      document.getElementById("exchanger_parameter_time_act").placeholder = array[5].replace(".", ",");
      document.getElementById("fan_parameter_on_temperature").placeholder = array[6].replace(".", ",");
      document.getElementById("fan_parameter_off_temperature").placeholder = array[7].replace(".", ",");
      
    }
  };
  xhttp.open("GET", "/actuators/parameter/getall", true);
  xhttp.send();
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
  var button = document.getElementById("button_act_" + actuator_name);
  if (state == true) {
    
    if (actuator_name == "exchanger"){
      button.innerHTML = "Interromper a troca";
      document.getElementById("exchanger_off").classList.add('d-none');
    }
    else
      button.innerHTML = "Desligar";
    
    button.value = "0";
    document.getElementById("status_" + actuator_name + "1").innerHTML = "Ligado";
    document.getElementById("status_" + actuator_name + "2").innerHTML = "Ligado";
    document.getElementById("status_" + actuator_name + "1").style.color = "green";
    document.getElementById("status_" + actuator_name + "2").style.color = "green";
  } else {
    if (actuator_name == "exchanger"){
      document.getElementById("exchanger_off").classList.remove('d-none');
      button.innerHTML = "Realizar a troca";
    }
    else
      button.innerHTML = "Ligar";

    button.value = "1";
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

function buttonSetParameter(url, inputID) {
  var xhttp = new XMLHttpRequest();
  xhttp.open("POST", "/actuators/parameter/" + url, true);
  xhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4) {
      if (this.status == 200) {
        alert("Parâmetro reconfigurado!");
      }
      if (this.status == 304) {
        alert("Erro em Modificar o parâmetro de acionamento do equipamento!");
      }
    }

  };
  xhttp.send("value=" + document.getElementById(inputID).value);
}

function updateActuatorsOpMode() { //atualizar o modo de operação dos atuadores
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var array = this.responseText.split(" ");

      //array index = |0: nebulizador|  |1: trocador de água|   |2: ventilador|
      
      updateOpMode(array[0], "nebulizer", "op_mode_nebulizer");

      
      updateOpMode(array[1], "exchanger", "op_mode_exchanger");

      
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

function buttonSetStatusActuator(button_value, actuator_name) {
  var xhttp = new XMLHttpRequest();

  xhttp.open("POST", "/actuators/status/set/" + actuator_name, true);
  xhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      updateActuatorState(button_value, actuator_name);
    }
  };
  var requestSend = "value=" + button_value;
  if (actuator_name == "exchanger")
    requestSend += "&time=" + document.getElementById("exchanger_parameter_time_act").value;

  xhttp.send(requestSend);
}

//taxa de atualização de 5 segundos
setInterval(function () {
  //atualizando o valor dos sensores
  /*updateSensors();
  updateActuatorsState();
  updateActuatorsOpMode();
  updateActuatorsParam();*/
}, 5000);