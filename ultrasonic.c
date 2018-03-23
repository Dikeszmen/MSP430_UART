#define VOICESPEEDHALF 0.034/2
#define CRITIC 5

int main(void)
{
const int trigPin=12;
const int echoPin=11;
const int RED=2;
const int GREEN=14;
long duration;
int distance;
pinMode(trigPin,OUTPUT);
pinMode(echoPin,INPUT);
pinMode(RED,OUTPUT);
pinMode(GREEN,OUTPUT);

  Serial.begin(9600);
  digitalWrite(RED,LOW);
  digitalWrite(GREEN,HIGH);
  while(1)
  {
  digitalWrite(trigPin,LOW);
  
  delayMicroseconds(2);

  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  
  duration=pulseIn(echoPin,HIGH);
  
  distance=duration*VOICESPEEDHALF;

  if(distance<=CRITIC)
  {
    digitalWrite(RED,HIGH);
    digitalWrite(GREEN,LOW);
  }
  else
  {
    digitalWrite(RED,LOW);
    digitalWrite(GREEN,HIGH);  
  }
  
  Serial.print("Distance: ");
  Serial.println(distance);
   
  }
  return 0;
}
  