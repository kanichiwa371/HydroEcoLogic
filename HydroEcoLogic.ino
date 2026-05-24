#include <WiFiManager.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Ticker.h>
#include <ESP.h>

Ticker timerRiego;


#define BOT_TOKEN "" // TU TOKEN AQUI
#define CHAT_ID "" // TU ID AQUI

WiFiClientSecure cliente;
UniversalTelegramBot bot(BOT_TOKEN, cliente);


const int pinHumedadSuelo = 34;
const int pinHumedadAire = 35;
const int pinTemperatura = 36;
const int pinElectroBomba = 26;
const int pinElectroValvula = 27;


float humedadSuelo = 0;
float humedadAire = 0;
float temperatura = 0;
bool sensoresConectados = true;


const int TEMP_MAX = 35;
const int HUMEDAD_AIRE_MAX = 80;
const int UMBRAL_HUMEDAD_SUELO = 30;
const int INTERVALO_LECTURA = 2000;


unsigned long tiempoAnteriorLectura = 0;
unsigned long tiempoUltimoComando = 0;
unsigned long tiempoInicioRiego = 0;
unsigned long tiempoFinRiego = 0;
const int COOLDOWN = 30000;
const int TIEMPO_RIEGO = 15000;
const int INTERVALO_COMANDOS = 1000;

bool reinicio_pendiente = false;
bool regando = false;
bool modoAutomatico = true;

String comandoPendiente = "";


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n🌱 HydroEcoLogic - Sistema completo");

  pinMode(pinElectroBomba, OUTPUT);
  pinMode(pinElectroValvula, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
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
  unsigned long ahora = millis();
  

  if (regando && (ahora >= tiempoFinRiego)) {
    regando = false;
    digitalWrite(pinElectroBomba, LOW);   
    digitalWrite(pinElectroValvula, LOW);
    Serial.println(">>> ⏹️ RIEGO DETENIDO POR TIEMPO <<<");
    bot.sendMessage(CHAT_ID, "⏹️ Riego detenido (15s cumplidos)");
  }


  if (!regando && (ahora - tiempoAnteriorLectura >= INTERVALO_LECTURA)) {
    tiempoAnteriorLectura = ahora;
    leerSensores();
    mostrarDatos();
  }

  
  if (ahora - tiempoUltimoComando >= INTERVALO_COMANDOS) {
    tiempoUltimoComando = ahora;
    manejarComandos();
  }

 
  if (!regando && modoAutomatico && sensoresConectados) {
    if (ahora - tiempoFinRiego >= COOLDOWN) {
      if (humedadSuelo < UMBRAL_HUMEDAD_SUELO && 
          temperatura < TEMP_MAX && 
          humedadAire < HUMEDAD_AIRE_MAX) {
        iniciarRiego("automático");
      }
    }
  }

  delay(10);
}


void leerSensores() {
  int valorSuelo = analogRead(pinHumedadSuelo);
  int valorAire = analogRead(pinHumedadAire);
  int valorTemp = analogRead(pinTemperatura);

  if (valorSuelo < 10 && valorAire < 10 && valorTemp < 10) {
    if (sensoresConectados) {
      sensoresConectados = false;
      Serial.println("⚠️ ADVERTENCIA: Sensores no detectados");
    }
    return;
  }

  sensoresConectados = true;
  humedadSuelo = map(valorSuelo, 0, 4095, 0, 100);
  humedadAire = map(valorAire, 0, 4095, 0, 100);
  temperatura = map(valorTemp, 0, 4095, 0, 50);

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
  Serial.print("🔧 Modo: ");
  Serial.println(modoAutomatico ? "AUTOMÁTICO" : "MANUAL");
}


