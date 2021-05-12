#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>

String CommOut=""; String CommIn=""; String lastComm = "";
int StepSpeed = 300; int lastSpeed = 300;
int SMov[32]={1440,1440,1440,0,1440,1440,1440,0,0,0,0,0,1440,1440,1440,0,1440,1440,1440,0,0,0,0,0,1440,1440,1440,0,1440,1440,1440,0};
int SAdj[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// (TRIM Section) set this to trim the leg position
int STrim[32]={0,84,15,-22,0,-12,-74,-82,0,0,0,0,0,0,-3,-62,0,41,59,69,0,0,0,0,0,-50,51,28,0,-47,45,42};

int StaBlink=0; int IntBlink=0;
int ClawPos=1500;

//SSID and Password to your ESP Access Point
const char* ssid = "HexapodWifi";
const char* password = "12345678";

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>ESP8266 Spider Hexapod</title><style>"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
#JD {text-align: center;}#JD {text-align: center;font-family: "Lucida Sans Unicode", "Lucida Grande", sans-serif;font-size: 24px;}.foot {text-align: center;font-family: "Comic Sans MS", cursive;font-size: 9px;color: #F00;}
.button {border: none;color: white;padding: 20px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;margin: 4px 2px;cursor: pointer;border-radius: 12px;width: 100%;}.red {background-color: #F00;}.green {background-color: #090;}.yellow {background-color:#F90;}.blue {background-color:#03C;}</style>
<script>var websock;function start() {websock = new WebSocket('ws://' + window.location.hostname + ':81/');websock.onopen = function(evt) { console.log('websock open'); };websock.onclose = function(evt) { console.log('websock close'); };websock.onerror = function(evt) { console.log(evt); }; 
websock.onmessage = function(evt) {console.log(evt);var e = document.getElementById('ledstatus');if (evt.data === 'ledon') { e.style.color = 'red';}else if (evt.data === 'ledoff') {e.style.color = 'black';} else {console.log('unknown event');}};} function buttonclick(e) {websock.send(e.id);}</script>
</head><body onload="javascript:start();">&nbsp;<table width="100%" border="1"><tr><td bgcolor="#FFFF33" id="JD">Kill All Humans</td></tr></table>
<table width="100" height="249" border="0" align="center">
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 20" type="button" onclick="buttonclick(this);" class="button red">Slide_Left</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 1 1" type="button" onclick="buttonclick(this);" class="button green">Forward</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 21"  type="button" onclick="buttonclick(this);" class="button red">Slide_Right</button> </label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 3 1" type="button" onclick="buttonclick(this);" class="button green">Turn_Left</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 1"  type="button" onclick="buttonclick(this);" class="button red">Stop</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 4 1"  type="button" onclick="buttonclick(this);" class="button green">Turn_Right</button></label></form></td></tr>
<tr><td>&nbsp;</td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 1" type="button" onclick="buttonclick(this);" class="button green">Backward</button></label></form></td><td>&nbsp;</td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 5 3" type="button" onclick="buttonclick(this);" class="button yellow">Cannon </button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 8 5"  type="button" onclick="buttonclick(this);" class="button blue">HeadUp</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 6 3"  type="button" onclick="buttonclick(this);" class="button yellow">Hello</button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 16" type="button" onclick="buttonclick(this);" class="button blue">Twist_Left</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 9 5"  type="button" onclick="buttonclick(this);" class="button blue">HeadDown</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 17"  type="button" onclick="buttonclick(this);" class="button blue">Twist_Right</button> </label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 11 5" type="button" onclick="buttonclick(this);" class="button blue">Body_left</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 13"  type="button" onclick="buttonclick(this);" class="button blue">BodyUp</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 10 5"  type="button" onclick="buttonclick(this);" class="button blue">Body_right</button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 12" type="button" onclick="buttonclick(this);" class="button yellow">Service</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 14"  type="button" onclick="buttonclick(this);" class="button blue">BodyDown</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 15"  type="button" onclick="buttonclick(this);" class="button yellow">Reset_Pose</button> </label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 0" type="button" onclick="buttonclick(this);" class="button yellow">Walk</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="s 1"  type="button" onclick="buttonclick(this);" class="button blue">Dance</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 7 1"  type="button" onclick="buttonclick(this);" class="button yellow">Run</button> </label></form></td></tr>
</table><p class="foot">A robot may not injure a human being or, through inaction, allow a human being to come to harm.<br>A robot must obey the orders given it by human beings except where such orders would conflict with the First Law.<br>A robot must protect its own existence as long as such protection does not conflict with the First or Second Laws.</p></body></html>
)rawliteral";


WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); 
  Serial.begin(9600);
  Serial.println (">> Setup");
  for(uint8_t t = 4;t > 0;t--) {
    Serial.flush();
    delay(1000);
  }
  delay(1000);
  WiFi.mode(WIFI_AP);           //Only Access point
  WiFi.softAP(ssid, password);  //Start HOTspot removing password will disable security
 
  IPAddress myIP = WiFi.softAPIP(); //Get IP address
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");   
  Serial.println(myIP);

    server.on("/", [](){
    server.send(200, "text/html", INDEX_HTML);
    });
    
    server.begin();
    
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

}

