
//************************Librerias necesarias******************************
#include <OneWire.h>
#include <DallasTemperature.h>

//***********************Puertos de los sensores****************************

#define TSL_FREQ  2 //Sensor de luz
#define IR_LED    3 //Alimentacion del LED
int ECPin = A0;     //Puerto de medida del conductimetro
int ECPower = A4;    //Puerto de alimentacion del conductimetro
#define ONE_WIRE_BUS 5 //Puerto del sensor de temperatura




//***********************Variables del conductimetro*************************
float R1 = 15000;       //Resistencia del divisor de tension
int Ra = 25;           //Resistencia de entrada de los pines de la Arduino
float PPMconversion = 0.64;  //Valor europeo ppm
//float PPMconversion = 0.5; //Valor USA ppm
//float PPMconversion = 0.7; //Valor Australia ppm
float TemperatureCoef = 0.019;
float K = 0.72; //Constante de celula calculado en la calibracion
float Temperature = 10;
float EC = 0;
float EC25 = 0;
float ppm = 0;
float raw = 0;
float Vin = 5;
float Vdrop = 0;
float Rc = 0;
float buffer = 0;
OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);

//*************************Variables de turbidimetro*************************
volatile unsigned long cnt = 0;
unsigned long oldcnt = 0;
unsigned long t = 0;
unsigned long last;
boolean flag = true;
int contador = 0;
float suma = 0;
long low = 1000000, high = 0;
float mediaEncendido = 0, mediaApagado = 0;
unsigned long MediaTotal = 0,NTU = 0;
float NTU_final = 0, MediaTotal_final=0;


//Constantes
#define n_muestras 6



void irq1()
{
  cnt++;
}

///////////////////////////////////////////////////////////////////
//
// SETUP
//
void setup() 
{
  Serial.begin(115200);
  Serial.println("START");
  pinMode(TSL_FREQ, INPUT);
  pinMode(IR_LED, OUTPUT);
  attachInterrupt(0, irq1, RISING);
  pinMode(ECPin, INPUT);
  pinMode(ECPower, OUTPUT);
  delay(100);// gives sensor time to settle
  sensors.begin();
  delay(100);
  R1 = (R1 + Ra);
}

///////////////////////////////////////////////////////////////////
//
// MAIN LOOP
//
void loop() 
{
  
  //Medidas con el LED encendido
  if ((millis() - last >= 1000)&flag==true)
  {
    digitalWrite(IR_LED, HIGH); //Encender el LED
    delay(200);
    last = millis();
    t = cnt;
    unsigned long hz = t - oldcnt; //Calculo del numero de muestras en 1 seg
    Serial.print("FREQ Encendido: "); 
    Serial.print(hz);
    Serial.print("\t = "); 
    Serial.print((hz+50)/100);  
    Serial.println(" mW/m2");
    if(hz > high){
        high = hz;
      }                 
     if(hz < low){
        low = hz;
     }
    oldcnt = t;
    suma += hz;
    contador++;
    if (contador==n_muestras){
      suma -= (low +high);
      mediaEncendido = ((suma/(contador-2))+50)/100; //Conversion a potencia y media
      flag = false;
      Serial.print("Media Encendido: ");
      Serial.print(mediaEncendido);  
      Serial.println(" mW/m2");
      contador = 0;
      suma = 0;
      low = 1000000;
      high = 0;
    }  
    //Medidas con el LED apagado
  }else if((millis() - last >= 1000)&(flag==false)){
    digitalWrite(IR_LED, LOW); //Apagar el LED
    delay(200);
    last = millis();
    t = cnt;
    unsigned long hz = t - oldcnt; //Calculo del numero de muestras en 1 seg
    Serial.print("FREQ Apagado: "); 
    Serial.print(hz);
    Serial.print("\t = "); 
    Serial.print((hz+50)/100);  
    Serial.println(" mW/m2");
    if(hz > high){
        high = hz;
      }                 
      if(hz < low){
        low = hz;
      }
    suma += hz;
    oldcnt = t;
    contador++;
    if (contador==n_muestras){
      suma -= (low +high);
      mediaApagado = ((suma/(contador-2))+50)/100; //Conversion a potencia y media
      flag = true;
      Serial.print("Media Apagado: ");
      Serial.print(mediaApagado);  // +50 == rounding last digit
      Serial.println(" mW/m2");
      contador=0; 
      suma=0; 
      low = 1000000;
      high = 0;
      MediaTotal = mediaEncendido-mediaApagado;
      MediaTotal_final = MediaTotal*10000;
      
      //Calculo de la rectas
      
      if(MediaTotal > 109700)        {NTU = 15961*MediaTotal-16017900;}
      else if(MediaTotal > 99200)    {NTU = 9542*MediaTotal-8972800;}
      else                           {NTU = 3093*MediaTotal-2576300;}

      NTU_final = NTU/10000;
      Serial.print("NTU = ");
      Serial.println(NTU_final);
      GetEC();      
      Serial.print("EC25: ");
      Serial.print((EC25*1000000), 2);
      Serial.println(" uS/cm  ");
      Serial.print(ppm, 7);
      Serial.println(" ppm  ");
      Serial.print(Temperature);
      Serial.println(" *C ");
    }
    
  }
   

}
  void GetEC() {


  //*****************Lectura del sensor de temperatura *******************//
  sensors.requestTemperatures();// Send the command to get temperatures
  Temperature = sensors.getTempCByIndex(0); //Stores Value in Variable




  //********************Medida de la resistividad *************************//
  digitalWrite(ECPower, HIGH);
  raw = analogRead(ECPin); 
  raw = analogRead(ECPin); //Se hacen dos medidas pero escogemos la segunda
  digitalWrite(ECPower, LOW);




  //******************* Resistividad a conductividad ***********************//
  Vdrop = (Vin * raw) / 1024.0;
  Rc = (Vdrop * R1) / (Vin - Vdrop);
  Rc = Rc - Ra; //acounting for Digital Pin Resitance
  EC = K/Rc;


  //*************Compensating For Temperaure********************//
  EC25  =  EC / (1 + TemperatureCoef * (Temperature - 25.0));
  ppm = (EC25) * (PPMconversion * 1000);

}
// END OF FILE