void iniciarRiego(String origen) {
  if (regando) return;
  
  regando = true;
  tiempoInicioRiego = millis();
  tiempoFinRiego = tiempoInicioRiego + TIEMPO_RIEGO;

  digitalWrite(pinElectroBomba, HIGH);   
  digitalWrite(pinElectroValvula, HIGH);

  Serial.print(">>> 💧 RIEGO INICIADO (");
  Serial.print(origen);
  Serial.println(") <<<");
  bot.sendMessage(CHAT_ID, "💧 Riego iniciado\n🌱 Origen: " + origen);
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
      respuesta += "/modo - Cambiar automático/manual\n";
      respuesta += "/resetwifi - Reiniciar configuración WiFi\n";
      respuesta += "/ayuda - Mostrar esta ayuda";
      bot.sendMessage(chatId, respuesta);
    }
    
    else if (texto == "/modo") {
      modoAutomatico = !modoAutomatico;
      String respuesta = "🔄 Modo cambiado a: ";
      respuesta += modoAutomatico ? "AUTOMÁTICO" : "MANUAL";
      respuesta += "\n\n";
      respuesta += modoAutomatico ? "🌱 El sistema regará automáticamente cuando el suelo esté seco." : "🔧 Solo regarás manualmente con /regar.";
      bot.sendMessage(chatId, respuesta);
    }

    else if (texto == "/resetwifi") {
      if (!reinicio_pendiente) {
        bot.sendMessage(chatId, "⚠️ ¿Estás seguro de que quieres reiniciar la WiFi?\nResponde 'Y' para sí, 'N' para no.");
        reinicio_pendiente = true;
        comandoPendiente = "resetwifi";
      } else {
        bot.sendMessage(chatId, "⚠️ Ya hay una operación pendiente. Responde 'Y' o 'N'.");
      }
    }

    else if (texto == "/estado") {
      String respuesta = "📊 ESTADO HYDROECOLOGIC\n\n";
      respuesta += "🌱 Humedad suelo: " + String(humedadSuelo) + "%\n";
      respuesta += "💧 Humedad aire: " + String(humedadAire) + "%\n";
      respuesta += "🌡️ Temperatura: " + String(temperatura) + "°C\n";
      respuesta += "💦 Riego: " + String(regando ? "ACTIVO" : "INACTIVO") + "\n";
      respuesta += "🔧 Modo: " + String(modoAutomatico ? "AUTOMÁTICO" : "MANUAL") + "\n";
      
      if (!sensoresConectados) {
        respuesta += "\n⚠️ SENSORES NO CONECTADOS";
      } else if (modoAutomatico && humedadSuelo < UMBRAL_HUMEDAD_SUELO && !regando) {
        respuesta += "\n⚠️ Suelo seco. El sistema regará automáticamente.";
      }
      bot.sendMessage(chatId, respuesta);
    }

    else if (texto == "/regar") {
      if (!sensoresConectados) {
        bot.sendMessage(chatId, "⚠️ Sensores no detectados. No se puede regar manualmente.");
      } else if (!regando) {
        iniciarRiego("manual (Telegram)");
        bot.sendMessage(chatId, "💧 Riego manual iniciado");
      } else {
        bot.sendMessage(chatId, "⚠️ Ya hay un riego en curso");
      }
    }
    
    else if (texto == "/parar") {
      if (regando) {
        regando = false;
        digitalWrite(pinElectroBomba, LOW);
        digitalWrite(pinElectroValvula, LOW);
        tiempoFinRiego = millis();
        Serial.println(">>> ⏹️ RIEGO DETENIDO MANUALMENTE <<<");
        bot.sendMessage(chatId, "⏹️ Riego detenido manualmente");
      } else {
        bot.sendMessage(chatId, "⚠️ No hay un riego activo");
      }
    }
    
    else if (texto == "Y" && reinicio_pendiente && comandoPendiente == "resetwifi") {
      reinicio_pendiente = false;
      bot.sendMessage(chatId, "🔄 Borrando configuración WiFi y reiniciando la placa...");
      WiFiManager wm;
      wm.resetSettings();
      delay(1000);
      ESP.restart();
    }

    else if (texto == "N" && reinicio_pendiente) {
      reinicio_pendiente = false;
      comandoPendiente = "";
      bot.sendMessage(chatId, "✅ Operación cancelada.");
    }
    
    else {
      bot.sendMessage(chatId, "❌ Comando no válido. Usa /ayuda");
    }
  }
}
