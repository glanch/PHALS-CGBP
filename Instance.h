#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

using OurTime = int;

using Coil = int;
using ProductionLine = int;
using Mode = int;
using StringerNeeded = int;
using DueDate = OurTime;
using MachiningTime = OurTime;
using SetupTime = OurTime;

class Instance
{
public:
   int numberOfCoils;
   int numberOfProductionLines;

   int maximumDelayedCoils;
   map<Coil, DueDate> dueDate;

   map<pair<Coil, ProductionLine>, vector<Mode>> modes;
   map<tuple<Coil, ProductionLine, Mode>, ProcessingTime> processingTimes;
   map<tuple<Coil, Mode, Coil, Mode, ProductionLine>, SetupTime> setupTimes;
   map<tuple<Coil, Mode, Coil, Mode, ProductionLine>, StringerNeeded> stringerNeeded;

   void read(string nameFile); // function to read data from a file
   void display(); // function to display the data
};