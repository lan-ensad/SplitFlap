#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <AccelStepper.h>

//****************************************
//**************** A4988 *****************
// MS1    MS2   MS3   Microstep Resolution
//****************************************
// Low  | Low  | Low  |  Full step
// High | Low  | Low  |  Half step
// Low  | High | Low  |	 Quarter step
// High |	High | Low  |  Eighth step
// High |	High | High |	 Sixteenth step
//****************************************

//PCB MAPPING
#define EN 21
#define MS1 7
#define MS2 6
#define MS3 5
#define STEP 3
#define DIR 2
#define MAXSPEED 2500

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AccelStepper stepper(AccelStepper::DRIVER, STEP, DIR);

const char *ssid = "YOU_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";

bool enableState, ms1State, ms2State, ms3State = 0;

int stp = 150;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>ESP Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html{
        font-family: Arial, Helvetica, sans-serif;
        text-align: center;
      }
      h1{
        font-size: 1.8rem;
        color: white;
      }
      h2{
        font-size: 1.5rem;
        font-weight: bold;
        color: #143642;
      }
      .left{
        float: left;
        margin-left: 10px;
      }
      .topnav{
        overflow: hidden;
        background-color: #143642;
      }
      body{
        margin: 0;
      }
      .content{
        padding: 30px;
        max-width: 600px;
        margin: 0 auto;
      }
      .card{
        display:block;
        background-color: #F8F7F9;
        box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
        padding-top:10px;
        padding-bottom:20px;
        margin-top: 15px;
        margin-bottom: 15px;
      }
      .button{
        padding: 15px 50px;
        font-size: 24px;
        text-align: center;
        outline: none;
        color: #fff;
        background-color: #0f8b8d;
        border: none;
        border-radius: 5px;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }
      .button:active{
        background-color: #0f8b8d;
        box-shadow: 2 2px #CDCDCD;
        transform: translateY(2px);
      }
      .state{
        font-size: 1.5rem;
        color:#8c8c8c;
        font-weight: bold;
      }
      .value{
        font-size: 1.5rem;
        color:#8c8c8c;
        font-weight: bold;
      }
    </style>
    <title>ESP Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=0.7">
    <link rel="icon" href="data:,">
  </head>
  <body>
    <div class="topnav">
      <h1>ESP WebSocket Server</h1>
    </div>
    <div class="content">
      <div class="card">
        <h2>Enable</h2>
        <p class="state">state : <span id="state">%STATE%</span></p>
        <p><button id="button" class="button">EN</button></p>
      </div>
      <div class="card">
          <h2>Step value</h2>
          <p class="value">value : <span id="value">%VALUE%</span></p>
          <input type="range" min="1" max="200" value="25" id="slider" class="sliders"></input>
      </div>
      <div class="card" style="height: 200px;">
          <h2>Micro Steps</h2>
          <div class="left">
            <p class="state">MS1 : <span id="Ms1">%MS1%</span></p>
            <p><button id="MS1" class="button">MS1</button></p>
          </div>
          <div class="left">
            <p class="state">MS2 : <span id="Ms2">%MS2%</span></p>
            <p><button id="MS2" class="button">MS2</button></p>
          </div>
          <div class="left">
            <p class="state">MS3 : <span id="Ms3">%MS3%</span></p>
            <p><button id="MS3" class="button">MS3</button></p>
          </div>
      </div>
    </div>
    <script>
      var gateway = `ws://${window.location.hostname}/ws`;
      var websocket, count;
      var state = "OFF", Ms1 = "OFF", Ms2="OFF", Ms3 = "OFF";
      window.addEventListener('load', onLoad);
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
        var msg = event.data.split(';');
        console.log(msg);
        if (msg[0]=="EN"){
            if(msg[1]=="1"){
                state = "ON";
            }else{
                state = "OFF";
            }
        }else if (msg[0]=="MS1"){
            if(msg[1]=="1"){
                Ms1 = "ON";
            }else{
                Ms1 = "OFF";
            }
        }else if (msg[0]=="MS2"){
            if(msg[1]=="1"){
                Ms2 = "ON";
            }else{
                Ms2 = "OFF";
            }
        }else if (msg[0]=="MS3"){
            if(msg[1]=="1"){
                Ms3 = "ON";
            }else{
                Ms3 = "OFF";
            }
        }
        document.getElementById('state').innerHTML = state;
        document.getElementById('Ms1').innerHTML = Ms1;
        document.getElementById('Ms2').innerHTML = Ms2;
        document.getElementById('Ms3').innerHTML = Ms3;
      }
      function onLoad(event) {
        initWebSocket();
        initButton();
        document.getElementById('state').innerHTML = state;
        document.getElementById('Ms1').innerHTML = Ms1;
        document.getElementById('Ms2').innerHTML = Ms2;
        document.getElementById('Ms3').innerHTML = Ms3;
        value();
      }
      function initButton() {
        document.getElementById('button').addEventListener('click', button);
        document.getElementById('MS1').addEventListener('click', ms1);
        document.getElementById('MS2').addEventListener('click', ms2);
        document.getElementById('MS3').addEventListener('click', ms3);
        document.getElementById('slider').addEventListener('input', value);
      }
      function button(){
        websocket.send('toggle');
      }
      function ms1(){
        websocket.send('ms1');
      }
      function ms2(){
        websocket.send('ms2');
      }
      function ms3(){
        websocket.send('ms3');
      }
      function value(){
        var value = document.getElementById("slider").value;
        document.getElementById("value").innerHTML = value;
        websocket.send("value;"+value);
      }
    </script>
  </body>
