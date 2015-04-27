/*    CONTROLADOR DE TEMPERATURA V1.0
Esto se usa mas o menos asi: en la primera parte hay tres arrays (variable con varios valores dentro), llamados "durante", "Tmin" y "Tmax". Entre llaves, hay que poner los valores deseados, en horas de cada etapa de fermentacion en el primero, y las temperaturas limite de cada etapa.
Ejemplo:
Si quiero que pasen dos dias (48hs.) entre 18 y 20; luego cuatro dias (96hs.) entre 20 y 22 y por ultimo tres dias (72hs.) entre 22 y 26, tengo que escribir:

int durante[] {48, 96, 72};
float Tmin[] {18, 20, 22};
float Tmax[] {20, 22, 26};

Pueden usarse cuantas etapas se quieran, pongo tres como base pero pueden ser las que se quiera. Por las dudas (el programa no sabe muy bien que hacer terminadas las etapas), recomiendo poner un numero delirante de horas en la ultima, tipo 1000 horas (como un perro...).

... todo lo demas ya es el programa en si; trato de comentar todo para que se entienda y se pueda mejorar, pero si no quiere hacer macanas, no toque mas que esos valores.

El controlador va a evaluar la temperatura CADA MINUTO y en caso de pasarse de los limites por debajo o por arriba va a prender un enfriador en el Relay 1 y un calentador en el Relay 2, respectivamente.
Ademas, en caso de no estar sirviendo el metodo de calentamiento/enfriado (por ej.: un fermbox con el hielo descongelado) va a titilar el led interno del Arduino durante todo ese minuto, o minutos si es que el problema continua.

Cualquier cosa, comunicarse conmigo en la lista de Somos Cerveceros o en la de Electrocerveceros...

Salud!!!
*/

int durante[] {72, 96, 72};
float Tmin[] {18, 20, 22};
float Tmax[] {20, 22, 26};

#include <OneWire.h>                  //Se añaden las bibliotecas necesarias para el funcionamiento del sensor de temperatura.
#include <DallasTemperature.h>        // Idem.

#define Sensor 2                     //Se declara el pin donde se conectará el Sensor.
#define led 13                       //El led que viene con el Arduino esta siempre en el pin 13, cambiar si se pone un led aparte.
#define Rele1 3                      //Declara cual es el pin para controlar el primer relay...
#define Rele2 4                      //...y el otro.

OneWire ourWire(Sensor);             //Se establece el pin declarado como bus para la comunicación OneWire, la que usa el sensor.
DallasTemperature sensors(&ourWire); //Se instancia la bibliotecas DallasTemperature, para interpretar la informacion del sensor como algo entendible.

int minutos = 0;                      //Vamos a usar esta variable para contar los minutos que lleva andando el programa en cada etapa.
int etapa = 0;                        //Vamos a usar esta variable para contar las etapas de fermentacion (la primera es la "cero").
float Tanterior = 0;                  //Vamos a usar esta variable para guardar una lectura de temperatura para poder compararla con la siguiente.
boolean primera = true;                //Vamos a usar esta variable de verdadero-falso para evitar una falsa alarma la primera vez que se prende alguno de los relays.

void setup() {
  pinMode(Rele1, OUTPUT);             //Definimos los pines de los Reles y el led como "salidas".
  pinMode(Rele2, OUTPUT);             //Idem.
  pinMode(led, OUTPUT);               //Idem.
  digitalWrite(Rele1,HIGH);           //...y hacemos que arranquen apagados (por alguna razon, en los reles quedo al reves, "HIGH" es apagado y viceversa)
  digitalWrite(Rele2,HIGH);           //Idem.
  digitalWrite(led,LOW);              //Idem.
  delay(1000);                        //Esperamos un segundito... (esto estaba en el primero que baje, lo dejo por las dudas)
  Serial.begin(9600);                 //Comienza a comunicarse con la computadora (si hay una conectada) a 9600 de frecuencia.
  sensors.begin();                    //Se inicia el sensor.
}

// A partir de aca empieza el "loop", o sea, el programa que se ejecuta cada un minuto.
void loop() {
  //Pide la lectura de temperatura al sensor (no es en tiempo real: hay que pedirla con esto cada vez).
  sensors.requestTemperatures();

  // Si los minutos que lleva el programa andando, completan las horas de la primera etapa...
  if (durante[etapa] == minutos / 60)
  {
    // ...entonces avanza a la etapa siguiente en el contador de etapas de fermentacion.
    etapa++;
    // y reinicia el cuenta-minutos.
    minutos = 0;
    Serial.println("Pasando a la siguiente etapa");
  }

  //Se imprime la temperatura en el monitor serie de la computadora...
  Serial.print(sensors.getTempCByIndex(0));
  //...junto con un espacio y la aclaracion de la unidad de medicion.
  Serial.println(" grados C");


  // Ahora selecciona entre tres opciones: temperatura mas caliente, mas fria, o en rango (dentro de la etapa); y actua en consecuencia, prendiendo relays y avisando por el serial.
  if (sensors.getTempCByIndex(0) >= Tmax[etapa])
  {
    digitalWrite(Rele1,LOW);
    digitalWrite(Rele2,HIGH);
    Serial.println("Enfriador encendido");
  }
  else if (sensors.getTempCByIndex(0) <= Tmin[etapa])
  {
    digitalWrite(Rele2,LOW);
    digitalWrite(Rele1,HIGH);
    Serial.println("Calentador encendido");
  }
  else
  {
    digitalWrite(Rele1,HIGH);
    digitalWrite(Rele2,HIGH);
    Serial.println("Temperatura en rango");
  }

  // Por ultimo, toma la informacion de los reles y la temperatura anterior para evaluar si emitir un alerta con el led y en la computadora o solo hacer pasar el minuto hasta la proxima.
  //Si el Enfriador esta prendido pero la temperatura actual es mayor o igual que la anterior...
  if (digitalRead(Rele1) == LOW && sensors.getTempCByIndex(0) >= Tanterior)
  {
    //...pero es la primera vez que pasa...
    if (primera == true)
    {
      //...entonces deja pasar y falsea el "primera" para la proxima
      primera = false;
      delay(60000);
    }
    else
    {
      // Si no es la primera, entonces avisa que anda algo mal, y...
      Serial.println("El enfriador no esta funcionando correctamente");
      // ... ademas inicia un loop de lucecitas prendiendo y apagando cuatro veces por segundo, por un minuto en total.
      for (int blips = 0; blips < 240; blips++)
      {
        digitalWrite(led,HIGH);
        delay(125);
        digitalWrite(led,LOW);
        delay(125);
      }
    }
  }
  //Sigue exactamente lo mismo pero para el calentador (otro relay y cambio de signo).
  else if (digitalRead(Rele2) == LOW && sensors.getTempCByIndex(0) <= Tanterior)
  {
    if (primera == true)
    {
      primera = false;
      delay(60000);
    }
    else
    {
      Serial.println("El calentador no esta funcionando correctamente");
      for (int blips = 0; blips < 240; blips++)
      {
        digitalWrite(led,HIGH);
        delay(125);
        digitalWrite(led,LOW);
        delay(125);
      }
    }
  }
  else
  {
    //Vuelve a true para que no active en la primera que detecta.
    primera = true;
    //Y como no hay nada que alertar, deja pasar un minuto y listo.
    delay(60000);
  }
  //Finalmente, guarda la temperatura anotada (sigue la misma aunque pase el minuto) como "Tanterior".
  Tanterior = sensors.getTempCByIndex(0);
  //Y suma un minuto a los recorridos.
  minutos++;
}
