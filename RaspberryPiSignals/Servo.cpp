#include "Servo.h"
#include <cstdio>
#include <iostream>

#define SERVO_DEVICE "/dev/servoblaster"

void Servo::attach(int pin){
  this->pin = pin;
  // check that the servoblaster device exists.
  std::FILE *test = std::fopen(SERVO_DEVICE, "a");
  if(!test){
    std::perror("ServoBlaster device does not exist");
    exit( EXIT_FAILURE);
  }
  std::fclose(test);
}


void Servo::write(int angle){

  std::FILE *device = std::fopen(SERVO_DEVICE, "a");
  if(!device){
    std::perror("ServoBlaster device does not exist");
    exit( EXIT_FAILURE);
  }

  // calculate % rotation from angle, 0% = 0 degrees, 100%=180 degrees
  int percent = angle / 1.8;
  std::cout << "Setting servo #" << pin << " to " << percent << "%" << std::endl;
  std::fprintf(device,"%d=%d%%\n", pin, percent);
  std::fclose(device);
  
}

