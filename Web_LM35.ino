#include <WiFi.h>

// Configuración del servidor Web en el puerto 80
WiFiServer server(80);

// Credenciales de WiFi
const char* ssid     = "INFINITUMF1AE";
const char* password = "EcFGaGG9ka";

// Configuración del sensor LM35
const int LM35_SENSOR_PIN = 33;

// Variables globales
int contconexion = 0;
String header; // Variable para guardar el HTTP request

// Función para leer la temperatura en grados Celsius
float leerTemperatura() {
  int lecturaADC = analogRead(LM35_SENSOR_PIN); // Lee el valor ADC del LM35
  float voltaje = lecturaADC * (3.3 / 4095.0);  // Convierte la lectura ADC a voltaje (3.3V para el ESP32)
  float temperatura = voltaje * 100.0;          // Convierte el voltaje a temperatura (10mV por grado Celsius)
  return temperatura;
}

// Página HTML
String generarPaginaHTML(float temperatura) {
  String pagina = "<!DOCTYPE html>"
                  "<html>"
                  "<head>"
                  "<meta charset='utf-8' />"
                  "<meta http-equiv='refresh' content='5' />"  // Actualiza la página cada 5 segundos
                  "<title>Servidor Web ESP32 - Temperatura</title>"
                  "</head>"
                  "<body>"
                  "<center>"
                  "<h1>Servidor Web ESP32</h1>"
                  "<p>Temperatura Actual: " + String(temperatura, 2) + " °C</p>"
                  "</center>"
                  "</body>"
                  "</html>";
  return pagina;
}

void setup() {
  Serial.begin(115200);

  // Configurar el pin del sensor LM35 como entrada
  pinMode(LM35_SENSOR_PIN, INPUT);

  // Conexión a la red WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && contconexion < 50) { 
    ++contconexion;
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado");
    Serial.println(WiFi.localIP()); // Imprime la dirección IP del ESP32
    server.begin();                 // Inicia el servidor
  } else {
    Serial.println("\nError de conexion");
  }
}

void loop() {
  // Verifica si hay un cliente disponible
  WiFiClient client = server.available();
  if (client) { // Si se conecta un nuevo cliente
    Serial.println("Nuevo cliente conectado.");
    String currentLine = "";
    
    // Lee el request del cliente
    while (client.connected()) {
      if (client.available()) { // Si hay datos para leer
        char c = client.read();
        Serial.write(c); // Muestra el request en el monitor serie
        header += c;
        if (c == '\n') {
          // Si la nueva línea está en blanco, significa que es el fin del HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) {
            // Lee la temperatura actual del LM35
            float temperatura = leerTemperatura();
            
            // Responde con los encabezados HTTP estándar
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Genera y envía la página HTML con la temperatura actual
            client.println(generarPaginaHTML(temperatura));

            // La respuesta HTTP termina con una línea en blanco
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    // Limpiamos la variable header y cerramos la conexión
    header = "";
    client.stop();
    Serial.println("Cliente desconectado.");
  }
}
