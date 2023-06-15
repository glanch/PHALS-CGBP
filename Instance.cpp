#include "Instance.h"
#include <numeric>

void Instance::read(string nameFile)
{
   ifstream infile(nameFile);
   if (!infile)
      throw std::runtime_error("Cannot read file");

   string line;

   while (getline(infile, line))
   {
      // if this line is empty, skip it
      if(line.length() == 0){
         continue;
      }

      istringstream ss(line);

      char par_name;

      ss >> par_name;

      switch (par_name) // Instead of using multiple if-statements, we use a switch statement: if(par_name == '')
      {
      case 'X':
      {
         string comment;
         std::getline(ss, comment);

         comment.erase(0, 1); // remove leading space due to not being removed because not reading from ss using >>
         this->comments.push_back(comment);

         break;
      }
      case 'I': // read number of coils
      {
         ss >> this->numberOfCoils;

         this->coils.resize(this->numberOfCoils);
         std::iota(std::begin(this->coils), std::end(this->coils), 0); // coils are indexed by 0,...,I-1
         this->startCoil = -1;                                         //this->numberOfCoils;
         this->endCoil = this->numberOfCoils;                          // + 1;

         break;
      }
      case 'M':
      {
         ss >> this->numberOfModes;
         this->allModes.resize(this->numberOfModes);
         std::iota(std::begin(this->allModes), std::end(this->allModes), 0); // coils are indexed by 0,...,I-1
      }

      case 'K': // read number of production lines
      {
         ss >> this->numberOfProductionLines;

         this->productionLines.resize(this->numberOfProductionLines);
         std::iota(this->productionLines.begin(), this->productionLines.end(), 0); // production lines are indexed by 0,...,K-1

         break;
      }

      case 'a': // read max number of delayed coils
      {
         ss >> this->maximumDelayedCoils;
         break;
      }
      case 'm': // read number of modes per coil and production line
      {
         Coil coil;
         ProductionLine line;
         Mode mode;

         int numberOfModes;
         bool modeEnabled;

         ss >> coil;
         ss >> line;
         ss >> mode;
         ss >> modeEnabled;

         if (modeEnabled)
            this->modes[make_tuple(coil, line)].push_back(mode);

         break;
      }
      case 'd': // read due date of coil
      {
         Coil coil;
         DueDate dueDate;

         ss >> coil;
         ss >> dueDate;

         this->dueDates[coil] = dueDate;
         break;
      }
      case 'p': // read processing time
      {
         Coil coil;
         ProductionLine line;
         Mode mode;
         ProcessingTime time;

         ss >> coil;
         ss >> line;
         ss >> mode;
         ss >> time;

         this->processingTimes[make_tuple(coil, line, mode)] = time;
         break;
      }
      case 't': // read setup time
      {
         Coil coil1;
         Mode mode1;
         Coil coil2;
         Mode mode2;
         ProductionLine line;
         SetupTime time;

         ss >> coil1;
         ss >> coil2;
         ss >> line;
         ss >> mode1;
         ss >> mode2;
         ss >> time;

         this->setupTimes[make_tuple(coil1, mode1, coil2, mode2, line)] = time;

         break;
      }
      case 'c': // read stringer infos
      {
         Coil coil1;
         Mode mode1;
         Coil coil2;
         Mode mode2;
         ProductionLine line;
         StringerNeeded stringerNeeded = true;
         StringerCosts stringerCosts;

         ss >> coil1;
         ss >> coil2;
         ss >> line;
         ss >> mode1;
         ss >> mode2;
         ss >> stringerCosts;

         auto tuple = make_tuple(coil1, mode1, coil2, mode2, line);

         this->stringerNeeded[tuple] = stringerNeeded;
         this->stringerCosts[tuple] = stringerCosts;

         break;
      }
         // if no of the key-chars is at the beginning, ignore the whole line and do nothing
      }
   }

   // initialize mode index sets for start and end coil: both can be produced on every line with every mode
   for (auto &line : this->productionLines)
   {
      modes[make_tuple(this->startCoil, line)] = {0}; //this->allModes;
      modes[make_tuple(this->endCoil, line)] = {0}; //this->allModes;
   }

   // add both to coils
   this->coils.push_back(this->startCoil);
   this->coils.push_back(this->endCoil);

   infile.close();
}

