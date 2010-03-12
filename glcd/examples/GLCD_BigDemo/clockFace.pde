/*
 * clockAnalog.pde
 *
 * analog clock using the GLCD library
 *
 * uses pushbuttons to set the time
 * defualt connections are pins 2 and 3
 * connect one side of a button to a pin, the other side to ground
 * one button advances time forward, the other moves it back
 *
 */
 
#include <Time.h>    // download from: http://www.arduino.cc/playground/Code/Time
#include <glcd.h>    // Graphics LCD library   
#include "fonts/SystemFont5x7.h"       // system font
#include "AnalogClock.h"  // header file for the analog clock class

#define PULL_UP  HIGH

const int  btnForward = 31;  // buttons to set clock hands
const int  btnBack = 32;

AnalogClock analogClock = AnalogClock();  // this creates an instance of the analog clock display. 

void clockBegin(){
  //digitalWrite(btnForward, PULL_UP);  // enable internal pull-up resistors
  //digitalWrite(btnBack, PULL_UP);  

  setTime(10,20,0,1,1,10); // set time to 7:20 am Jan 1 2010  
}

//show clock for the given number of seconds
void  clock( int duration)
{  

  GLCD.SelectFont(System5x7);
  GLCD.ClearScreen();
  analogClock.Init(32,32,28);  // draw the clock face  
  unsigned long startTime = now() ;   
  unsigned long  prevtime;
  while( now() < startTime + duration )
  {
    analogClock.DisplayTime(hour(), minute(), second() ); // update analog clock  
    prevtime = now(); 
    while( prevtime == now() )
    {
      // check if set buttons pressed while waiting for second to rollover
      //checkSetButton(btnForward, 1);    
      //checkSetButton(btnBack, -1);
    }    
  }
}


void checkSetButton(int button, int direction)
{
const int stepSecs[] = {1,30,300};  // values used to accelerate movement when button held down
int stepIndex = 0; // the index to determine accelartion value
int step = 0; // counts steps between each index increment

  if(digitalRead(button)== LOW)
  {
     delay(50);  // debounce time     
     unsigned long startTime = millis() + 1000;
     while(digitalRead(button)== LOW)
     {
       if(millis() - startTime > 100){
         startTime = millis();
         adjustTime(stepSecs[stepIndex] * direction);
         analogClock.DisplayTime(hour(), minute(), second() ); // update analog clock  
         if( ++step > 30){
            step=0;
            if(stepIndex < 2){
               stepIndex++; 
            }
         }        
       }
     } 
  }   
}






