#include <Stepper.h>
#include <math.h>
#include <string.h>


// Pines de sensor y láser
const int pinLaser = 3; // Láser KY-008
const int pinLDR = 2;   // Salida digital del sensor LM393

// Variables globales
const int local = 1; // Dirección local
char ini[4] = "INI";
char vlt[4] = "VLT";
char srv[4] = "SRV";
long pasosDados = 0;

// FUNCIONES DE PROTOCOLO
int LRC(char message[], int length) {
  int chk = 0;
  for (int i = 0; i < length; i++)
    chk += message[i];
  return chk % 128;
}

void subchar(char result[], char buffer[], int start, int length) {
  strncpy(result, buffer + start, length);
  result[length] = '\0';
}

int chartoInt(char array[], int length) {
  int result = 0;
  for (int i = 0; i < length; i++) {
    result += (array[i] - '0') * int(pow(10, length - i - 1) + 1e-5);
  }
  return result;
}

bool charEqual(char A[], char B[], int length) {
  for (int i = 0; i < length; i++) {
    if (A[i] != B[i]) return false;
  }
  return true;
}

void writeMsg(char add[], char cmd[], char data[], int length) {
  char message[9 + length] = {0};
  strcpy(message, add);
  strcat(message, cmd);
  strcat(message, data);
  char lrc[4] = {0};
  sprintf(lrc, "%03d", LRC(message, 5 + length));
  strcat(message, lrc);
  Serial.println(message);
}

// FUNCIÓN INI (NO MODIFICAR)
int INI(char data_out[], char data_in[]) {
  char measure[3] = "OK";
  subchar(data_out, measure, 0, 2);
  return 2;
}


// SETUP Y LOOP CON PROCESAMIENTO DE COMANDOS
void setup() {
  Serial.begin(9600);
  myStepper.setSpeed(10); // RPM
  pinMode(pinLaser, OUTPUT);
  pinMode(pinLDR, INPUT);
  digitalWrite(pinLaser, LOW); // Apaga láser por defecto
}

void loop() {
  if (Serial.available()) {
    String buf_str = Serial.readStringUntil('\n');
    Serial.flush();
    int length = buf_str.length();
    char buffer[length + 1];
    buf_str.toCharArray(buffer, length + 1);

    char message[length - 3];
    subchar(message, buffer, 0, length - 4);
    int check = LRC(message, length - 3);

    char lrc[4];
    subchar(lrc, buffer, length - 4, 3);

    char add[3];
    subchar(add, buffer, 0, 2);

    char cmd[4];
    subchar(cmd, buffer, 2, 3);

    char data[length - 8];
    subchar(data, buffer, 5, length - 9);

    if (chartoInt(lrc, 3) == check) {
      if (chartoInt(add, 2) == local) {
        if (charEqual(cmd, ini, 3)) {
          char measure[20] = {0};
          int size = INI(measure, data);
          writeMsg(add, cmd, measure, size);
        } else if (charEqual(cmd, srv, 3)) {
          char measure[20] = {0};
          int size = SRV(measure, data);
          writeMsg(add, cmd, measure, size);
          }else if (charEqual(cmd, vlt, 3)) {
          char measure[20] = {0};
          int size = VLT(measure, data);
          writeMsg(add, cmd, measure, size);
        } else {
          char measure[8] = "CMD_ERR";
          writeMsg(add, cmd, measure, 8);
        }
      } else {
        char measure[8] = "ADD_ERR";
        writeMsg(add, cmd, measure, 8);
      }
    } else {
      char measure[8] = "LRC_ERR";
      writeMsg(add, cmd, measure, 8);
    }
  }
}

int SRV(char data_out[], char data_in[]) { // LAS = laser

  int angulo = atoi(data_in); // Convierte el string a entero 
  
  // Verifica que el angulo esté en el rango permitido
  if (angulo < 0) angulo = 0;
  if (angulo > 180) angulo = 180;

  // Ajusta el ángulo del servomotor
  if(myServo.read() > angulo){
    for(int i=myServo.read();i>=angulo;i--){   
      myServo.write(i);
      delay(30);
    }
  } else if(myServo.read() < angulo){
    for(int i=myServo.read();i<=angulo; i++){   
      myServo.write(i);
      delay(30);
    }
  }

  char servoStr[8];
  dtostrf(angulo, 6, 2, servoStr);

  // Inserta el string en data_out usando subchar
  subchar(data_out, servoStr, 0, 7);

  return 7; // Devuelve el número de caracteres agregados
}

int VLT(char data_out[], char data_in[]) {
  const float Vref = 5.0; // Voltaje de referencia
  int valorLeido = analogRead(A2);
  float voltaje = valorLeido * (Vref / 1023.0);
  
  char vltStr[8]; 
  dtostrf(voltaje, 6, 2, vltStr);
  
  subchar(data_out, vltStr, 0, 7);
  return 7; 
}