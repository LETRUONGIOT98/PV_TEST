vint state = 0; //khai báo biến trạng thái để xử lý trên winform
int I; //khai báo biến dòng điện
int U; //khai báo biến điện áp
const int solando = 100; //số cặp giá trị muốn lấy 1 lần đo
float thoigiando = 0.1;
#define PIN_PWM 9 //chân pwm để kích cho mosfet
#define cambienI A0 //chân cảm biến ACS712
#define cambienU A1 //chân đọc điện áp
int tt = true; //biến để reset arduino
float pwm = 255;
float old1, old2;

// Khai báo mảng để lưu dữ liệu
float currentMeasurements[solando];
float voltageMeasurements[solando];

// Mảng lưu giá trị trung bình của mỗi 10 lần đo
float avgCurrentMeasurements[solando / 10];
float avgVoltageMeasurements[solando / 10];

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    char temp = Serial.read();
    if (temp == '0') //dừng
      state = 0;
    if (temp == '1') { //lấy dữ liệu
      state = 1;
      tt = true;
    }
    if (temp == '2') //reset
      state = 2;
  }

  switch (state) {
    case 0: //trạng thái dừng
      break;
    case 1: //trạng thái lấy mẫu
      if (tt) {
        for (int i = 0; i < solando; i++) {
          analogWrite(PIN_PWM, pwm);
          float sum = 0.0, sum1 = 0.0, I = 0.0, U = 0.0;
          for (int j = 0; j < 50; j++) {
            int adc = analogRead(cambienI);
            float val = (adc * 60) / 1023.0;
            float I = val - 30.15;
            if (I < 0.1) {
              I = 0;
            }
            sum = sum + I;
            delay(1);
          }
          sum = sum / 50.0;
          I = sum;

          if (I < 0.4) I = 0;
          if (I < 0) I = 0;
          for (int j = 0; j < 50; j++) {
            int adc1 = analogRead(cambienU);
            float val1 = adc1 / 1023.0;
            float U = val1 * 25.0;
            if (U < 0.3) {
              U = 0;
            }
            sum1 = sum1 + U;
            delay(1);
          }
          sum1 = sum1 / 50.0;
          U = sum1;
          if (U < 0) U = 0;

          // Lưu giá trị vào mảng
          currentMeasurements[i] = I;
          voltageMeasurements[i] = U;

          Serial.print(I);
          Serial.print(",");
          Serial.println(U);
          delay(thoigiando * 1000);
          pwm = pwm - 255 / solando;
          if (pwm <= 0) pwm = 0;
        }
        
        // Tính giá trị trung bình của mỗi 10 lần đo
        for (int k = 0; k < solando / 10; k++) {
          float sumCurrent = 0.0;
          float sumVoltage = 0.0;
          for (int m = 0; m < 10; m++) {
            sumCurrent += currentMeasurements[k * 10 + m];
            sumVoltage += voltageMeasurements[k * 10 + m];
          }
          avgCurrentMeasurements[k] = sumCurrent / 10.0;
          avgVoltageMeasurements[k] = sumVoltage / 10.0;
/*
          Serial.print("Avg Current for batch ");
          Serial.print(k + 1);
          Serial.print(": ");
          Serial.println(avgCurrentMeasurements[k]);

          Serial.print("Avg Voltage for batch ");
          Serial.print(k + 1);
          Serial.print(": ");
          Serial.println(avgVoltageMeasurements[k]);*/
        }
        
        tt = false;
      }
      break;
    case 2: //trạng thái reset
      I = 0;
      U = 0;
      state = 0;
  }
  analogWrite(PIN_PWM, 0); //đưa pwm về 0
}
