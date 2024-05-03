int UVOUT = A0; //Output from the sensor
int REF_3V3 = A1; //3.3V power on the Arduino board
int buzzerPin = 8;

//Note: all the calculation are done in the power of milli (10^-3)
const float limitPerSec = 1; // (mJ/cm^2) TLV as by ASGIH
const float restTime = 15.0; //arbitary rest time

long long unsigned int counts = 0;
float avgExposure = 0.0;
float totalExposure = 0.0;
float safeExposure;
float requiredRestTime = 0.0;


void setup()
{
  Serial.begin(9600);

  pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop()
{

  counts += 1;
  int uvLevel = averageAnalogRead(UVOUT);
  int refLevel = averageAnalogRead(REF_3V3);

  //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  float outputVoltage = 3.3 / refLevel * uvLevel;

  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level

  Serial.print("ref Voltage (V): ");
  Serial.print(refLevel);

  Serial.print(" / ML8511 output: ");
  Serial.print(uvLevel);

  Serial.print(" / ML8511 voltage: ");
  Serial.print(outputVoltage);

  Serial.print(" / UV Intensity (mW/cm^2): ");
  Serial.print(uvIntensity);
  
  Serial.print("/ Avg Exposure (mJ/cm^2): ");
  Serial.print(avgExposure);

  Serial.print(" / Total Exposure (mJ/cm^2): ");
  Serial.print(totalExposure);


  totalExposure += uvIntensity;

  avgExposure = (((avgExposure*(counts-1)) + uvIntensity) / counts);

  safeExposure = counts * limitPerSec;

  if (totalExposure <= safeExposure) {
    Serial.print(" / Mode: Safe");
  } else {
    Serial.print(" / Mode: Limit Exceeded");

    if(isTime()) {
      buzzer();
    }

  }

  Serial.println();

  delay(1000); 
}



int averageAnalogRead(int pinToRead) {
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 

  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);  
}


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void buzzer() {
  digitalWrite(buzzerPin, HIGH); // Turn the buzzer on
  delay(200); // Delay for 1 second (adjust as needed for your desired beep duration)
  digitalWrite(buzzerPin, LOW); // Turn the buzzer off
}


bool isTime() {
  float diff = totalExposure - safeExposure;
  float reqRestTime = diff / avgExposure;
  Serial.print(" / Req Rest time (sec): ");
  Serial.print(reqRestTime);
  if (reqRestTime < restTime) return false;
  else requiredRestTime = reqRestTime;
  return true;
}