void Instance::readPhals(string nameFile)
{
   ifstream infile(nameFile);
   if (!infile)
      throw std::runtime_error("Cannot read file");

   string line;

   while (getline(infile, line))
   {
      istringstream ss(line);

      char par_name;

      ss >> par_name;

      switch (par_name) // Instead of using multiple if-statements, we use a switch statement: if(par_name == '')
      {
      case 'I': // read number of coils
      {
         ss >> this->numberOfCoils;

         this->coils.resize(this->numberOfCoils);
         std::iota(std::begin(this->coils), std::end(this->coils), 0); // coils are indexed by 0,...,I-1
         break;
      }

      case 'K': // read number of production lines
      {
         ss >> this->numberOfProductionLines;

         this->coils.resize(this->numberOfProductionLines);
         std::iota(std::begin(this->productionLines), std::end(this->productionLines), 0); // production lines are indexed by 0,...,K-1

         break;
      }

      case 'd': // read max number of delayed coils
      {
         ss >> this->maximumDelayedCoils;
         break;
      }
      case 'M': // read number of modes per coil and production line
      {
         Coil coil;
         ProductionLine line;
         int numberOfModes;

         ss >> coil;
         ss >> line;
         ss >> numberOfModes;

         // this->modes[make_tuple(coil, line)] = vector<Mode>(numberOfModes);

         std::iota(std::begin(
                       this->modes[make_tuple(coil, line)]),
                   std::end(this->modes[make_tuple(coil, line)]), 0); // modes are index by ascending integer, starting from 0
         break;
      }
      case 'D': // read due date of coil
      {
         Coil coil;
         DueDate dueDate;

         ss >> coil;
         ss >> dueDate;

         this->dueDates[coil] = dueDate;
         break;
      }
      case 'P': // read processing time
      {
         Coil coil;
         ProductionLine line;
         Mode mode;
         ProcessingTime time;

         ss >> coil;
         ss >> mode;
         ss >> line;
         ss >> time;

         this->processingTimes[make_tuple(coil, line, mode)] = time;
         break;
      }
      case 'S': // read setup time
      {
         Coil coil1;
         Mode mode1;
         Coil coil2;
         Mode mode2;
         ProductionLine line;
         SetupTime time;

         ss >> coil1;
         ss >> mode1;
         ss >> coil2;
         ss >> mode2;
         ss >> line;
         ss >> time;

         this->setupTimes[make_tuple(coil1, mode1, coil2, mode2, line)] = time;
      }
      case 'c': // read stringer infos
      {
         Coil coil1;
         Mode mode1;
         Coil coil2;
         Mode mode2;
         ProductionLine line;
         StringerNeeded stringerNeeded = true;
         StringerCosts stringerCosts;

         ss >> coil1;
         ss >> mode1;
         ss >> coil2;
         ss >> mode2;
         ss >> line;
         ss >> stringerCosts;

         auto tuple = make_tuple(coil1, mode1, coil2, mode2, line);
         this->stringerNeeded[tuple] = stringerNeeded;
         this->stringerCosts[tuple] = stringerCosts;
      }
         // if no of the key-chars is at the beginning, ignore the whole line and do nothing
      }
   }
   infile.close();
}

void Instance::display()
{
   cout << "Instance " << endl;
   for (auto &comment : this->comments)
   {
      cout << "\t" << comment << endl;
   }
   cout << "Number of coils I: " << numberOfCoils << endl;
   cout << "Number of production lines K: " << numberOfProductionLines << endl;
   for (auto coil : this->coils)
   {
      cout << "Coil " << coil << endl;

      for (auto line : this->productionLines)
      {
         cout << "\tModes on line " << line << endl;
         for (auto mode : this->modes[make_tuple(coil, line)])
         {
            cout << "\t\tMode " << mode << endl;
            cout << "\t\t\tProcessingTime: " << processingTimes[make_tuple(coil, line, mode)] << endl;
            cout << "\t\t\tSetup times and stringer costs " << endl;
            for (auto other_coil : this->coils)
            {
               cout << "\t\t\t\t Other Coil " << other_coil << endl;
               for (auto other_mode : this->modes[make_tuple(other_coil, line)])
               {
                  auto tuple = make_tuple(coil, mode, other_coil, other_mode, line);

                  if (this->stringerNeeded[tuple])
                  {
                     auto setupTime = this->setupTimes[tuple];
                     auto costs = this->stringerCosts[tuple];
                     cout << "\t\t\t\t\tMode " << other_mode << ": Setup Time " << setupTime << "/ Costs " << costs << endl;
                  }
               }
            }
            cout << endl;
         }
      }
   }
}

bool Instance::IsStartCoil(Coil i)
{
   return i == this->startCoil;
}

bool Instance::IsEndCoil(Coil i)
{
   return i == this->endCoil;
}

bool Instance::IsRegularCoil(Coil i)
{
   return !IsStartCoil(i) && !IsEndCoil(i);
}