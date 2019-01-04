#include <SPI.h>
#include <Ethernet.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht (DHTPIN,DHTTYPE);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 8, 200);
IPAddress dns(192, 168, 8, 1);
int port = 80;
EthernetServer serveur(port);

/** Fonction de calcul rapide du point de rosée en fonction de la température et de l'humidité ambiante */
double rosee(double celsius, double humidity) {

  // Constantes d'approximation
  // Voir http://en.wikipedia.org/wiki/Dew_point pour plus de constantes
  const double a = 17.27;
  const double b = 237.7;

  // Calcul (approximation)
  double temp = (a * celsius) / (b + celsius) + log(humidity * 0.01);
  return (b * temp) / (a - temp);
}

void setup() {
  Serial.begin(250000);
  while (!Serial) {
    ;
  }
  Serial.println("Ethernet WebServer");

 // On démarre la voie Ethernet
  Serial.println("Inicialisation Ethernet avec DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Erreur de configuration Ethernet avec DHCP");
    // On vois si le Ethernet shield est OK
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Le Ethernet shield n'a pas été trouvé. Pas posible de se connecter sans matériel. :(");
      while (true) {
        delay(1);
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println(" Cable Ethernet n'est pas connecte.");
    }
    // on essaye de se connecter sans DHCP
    Ethernet.begin(mac, ip, dns);
  } else {
    Serial.print("  DHCP à assigée l'IP ");
    Serial.print(Ethernet.localIP());
    Serial.print(":");
    Serial.println(port);
  }
   Serial.println("Init...");// Donne une seconde au shield pour s'initialiser et initialiser le reste de capteurs
   dht.begin();//On initialise le DHT11
   delay(1000);
   // On lance le serveur
   serveur.begin();
   Serial.println("Status: ONLINE");
}

void loop(){
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  double ros = rosee(temp, humid);
  EthernetClient client = serveur.available(); 
  if (client)
  {
    Serial.println("Nouveau client");
    bool currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        Serial.write(c);
        
        // Al recibir linea en blanco, servir página a cliente
        if (c == '\n' && currentLineIsBlank)
        {
          client.println(F("HTTP/1.1 200 OK\nContent-Type: text/html"));
          client.println();
 
          client.println(F("<html>\n<head>\n<title>Station Meteo Web</title>\n<meta charset='utf-8'></head>\n<body style='text-align:center;background-color:black;color:white'>"));
          client.println(F("<div >"));
          client.println(F("<br /><br />"));
          client.println(F("<fieldset style='display:inline-block;width:'20%';'>"));
          
          client.print(F("<h2>Temperature</h2>"));
          client.print(F("<p> "));
          client.print(temp);
          client.println(F(" ºC </p>"));
          client.print(F("<meter value="));
          client.print(temp);
          client.print(F(" min "));
          client.print(0);
          client.print(F(" max="));
          client.print(50);
          client.println(F(">Graph non Disponible</meter>"));
          client.println(F("<br />"));
          client.println(F("<br /><br />"));

          client.print(F("<h2>Humidite</h2>"));
          client.print(F("<p> "));
          client.print(humid);
          client.println(F(" % </p>"));
          client.print(F("<meter value="));
          client.print(humid);
          client.print(F(" min "));
          client.print(0);
          client.print(F(" max="));
          client.print(100);
          client.println(F(">Graph non Disponible</meter>"));
          client.println(F("<br />"));
          client.println(F("<br /><br />"));

          client.print(F("<h2>Temperature de Rosée</h2>"));
          client.print(F("<p> "));
          client.print(ros);
          client.println(F(" ºC </p>"));
          client.print(F("<meter value="));
          client.print(ros);
          client.print(F(" min "));
          client.print(-30);
          client.print(F(" max="));
          client.print(50);
          client.println(F(">Graph non Disponible</meter>"));
          client.println(F("<br />"));
          client.println(F("<br /><br />"));

          client.println(F("</fieldset>"));
          client.println(F("<br /><br />"));
          
          client.print(F("<button type='button'><a href='http://"));
          client.print(Ethernet.localIP());
          client.print(":");
          client.print(port);
          client.print(F("'>Refrescar</a></button>"));
          client.println(F("</div>\n</body></html>"));
          break;
        }
        if (c == '\n') 
        {
          currentLineIsBlank = true;
        }
        else if (c != '\r') 
        {
          currentLineIsBlank = false;
        }
      }
    }
 
    delay(1);
    client.stop();
  }
}
