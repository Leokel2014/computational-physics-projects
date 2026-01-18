#include <Stepper.h>

int local = 1;
char ini[4] = "INI";
char prs[4] = "PRS";
char dif[4] = "DIF";

// Definir parámetros del motor
const int pasosPorRevolucion = 3600;
Stepper motor(pasosPorRevolucion, 8, 10, 9, 11);

// Definir pin del fotodiodo
const int fotoDiodoPin = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  motor.setSpeed(5);  // Velocidad en RPM
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()){
    String buf_str = Serial.readStringUntil('\n');
    Serial.flush();
    int length = buf_str.length();
    char buffer[length] = {0};
    buf_str.toCharArray(buffer, buf_str.length());
    //
    char message[length-3] = {0};
    subchar(message,buffer,0,length-4);  
    int check = LRC(message, length-3);
    char lrc[4] = {0};
    subchar(lrc,buffer,length-4,3);
    //
    char add[3] = {0};
    subchar(add,buffer,0,2);
    char cmd[4] = {0};
    subchar(cmd,buffer,2,3);
    char data[length-8] = {0};
    subchar(data,buffer,5,length-9);

    if (chartoInt(lrc,3) == check){
      if (chartoInt(add,2) == local){
        if (charEqual(cmd,ini,3)){
          char measure[20] = {0};
          int size = INI(measure,data);
          writeMsg(add,cmd,measure,size);
        } else if (charEqual(cmd,prs,3)) {
          char measure[20] = {0};
          int size = PRS(measure,data);
          writeMsg(add,cmd,measure,size);
        }else if (charEqual(cmd,dif,3)) {
          char measure[20] = {0};
          int size = DIF(measure,data);
          writeMsg(add,cmd,measure,size);
        } else {
          char measure[8] = "CMD_ERR";
          writeMsg(add,cmd,measure,8);
        }
      } else {
        char measure[8] = "ADD_ERR";
        writeMsg(add,cmd,measure,8);
      }
    } else {
      char measure[8] = "LRC_ERR";
      writeMsg(add,cmd,measure,8);
    }
  }
}

int LRC(char message[], int length){
  int chk = 0;
  for (int i=0; i<length; i++)
    chk += message[i];
  return chk % 128;
}

void subchar(char result[], char buffer[], int start, int length){
  for (int i = 0 ; i < length ; i++) 
    result[i] = buffer[start+i];
}

int chartoInt(char array[], int length){
  int result = 0;
  for (int i = 0; i < length; i++){
    result += (array[i]-48)*int(pow(10,length-i-1)+1e-5);
  }
  return result;
}

bool charEqual(char A[], char B[], int length){
  bool eq = 1;
  for (int i = 0; i < length; i++){
    eq = eq && A[i] == B[i];
  }
  return eq;
}

void writeMsg(char add[], char cmd[], char data[], int length){
  char message[9+length] = {0};
  strcpy(message,add);
  strcat(message,cmd);
  strcat(message,data);
  char lrc[4] = {0};
  sprintf(lrc,"%03d",LRC(message, 5+length));
  strcat(message,lrc);
  Serial.println(message);
}

int INI(char data_out[], char data_in[]){
  char measure[3] = "OK";
  subchar(data_out,measure,0,2);
  return 2;
}

int PRS(char data_out[], char data_in[]){
  char measure[8] = "1.62E-6";
  subchar(data_out,measure,0,7);
  return 7;
}

int DIF(char data_out[], char data_in[]){
  char measure[10] = {0};
  float intensidad = 1000-analogRead(fotoDiodoPin);

  motor.step(-20);

  dtostrf(intensidad, 5, 2, measure);
  subchar(data_out, measure, 0, strlen(measure));
  return strlen(measure);
}
