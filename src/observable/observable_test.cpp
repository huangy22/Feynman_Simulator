//
//  observable_test.cpp
//  Feynman_Simulator
//
//  Created by Kun Chen on 10/13/14.
//  Copyright (c) 2014 Kun Chen. All rights reserved.
//

#include "observable.h"
#include "cnpy.h"
#include "sput.h"
using namespace std;

void TestObservableComplex();
void TestObservableReal();

int TestObservable()
{
    sput_start_testing();
    sput_enter_suite("Test Observable...");
    //complex and real Estimator have to be tested separatelly,
    //since Estimator._update() is different for those two types
    sput_run_test(TestObservableComplex);
    sput_run_test(TestObservableReal);
    sput_finish_testing();
    return sput_get_return_value();
}

void TestObservableComplex()
{
    Estimator<Complex> quan1("1");
    Estimator<Complex> quan2("2");
    Complex a[10];
    for(int i=0;i<10;i++)
    {
        a[i]=Complex(i+1,10-i);
        quan1.AddStatistics(a[i]);
        quan2.AddStatistics(-a[i]);
    }
    Estimate<Complex> ExpectedResult(Complex(5.0, 5.0),Complex(1.5, 0.9));
    //!!!This two value only works if you set _norm=1.0 and ThrowRatio=1.0/3
    sput_fail_unless(Equal(quan1.Estimate().Mean, ExpectedResult.Mean),
                     "check the Mean value.");
    sput_fail_unless(Equal(quan1.Estimate().Error, ExpectedResult.Error),
                     "check the Error value.");
    
    //Estimator IO operation
    
    EstimatorVector<Complex> QuanVector;
    QuanVector.push_back(quan1);
    QuanVector.push_back(quan2);
    QuanVector.SaveState("TestObservable", "w");
    QuanVector.ClearStatistics();
    QuanVector.ReadState("TestObservable");
    sput_fail_unless(Equal(QuanVector[1].Estimate().Mean, -ExpectedResult.Mean),
                     "check the Mean value.");
    sput_fail_unless(Equal(QuanVector[1].Estimate().Error, ExpectedResult.Error),
                     "check the Error value.");
}

void TestObservableReal()
{
    Estimator<real> quan1("1");
    Estimator<real> quan2("2");
    real a[10];
    for(int i=0;i<10;i++)
    {
        a[i]=i+1;
        quan1.AddStatistics(a[i]);
    }
    Estimate<Complex> ExpectedResult(5.0,1.5);
    //!!!This two value only works if you set _norm=1.0 and ThrowRatio=1.0/3
    sput_fail_unless(Equal(quan1.Estimate().Mean, ExpectedResult.Mean),
                     "check the Mean value.");
    sput_fail_unless(Equal(quan1.Estimate().Error, ExpectedResult.Error),
                     "check the Error value.");
}