#include <Servo.h> // include servo library

Servo lock;
int mic = 2, servo = 3; // mic and servo pins
float inDelay = 0.12; // minimum time between knocks (in seconds)
float threshold = 0.40; // required accuracy (as a portion of a beat)
float beat; // average time between knocks divided by pattern numbers
unsigned long inTime = 0; // time of last input

unsigned int pattern[] = { 2, 1, 1, 2, 4, 2 }; // selected pattern (by time between knocks)
unsigned long times[sizeof(pattern)/2+1]; // time of knocks

void setup()
{
  pinMode(mic, INPUT);
  lock.attach(servo);
  Lock(true); // lock

  // set all time values to 0
  for(int i=0; i<sizeof(times)/4; i++)
  {
    times[i] = 0;
  }
  
  Serial.begin(9600);
}

void loop()
{
  if(digitalRead(mic) && millis() - inTime >= inDelay * 1000) // check if knock is recognized and enough time has passed since the last knock
  {
    inTime = millis(); // set time of last input
    
    bool set = false;
    for(int i=0; i<sizeof(times)/4; i++) // check if the time array is full
    {
      if(times[i] == 0)
      {
        times[i] = millis();
        set = true;
        break;
      }
    }
    if(set == false) // check if time array is full
    {
      // move array values to fit in a new one
      for(int i=0; i<sizeof(times)/4-1; i++)
      {
        times[i] = times[i+1];
      }
      times[sizeof(times)/4-1] = millis();

      // calculate total of times between knocks divided by pattern times
      float sum = 0.0;
      for(int i=0; i<sizeof(pattern)/2; i++)
      {
        sum += (times[i+1] - times[i])/pattern[i];
      }
      beat = sum/(sizeof(pattern)/2); // calculate average time of a "1" in pattern

      // print data in serial monitor
      Serial.print(beat);
      Serial.print(" --> ");
      for(int i=0; i<sizeof(pattern)/2; i++)
      {
        if(i != 0)
        {
          Serial.print(" - ");
        }
        Serial.print(abs(((times[i+1]-times[i])/pattern[i]-beat)/beat));
      }
      Serial.println();

      // check if time between knocks is close enough to selected pattern
      bool correct = true;
      for(int i=0; i<sizeof(pattern)/2; i++)
      {
        if(abs(((times[i+1]-times[i])/pattern[i]-beat)/beat) > threshold)
        {
          correct = false;
          break;
        }
      }

      if(correct) // correct pattern
      {
        Lock(false); // unlock
      }
      else // incorrect pattern
      {
        // calculate average time between knocks
        sum = 0.0;
        int l = min(sizeof(pattern)/2, 4); // set value to either pattern length or 4, based on which is smaller
        for(int i=0; i<l; i++)
        {
          sum += (times[(sizeof(pattern)/2-l)+i+1] - times[(sizeof(pattern)/2-l)+i]);
        }
        float avg = sum/l;

        // check if time between each knock is the same
        bool lockSeq = true;
        for(int i=0; i<l; i++)
        {
          if(abs(((times[(sizeof(pattern)/2-l)+i+1]-times[(sizeof(pattern)/2-l)+i])-avg)/avg) > threshold)
          {
            lockSeq = false;
            break;
          }
        }

        // if time between each knock is the same, lock the box
        if(lockSeq)
        {
          Lock(true); // lock
        }
      }
    }
  }
}

void Lock(bool lLock) // true = lock, false = unlock
{
  if(lLock)
  {
    lock.write(45); // servo position when locked
  }
  else
  {
    lock.write(120); // servo position when unlocked
  }
}
