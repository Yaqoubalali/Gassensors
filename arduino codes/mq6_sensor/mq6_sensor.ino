/************************MQ6sensor************************************/
/************************Hardware Related Macros************************************/
#define         MQ6PIN                       (3)      //define which analog input channel you are going to use
#define         RL_VALUE_MQ6                 (1)      //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR_MQ6      (9.94)   //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                      //which is derived from the chart in datasheet

/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation

/**********************Application Related Macros**********************************/
#define         GAS_LPG                      (1)
#define         GAS_HYDROGEN                 (0)
#define         GAS_METHANE                  (2)
#define         GAS_CARBON_MONOXIDE          (3)
#define         GAS_ALCOHOL                  (4)
#define         accuracy                     (0)    //for linearcurves
//#define         accuracy                   (1)    //for nonlinearcurves, un comment this line and comment the above line if calculations 
                                                    //are to be done using non linear curve equations
/*****************************Globals***********************************************/
float           Ro = 10;                            //Ro is initialized to 10 kilo ohms

void setup() {
  Serial.begin(9600);                               //UART setup, baudrate = 9600bps
  Serial.print("Calibrating...\n");                
  Ro = MQCalibration(MQ6PIN);                       //Calibrating the sensor. Please make sure the sensor is in clean air 
                                                    //when you perform the calibration                    
  Serial.print("Calibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");

}

void loop() {

   Serial.print("LPG:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ6PIN)/Ro,GAS_LPG) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("HYDROGEN:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ6PIN)/Ro,GAS_HYDROGEN) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("METHANE:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ6PIN)/Ro,GAS_METHANE) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("CARBON_MONOXIDE:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ6PIN)/Ro,GAS_CARBON_MONOXIDE) );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("ALCOHOL:"); 
   Serial.print(MQGetGasPercentage(MQRead(MQ6PIN)/Ro,GAS_ALCOHOL) );
   Serial.print( "ppm" );
   Serial.print("\n");
   delay(200);
}

/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/ 
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE_MQ6*(1023-raw_adc)/raw_adc));
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use  
         MQResistanceCalculation to calculates the sensor resistance in clean air 
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about 
         10, which differs slightly between different sensors.
************************************************************************************/ 
float MQCalibration(int mq_pin)
{
  int i;
  float RS_AIR_val=0,r0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {                    //take multiple samples
    RS_AIR_val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  RS_AIR_val = RS_AIR_val/CALIBARAION_SAMPLE_TIMES;             //calculate the average value

  r0 = RS_AIR_val/RO_CLEAN_AIR_FACTOR_MQ6;                      //RS_AIR_val divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                                //according to the chart in the datasheet 

  return r0; 
}

/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/ 
float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;  
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function uses different equations representing curves of each gas to 
         calculate the ppm (parts per million) of the target gas.
************************************************************************************/ 
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{ 
  if ( accuracy == 0 ) {
  if ( gas_id == GAS_LPG ) {
    return (pow(10,((-2.351*(log10(rs_ro_ratio))) + 3.014)));
  } else if ( gas_id == GAS_HYDROGEN ) {
    return (pow(10,((-3.613*(log10(rs_ro_ratio))) + 4.962)));
  } else if ( gas_id == GAS_METHANE ) {
    return (pow(10,((-2.501*(log10(rs_ro_ratio))) + 3.341 )));
  } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
    return (pow(10,((-12.51*(log10(rs_ro_ratio))) + 14.21)));
  } else if ( gas_id == GAS_ALCOHOL ) {
    return (pow(10,((-5.885*(log10(rs_ro_ratio))) + 7.643)));
  }   
} 

  else if ( accuracy == 1 ) {
    if ( gas_id == GAS_LPG ) {
    return (pow(10,((-2.351*(log10(rs_ro_ratio))) + 3.014)));
  } else if ( gas_id == GAS_HYDROGEN ) {
    return (pow(10,(73.80*pow((log10(rs_ro_ratio)), 4) - 172.7*pow((log10(rs_ro_ratio)), 3) + 150.3*pow((log10(rs_ro_ratio)), 2) - 60.77*(log10(rs_ro_ratio)) + 12.87)));
  } else if ( gas_id == GAS_METHANE ) {
    return (pow(10,((-2.501*(log10(rs_ro_ratio))) + 3.341 )));
  } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
    return (pow(10,(-5659*pow((log10(rs_ro_ratio)), 4) + 20365*pow((log10(rs_ro_ratio)), 3) - 27456*pow((log10(rs_ro_ratio)), 2) + 16423*(log10(rs_ro_ratio)) - 3671)));
  } else if ( gas_id == GAS_ALCOHOL ) {
    return (pow(10,(-26.73*pow((log10(rs_ro_ratio)), 3) + 62.35*pow((log10(rs_ro_ratio)), 2) - 54.01*(log10(rs_ro_ratio)) + 19.93)));
  }
}    
  return 0;
}

