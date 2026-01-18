#include <Wire.h>
#include <string.h>
#include "max6675.h"


int local = 1; // ID de la unidad local, utilizado para filtrar mensajes que están destinados a este dispositivo.
char ini[4] = "INI"; // Comando de inicialización.
char sns[4] = "SNS";
char tm1[4] = "TM1";
char tm2[4] = "TM2";
char tm3[4] = "TM3";

int thermoSO =  8;
int thermoCS =  9;
int thermoSCK = 10;

int thermoSO2 =  7;
int thermoCS2 =  6;
int thermoSCK2 = 5;

int thermoSO3 =  4;
int thermoCS3 =  3;
int thermoSCK3 = 2;

// Declaracion de variables globales
float tempC; // Variable para almacenar el valor obtenido del sensor (0 a 1023)
int pinLM35 = 0; // Variable del pin de entrada del sensor (A0)
 
MAX6675 thermocouple(thermoSCK,thermoCS,thermoSO);

MAX6675 thermocouple2(thermoSCK2,thermoCS2,thermoSO2);

MAX6675 thermocouple3(thermoSCK3,thermoCS3,thermoSO3);


void setup() {
  Serial.begin(9600); // Inicializa la comunicación serial a 9600 baudios.

}

void loop() {
  if (Serial.available()) { // Si hay datos disponibles en el puerto serial:
    String buf_str = Serial.readStringUntil('\n'); // Lee la cadena hasta encontrar un salto de línea.
    Serial.flush(); // Limpia el buffer serial.
    int length = buf_str.length(); // Obtiene la longitud de la cadena recibida.
    char buffer[length + 1]; // Ajuste en la longitud del buffer.
    buf_str.toCharArray(buffer, length + 1); // Convierte el mensaje de String a char array.
    
    // Extrae el mensaje principal del buffer (sin los últimos 4 caracteres que corresponden al LRC)
    char message[length - 3];
    subchar(message, buffer, 0, length - 4);
    int check = LRC(message, length - 3); // Calcula el LRC para el mensaje.

    // Extrae el LRC del mensaje
    char lrc[4];
    subchar(lrc, buffer, length - 4, 3);

    // Extrae el campo de dirección
    char add[3];
    subchar(add, buffer, 0, 2);

    // Extrae el campo de comando
    char cmd[4];
    subchar(cmd, buffer, 2, 3);

    // Extrae el campo de datos del mensaje
    char data[length - 8];
    subchar(data, buffer, 5, length - 9);

    // Verifica el LRC y otros parámetros del mensaje
    if (chartoInt(lrc, 3) == check) { // Si el LRC calculado coincide con el LRC recibido:
      if (chartoInt(add, 2) == local) { // Si el mensaje está destinado al ID local:
        if (charEqual(cmd, ini, 3)) { // Si el comando es "INI":
          char measure[20] = {0}; // Define el espacio para almacenar la respuesta.
          int size = INI(measure, data); // Llama a la función INI para generar una respuesta.
          writeMsg(add, cmd, measure, size); // Envía la respuesta.
        } else if (charEqual(cmd, sns, 3)) { // Si el comando es "TM1":
          char measure[20] = {0}; // Define el espacio para almacenar la respuesta.
          int size = SNS(measure, data); // Llama a la función TM1 para generar una respuesta.
          writeMsg(add, cmd, measure, size); // Envía la respuesta.
        } else if (charEqual(cmd, tm1, 3)) { // Si el comando es "TM1":
          char measure[20] = {0}; // Define el espacio para almacenar la respuesta.
          int size = TM1(measure, data); // Llama a la función TM1 para generar una respuesta.
          writeMsg(add, cmd, measure, size); // Envía la respuesta.
        } else if (charEqual(cmd, tm2, 3)) { // Si el comando es "TM2":
          char measure[20] = {0}; // Define el espacio para almacenar la respuesta.
          int size = TM2(measure, data); // Llama a la función TM2 para generar una respuesta.
          writeMsg(add, cmd, measure, size); // Envía la respuesta.
        } else if (charEqual(cmd, tm3, 3)) { // Si el comando es "TM1":
          char measure[20] = {0}; // Define el espacio para almacenar la respuesta.
          int size = TM3(measure, data); // Llama a la función TM1 para generar una respuesta.
          writeMsg(add, cmd, measure, size); // Envía la respuesta.
        } 
        else {
          char measure[8] = "CMD_ERR"; // Mensaje de error de comando no válido.
          writeMsg(add, cmd, measure, 8); // Envía el mensaje de error.
        }
      } else {
        char measure[8] = "ADD_ERR"; // Mensaje de error de dirección.
        writeMsg(add, cmd, measure, 8); // Envía el mensaje de error.
      }
    } else {
      char measure[8] = "LRC_ERR"; // Mensaje de error de LRC no coincidente.
      writeMsg(add, cmd, measure, 8); // Envía el mensaje de error.
    }
  }
}

