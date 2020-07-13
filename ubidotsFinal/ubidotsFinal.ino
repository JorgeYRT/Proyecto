/*************************
     L I B R E R I A S
*************************/

#include <WiFi.h>//Libreria para utilizar el Wifi
#include <PubSubClient.h> //Libreria para utilizar el protocolo MQTT


/*************************
       DEFINICIONES
*************************/

//Credenciales de Red
#define WIFISSID "Neffos X1 Lite" // Nombre de la Señal de Wifi
#define PASSWORD "12345678" // Contraseña de la señal

//Identificadores para Ubidots
#define TOKEN "BBFF-jvpiMWOaCZniqhuRs3ecTmB9cRTt0F" // TOKEN (Este lo otorga la página de ubidots)
#define MQTT_CLIENT_NAME "Roa1614J" // Un nombre de cliente, puedes ser aleatorio y debe ser unico. Solo es para identificar
#define DEVICE_LABEL "esp32_george" // Assig the device label

//Constantes
#define VARIABLE_LABEL "var_poten" // Hacia que variable (de ubidots) será dirigido/recibido el valor del potencipometro
#define VARIABLE_LED "var_led" //Hacia que variable (de ubidots) sera dirigido/recubido el valor de la variable

//Definiciones Locales
#define led 13 // Pin para el encendido/apagado del LED
#define potenciometro 39 //Pin para la lectura del Potenciometro

char mqttBroker[]  = "industrial.api.ubidots.com"; //Direccion hacia donde se manda/recibe la info
char payload[100]; //No sé
char topic[150]; //No sé
char topicSubscribe[100];//No sé
char str_sensor[10];// Espacio para guardar los valores a enviar

/*************************
         FUNCIONES
*************************/

WiFiClient ubidots;
PubSubClient client(ubidots);

//***************************************************************************************************************************************
void callback(char* topic, byte* payload, unsigned int length) { //Es para estar recibiendo los valores que la página de ubidots manda

  char p[length + 1];
  memcpy(p, payload, length);//Creo que es para tomar el valor que llega desde ubidots (p) y lo pasa a la otra variable (payload)
  p[length] = NULL; //Limpiar el TAMAÑO de p
  String message(p); //Extrae el mensaje que llegó y lo guarda en la variable menssaje
  Serial.println("message = " + message); //Imprime el mensaje

  if (message == "0") { //Sí el mensaje que llegó es un "0" se apaga el led, de lo contrario, se encenderá
    digitalWrite(led, LOW);//Apagar led
  } else {
    digitalWrite(led, HIGH);//Encender led
  }

  Serial.write(payload, length);
  Serial.print("topic: "); Serial.println(topic);
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
void reconnect() {//Si la conexion "se cayo", entonces, se intentará reconectar con la página de ubidots

  while (!client.connected()) {// Mientras no se esté conectado se ejecutará este ciclo (O sea mientras no haya señal o conexion con la pagina de ubidots y el ESP32)
    Serial.println("Attempting MQTT connection...");

    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {// Una vez conectado, manda el mensaje de conectado
      Serial.println("Connected");
      client.subscribe(topicSubscribe);
    } else {//Si no jala nos lo hará saber
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }//Aquí terminba el ciclo
}
//***************************************************************************************************************************************


/*********************************************************************************
                              Parametros Iniciales
       (Cosas que se ejecutan una vez, o sea, cuando se enciende el ESP32)
*********************************************************************************/

void setup() {//Este void es el primero que se ejecuta; se ejecuta cada vez que el ESP32 se prende
  Serial.begin(115200);//Configurando el clockrate (La frecuencia de reloj en relación a un procesador o microprocesador indica la frecuencia a la cual los transistores que lo conforman conmutan eléctricamente, es decir, abren y cierran el flujo de una corriente eléctrica. [Fuente: Wikipedia])


  //pinMode(potenciometro, INPUT); //Configurando el pin del ESP32 para que lea los datos
  pinMode(led, OUTPUT); //Configurando el pin del ESP32 para "mande la señal" de encendido o apagado

  // val = analogRead(potenciometro);
  // Serial.println("Valor de lectura:" + val);

  Serial.println();
  WiFi.begin(WIFISSID, PASSWORD); //Inicia (o al menos eso intenta) conectarse al modem (y con ello tendra internet)
  Serial.print("Warten WiFi..."); //Dice: Esperando el Wifi

  while (WiFi.status() != WL_CONNECTED) { //Mientras no logre conectarse al internet se imprimira en el monitor serie un puntito cada medio segundo (Es para que se vea pro y se pueda saber que aun no hay conexión)
    Serial.print(".");
    delay(500);
  }//Si ya se conecto el ESP32 a la red entonces este ciclo termina y sa pasa a las lineas de abajo

  Serial.println("");
  Serial.println("!");
  Serial.println("WiFi Verbuden");//DIce: Wifi COnectado
  Serial.print("IP Adresse: ");
  Serial.println(WiFi.localIP());//Imprime la direccion IP que el modem nos asigno
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
  sprintf(topicSubscribe, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, VARIABLE_LED);
  client.subscribe(topicSubscribe);

}

void loop() {
  //val = analogRead(potenciometro);
  //Serial.println("Valor de lectura:" + val);
  //val = (val * 1000) / 4095;
  //Serial.println("Resultado Operacion:" + val);

  if (!client.connected()) {
    client.subscribe(topicSubscribe);
    reconnect();
  }

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL); //Acomoda los datos en una sola linea*
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Adds the variable label

  int sensor = analogRead(potenciometro);// Lee el valor del potenciometro y lo guarda en la variable "sensor"
  sensor = (sensor * 1000) / 4095;
  Serial.print("Valor del potenciometro:"); Serial.println(sensor); //Imprime el valor previamente leído

  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(sensor, 4, 2, str_sensor);//El valor guardado en la variable "sensor" se pasa a la variable "srt_sensor" en forma de cadena

  Serial.print("str_sendor: "); Serial.println(str_sensor);

  sprintf(payload, "%s {\"value\": %s}}", payload, str_sensor); // Adds the value (No tengo ni la menor idea)
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);//Manda el valor leído a ubidots
  client.loop();//No tengo ni la menor idea


  delay(1000);

}
