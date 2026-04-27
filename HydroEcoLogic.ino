#include <WiFiManager.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>


// CONFIGURACIÓN TELEGRAM

#define BOT_TOKEN "" // pon el token de tu bot aqui
#define CHAT_ID "" // tu id de telegram (si no sabes cuál es, lee el README.md)

WiFiClientSecure cliente;
UniversalTelegramBot bot(BOT_TOKEN, cliente);

 
// Declaración pines (cambiar cuando tengas los componentes)

const int pinHumedadSuelo = ;    
const int pinHumedadAire = ;     
const int pinTemperatura = ;     // Pon los pines que uses aquí
const int pinElectroBomba = ;    
const int pinElectroValvula = ;  


// Variables de sensores

float humedadSuelo = 0;
float humedadAire = 0;
float temperatura = 0;

// Umbrales para los sensores (ajustalos cuando tengas los sensores calibrados)

const int TEMP_MAX = 35;               // 35ºC
const int HUMEDAD_AIRE_MAX = 80;       // 80%
const int UMBRAL_HUMEDAD_SUELO = 30;   // 30%
const int INTERVALO_LECTURA = 2000;    // leer sensores cada 2 segundos


// Control de tiempos

unsigned long tiempoAnteriorLectura = 0;
unsigned long tiempoAnteriorBot = 0;
unsigned long tiempoInicioRiego = 0;
const int INTERVALO_BOT = 1000;
const int TIEMPO_RIEGO = 15000;        // 15 segundos

bool regando = false;


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n🌱 HydroEcoLogic - Sistema completo");

  
  pinMode(pinElectroBomba, OUTPUT);
  pinMode(pinElectroValvula, OUTPUT);
  digitalWrite(pinElectroBomba, LOW);
  digitalWrite(pinElectroValvula, LOW);

 
  WiFiManager wifiManager;
  wifiManager.autoConnect("HydroEcoLogic");
  Serial.print("✅ Conectado. IP: ");
  Serial.println(WiFi.localIP());


  cliente.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  String mensaje = "🌱 HydroEcoLogic en linea\n✅ Conectado a WiFi\n📡 IP: " + WiFi.localIP().toString();
  bot.sendMessage(CHAT_ID, mensaje);
  Serial.println("📨 Telegram inicializado");
}


void loop() {
  unsigned long tiempoActual = millis();

  
  if (tiempoActual - tiempoAnteriorLectura >= INTERVALO_LECTURA) {
    tiempoAnteriorLectura = tiempoActual;
    leerSensores();
    mostrarDatos();
  }


  if (tiempoActual - tiempoAnteriorBot >= INTERVALO_BOT) {
    tiempoAnteriorBot = tiempoActual;
    manejarComandos();
  }


  if (!regando) {
    if (humedadSuelo < UMBRAL_HUMEDAD_SUELO && 
        temperatura < TEMP_MAX && 
        humedadAire < HUMEDAD_AIRE_MAX) {
      iniciarRiego("automático");
    }
  } else {
    // Si está regando, comprobar tiempo máximo
    if (tiempoActual - tiempoInicioRiego >= TIEMPO_RIEGO) {
      detenerRiego();
    }
  }

  delay(100);
}


void leerSensores() {
  int valorSensor = analogRead(pinHumedadSuelo);
  humedadSuelo = map(valorSensor, 0, 4095, 0, 100);

  valorSensor = analogRead(pinHumedadAire);
  humedadAire = map(valorSensor, 0, 4095, 0, 100);

  valorSensor = analogRead(pinTemperatura);
  temperatura = map(valorSensor, 0, 4095, 0, 50);

  humedadSuelo = constrain(humedadSuelo, 0, 100);
  humedadAire = constrain(humedadAire, 0, 100);
  temperatura = constrain(temperatura, 0, 50);
}

void mostrarDatos() {
  Serial.println("------------");
  Serial.print("🌱 Humedad suelo: ");
  Serial.print(humedadSuelo);
  Serial.println("%");

  Serial.print("💧 Humedad aire: ");
  Serial.print(humedadAire);
  Serial.println("%");

  Serial.print("🌡️ Temperatura: ");
  Serial.print(temperatura);
  Serial.println("ºC");

  Serial.print("💦 Estado riego: ");
  Serial.println(regando ? "ACTIVO" : "INACTIVO");
}


void iniciarRiego(String origen) {
  regando = true;
  tiempoInicioRiego = millis();
  digitalWrite(pinElectroBomba, HIGH);
  digitalWrite(pinElectroValvula, HIGH);

  Serial.print(">>> 💧 RIEGO INICIADO (");
  Serial.print(origen);
  Serial.println(") <<<");
  
  bot.sendMessage(CHAT_ID, "💧 Riego iniciado\n🌱 Origen: " + origen);
}

void detenerRiego() {
  regando = false;
  digitalWrite(pinElectroBomba, LOW);
  digitalWrite(pinElectroValvula, LOW);

  Serial.println(">>> ⏹️ RIEGO DETENIDO <<<");
  bot.sendMessage(CHAT_ID, "⏹️ Riego detenido");
}


void manejarComandos() {
  int numMensajes = bot.getUpdates(bot.last_message_received + 1);

  for (int i = 0; i < numMensajes; i++) {
    String texto = bot.messages[i].text;
    String chatId = bot.messages[i].chat_id;
    String nombre = bot.messages[i].from_name;

    if (chatId != CHAT_ID) {
      bot.sendMessage(chatId, "❌ No autorizado");
      continue;
    }

    Serial.println("📩 Comando Telegram: " + texto);

    if (texto == "/ayuda") {
      String respuesta = "🌱 Hola " + nombre + "!\n";
      respuesta += "HydroEcoLogic - Sistema completo\n\n";
      respuesta += "📋 COMANDOS:\n";
      respuesta += "/regar - Activar riego manual\n";
      respuesta += "/parar - Detener riego\n";
      respuesta += "/estado - Ver estado del sistema\n";
      respuesta += "/ayuda - Mostrar esta ayuda";
      bot.sendMessage(chatId, respuesta);
    }
    
    else if (texto == "/regar") {
      if (!regando) {
        iniciarRiego("manual (Telegram)");
        bot.sendMessage(chatId, "💧 Riego manual iniciado");
      } else {
        bot.sendMessage(chatId, "⚠️ Ya hay un riego en curso");
      }
    }
    
    else if (texto == "/parar") {
      if (regando) {
        detenerRiego();
        bot.sendMessage(chatId, "⏹️ Riego detenido manualmente");
      } else {
        bot.sendMessage(chatId, "⚠️ No hay un riego activo");
      }
    }
    
    else if (texto == "/estado") {
      String respuesta = "📊 ESTADO HYDROECOLOGIC\n\n";
      respuesta += "🌱 Humedad suelo: " + String(humedadSuelo) + "%\n";
      respuesta += "💧 Humedad aire: " + String(humedadAire) + "%\n";
      respuesta += "🌡️ Temperatura: " + String(temperatura) + "°C\n";
      respuesta += "💦 Riego: " + String(regando ? "ACTIVO" : "INACTIVO") + "\n";
      
      if (humedadSuelo < UMBRAL_HUMEDAD_SUELO && !regando) {
        respuesta += "\n⚠️ Suelo seco. Condiciones óptimas para regar.";
      }
      bot.sendMessage(chatId, respuesta);
    }
    
    else {
      bot.sendMessage(chatId, "❌ Comando no válido. Usa /ayuda");
    }
  }
}
