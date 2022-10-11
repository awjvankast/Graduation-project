var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event) {
  console.log('Connection opened');
}

function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
  var state;
  var LoRaData;
  var timeData;
  var GPSData;
  var AltData;
  var SD;

  // Get the current time and display it on the HTML page every time any value changes
  var today = new Date();
  // today.getFullYear()+'-'+(today.getMonth()+1)+'-'+today.getDate() + '\n' +
  var now =  today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
  // document.getElementById('now').innerHTML = now;
  // Some logging on the console for debugging purposes
  console.log('Received a notification from ${event.origin}');
  console.log(event);

  if (event.data == "1"){ // Led switch on
    state = "ON";
    document.getElementById('state').innerHTML = state;
  }
  else if(event.data == "0"){ // Led switch off
    state = "OFF";
    document.getElementById('state').innerHTML = state;
  }
  else if(event.data.charAt(0)=="A") { // Received messag from Tx, Alex
    LoRaData = event.data;
    document.getElementById('LoRaData_Eva').innerHTML = LoRaData;
    document.getElementById('now_Eva').innerHTML = now;
  }
  else if(event.data.charAt(0)=="B") { // Received messag from Tx, Brooke
    LoRaData = event.data;
    document.getElementById('LoRaData_Brooke').innerHTML = LoRaData;
    document.getElementById('now_Brooke').innerHTML = now;
  }
  else if(event.data.charAt(0)=="C") { // Received messag from Tx, Caldwell
    LoRaData = event.data;
    document.getElementById('LoRaData_Caldwell').innerHTML = LoRaData;
    document.getElementById('now_Caldwell').innerHTML = now;
  }
  else if(event.data.charAt(0)=="D") { // Received messag from Tx, DiGiulian
    LoRaData = event.data;
    document.getElementById('LoRaData_DiGiulian').innerHTML = LoRaData;
    document.getElementById('now_DiGiulian').innerHTML = now;
  }
  else if(event.data.charAt(0)=="E") { // Received messag from Tx, Eva
    LoRaData = event.data;
    document.getElementById('LoRaData_Eva').innerHTML = LoRaData;
    document.getElementById('now_Eva').innerHTML = now;
  }
  else if(event.data.charAt(0)=="F") { // Received messag from Tx, Famke
    LoRaData = event.data;
    document.getElementById('LoRaData_Famke').innerHTML = LoRaData;
    document.getElementById('now_Famke').innerHTML = now;
  }
  else if(event.data.charAt(0)=="G") { // Received messag from Tx, Gruber
    LoRaData = event.data;
    document.getElementById('LoRaData_Gruber').innerHTML = LoRaData;
    document.getElementById('now_Gruber').innerHTML = now;
  }
  else if (event.data.charAt(0)=="Z"){ // Display the GPS data
    GPSData = event.data;
    if (GPSData !== null) {
    document.getElementById('GPSData').innerHTML = GPSData;
    }
  }
  else if (event.data.charAt(0)=="Q"){ // display the altimeter value
    AltData = event.data;
    if (GPSData !== null) {
    document.getElementById('AltData').innerHTML = AltData.substr(2);
    }
  }
  else if (event.data == "SDP"){ // The SD card is Present
    SD = "Present";
    document.getElementById('SD').innerHTML = SD;
  }
  else if (event.data == "SDNP"){ // The SD card is not Present
    SD = "Not present";
    document.getElementById('SD').innerHTML = SD;
  }
  
}

window.addEventListener('load', onLoad);
function onLoad(event) {
  initWebSocket();
  initButton();
}

function initButton() {
  document.getElementById('button').addEventListener('click', toggle);
}
function toggle(){
  websocket.send('toggle');
}