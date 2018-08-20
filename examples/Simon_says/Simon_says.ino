#include <VMW100.h>

#define MAX_SEQUENCE_LENGTH 6
#define INPUT_RED 5
#define INPUT_GREEN 8
#define INPUT_BLUE 11
#define INPUT_YELLOW 2

Velleman_VMW100 watch = Velleman_VMW100();
Adafruit_NeoPixel strip;
char sequence[MAX_SEQUENCE_LENGTH];
char userSequence[MAX_SEQUENCE_LENGTH];
int step = 0;
void (*colors[4])();
boolean flag;
void setup()
{
	/*Serial.begin(9600);
	while(!Serial);*/
	pinMode(INPUT_RED,INPUT_PULLUP);
	pinMode(INPUT_GREEN,INPUT_PULLUP);
	pinMode(INPUT_YELLOW,INPUT_PULLUP);
	pinMode(INPUT_BLUE,INPUT_PULLUP);
	watch.setBrightness(25);
	strip = watch.getStrip();
	strip.begin();
	
	colors[0] = showRed;
	colors[1] = showYellow;
	colors[2] = showBlue;
	colors[3] = showGreen;
	//Black
	strip.clear();
	
	showRed();
	showYellow();
	showBlue();
	showGreen();
	strip.show();
	
}


void generateSequence()
{
	randomSeed(analogRead(0));
	for (int i = 0; i < MAX_SEQUENCE_LENGTH; i++)
	{
		sequence[i] = random(0,4);
	}
}
void loop()
{
	clearStrip();
	generateSequence();
	playSequence();
	delay(1000);
	getSequence();	
}

void getSequence()
{
	int previousStep = 0;
	step = 0;
	flag = true;
	boolean win = false;
	while(flag)
	{
		if(digitalRead(INPUT_BLUE) == LOW)
		{
			userSequence[step] = 2;
			step++;
			showBlue();
		}
		if(digitalRead(INPUT_YELLOW) == LOW)
		{
			userSequence[step] = 1;
			step++;
			showYellow();
		}
		if(digitalRead(INPUT_GREEN) == LOW)
		{
			userSequence[step] = 3;
			step++;
			showGreen();
		}
		if(digitalRead(INPUT_RED) == LOW)
		{
			userSequence[step] = 0;
			step++;
			showRed();
		}
		strip.show();
		if(previousStep < step)
		{
			previousStep = step;
			for(int i=0; i<step;i++)
			{
				/*Serial.print("User: ");
				Serial.println((int)userSequence[i]);
				Serial.print("Generated: ");
				Serial.println((int)sequence[i]);*/
				if(userSequence[i] != sequence[i])
				{
					flag = false;				
				}
				if(step == MAX_SEQUENCE_LENGTH)
				{
					win = true;
					flag = false;
				}
			}
		}
		//Serial.print("Step: ");
		//Serial.println(step);
		while(!(digitalRead(INPUT_BLUE) && digitalRead(INPUT_GREEN) && digitalRead(INPUT_RED) && digitalRead(INPUT_YELLOW))){
			delay(10);
		}
		clearStrip();
		delay(200);
		
	}
	win ? correctSequence() : wrongSequence();
	
}

void wrongSequence()
{
	for(int i=0;i<3;i++)
	{
		clearStrip();
		showAllColors();
		strip.show();
		delay(300);
		showAllColors();
		delay(300);
	}
}

void correctSequence()
{
	clearStrip();
	for(int i =0; i< 4;i++)
	{
		clearStrip();
		(*colors[i])();
		strip.show();
		delay(300);
	}
	delay(300);
	clearStrip();
	delay(300);
}
void playSequence()
{
	for(int i=0;i < MAX_SEQUENCE_LENGTH; i++)
	{
		(*colors[sequence[i]])();
		strip.show();
		delay(500);
		strip.clear();
		strip.show();
		delay(500);
	}	
}

void clearStrip()
{
	strip.clear();
	strip.show();
}

void showAllColors()
{
	showRed();
	showYellow();
	showGreen();
	showBlue();
}

void showRed()
{
	strip.setPixelColor(11,255,0,0);
	strip.setPixelColor(0,255,0,0);
	strip.setPixelColor(1,255,0,0);
	strip.setPixelColor(23,255,0,0);
	strip.setPixelColor(12,255,0,0);
	strip.setPixelColor(13,255,0,0);
}

void showYellow()
{
	strip.setPixelColor(3,255,255,0);
	strip.setPixelColor(2,255,255,0);
	strip.setPixelColor(4,255,255,0);
	strip.setPixelColor(15,255,255,0);
	strip.setPixelColor(16,255,255,0);
	strip.setPixelColor(14,255,255,0);
}

void showBlue()
{
	strip.setPixelColor(6,0,0,255);
	strip.setPixelColor(7,0,0,255);
	strip.setPixelColor(5,0,0,255);
	strip.setPixelColor(18,0,0,255);
	strip.setPixelColor(19,0,0,255);
	strip.setPixelColor(17,0,0,255);
}

void showGreen()
{
	strip.setPixelColor(9,0,255,0);
	strip.setPixelColor(10,0,255,0);
	strip.setPixelColor(8,0,255,0);
	strip.setPixelColor(22,0,255,0);
	strip.setPixelColor(21,0,255,0);
	strip.setPixelColor(20,0,255,0);
}