void loop() {
   if (Serial.available() > 0){
      char c[] = {(char)Serial.read()};
      webSocket.broadcastTXT(c, sizeof(c));
   }
    
   IntBlink +=1;
   if (IntBlink >=5000){
      if (StaBlink ==0){
        digitalWrite(LED_BUILTIN, HIGH); 
        StaBlink=1;
      } else {
        digitalWrite(LED_BUILTIN, LOW); 
        StaBlink=0;
      }
    IntBlink =0;
   }
   webSocket.loop();
   server.handleClient();
   if (CommOut == "w 0 1") Move_STP(); //~~~~~~~~ Stop
   if (CommOut == "w 1 1") Move_FWD(); //~~~~~~~~ Forward
   if (CommOut == "w 2 1") Move_BWD(); //~~~~~~~~ backward
   if (CommOut == "w 3 1") Turn_LFT(); //~~~~~~~~ move left
   if (CommOut == "w 4 1") Turn_RGT(); //~~~~~~~~ move right
   if (CommOut == "w 20") Slide_LFT();//~~~~~~~~ turn left
   if (CommOut == "w 21") Slide_RGT();//~~~~~~~~ turn Right


   //~~~~~~~~ dance
   if (CommOut == "s 1"){
     lastSpeed = StepSpeed;StepSpeed = 500;
     Dance();StepSpeed = lastSpeed;CommOut=lastComm;
   }

   //~~~~~~~~ shoot cannon
  /* 
     if (CommOut == "w 5 3"){
     lastSpeed = StepSpeed;StepSpeed = 300;
     Shoot_cannon();StepSpeed = lastSpeed;CommOut=lastComm;
   }
   */
   //~~~~~~~~ shake hand
   if (CommOut == "w 5 3"){
     lastSpeed = StepSpeed;StepSpeed = 300;
     Move_SHK();StepSpeed = lastSpeed;CommOut=lastComm;
   }
  
   //~~~~~~~~ waving
   if (CommOut == "w 6 3"){
     lastSpeed = StepSpeed;StepSpeed = 300;
     Move_WAV();StepSpeed = lastSpeed;CommOut=lastComm;
   }
   //~~~~~~~~ pos initial
   if (CommOut == "w 15") {
    Pos_INT();CommOut=lastComm;
   }
   //~~~~~~~~ pos service
   if (CommOut == "w 12") {
    Pos_SRV();CommOut=lastComm;
   }
   //~~~~~~~~ adjust body left
   if (CommOut == "w 11 5") {
    Adj_LF();CommOut=lastComm;
   }
   //~~~~~~~~ adjust body right
   if (CommOut == "w 10 5") {
    Adj_RG();CommOut=lastComm;
   }
   //~~~~~~~~ adjust head up
   if (CommOut == "w 8 5") {
    Adj_HU();CommOut=lastComm;
   }
   //~~~~~~~~ adjust head down
   if (CommOut == "w 9 5") {
    Adj_HD();CommOut=lastComm;
   }
   //~~~~~~~~ adjust body higher
   if (CommOut == "w 13") {
    Adj_HG();CommOut=lastComm;
   }
   //~~~~~~~~ adjust body lower
   if (CommOut == "w 14") {
    Adj_LW();CommOut=lastComm;
   }
   //~~~~~~~~ adjust twist left
   if (CommOut == "w 16") {
    Adj_TL();CommOut=lastComm;
   }
   //~~~~~~~~ adjust twist right
   if (CommOut == "w 17") {
    Adj_TR();CommOut=lastComm;
   }
   //~~~~~~~~ adjust speed walk
   if (CommOut == "w 0 0") {
    StepSpeed = 300;CommOut=lastComm;
   }
   //~~~~~~~~ adjust speed run
   if (CommOut == "w 7 1") {
    StepSpeed = 50;CommOut=lastComm;
   }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  switch(type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
      }
      break;
    case WStype_TEXT:
      lastComm = CommOut; CommOut = "";
      for(int i = 0;i < length;i++) CommOut += ((char) payload[i]);
      // send data to all connected clients
      webSocket.broadcastTXT(payload, length);
      break;
    case WStype_BIN:
      hexdump(payload, length);
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;default:break;
  }
}

