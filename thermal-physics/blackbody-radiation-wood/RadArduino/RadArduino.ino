#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include "max6675.h"
#include <string.h>


const int lm35PinA0 = A0; // Sensor LM35 en A0
const int lm35PinA1 = A1; // Sensor LM35 en A1 (reemplazo de MAX6675)

int thermoSO =  4;
int thermoCS =  5;
int thermoSCK = 6;

MAX6675 thermocouple(thermoSCK,thermoCS,thermoSO);

double new_emissivity = 0.95;
int local = 1;
char ini[4] = "INI";
char tm1[4] = "TM1";
char tob[4] = "TOB";
char tam[4] = "TAM";
char emi[4] = "EMI";

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
    Serial.begin(9600);
    Serial.flush();
    
    if (!mlx.begin()) {
        Serial.println("Error al iniciar el sensor MLX90614.");
        while (1);
    }
    delay(1000);
    
    mlx.writeEmissivity(new_emissivity);
    Serial.flush();
}

void loop() {
    if (Serial.available()) {
        String buf_str = Serial.readStringUntil('\n');
        Serial.flush();
        
        int length = buf_str.length();
        char buffer[length + 1] = {0};
        buf_str.toCharArray(buffer, length + 1);
        
        char message[length - 3] = {0};
        subchar(message, buffer, 0, length - 4);
        int check = LRC(message, length - 3);
        char lrc[4] = {0};
        subchar(lrc, buffer, length - 4, 3);
        char add[3] = {0};
        subchar(add, buffer, 0, 2);
        
        char cmd[4] = {0};
        subchar(cmd, buffer, 2, 3);
        
        if (chartoInt(lrc, 3) == check && chartoInt(add, 2) == local) {
            char measure[20] = {0};
            int size = 0;
            
            if (charEqual(cmd, ini, 3)) {
                size = INI(measure);
            } else if (charEqual(cmd, tm1, 3)) {
                size = TM1(measure);
            } else if (charEqual(cmd, tob, 3)) {
                size = readTOB(measure);
            } else if (charEqual(cmd, tam, 3)) {
                size = readLM35_A0(measure);
            } else if (charEqual(cmd, emi, 3)) {
                size = readEmissivity(measure);
            } else {
                enviarError(add, cmd, "CMD_ERR");
                return;
            }
            writeMsg(add, cmd, measure, size);
        } else {
            enviarError(add, cmd, "LRC_ERR");
        }
    }
}
int INI(char data_out[]) {
    strcpy(data_out, "OK");
    return 2;
}

int readTOB(char data_out[]) {
    float sum = 0;
    
    for (int i = 0; i < 25; i++) {
        sum += mlx.readObjectTempC(); // Read object temperature from MLX90614
        delay(40); // Ensures 25 samples in 1 second
    }

    float avgTemp = sum / 25.0; // Calculate the average temperature
    dtostrf(avgTemp, 7, 2, data_out); // Convert float to string
    return strlen(data_out);
}

int TM1(char data_out[]) {
 
  float temperature = thermocouple.readCelsius();

  String temp_string = String(temperature,3);
  int length = temp_string.length()+1;
  char buffer[length]={0};
  temp_string.toCharArray(buffer, length);
  subchar(data_out, buffer, 0, length);

  return length;
  }


int readLM35_A0(char data_out[]) {
    float sum = 0;
    for (int i = 0; i < 25; i++) {
        int lm35Val = analogRead(lm35PinA0);
        sum += (lm35Val * 5.0 * 100.0) / 1024.0;
        delay(40); // Ensures 25 samples in 1 second
    }
    float avgTemp = sum / 25.0 -3.4;
    dtostrf(avgTemp, 7, 2, data_out);
    return strlen(data_out);
}

int readLM35_A1(char data_out[]) {
    float sum = 0;
    for (int i = 0; i < 25; i++) {
        int lm35Val = analogRead(lm35PinA1);
        sum += (lm35Val * 5.0 * 100.0) / 1024.0;
        delay(40);
    }
    float avgTemp = sum / 25.0;
    dtostrf(avgTemp, 7, 2, data_out);
    return strlen(data_out);
}

int readEmissivity(char data_out[]) {
    float sum_T_obj = 0, sum_T_amb = 0, sum_T_tcup = 0;

    for (int i = 0; i < 25; i++) {
        sum_T_obj += mlx.readObjectTempC() + 273.15;
        sum_T_amb += analogRead(lm35PinA0) * 5.0 * 100.0 / 1024.0 + 273.15;
        sum_T_tcup += thermocouple.readCelsius();//analogRead(lm35PinA1) * 5.0 * 100.0 / 1024.0 + 273.15;
        delay(40);
    }

    float T_obj = sum_T_obj / 25.0;
    float T_amb = sum_T_amb / 25.0;
    float T_tcup = sum_T_tcup / 25.0;

    auto f_T = [](float T) {
        return pow(T, -4) - (1.95e4 * pow(T, -2)) + 0.71;
    };

    float f_T_obj = f_T(T_obj);
    float f_T_amb = f_T(T_amb);
    float f_T_tcup = f_T(T_tcup);

    float numerator = f_T_obj * pow(T_obj, 4) - f_T_amb * pow(T_amb, 4);
    float denominator = f_T_tcup * pow(T_tcup, 4) - f_T_amb * pow(T_amb, 4);
    float emissivity = (denominator != 0) ? ((new_emissivity * numerator) / denominator) : 0;

    dtostrf(emissivity, 4, 2, data_out);
    return strlen(data_out);
}




void enviarError(char add[], char cmd[], const char* error) {
    char measure[8] = {0};
    strcpy(measure, error);
    writeMsg(add, cmd, measure, strlen(measure));
}

int LRC(char message[], int length) {
    int chk = 0;
    for (int i = 0; i < length; i++)
        chk += message[i];
    return chk % 128;
}

void subchar(char result[], char buffer[], int start, int length) {
    for (int i = 0; i < length; i++)
        result[i] = buffer[start + i];
}

int chartoInt(char array[], int length) {
    int result = 0;
    for (int i = 0; i < length; i++) {
        result += round((array[i]-'0')*pow(10, length - i - 1));
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