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
  else {
    LoRaData = event.data;
    document.getElementById('LoRaData').innerHTML = LoRaData;
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