void Send_Comm() {
  String SendString;int i;
  for (i = 1;i < 32;i += 1) {
    if (SMov[i]>=600 and SMov[i]<=2280){
      SendString = SendString +"#" +i +"P" +String(SMov[i]);
    }
  }
  digitalWrite(LED_BUILTIN, HIGH); 
  digitalWrite(D5, LOW); 
  Serial.println(SendString +"T"+String(StepSpeed));wait_serial_return_ok();  //--> it will wait until servo driver return OK (it will loop when u don't connect the servo driver)
  digitalWrite(LED_BUILTIN, LOW); 
  digitalWrite(D5, HIGH); 
}

void wait_serial_return_ok()
{  
 // int TimeOut=0; 
  int num=0; char c[16]; 
  
  
  
   // TimeOut +=1;
    while(Serial.available() > 0)
    {
      webSocket.loop();server.handleClient();
      c[num]=Serial.read();num++;
      if(num>=15) num=0;
      if(c[num-2]=='C'&&c[num-1]=='C') break;
    }
    
    
  //  if(TimeOut > 800000) break;
  
}


//================================================================================= Servo Move =======================================================================

//~~~~~~~~~shoot 
void Shoot_cannon(){
  digitalWrite(D6, HIGH); 
  Serial.println ("#4P700T1000");wait_serial_return_ok(); 
  Serial.println ("#4P1500T1000");wait_serial_return_ok(); 
  digitalWrite(D6, LOW); 
}

//~~~~~~~~~~

//~~~~~~~~ Service position
void Pos_SRV(){
  //lastSpeed = StepSpeed;StepSpeed = 50;
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1440;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1440;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  //StepSpeed = lastSpeed;
  CommOut == "";
}
//~~~~~~~~ Initial position (adjust all init servo here)
void Pos_INT(){
  SAdj[29]=STrim[29];SAdj[30]=STrim[30];SAdj[31]=STrim[31];SAdj[17]=STrim[17];SAdj[18]=STrim[18];SAdj[19]=STrim[19];SAdj[1]=STrim[1];SAdj[2]=STrim[2];SAdj[3]=STrim[3];SAdj[5]=STrim[5];SAdj[6]=STrim[6];SAdj[7]=STrim[7];SAdj[13]=STrim[13];SAdj[14]=STrim[14];SAdj[15]=STrim[15];SAdj[25]=STrim[25];SAdj[26]=STrim[26];SAdj[27]=STrim[27];Send_Comm();
  Pos_SRV();
  CommOut == "";
}
//~~~~~~~~ Stop motion
void Move_STP(){
  SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;Send_Comm();
  CommOut == "";
}

