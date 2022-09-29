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

  var today = new Date();
  // today.getFullYear()+'-'+(today.getMonth()+1)+'-'+today.getDate() + '\n' +
  var now =  today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
  document.getElementById('now').innerHTML = now;

  console.log('Received a notification from ${event.origin}');
  console.log(event);

  if (event.data == "1"){
    state = "ON";
    document.getElementById('state').innerHTML = state;
  }
  else if(event.data == "0"){
    state = "OFF";
    document.getElementById('state').innerHTML = state;
  }
  else if(event.data.charAt(0)=="I") {
    LoRaData = event.data;
    console.log('Correctly identified first characte');
    document.getElementById('LoRaData').innerHTML = LoRaData;
  }
  else if (event.data.charAt(0)=="G"){
    GPSData = event.data;
    document.getElementById('GPSData').innerHTML = GPSData;
  }
  else if (event.data.charAt(0)=="A"){
    AltData = event.data;
    document.getElementById('AltData').innerHTML = AltData;
  }
  else if (event.data == "SDP"){
    SD = "Present";
    document.getElementById('SD').innerHTML = SD;
  }
  else if (event.data == "SDNP"){
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