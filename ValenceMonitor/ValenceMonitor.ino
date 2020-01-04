//Definitions


//Other Functions will reference these tokens to send, recieve, parse and display each data field
const int Voltage =  0;
const int Current =  1;
const int SOC    =   2;


byte input_line[32] = { 0 };



//Data Request messages
const byte send_message[3][10] = {
  {0x01, 0x03, 0x00, 0x45, 0x00, 0x09, 0x94, 0x19, 0x0d, 0x0a},
  {0x01, 0x03,  0x00, 0x6a, 0x00, 0x0c, 0x65, 0xd3, 0x0d, 0x0a},
  {0x01, 0x03, 0x00, 0x39, 0x00, 0x0a, 0x15, 0xc0, 0x0d, 0x0a}
};

//Message Parsing Instruction
//start byte, bytes to read
//this isnt really used other than start position because all data are 2 byte words and that is how the parser is hardcoded. 

const int message_data[3][2] = {
  { 7, 2},
  { 17, 2},
  { 3, 2}
};

//global variable to store data
float batt_voltage;
float batt_current;
float batt_soc;
float prev_batt_voltage;
float prev_batt_current;
float prev_batt_soc;

int CharTimeout = 1000;
int CommTimeout = 10000;

//voltage test hex reply 01 03 12 0c d1 0c ce 33 2f 0c cf 0c ce 0c d0 0c d1 88 00 44 20 d6 85 0d 0a
//current test hex reply 01 03 14 02 ac 01 cd 90 ab c2 b2 6c 2e 95 74 3f FA ff ce 02 79 33 5a d1 77 0d 0a
//soc test hex reply     01 03 18 03 D4 00 00 00 00 00 00 79 b3 62 eb 6c 2f cb 2f 00 04 00 00 00 01 00 01 1b 17 0d 0a


void setup() {

  Serial.begin(115200, SERIAL_8N1);
  Serial1.begin(9600, SERIAL_8N2);
  //Serial.println("Sending Init Message");
  byte start_message[] = {0x00, 0x00, 0x01, 0x01, 0xc0, 0x74, 0x0d, 0x0a, 0x00, 0x00 };
  Serial1.write(start_message, sizeof(start_message));
  
  incoming_line();
  //Serial.println("RX Init Message");
  delay(1000);
    
  Serial1.flush();
  Serial1.begin(115200, SERIAL_8N2);
  while (Serial1.available()) Serial1.read();
  delay(1000);
}

void loop() {
  for (int i = Voltage; i <= SOC; i++) {
    update_param(i);
    delay(10000);
  }
}

void update_param(int item) {
  Serial1.write(send_message[item], sizeof(send_message[item]));
  incoming_line();
  process_data(item);
  
}






void process_data (int item)
{
  //Convert to a 2 byte signed long
  //Could make this looping based on the array defninig the bytes of interest ( if more than 2)
  long output_value = (input_line[message_data[item][0]]<<8)|(input_line[message_data[item][0] + 1]);

  switch (item)
  {
    case Voltage:
      batt_voltage = float(output_value) / 1000.000;
      //Serial.print(batt_voltage);
      break;
    case Current:
      batt_current = float(output_value) / 10.0;
      //Serial.print(batt_current);
      break;
    case SOC:
      batt_soc = float(output_value) / 10.0;
      //Serial.print(batt_soc);
      break;

  }
}

void incoming_line()
{
  //Serial.println("Incoming Line");
  uint32_t  char_timer = millis();
  uint32_t  comm_timer = millis();
  memset(input_line, 0, sizeof(input_line));
  unsigned int input_pos = 0;
  bool string_complete = 0;
  bool string_started = 0;
  while (1)
  {
    if ( Serial1.available() > 0)
    {
      Serial.print(Serial1.available());
      //Serial.println("Started");
      string_started = 1;
      char_timer = millis();
      byte incoming_byte = Serial1.read();
      Serial.write(incoming_byte);
      input_line[input_pos++] = incoming_byte;
      switch (incoming_byte)
      {
        case '\n':
          string_complete = 1;
          break;

        default:

          break;
      }
    }
    else if ((millis() - char_timer > CharTimeout) && string_started )
    {
      //Serial.print("char timeout");
      break;
    }
    else if (millis() - comm_timer > CommTimeout)
    {
      //Serial.print("comm timeout");
      break;
    }
    else if (string_complete)
    {
      break;
    }
  }
}
