#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

int rele = D8;
int button= D3;
Adafruit_PCD8544 display = Adafruit_PCD8544(D5, D7, D6, D1, D2);
bool statusRele = false;

const char* ssid = "FAVERI";
const char* password = "05061983";

ESP8266WebServer server(80);

const char* www_username = "admin";
const char* www_password = "admin";

void handlerFunction(){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    String captionButton;
    String sEstado;
    if (statusRele) {
      captionButton = "Desligar";
      sEstado = "<span style=\"color:red\">Ligada</span>";
    } else {
      captionButton = "Ligar";
      sEstado = "<span style=\"color:blue\">Desligado</span>";
    }
    String htmlpage = "<!DOCTYPE html>"
                        "<html>"
                          "<head>"
                            "<meta charset=\"UTF-8\"><title>Cafeteira</title>"
                            "<link rel=\"stylesheet\" "
                               "href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\""
                               "integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\""
                               "crossorigin=\"anonymous\">"
                          "</head>"
                          "<body>"
                            "<h1>Estado da cafeteira: " + sEstado + "</h1>"
                            "<hr>"
                            "<form action=\"/inverteEstado\" method=\"POST\">"
                              "<button class=\"btn btn-default\" name=\"cafeteira\" type=\"submit\" value=\"0\">" + captionButton + "</button>"
                            "</form>"
                         "</body>"
                      "</html>";


    server.sendContent_P(htmlpage.c_str());
}

void setupDisplay(void) {
  display.begin();
  display.setContrast(50); //Ajusta o contraste do display
  display.clearDisplay();   //Apaga o buffer e o display
  display.setTextSize(1);  //Seta o tamanho do texto
  display.setTextColor(BLACK); //Seta a cor do texto
  display.setCursor(0,0);  //Seta a posição do cursor
  display.drawRoundRect(0,0, 80,24, 3, 2);
  display.setCursor(11,3);  // Seta a posição do cursor
  display.println("Estado:");  
  display.setCursor(9,14);
  display.println("Desligado"); 
  display.display();
  Serial.println("Display inicializado");
}

void setupHttpServer() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Falhou ao conectar! Reiniciando...");
    delay(1000);
    ESP.restart();
  }
  ArduinoOTA.begin();

  server.on("/", handlerFunction);
  server.on("/inverteEstado", handlerInverteEstado);
  server.begin();

  Serial.print("Server em http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  
}

void setupRele() {
  pinMode(rele, OUTPUT);
  digitalWrite(rele, LOW);
  Serial.println("Rele inicializado. \n Estado: Desligado");
  statusRele = false;
}

void setupButton() {
  pinMode(button, INPUT);
}

void setup() {

  Serial.begin(115200);
  Serial.println();
  setupDisplay();
  setupHttpServer();
  setupRele();
  setupButton();
}

int lastButtonState;
void loop() {
  // WebServer Loop
  ArduinoOTA.handle();
  server.handleClient();

  // Button Loop
  int buttonState = digitalRead(button);
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Botao solto");
  }
  if (buttonState) {
    Serial.println("Botao pressionado");
    inverteEstado();
    delay(500); // Wait a bit between retransmissions
  } 
  lastButtonState = buttonState;
}

void inverteEstado() {
  if (statusRele) {
    digitalWrite(rele, LOW);
    statusRele = false;
    Serial.println("Rele Desligado!");
  } else {
    digitalWrite(rele, HIGH);
    statusRele = true;
    Serial.println("Rele Ligado!");
  }
  atualizaDisplay();
}

void handlerInverteEstado() {
  inverteEstado();
  handlerFunction();
}

void atualizaDisplay() {
  display.fillRect(8,13, 70,10, 0);
  display.setCursor(9,14);
  if (statusRele) {
    display.println("Ligado");   
  } else {
    display.println("Desligado");
  }
  display.display();
}

