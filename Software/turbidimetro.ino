
#define TSL_FREQ  2
#define IR_LED    3

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
      //Calculo de las rectas
      
      if(MediaTotal > 109700)        {NTU = 15961*MediaTotal-16017900;}
      else if(MediaTotal > 99200)    {NTU = 9542*MediaTotal-8972800;}
      else                           {NTU = 3093*MediaTotal-2576300;}

      NTU_final = NTU/10000;
      Serial.print("NTU = ");
      Serial.println(NTU_final);
    }
    
  }
   
   
}
// END OF FILE
