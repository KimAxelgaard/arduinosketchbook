/* 
add a higher load
add a way of switching lower supply voltage
*/
#define AREF 3300.0
/* h2mdk defs */
    static const int PURGE = 3;
    static const int LOAD = 4;  // 3w can disconnect load
    static const int SHORT = 5;
    static const int STATUS_LED = 6;

//analog pins
    static const int VOLTAGE_SENSE = A1;
    static const int CURRENT_SENSE = A2;
    static const int CAP_V_SENSE = A3;

/* extra defs */

    static const int supplyCurrent= A0;
    static const int outputVoltage = A4;
    
    static const int connectSupply = 7;
    static const int connectLoad1 = 8;
    
    
void setup()
{
  Serial.begin(9600);
  Serial.println( "started" );

  pinMode( connectSupply, OUTPUT );
  pinMode( connectLoad1, OUTPUT );
  pinMode(PURGE,OUTPUT);
  pinMode(LOAD,OUTPUT);
  pinMode(SHORT,OUTPUT);
  pinMode(STATUS_LED,OUTPUT);

  allOff();
  analogReference(EXTERNAL);
}


void loop()
{
  if( Serial.available() )
  {
    char command = Serial.read();
    switch( command )
    {
      case '2': //check op is 5v when we apply power, and supply current is < 0.75
        Serial.println( "test 2" );
        digitalWrite( LOAD, HIGH );
        externalMosfet( connectSupply, true );
        externalMosfet( connectLoad1, false );
        delay(500);
        for( int i = 0; i < 5 ; i ++)
        {
          float v= measureOutputVoltage();
          float i = measureSupplyCurrent();
          if( v < 4500 )
          {
            Serial.println( F( "fail - voltage too low")  );
          }
          else if(i > 0.75)
          {
            Serial.println( F("fail - current too high") );
          }
          else if( i < 0)
          {
            Serial.println( F("fail - current too low, check ref voltage") );
          }
          else
          {
            Serial.println( F("pass" ) );
          }

        }
      //  allOff();
        break;
      case '4': //check output current and voltage with a load attached
        Serial.println( "test 4" );
        chargeCaps();
        digitalWrite( LOAD, HIGH );
        externalMosfet( connectSupply, true );
        externalMosfet( connectLoad1, true );
        delay(500);
        for( int i = 0; i < 5 ; i ++)
        {
          if( measureOutputVoltage() > 4500 && measureSupplyCurrent() > 0.6 )
          {
            Serial.println( "pass" );
          }
          else
          {
            Serial.println( "fail" );
          }
        }
        allOff();
        break;
      case '8': //solenoid
        Serial.println( "solenoid" );
        chargeCaps();
        digitalWrite( LOAD, HIGH );
        externalMosfet( connectSupply, true );
        if( measureSupplyCurrent() > 0.2 )
        {
          Serial.println( "supply current too high" );
          break;
        }
        for( int i = 0; i < 5 ; i ++ )
        {
          digitalWrite( PURGE, HIGH );
          delay(50);
          if( measureSupplyCurrent() > 0.5 )
          {
            Serial.println("pass");
          }
          else
          {
            Serial.println("fail");
          }
          digitalWrite( PURGE, LOW );
          delay(50);
        }
        break;
      case '9': //short
        Serial.println( "solenoid" );
        chargeCaps();
        digitalWrite( LOAD, HIGH );
        externalMosfet( connectSupply, true );
        if( measureSupplyCurrent() > 0.2 )
        {
          Serial.println( "supply current too high" );
          break;
        }
        for( int i = 0; i < 5 ; i ++ )
        {
          digitalWrite( SHORT, HIGH );
          delay(50);
          if( measureSupplyCurrent() > 1.4 )
          {
            Serial.println("pass");
          }
          else
          {
            Serial.println("fail");
          }
          digitalWrite( SHORT, LOW );
          delay(50);
        }
        break;
      case 'c':
        drainCaps();
      default:
        Serial.println( "bad test" );
        break;
    }
  }
}

void allOff()
{
  digitalWrite( LOAD, LOW );
  externalMosfet( connectSupply, false );
  externalMosfet( connectLoad1, false );
}
void externalMosfet( int pin, boolean state )
{
  digitalWrite( pin, ! state );
}

void chargeCaps()
{
  Serial.println( "charge caps" );
  digitalWrite( LOAD, LOW );
  externalMosfet( connectSupply, true );
  externalMosfet( connectLoad1, false );
  //externalMosfet( connectLoad2, false );
  int count = 0;
  while( measureCapVoltage() < 4500) //todo
  {
    delay(100);
    if( count ++ > 1000 )
      break;
  }
}

void drainCaps()
{
  Serial.println( "drain caps" );
  digitalWrite( LOAD, HIGH );
  externalMosfet( connectSupply, false );
  externalMosfet( connectLoad1, true );
  //externalMosfet( connectLoad2, true );
  while( measureCapVoltage() > 3000 ) //todo
  {
    delay(100);
  }
}

float measureCapVoltage()
{
  float v = 2*AREF/1024.0*analogRead(CAP_V_SENSE) ;
  Serial.print( "cap v: ");
  Serial.println( v );
  return v;
}
float measureOutputVoltage()
{
  float v = 2*AREF/1024.0*analogRead(outputVoltage) ;
  Serial.print( "output v: ");
  Serial.println( v );
  return v;
}
float measureSupplyCurrent()
{
  float v = AREF/1024.0*analogRead(supplyCurrent) ;
  float i =( v - 5000 / 2 ) / 185; //185mv per amp
  Serial.print( "supply i: ");
  Serial.println( i );
  return i;
}