//~~~~~~~~ Dance
void Dance(){
  SAdj[6]+=200;SAdj[7]+=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[30]+=200;SAdj[31]+=220;SAdj[5]+=200;SAdj[13]+=200;SAdj[25]+=200;SAdj[1]+=200;SAdj[17]+=200;SAdj[29]+=200;Send_Comm();Pos_SRV();
  SAdj[5]-=400;SAdj[13]-=400;SAdj[25]-=400;SAdj[1]-=400;SAdj[17]-=400;SAdj[29]-=400;Send_Comm();Pos_SRV();
  SAdj[5]+=400;SAdj[13]+=400;SAdj[25]+=400;SAdj[1]+=400;SAdj[17]+=400;SAdj[29]+=400;SAdj[6]-=400;SAdj[7]-=440;SAdj[26]+=400;SAdj[27]+=440;SAdj[2]+=400;SAdj[3]+=440;SAdj[30]-=400;SAdj[31]-=440;Send_Comm();Pos_SRV();
  SAdj[5]-=400;SAdj[13]-=400;SAdj[25]-=400;SAdj[1]-=400;SAdj[17]-=400;SAdj[29]-=400;Send_Comm();Pos_SRV();
  Pos_INT();
  StepSpeed = 100;
  SAdj[6]+=400;SAdj[7]+=440;SAdj[26]-=400;SAdj[27]-=440;SAdj[2]-=400;SAdj[3]-=440;SAdj[30]+=400;SAdj[31]+=440;
  Send_Comm();Pos_SRV();
  SAdj[6]+=200;SAdj[7]+=220;SAdj[14]+=200;SAdj[15]+=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[18]+=200;SAdj[19]+=220;SAdj[30]+=200;SAdj[31]+=220;
  SAdj[6]-=200;SAdj[7]-=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[30]-=200;SAdj[31]-=220;
  Send_Comm();Pos_SRV();
  SAdj[6]+=200;SAdj[7]+=220;SAdj[14]+=200;SAdj[15]+=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[18]+=200;SAdj[19]+=220;SAdj[30]+=200;SAdj[31]+=55;
  SAdj[6]-=200;SAdj[7]-=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[30]-=200;SAdj[31]-=55;
  Send_Comm();Pos_SRV();
  SAdj[6]-=200;SAdj[7]-=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[30]-=200;SAdj[31]-=55;
  SAdj[6]-=200;SAdj[7]-=220;SAdj[14]-=200;SAdj[15]-=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[18]-=200;SAdj[19]-=220;SAdj[30]-=200;SAdj[31]-=55;
  Send_Comm();Pos_SRV();
  SAdj[6]-=200;SAdj[7]-=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[30]-=200;SAdj[31]-=55;
  SAdj[6]-=200;SAdj[7]-=220;SAdj[14]-=200;SAdj[15]-=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[18]-=200;SAdj[19]-=220;SAdj[30]-=200;SAdj[31]-=55;
  Send_Comm();Pos_SRV();
  SAdj[6]-=200;SAdj[7]-=220;SAdj[14]-=200;SAdj[15]-=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[18]-=200;SAdj[19]-=220;SAdj[30]-=200;SAdj[31]-=55;
  SAdj[6]+=200;SAdj[7]+=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[30]+=200;SAdj[31]+=55;
  Send_Comm();Pos_SRV();
  SAdj[6]-=200;SAdj[7]-=220;SAdj[14]-=200;SAdj[15]-=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[18]-=200;SAdj[19]-=220;SAdj[30]-=200;SAdj[31]-=55;
  SAdj[6]+=200;SAdj[7]+=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[30]+=200;SAdj[31]+=55;
  Send_Comm();Pos_SRV();
  SAdj[6]+=200;SAdj[7]+=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[30]+=200;SAdj[31]+=55;
  SAdj[6]+=200;SAdj[7]+=220;SAdj[14]+=200;SAdj[15]+=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[18]+=200;SAdj[19]+=220;SAdj[30]+=200;SAdj[31]+=55;
  Send_Comm();Pos_SRV();
  Pos_INT();
  SAdj[6]+=400;SAdj[7]+=440;SAdj[26]-=400;SAdj[27]-=440;SAdj[2]-=400;SAdj[3]-=440;SAdj[30]+=400;SAdj[31]+=440;
  Send_Comm();Pos_SRV();
  SAdj[6]+=200;SAdj[7]+=220;SAdj[14]+=200;SAdj[15]+=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[18]+=200;SAdj[19]+=220;SAdj[30]+=200;SAdj[31]+=220;
  SAdj[6]-=200;SAdj[7]-=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[30]-=200;SAdj[31]-=220;
  Send_Comm();Pos_SRV();
  SAdj[6]+=200;SAdj[7]+=220;SAdj[14]+=200;SAdj[15]+=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[18]+=200;SAdj[19]+=220;SAdj[30]+=200;SAdj[31]+=55;
  SAdj[6]-=200;SAdj[7]-=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[30]-=200;SAdj[31]-=55;
  Send_Comm();Pos_SRV();
  SAdj[6]-=200;SAdj[7]-=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[30]-=200;SAdj[31]-=55;
  SAdj[6]-=200;SAdj[7]-=220;SAdj[14]-=200;SAdj[15]-=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[18]-=200;SAdj[19]-=220;SAdj[30]-=200;SAdj[31]-=55;
  Send_Comm();Pos_SRV();
  SAdj[6]-=200;SAdj[7]-=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[30]-=200;SAdj[31]-=55;
  SAdj[6]-=200;SAdj[7]-=220;SAdj[14]-=200;SAdj[15]-=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[18]-=200;SAdj[19]-=220;SAdj[30]-=200;SAdj[31]-=55;
  Send_Comm();Pos_SRV();
  SAdj[6]-=200;SAdj[7]-=220;SAdj[14]-=200;SAdj[15]-=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[18]-=200;SAdj[19]-=220;SAdj[30]-=200;SAdj[31]-=55;
  SAdj[6]+=200;SAdj[7]+=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[30]+=200;SAdj[31]+=55;
  Send_Comm();Pos_SRV();
  SAdj[6]-=200;SAdj[7]-=220;SAdj[14]-=200;SAdj[15]-=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[18]-=200;SAdj[19]-=220;SAdj[30]-=200;SAdj[31]-=55;
  SAdj[6]+=200;SAdj[7]+=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[30]+=200;SAdj[31]+=55;
  Send_Comm();Pos_SRV();
  SAdj[6]+=200;SAdj[7]+=220;SAdj[26]-=200;SAdj[27]-=220;SAdj[2]-=200;SAdj[3]-=220;SAdj[30]+=200;SAdj[31]+=55;
  SAdj[6]+=200;SAdj[7]+=220;SAdj[14]+=200;SAdj[15]+=220;SAdj[26]+=200;SAdj[27]+=220;SAdj[2]+=200;SAdj[3]+=220;SAdj[18]+=200;SAdj[19]+=220;SAdj[30]+=200;SAdj[31]+=55;
  Send_Comm();Pos_SRV();
  Pos_INT();
  StepSpeed = 500;
  SAdj[6]+=400;SAdj[7]+=440;SAdj[14]+=400;SAdj[15]+=440;SAdj[26]+=400;SAdj[27]+=440;SAdj[2]-=400;SAdj[3]-=440;SAdj[18]-=400;SAdj[19]-=440;SAdj[30]-=400;SAdj[31]-=440;
  Send_Comm();Pos_SRV();
  StepSpeed = 10;
  SAdj[6]-=400;SAdj[7]-=440;SAdj[14]-=400;SAdj[15]-=440;SAdj[26]-=400;SAdj[27]-=440;SAdj[2]+=400;SAdj[3]+=440;SAdj[18]+=400;SAdj[19]+=440;SAdj[30]+=400;SAdj[31]+=440;
  Send_Comm();Pos_SRV();
  Pos_INT();
  SAdj[6]+=400;SAdj[7]+=440;SAdj[14]+=400;SAdj[15]+=440;SAdj[26]+=400;SAdj[27]+=440;SAdj[2]-=400;SAdj[3]-=440;SAdj[18]-=400;SAdj[19]-=440;SAdj[30]-=400;SAdj[31]-=440;
  Send_Comm();Pos_SRV();
  SAdj[6]-=400;SAdj[7]-=440;SAdj[14]-=400;SAdj[15]-=440;SAdj[26]-=400;SAdj[27]-=440;SAdj[2]+=400;SAdj[3]+=440;SAdj[18]+=400;SAdj[19]+=440;SAdj[30]+=400;SAdj[31]+=440;
  Send_Comm();Pos_SRV();
  Pos_INT();
  CommOut == "";
}

