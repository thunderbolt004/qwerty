#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <chrono>
#include "TextGenerator.cpp"
#include "Timer.cpp"

int get_wpm(int totalKeyPress, int errors, double elapsedTimeInMinutes) {
    int correct = totalKeyPress/5;
    int wpm = (correct - errors)/elapsedTimeInMinutes;
    return std::max(0, wpm);
}

int get_accuracy(int charsTypedTillNow, int errors){
    double acc = (double)errors/charsTypedTillNow;
    acc = 1 - acc;
    return acc*100;
}