int LRC(char message[], int length) {
  int chk = 0; // Inicializa el valor de verificación.
  for (int i = 0; i < length; i++)
    chk += message[i]; // Suma el valor ASCII de cada carácter.
  return chk % 128; // Devuelve el LRC (módulo 128).
}

void subchar(char result[], char buffer[], int start, int length) {
  strncpy(result, buffer + start, length);
  result[length] = '\0'; // Asegura el carácter nulo al final de la cadena
}

int chartoInt(char array[], int length) {
  int result = 0; // Inicializa el resultado.
  for (int i = 0; i < length; i++) {
    result += (array[i] - '0') * int(pow(10, length - i - 1) + 1e-5); // Convierte y acumula cada dígito.
  }
  return result; // Devuelve el número entero resultante.
}

bool charEqual(char A[], char B[], int length) {
  for (int i = 0; i < length; i++) {
    if (A[i] != B[i]) return false;
  }
  return true;
}

void writeMsg(char add[], char cmd[], char data[], int length) {
  char message[9 + length] = {0}; // Define el array para el mensaje completo.
  strcpy(message, add); // Copia la dirección.
  strcat(message, cmd); // Agrega el comando.
  strcat(message, data); // Agrega los datos.
  
  char lrc[4] = {0};
  sprintf(lrc, "%03d", LRC(message, 5 + length)); // Calcula el LRC y lo convierte a string.
  strcat(message, lrc); // Agrega el LRC al final del mensaje.
  Serial.println(message); // Envía el mensaje serialmente.
}


int INI(char data_out[], char data_in[]) {
  char measure[3] = "OK"; // Respuesta de "OK".
  subchar(data_out, measure, 0, 2); // Copia la respuesta a data_out.
  return 2; // Devuelve la longitud de la respuesta.
}

int SNS(char data_out[], char data_in[]) {
  int pinLM35 = A0; // Pin analógico donde está conectado el sensor LM35
  float tempC = analogRead(pinLM35); // Leer el valor analógico del sensor
  tempC = (5.0 * tempC * 100.0) / 1024.0; // Convertir el valor a temperatura en grados Celsius

  // Convertir la temperatura a una cadena con 3 decimales
  String temp_string = String(tempC, 3);
  int length = temp_string.length() + 1; // +1 para el carácter nulo
  char buffer[length] = {0};
  temp_string.toCharArray(buffer, length);
  
  // Copiar la cadena al buffer de salida
  subchar(data_out, buffer, 0, length);

  return length;
}

  
int TM1(char data_out[], char data_in[]) {
 
  float temperature = thermocouple.readCelsius();

  String temp_string = String(temperature,3);
  int length = temp_string.length()+1;
  char buffer[length]={0};
  temp_string.toCharArray(buffer, length);
  subchar(data_out, buffer, 0, length);

  return length;
  }

int TM2(char data_out[], char data_in[]) {
 
  float temperature = thermocouple2.readCelsius();

  String temp_string = String(temperature,3);
  int length = temp_string.length()+1;
  char buffer[length]={0};
  temp_string.toCharArray(buffer, length);
  subchar(data_out, buffer, 0, length);

  return length;
  }

  int TM3(char data_out[], char data_in[]) {
 
  float temperature = thermocouple3.readCelsius();

  String temp_string = String(temperature,3);
  int length = temp_string.length()+1;
  char buffer[length]={0};
  temp_string.toCharArray(buffer, length);
  subchar(data_out, buffer, 0, length);

  return length;
  }