//~~~~~~~~ Shake hand
void Move_SHK(){
  SMov[5]=SAdj[5]+1117;SMov[6]=SAdj[6]+2218;SMov[7]=SAdj[7]+828;Send_Comm();
  SMov[7]=SAdj[7]+1246;Send_Comm();
  SMov[7]=SAdj[7]+1795;Send_Comm();
  SMov[7]=SAdj[7]+1182;Send_Comm();
  SMov[7]=SAdj[7]+1763;Send_Comm();
  SMov[7]=SAdj[7]+1117;Send_Comm();
  SMov[5]=SAdj[5]+1440;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;Send_Comm();
  CommOut == "";
}
//~~~~~~~~ Waving hand
void Move_WAV(){
  SMov[5]=SAdj[5]+1058;SMov[6]=SAdj[6]+2075;SMov[7]=SAdj[7]+880;Send_Comm();
  SMov[5]=SAdj[5]+1478;Send_Comm();
  SMov[5]=SAdj[5]+1096;Send_Comm();
  SMov[5]=SAdj[5]+1478;Send_Comm();
  SMov[5]=SAdj[5]+1096;Send_Comm();
  SMov[5]=SAdj[5]+1478;Send_Comm();
  SMov[5]=SAdj[5]+1440;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;Send_Comm();
  CommOut == "";
}
//~~~~~~~~ adjust body higher
void Adj_HG(){
   SAdj[6]-=50;SAdj[7]-=55;SAdj[14]-=50;SAdj[15]-=55;SAdj[26]-=50;SAdj[27]-=55;SAdj[2]+=50;SAdj[3]+=55;SAdj[18]+=50;SAdj[19]+=55;SAdj[30]+=50;SAdj[31]+=55;
   Pos_SRV();
   CommOut == "";
}
//~~~~~~~~ adjust body lower
void Adj_LW(){
  SAdj[6]+=50;SAdj[7]+=55;SAdj[14]+=50;SAdj[15]+=55;SAdj[26]+=50;SAdj[27]+=55;SAdj[2]-=50;SAdj[3]-=55;SAdj[18]-=50;SAdj[19]-=55;SAdj[30]-=50;SAdj[31]-=55;
  Pos_SRV();
  CommOut == "";
}
//~~~~~~~~ adjust head up
void Adj_HU(){
   SAdj[6]-=50;SAdj[7]-=55;SAdj[26]+=50;SAdj[27]+=55;SAdj[2]+=50;SAdj[3]+=55;SAdj[30]-=50;SAdj[31]-=55;
   Pos_SRV();
   CommOut == "";
}
//~~~~~~~~ adjust head down
void Adj_HD(){
   SAdj[6]+=50;SAdj[7]+=55;SAdj[26]-=50;SAdj[27]-=55;SAdj[2]-=50;SAdj[3]-=55;SAdj[30]+=50;SAdj[31]+=55;
   Pos_SRV();
   CommOut == "";
}
//~~~~~~~~ adjust body left
void Adj_LF(){
  SAdj[6]+=50;SAdj[7]+=55;SAdj[14]+=50;SAdj[15]+=55;SAdj[26]+=50;SAdj[27]+=55;SAdj[2]+=50;SAdj[3]+=55;SAdj[18]+=50;SAdj[19]+=55;SAdj[30]+=50;SAdj[31]+=55;
  Pos_SRV();
  CommOut == "";
}
//~~~~~~~~ adjust body right
void Adj_RG(){
  SAdj[6]-=50;SAdj[7]-=55;SAdj[14]-=50;SAdj[15]-=55;SAdj[26]-=50;SAdj[27]-=55;SAdj[2]-=50;SAdj[3]-=55;SAdj[18]-=50;SAdj[19]-=55;SAdj[30]-=50;SAdj[31]-=55;
  Pos_SRV();
  CommOut == "";
}
//~~~~~~~~ adjust twist left
void Adj_TL(){
   SAdj[5]-=50;SAdj[13]-=50;SAdj[25]-=50;SAdj[1]-=50;SAdj[17]-=50;SAdj[29]-=50;
   Pos_SRV();
   CommOut == "";
}
//~~~~~~~~ adjust twist right
void Adj_TR(){
   SAdj[5]+=50;SAdj[13]+=50;SAdj[25]+=50;SAdj[1]+=50;SAdj[17]+=50;SAdj[29]+=50;
   Pos_SRV();
   CommOut == "";
}


