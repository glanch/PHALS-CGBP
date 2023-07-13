#pragma once

namespace Settings
{
    constexpr int kSCIPMaxStringLength = 1024;
    constexpr double kBigM = 1000;

    
    constexpr bool kGenerateInitialTrivialColumn = false;

    constexpr double kDynamicGapMaxRounds = 20;
    constexpr double kDynamicGap = 50;
    constexpr double kDynamicGapLowerBound = 0.01;
    constexpr double kDynamicGapTimeLimitInSeconds = 10*60;

    constexpr double kInitialSolveGap = 0;
    constexpr double kInitialSolveTimeTimeLimitInSeconds = 1e+20;
    constexpr bool kInitialSolveEnabled = false;
    constexpr bool kOnlyInitialSolve = false;
    
    constexpr bool kEnableSubproblemInterruption = true;

    constexpr double kDefaultMasterTimeLimit = 1e+20;
    
    constexpr bool kReconstructScheduleFromSolution = false;
    constexpr bool kEnableReoptimization = true;
}