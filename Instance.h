#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

using OurTime = int;

using Coil = int;
using ProductionLine = int;
using Mode = int;
using StringerNeeded = bool;
using DueDate = OurTime;
using ProcessingTime = OurTime;
using SetupTime = OurTime;

using std::tuple, std::map, std::pair;
class Instance
{
public:
   int numberOfCoils;
   int numberOfProductionLines;

   int maximumDelayedCoils;

   vector<ProductionLine> productionLines;
   vector<Coil> coils;

   map<Coil, DueDate> dueDates;

   std::map<tuple<Coil, ProductionLine>, vector<Mode>> modes;
   map<tuple<Coil, ProductionLine, Mode>, ProcessingTime> processingTimes;
   map<tuple<Coil, Mode, Coil, Mode, ProductionLine>, SetupTime> setupTimes;
   map<tuple<Coil, Mode, Coil, Mode, ProductionLine>, StringerNeeded> stringerNeeded;

   void read(string nameFile); // function to read data from a file
   void display(); // function to display the data
};