//~~~~~~~~ move forward
void Move_FWD(){   
  SMov[29]=SAdj[29]+1490;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1090;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1520;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+1460;SMov[3]=SAdj[3]+1540;SMov[5]=SAdj[5]+1280;SMov[6]=SAdj[6]+1380;SMov[7]=SAdj[7]+1140;SMov[13]=SAdj[13]+1630;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1350;SMov[25]=SAdj[25]+1215;SMov[26]=SAdj[26]+1410;SMov[27]=SAdj[27]+1240;Send_Comm();
  SMov[29]=SAdj[29]+1690;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1090;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1490;SMov[1]=SAdj[1]+1225;SMov[2]=SAdj[2]+1450;SMov[3]=SAdj[3]+1490;SMov[5]=SAdj[5]+1355;SMov[6]=SAdj[6]+1390;SMov[7]=SAdj[7]+1190;SMov[13]=SAdj[13]+1480;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+1840;SMov[25]=SAdj[25]+1290;SMov[26]=SAdj[26]+1400;SMov[27]=SAdj[27]+1190;Send_Comm();
  SMov[29]=SAdj[29]+1890;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1150;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1430;SMov[6]=SAdj[6]+1400;SMov[7]=SAdj[7]+1240;SMov[13]=SAdj[13]+1330;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+1840;SMov[25]=SAdj[25]+1365;SMov[26]=SAdj[26]+1390;SMov[27]=SAdj[27]+1140;Send_Comm();
  SMov[29]=SAdj[29]+1815;SMov[30]=SAdj[30]+1450;SMov[31]=SAdj[31]+1540;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1490;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1040;SMov[5]=SAdj[5]+1500;SMov[6]=SAdj[6]+1410;SMov[7]=SAdj[7]+1290;SMov[13]=SAdj[13]+1180;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1350;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1380;SMov[27]=SAdj[27]+1090;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+1460;SMov[31]=SAdj[31]+1590;SMov[17]=SAdj[17]+1325;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1520;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1040;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+1420;SMov[7]=SAdj[7]+1340;SMov[13]=SAdj[13]+1255;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1380;SMov[25]=SAdj[25]+1390;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+1840;Send_Comm();
  SMov[29]=SAdj[29]+1665;SMov[30]=SAdj[30]+1470;SMov[31]=SAdj[31]+1640;SMov[17]=SAdj[17]+1250;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1550;SMov[1]=SAdj[1]+1600;SMov[2]=SAdj[2]+1500;SMov[3]=SAdj[3]+1740;SMov[5]=SAdj[5]+1655;SMov[6]=SAdj[6]+1430;SMov[7]=SAdj[7]+1390;SMov[13]=SAdj[13]+1330;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1410;SMov[25]=SAdj[25]+1190;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+1840;Send_Comm();
  SMov[29]=SAdj[29]+1590;SMov[30]=SAdj[30]+1480;SMov[31]=SAdj[31]+1690;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1090;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+1490;SMov[3]=SAdj[3]+1690;SMov[5]=SAdj[5]+1730;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1405;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+990;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1515;SMov[30]=SAdj[30]+1490;SMov[31]=SAdj[31]+1740;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1090;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+1480;SMov[3]=SAdj[3]+1640;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+1840;SMov[13]=SAdj[13]+1480;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1410;SMov[25]=SAdj[25]+1065;SMov[26]=SAdj[26]+1430;SMov[27]=SAdj[27]+1340;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1500;SMov[31]=SAdj[31]+1790;SMov[17]=SAdj[17]+1700;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1550;SMov[1]=SAdj[1]+1380;SMov[2]=SAdj[2]+1470;SMov[3]=SAdj[3]+1590;SMov[5]=SAdj[5]+1430;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+1840;SMov[13]=SAdj[13]+1555;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1380;SMov[25]=SAdj[25]+1140;SMov[26]=SAdj[26]+1420;SMov[27]=SAdj[27]+1290;Send_Comm();
}
//~~~~~~~~ move backward
void Move_BWD(){
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1500;SMov[31]=SAdj[31]+1790;SMov[17]=SAdj[17]+1700;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1550;SMov[1]=SAdj[1]+1380;SMov[2]=SAdj[2]+1470;SMov[3]=SAdj[3]+1590;SMov[5]=SAdj[5]+1430;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+1840;SMov[13]=SAdj[13]+1555;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1380;SMov[25]=SAdj[25]+1240;SMov[26]=SAdj[26]+1420;SMov[27]=SAdj[27]+1290;Send_Comm();
  SMov[29]=SAdj[29]+1515;SMov[30]=SAdj[30]+1490;SMov[31]=SAdj[31]+1740;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1090;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+1480;SMov[3]=SAdj[3]+1640;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+1840;SMov[13]=SAdj[13]+1480;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1410;SMov[25]=SAdj[25]+1065;SMov[26]=SAdj[26]+1430;SMov[27]=SAdj[27]+1340;Send_Comm();
  SMov[29]=SAdj[29]+1590;SMov[30]=SAdj[30]+1480;SMov[31]=SAdj[31]+1690;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1090;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+1490;SMov[3]=SAdj[3]+1690;SMov[5]=SAdj[5]+1730;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1405;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+990;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1665;SMov[30]=SAdj[30]+1470;SMov[31]=SAdj[31]+1640;SMov[17]=SAdj[17]+1250;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1550;SMov[1]=SAdj[1]+1600;SMov[2]=SAdj[2]+1500;SMov[3]=SAdj[3]+1740;SMov[5]=SAdj[5]+1655;SMov[6]=SAdj[6]+1430;SMov[7]=SAdj[7]+1390;SMov[13]=SAdj[13]+1330;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1410;SMov[25]=SAdj[25]+1190;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+1840;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+1460;SMov[31]=SAdj[31]+1590;SMov[17]=SAdj[17]+1325;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1520;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1040;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+1420;SMov[7]=SAdj[7]+1340;SMov[13]=SAdj[13]+1255;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1380;SMov[25]=SAdj[25]+1390;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+1840;Send_Comm();
  SMov[29]=SAdj[29]+1815;SMov[30]=SAdj[30]+1450;SMov[31]=SAdj[31]+1540;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1490;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1040;SMov[5]=SAdj[5]+1500;SMov[6]=SAdj[6]+1410;SMov[7]=SAdj[7]+1290;SMov[13]=SAdj[13]+1180;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1350;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1380;SMov[27]=SAdj[27]+1090;Send_Comm();
  SMov[29]=SAdj[29]+1890;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1150;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1430;SMov[6]=SAdj[6]+1400;SMov[7]=SAdj[7]+1240;SMov[13]=SAdj[13]+1330;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+1840;SMov[25]=SAdj[25]+1365;SMov[26]=SAdj[26]+1390;SMov[27]=SAdj[27]+1140;Send_Comm();
  SMov[29]=SAdj[29]+1690;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1090;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1490;SMov[1]=SAdj[1]+1225;SMov[2]=SAdj[2]+1450;SMov[3]=SAdj[3]+1490;SMov[5]=SAdj[5]+1355;SMov[6]=SAdj[6]+1390;SMov[7]=SAdj[7]+1190;SMov[13]=SAdj[13]+1480;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+1840;SMov[25]=SAdj[25]+1290;SMov[26]=SAdj[26]+1400;SMov[27]=SAdj[27]+1190;Send_Comm();
  SMov[29]=SAdj[29]+1490;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1090;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1520;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+1460;SMov[3]=SAdj[3]+1540;SMov[5]=SAdj[5]+1280;SMov[6]=SAdj[6]+1380;SMov[7]=SAdj[7]+1140;SMov[13]=SAdj[13]+1630;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1350;SMov[25]=SAdj[25]+1215;SMov[26]=SAdj[26]+1410;SMov[27]=SAdj[27]+1240;Send_Comm();
}
//~~~~~~~~ slide left
void Slide_LFT(){
  SMov[29]=SAdj[29]+1840;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1240;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1640;SMov[1]=SAdj[1]+1040;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+12040;SMov[5]=SAdj[5]+1840;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1640;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1040;SMov[25]=SAdj[25]+1040;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1640;Send_Comm();
  SMov[29]=SAdj[29]+1840;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1040;SMov[19]=SAdj[19]+1240;SMov[1]=SAdj[1]+1040;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1840;SMov[6]=SAdj[6]+1840;SMov[7]=SAdj[7]+1240;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1040;SMov[26]=SAdj[26]+1840;SMov[27]=SAdj[27]+1240;Send_Comm();
  SMov[29]=SAdj[29]+1840;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1640;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1240;SMov[1]=SAdj[1]+1040;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1640;SMov[5]=SAdj[5]+1840;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1240;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1640;SMov[25]=SAdj[25]+1040;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1240;Send_Comm();
  SMov[29]=SAdj[29]+1840;SMov[30]=SAdj[30]+1040;SMov[31]=SAdj[31]+1240;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1040;SMov[2]=SAdj[2]+1040;SMov[3]=SAdj[3]+1240;SMov[5]=SAdj[5]+1840;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1840;SMov[15]=SAdj[15]+1640;SMov[25]=SAdj[25]+1040;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  }
