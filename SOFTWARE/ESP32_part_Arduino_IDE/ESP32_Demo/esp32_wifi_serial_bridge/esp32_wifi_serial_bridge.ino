#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

enum STATE_MACHINE {

    STATE_INIT,
    STATE_CLIENT,
    STATE_SERVER
   
} SM_STATE;


uint8_t SM_TASKS;

 
const char *ssid = "YourSSID";
const char *password = "YourPassword";

AsyncWebServer http_server(80); // We need http server for manual trying to connect with ip.
WiFiServer my_server(1111);
WiFiClient my_client;
WiFiClient remote_client;

String header;
String RX_BUFFER;
String TX_BUFFER;

String Rip; //remote ip in string format

IPAddress RemoteIP = IPAddress(0,0,0,0); //remote ip address in IPAddress type;

void wait_client()
{
  WiFiClient this_client;
  RemoteIP = IPAddress(0,0,0,0); // Wait for setup via WEB  or TCP connection
  
    Serial.println("\n=========== Waiting for income Connection ===========\n");
    while(RemoteIP == IPAddress(0,0,0,0))
    {
    this_client = my_server.available();
    if(this_client) {
      if(this_client.connected())
      {
        Serial.println("\n=========== Someone is trying to connect ===========\n");
        Serial.print("IP address: ");
        Serial.println(this_client.remoteIP());
        if((this_client.remoteIP() == RemoteIP) || (RemoteIP == IPAddress(0,0,0,0))
        )
        {
          Serial.println("*** Connection Accepted ***\n\n");
          remote_client = this_client;
          SM_STATE = STATE_SERVER;
          break;
        }
        else {
            Serial.println("*** Unauthorized client. Connection closed!!! ***");
            this_client.stop();
        }
      }
    }
  }
}

void client_state()
{
  uint8_t tx_buffer, rx_buffer;
  Serial.flush();
  my_client.flush();
  Serial.print("\n\n=========== Trying to connect to \"");
  Serial.print(RemoteIP);
  Serial.println("\"===========\n\n");

  if(my_client.connect(RemoteIP, 1111))
  {
    Serial.println("**** CONNECTED ****");
    while(my_client.connected())
    {
        if(Serial.available() || my_client.available())
        {
          if(Serial.available())
          {
            my_client.write(Serial.read());
          }

          if(my_client.available())
          {
            Serial.write(my_client.read());
          }
          
        }                  
    }
    
    Serial.println("\n\n=========== Connection was closed by Server ===========\n\n");
    SM_STATE = STATE_INIT;
  }
  
}

void server_state()
{
  char rx_buffer=0, tx_buffer=0;
  Serial.flush();
  my_client.flush();
  
   while(remote_client.connected())
    {
        if(Serial.available() || remote_client.available())
        {
          if(Serial.available())
          {
            remote_client.write(Serial.read());
          }

          if(remote_client.available())
          {
            Serial.write(remote_client.read());
          }
          
        }           
    }
    
    Serial.println("\n\n=========== Connection was closed by Client  ===========\n\n");
  SM_STATE = STATE_INIT;
}

void setup()
{
  SM_STATE = STATE_INIT;
  Serial.begin(2000000);
  while(Serial.read() < 0);
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print("\nIP: ");
  Serial.println(WiFi.localIP());
  my_server.begin();
  http_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");
  response->printf("<!DOCTYPE html><html><head><title>ESP32 Connection Setup </title></head>");
  response->print("<body>");
  //response->print("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script><style>");
  response->print("<h2>ESP32 Connection Setup</h2>");
  response->print("<p>Enter the IP in the textbox and click SAVE button </br> </p>");
  response->print("<fieldset>");
  
  response->print("<legend>ESP32 UART: setup</legend>");
  response->print("<form  method=\"post\">");
  response->printf("Destination IP:<br>");
  response->printf("<input type=\"text\" name=\"IP\" value=\"Enter destination IP\" style=\"width: 200px\">");
  response->print("<br><br>");
  response->printf("<input type=\"submit\" value=\"SAVE\">");
  response->printf(" </fieldset>");
  response->printf("</form>");
  response->printf("</body>");
  response->printf(" </html>");
  
  //send the response last
  request->send(response);
                         
    });

  http_server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.print("HOST: ");
    Serial.println(request->host());
    RemoteIP.fromString(request->arg("IP"));
    Serial.print("***** RemoteIP  TO: ");
    Serial.print(RemoteIP);
    Serial.println(" *****");
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  AsyncClient *client = request->client();
  //Serial.print(client->remoteIP());
 
  response->addHeader("Server","ESP Async Web Server");
  response->printf("<!DOCTYPE html><html><head><title>ESP32 Connection Setup </title></head>");
  response->print("<body>");
  //response->print("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script><style>");
  response->print("<h2>ESP32 Connection Setup</h2>");
  response->print("<p>Enter the IP in the textbox and click SAVE button </br> </p>");
  response->print("<fieldset>");
  
  response->print("<legend>ESP32 UART: setup</legend>");
  response->print("<form  method=\"post\">");
  response->printf("Destination IP:<br>");
  
  response->printf("<input type=\"text\" name=\"IP\" value=\"\" style=\"width: 200px\">");
  
  response->print("<br><br>");
  response->printf("<input type=\"submit\" value=\"SAVE\">");
  response->printf(" </fieldset>");
  response->printf("</form>");
  response->printf("</body>");
  response->printf(" </html>");
  response->printf("Settings saved.");
  //send the response last
  request->send(response);
  SM_STATE = STATE_CLIENT; // If we change the IP address, we need to reinitialize the TCP/IP connection with the client
  });
  
  http_server.begin(); 
}

void loop()
{
  while(1)
  {
    switch (SM_STATE) {
      case STATE_INIT: wait_client();    break;
      case STATE_CLIENT: client_state(); break;
      case STATE_SERVER: server_state(); break;
      default: break;
    }
  }
 
}

