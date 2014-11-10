//
//  markov_test.cpp
//  Feynman_Simulator
//
//  Created by Kun Chen on 10/20/14.
//  Copyright (c) 2014 Kun Chen. All rights reserved.
//

#include "markov.h"
#include "utility/sput.h"
#include "module/diagram/diagram.h"
#include "module/weight/weight.h"
#include "module/parameter/parameter.h"
using namespace std;
using namespace mc;

void Test_CreateWorm();

int mc::TestMarkov()
{
    sput_start_testing();
    sput_enter_suite("Test Updates:");
    sput_run_test(Test_CreateWorm);
    sput_finish_testing();
    return sput_get_return_value();
}

void Test_CreateWorm()
{
    para::ParaMC Para;
    Para.SetTest();
    weight::Weight Weight;
    Weight.SetTest(Para);
    diag::Diagram Diag;
    Diag.SetTest(Para.Lat, Para.RNG, Weight.G, Weight.W);
    Markov markov;
    markov.BuildNew(Para, Diag, Weight);
    
    for (int i = 0; i < 10; i++) {
        markov.Hop(50);
        sput_fail_unless(markov.Diag->CheckDiagram(), "Weight check for all the random steps");
    }
    LOG_INFO("Updates(Create,Delete, and Move Worm) are done!");
}