//~~~~~~~~ slide right
void Slide_RGT(){
  SMov[29]=SAdj[29]+1840;SMov[30]=SAdj[30]+1040;SMov[31]=SAdj[31]+1240;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1040;SMov[2]=SAdj[2]+1040;SMov[3]=SAdj[3]+1240;SMov[5]=SAdj[5]+1840;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1840;SMov[15]=SAdj[15]+1640;SMov[25]=SAdj[25]+1040;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  SMov[29]=SAdj[29]+1840;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1640;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1240;SMov[1]=SAdj[1]+1040;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1640;SMov[5]=SAdj[5]+1840;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1240;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1640;SMov[25]=SAdj[25]+1040;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1240;Send_Comm();
  SMov[29]=SAdj[29]+1840;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1040;SMov[19]=SAdj[19]+1240;SMov[1]=SAdj[1]+1040;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1840;SMov[6]=SAdj[6]+1840;SMov[7]=SAdj[7]+1240;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1040;SMov[26]=SAdj[26]+1840;SMov[27]=SAdj[27]+1240;Send_Comm();
  SMov[29]=SAdj[29]+1840;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1240;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1640;SMov[1]=SAdj[1]+1040;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1240;SMov[5]=SAdj[5]+1840;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1640;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1240;SMov[25]=SAdj[25]+1040;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1640;Send_Comm();
 }