</html>
)rawliteral";

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    // Parse the websocket.send(data)
    data[len] = 0;
    String msg;
    for (int i = 0; i < len; i++) {
      msg += char(data[i]);
    }
    // Serial.println(msg);
    if (msg == "toggle") {
      Serial.print("EN state : ");
      Serial.println(enableState);
      enableState = !enableState;
      ws.textAll("EN;" + String(enableState)); //response to webUI
    } else if (msg == "ms1") {
      Serial.print("MS1 state : ");
      Serial.println(ms1State);
      ms1State = !ms1State;
      ws.textAll("MS1;" + String(ms1State));  //response to webUI
    } else if (msg == "ms2") {
      Serial.print("MS2 state : ");
      Serial.println(ms2State);
      ms2State = !ms2State;
      ws.textAll("MS2;" + String(ms2State));  //response to webUI
    } else if (msg == "ms3") {
      Serial.print("MS3 state : ");
      Serial.println(ms3State);
      ms3State = !ms3State;
      ws.textAll("MS3;" + String(ms3State));  //response to webUI
    } else if (msg.charAt(5) == ';') {
      msg.remove(0, 6);
      stp = msg.toInt();
      Serial.print("step = ");
      Serial.println(stp);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

//Response for html sending
//  Look at setup()
String processor(const String &var) {
  Serial.println(var);
  if (var == "STATE") {
    if (enableState) {
      return "ON";
    } else {
      return "OFF";
    }
  } else if (var == "MS1") {
    if (ms1State) {
      return "ON";
    } else {
      return "OFF";
    }
  } else if (var == "MS2") {
    if (ms2State) {
      return "ON";
    } else {
      return "OFF";
    }
  } else if (var == "MS3") {
    if (ms3State) {
      return "ON";
    } else {
      return "OFF";
    }
  }
  return String();
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  AsyncElegantOTA.begin(&server);
  server.begin();

  ws.textAll("MS1;" + String(ms1State));
  ws.textAll("MS2;" + String(ms2State));
  ws.textAll("MS3;" + String(ms3State));

  pinMode(EN, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);

  digitalWrite(EN, 1);  //0=ON ; 1=OFF
  digitalWrite(MS1, 0);
  digitalWrite(MS2, 0);
  digitalWrite(MS2, 0);

  stepper.setMaxSpeed(MAXSPEED);
  stepper.setAcceleration(MAXSPEED);
  stepper.setCurrentPosition(0);
  stepper.moveTo(stp);
}

void loop() {
  ws.cleanupClients();

  digitalWrite(EN, enableState);
  digitalWrite(MS1, ms1State);
  digitalWrite(MS2, ms2State);
  digitalWrite(MS3, ms3State);

  if (stepper.distanceToGo() == 0) {
    stepper.moveTo(stepper.currentPosition() + stp);
    // delay(2000);
  }
  stepper.run();
}