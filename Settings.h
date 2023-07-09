#pragma once

namespace Settings
{
    constexpr int kSCIPMaxStringLength = 1024;
    constexpr double kBigM = 1000;

    
    constexpr bool kGenerateInitialTrivialColumn = true;

    constexpr double kDynamicGapMaxRounds = 5;
    constexpr double kDynamicGap = 50;
    constexpr double kDynamicGapLowerBound = 0.01;
    constexpr double kDynamicGapTimeLimitInSeconds = 60;

    constexpr double kInitialSolveGap = 0;
    constexpr double kInitialSolveTimeTimeLimitInSeconds = 1e+20;
    constexpr bool kInitialSolveEnabled = true;
    constexpr bool kOnlyInitialSolve = true;

    constexpr bool kEnableSubproblemInterruption = true;
}