//~~~~~~~~ turn left
void Turn_LFT(){
  SMov[29]=SAdj[29]+1490;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1090;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1430;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+1840;SMov[13]=SAdj[13]+1555;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1140;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1690;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1090;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1225;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+1840;SMov[13]=SAdj[13]+1480;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1065;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1890;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1150;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1730;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1405;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+990;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1815;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1040;SMov[5]=SAdj[5]+1655;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1330;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1190;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+1840;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1325;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1040;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1255;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1390;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+1840;Send_Comm();
  SMov[29]=SAdj[29]+1665;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1250;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1600;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1500;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1180;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1590;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1090;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1430;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1230;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+1840;SMov[25]=SAdj[25]+1365;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1515;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1090;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1355;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1480;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+1840;SMov[25]=SAdj[25]+1290;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1700;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1380;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1280;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1630;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1215;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  }
//~~~~~~~~ turn right
void Turn_RGT(){
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1700;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1380;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1280;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1630;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1215;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1515;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1090;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1355;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1480;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+1840;SMov[25]=SAdj[25]+1290;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1590;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1090;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1430;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1330;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+1840;SMov[25]=SAdj[25]+1365;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1665;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1250;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1600;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1500;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1180;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1325;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1040;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1255;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1390;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+1840;Send_Comm();
  SMov[29]=SAdj[29]+1815;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1040;SMov[5]=SAdj[5]+1655;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1330;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1190;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+1840;Send_Comm();
  SMov[29]=SAdj[29]+1890;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1150;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1730;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1405;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+990;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1690;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1090;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1225;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+1840;SMov[13]=SAdj[13]+1480;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1065;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1490;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1090;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1430;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+1840;SMov[13]=SAdj[13]+1555;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1140;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
}


//================================================================================= Servo Move end =======================================================================
