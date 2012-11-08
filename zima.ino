
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 2, 222);
IPAddress dns_server(172, 16, 0, 1);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);

//output pin can not use 11-13, cause w5100 use it for ethernet.
int output_pin = 8;

EthernetServer server(80);

String token = "";

void setup() {
  Serial.begin(9600);

  Serial.print("config: ");
  Serial.print(ip);
  Serial.print(" ");
  Serial.print(dns_server);
  Serial.print(" ");
  Serial.print(gateway);
  Serial.print(" ");
  Serial.print(subnet);
  Serial.println();

  Ethernet.begin(mac, ip, dns_server, gateway, subnet);

  Serial.print("Begin with: ");
  Serial.print(Ethernet.localIP());
  Serial.println();

  server.begin();

  pinMode(output_pin, OUTPUT);

  //init set
  digitalWrite(output_pin, LOW);
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    String currentLine = "";
    String request = "";
    int notice_length = 0;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        currentLine += c;
        notice_length++;
        if (c == '\n' && currentLineIsBlank) {
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          if (currentLine.substring(0, 4) == "GET ") {
            request = currentLine.substring(4, notice_length - 11);
          }
          currentLine = "";
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    //Serial.println(request);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connnection: close");
    client.println();

    if (request.substring(0, 6) == "/heart") {
      client.println(millis());
    } else if (request.substring(0, 6) == "/open/" &&  request.substring(6, 6 + 16) == token) {
      open_door();
      refresh_token();
      client.println(token);
    } else if (request.substring(0, 6) == "/open/") {
      client.println("invalid token");
      
      Serial.print("token:");
      Serial.println(token);
      /*
      client.print("real token:[");
      client.print(token);
      client.println("]");
      */
    } else {
      // some other request
    }

    delay(100);
    client.stop();
  }
}

void open_door() {
  Serial.println("open");
  digitalWrite(output_pin, HIGH);
  delay(1000);
  digitalWrite(output_pin, LOW);
}

void refresh_token() {
  String new_token = "";
  char c;
  randomSeed(analogRead(0));
  for(int i = 0; i < 16; i++) {
    if (random(0, 100) % 2 == 0) c = 'A' + random(1, 26);
    else c = 'a' + random(1, 26);
    new_token += c;
  }
  token = new_token;
}

