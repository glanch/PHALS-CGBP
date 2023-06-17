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
using StringerCosts = int;
using DueDate = OurTime;
using ProcessingTime = double;
using SetupTime = OurTime;

using std::tuple, std::map, std::pair;
class Instance
{
public:
   Coil startCoil;
   Coil endCoil;

   int numberOfCoils;
   int numberOfProductionLines;
   int numberOfModes;
   int maximumDelayedCoils;

   vector<string> comments;

   vector<ProductionLine> productionLines;
   vector<Coil> coils;

   vector<Mode> allModes;

   map<Coil, DueDate> dueDates;

   std::map<tuple<Coil, ProductionLine>, vector<Mode>> modes;
   map<tuple<Coil, ProductionLine, Mode>, ProcessingTime> processingTimes;
   map<tuple<Coil, Mode, Coil, Mode, ProductionLine>, SetupTime> setupTimes;
   map<tuple<Coil, Mode, Coil, Mode, ProductionLine>, StringerNeeded> stringerNeeded;
   map<tuple<Coil, Mode, Coil, Mode, ProductionLine>, StringerCosts> stringerCosts;

   void read(string nameFile);      // function to read data from a file
   void readPhals(string nameFile); // function to read data from a phals file

   void display(); // function to display the data

   bool IsStartCoil(Coil i);
   bool IsEndCoil(Coil i);
   bool IsRegularCoil(Coil i);
   void printStructured();
};