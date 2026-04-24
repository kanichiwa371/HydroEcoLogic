// Disclaimer: not final code, some thing will change in the future, this is just a MVP

// declaracion de pines

const int pinHumedadSuelo = 1;
const int pinHumedadAire = 2;
const int pinTemperatura = 3;

const int pinElectroBomba = 4;
const int pinElectroValvula = 5;

// variables de sensores

float humedadSuelo = 0;
float humedadAire = 0;
float temperatura = 0;

// umbrales (ajustarlos bien cuando sepamos los baremos de los sensores)

const int TEMP_MAX = 35; // 35ºC
const int HUMEDAD_AIRE_MAX = 80; //80%
const int Umbral_HumedadSuelo = 30; // 30%
const int Intervalo_Lectura = 2000; // lee los sensores cada 2s

unsigned long tiempoAnterior = 0;
bool regando = false;
unsigned long tiempoInicioRiego = 0;
const int Tiempo_Riego = 15000; // tiempo de riego, 15 segundos, si eso lo subimos más


void setup() {
  Serial.begin(115200);

  pinMode(pinElectroBomba, OUTPUT);
  pinMode(pinElectroValvula, OUTPUT);

  digitalWrite(pinElectroBomba, LOW);
  digitalWrite(pinElectroValvula, LOW);

  Serial.println("Iniciando riego..."); // para debug y ver que va bien, genera una terminal 
  Serial.println("Esperando sensores...");
}

void leerSensores() {
  int valorSensor = analogRead(pinHumedadSuelo);  
  humedadSuelo = map(valorSensor, 0 ,4095, 0, 100);

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
  Serial.print("Humedad suelo:");
  Serial.print(humedadSuelo);
  Serial.println("%");

  Serial.print("Humedad aire:");
  Serial.print(humedadAire);
  Serial.println("%");

  Serial.print("Temperatura; ");
  Serial.print(temperatura);
  Serial.println("ºC");

  Serial.print("Estado riego: ");
  if (regando) {
    Serial.println("ACTIVO");
  } else {
    Serial.println("INACTIVO");
  }
}


void iniciarRiego() {
  regando = true;
  tiempoInicioRiego = millis();
  digitalWrite(pinElectroBomba, HIGH);
  digitalWrite(pinElectroValvula, HIGH);

  Serial.println("Condiciones óptimas, iniciando riego...");
}

void detenerRiego() {
  regando = false;

  digitalWrite(pinElectroBomba, LOW);
  digitalWrite(pinElectroValvula, LOW);

  Serial.println("Riego detenido");
}


void loop() {
  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= Intervalo_Lectura) {
    tiempoAnterior = tiempoActual;
    leerSensores();
    mostrarDatos();
  }
  
  if (!regando) {    // Si NO esta regando y la humedad de suelo es menor de la aceptada (tengo que añadir que tambien mida las componentes del aire, de momento solo humedad del suelo) y decida en base a eso)
    if (humedadSuelo < Umbral_HumedadSuelo && temperatura < TEMP_MAX && humedadAire < HUMEDAD_AIRE_MAX) {
      iniciarRiego();
    }
  } else { // Si esta regando, cuando llegue a los 10 segundos, para
    if (tiempoActual - tiempoInicioRiego >= Tiempo_Riego) {
      detenerRiego();
    }
  }

  delay(100);
}


