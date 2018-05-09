// Assign your channel in pins
#define MAIN_IN_PIN 2
#define ORANGE_IN_PIN 3

#define MAIN_LIGHT_FLAG 1
#define ORANGE_LIGHT_FLAG_FLAG 2

#define GREEN_LIGHT 12
#define RED_LIGHT 9
#define WHITE_NAV_LIGHT 10
#define WHITE_LIGHT 11
#define ORANGE_LIGHT 8

// holds the update flags defined above
volatile uint8_t bUpdateFlagsShared;

volatile uint16_t mainLightInShared;
volatile uint16_t orangeLightInShared;

uint32_t ulMainLightStart;
uint32_t ulOrangeLightStart;

#define IDLE_MAX 50

unsigned long pulse_time;

void setup()
{
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  pinMode(GREEN_LIGHT, OUTPUT);
  pinMode(RED_LIGHT, OUTPUT);
  pinMode(WHITE_NAV_LIGHT, OUTPUT);
  pinMode(WHITE_LIGHT, OUTPUT);
  pinMode(ORANGE_LIGHT, OUTPUT);
  
  Serial.begin(9600);

  Serial.println("hello");
  
  attachInterrupt(digitalPinToInterrupt(MAIN_IN_PIN) ,calcMainLight, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ORANGE_IN_PIN) ,calcOrangeLight, CHANGE);
  pulse_time =millis() ;
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Setup done");
}

void loop()
{
  static uint16_t mainLightIn;
  static uint16_t orangeLightIn;
  static uint8_t bUpdateFlags;

  if(bUpdateFlagsShared)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    //Serial.println("Detected change"); // Do the serial write outside the "NoInterupt" section
    
    noInterrupts(); 
    pulse_time =millis() ;
    bUpdateFlags = bUpdateFlagsShared;


    if(bUpdateFlags & MAIN_LIGHT_FLAG)
    {
      mainLightIn = mainLightInShared;
    }

    if(bUpdateFlags & ORANGE_LIGHT_FLAG_FLAG)
    {
      orangeLightIn = orangeLightInShared;
    }
    bUpdateFlagsShared = 0;

    interrupts();
  }

  if(bUpdateFlags & MAIN_LIGHT_FLAG)
  {
    if (mainLightIn < 1250) {
      Serial.println("Main < 1250");
      // All lights off
      digitalWrite(GREEN_LIGHT, LOW);
      digitalWrite(RED_LIGHT, LOW);
      digitalWrite(WHITE_NAV_LIGHT, LOW);
      digitalWrite(WHITE_LIGHT, LOW);
    }
    else if (mainLightIn >= 1250 && mainLightIn <= 1750) 
    {
      Serial.println("Main middle");
      // Only nav lights on
      digitalWrite(GREEN_LIGHT, HIGH);
      digitalWrite(RED_LIGHT, HIGH);
      digitalWrite(WHITE_NAV_LIGHT, HIGH);
      digitalWrite(WHITE_LIGHT, LOW);
    }
    else if (mainLightIn > 1750 && mainLightIn <= 2250) 
    {
      Serial.println("Main > 1750 < 2250");
      digitalWrite(GREEN_LIGHT, HIGH);
      digitalWrite(RED_LIGHT, HIGH);
      digitalWrite(WHITE_NAV_LIGHT, HIGH);
      digitalWrite(WHITE_LIGHT, HIGH);
    }
    else {
      Serial.println("Wrong?");
      // Something went wrong? 
      // Value should be between 1000 & 2000 
      // but it wasn't :(
      // I guess we power off all lights?
      digitalWrite(GREEN_LIGHT, LOW);
      digitalWrite(RED_LIGHT, LOW);
      digitalWrite(WHITE_NAV_LIGHT, LOW);
      digitalWrite(WHITE_LIGHT, LOW);
    }
   
  }

  if(bUpdateFlags & ORANGE_LIGHT_FLAG_FLAG)
  {
    if (orangeLightIn <= 1500) {
      Serial.println("Orange off");
      digitalWrite(ORANGE_LIGHT, LOW);
    }
    else {
      Serial.println("Orange on");
      digitalWrite(ORANGE_LIGHT, HIGH);
    }
  }
  
  bUpdateFlags = 0;
  
  digitalWrite(LED_BUILTIN, LOW);
}


// simple interrupt service routine
void calcMainLight()
{
  // if the pin is high, its a rising edge of the signal pulse, so lets record its value
  if(digitalRead(MAIN_IN_PIN) == HIGH)
  {
    ulMainLightStart = micros();
  }
  else
  {
    // else it must be a falling edge, so lets get the time and subtract the time of the rising edge
    // this gives use the time between the rising and falling edges i.e. the pulse duration.
    mainLightInShared = (uint16_t)(micros() - ulMainLightStart);
    // use set the throttle flag to indicate that a new throttle signal has been received
    bUpdateFlagsShared |= MAIN_LIGHT_FLAG;
  }
}

void calcOrangeLight()
{
  if(digitalRead(ORANGE_IN_PIN) == HIGH)
  {
    ulOrangeLightStart = micros();
  }
  else
  {
    orangeLightInShared = (uint16_t)(micros() - ulOrangeLightStart);
    bUpdateFlagsShared |= ORANGE_LIGHT_FLAG_FLAG;
  }
}

