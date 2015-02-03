'''This is the input file of all jobs. 
   You have to add new job objects to TO_DO list
   if you want to run simulation.'''
CPU = 4
SLEEP = 1    #check job status for every SLEEP seconds
#common dictionary for all jobs
# monte carlo job defintion
MonteCarlo={
"Control": {
    "__Execute" : "./simulator.exe",
    "__Duplicate" : 0,
    "__IsCluster" : False,
    "__AutoRun" : True,
    "__KeepCPUBusy": True,
    },
"Job": {
    "DoesLoad" : False,
    "Sample" : 100000000  ##0.8 min for 1000000(*1000) Samples in MC
    }
}

Dyson={
"Control": {
    "__Execute" : ["python", "./calculator/main.py"],
    "__Duplicate" : 1,
    "__IsCluster" : False,
    "__AutoRun" : True, 
    "__KeepCPUBusy": False,
    },
"Job": {
    "DysonOnly": True
    }
}

beta=0.8
Order=3
Common={
"Tau": {
    "MaxTauBin" : 64,
    "Beta": beta,
    "DeltaBeta" :  0.00,
    "FinalBeta" :  beta,
    },
"Lattice":  {
    #"Name": "Square",
    #"NSublat": 1,
    #"L": [8,8],
    "Name": "Cubic",
    "NSublat": 1,
    "L": [8,8,8],
    #"Name": "Pyrochlore",
    #"NSublat": 4,
    #"L": [8,8,8]
    #"Name": "Checkboard",
    #"NSublat": 2,
    #"L": [8,8]
    #"Name": "3DCheckerboard",
    #"NSublat": 2,
    #"L": [16,16,16]
    #"L": [8,8,8]
    },
"Model": {
    "Name": "J1J2",
    "Interaction": [1.0,0.0],
    "ExternalField": [0.0, -0.0, 0.0, 0.0]
    #ExternalField on Sublattice A and B
    },

"Markov": {
    "Order": Order,
    #Start from order 0, so that OrderReWeight has Order+1 elements
    "Sweep" : 10,
    "Toss" : 1000,
    "OrderReWeight" : [1.0, 0.1, 0.5, 0.1],
    "SqueezeFactor" : 10.0,
    "WormSpaceReweight" : 0.05,
    "PolarReweight" : 2.0,
    "OrderTimeRatio" : [1.0, 1.0, 1.0, 2.0],
    "Timer": {
        "PrinterTimer": 100,
        "DiskWriterTimer": 256,
        "MessageTimer": 350,
        "ReweightTimer": 600
        },
    },
"Dyson": {
    "Order": Order,
    "OrderAccepted": 1,
    "ErrorThreshold": 0.1,
    "BetaIncrease": 0.0,
    "SleepTime": 300,
    "Annealing": {
        "DeltaField": [1.0,  0.0, 0.0, 0.0],
        "Interval": [-0.05, 0.0, 0.0, 0.0]
        }
    }
}
import job_class as job
TO_DO = []
MonteCarlo.update(Common)
TO_DO.append(job.JobMonteCarlo(MonteCarlo))
Dyson.update(Common)
TO_DO.append(job.JobDyson(Dyson))
