#!/usr/bin/env python
import numpy as np
import os, sys, weight
from logger import *
import parameter as para

StatisFilePattern="_statis"

def Smooth(x,y):
    """return: smoothed funciton s(x) and estimation of sigma of y for one data point"""
    from scipy.interpolate import LSQUnivariateSpline
    #define segments to do spline smoothing
    t=np.linspace(0,len(x),5)[1:-1]
    sr = LSQUnivariateSpline(x, y.real, t)
    si = LSQUnivariateSpline(x, y.imag, t)
    return sr(x)+si(x)*1j, (sr.get_residual()/len(x))**0.5+(si.get_residual()/len(x))**0.5*1j

class WeightEstimator():
    def __init__(self, Weight, Order): 
        self.Shape=[Order, Weight.NSpin**2, Weight.NSublat**2]+Weight.Shape[Weight.VOLDIM:]
        self.__Map=Weight.Map
        self.__Weight=Weight
        self.WeightAccu=np.zeros(self.Shape, dtype=complex)
        self.NormAccu=0.0
        self.Norm=None
        self.Order=Order

    def Copy(self):
        """return a deep copy of Weight instance"""
        import copy
        return copy.deepcopy(self)

    def Merge(self, _WeightEstimator):
        """add data from another WeightEstimator"""
        self.WeightAccu+=_WeightEstimator.WeightAccu
        self.NormAccu+=_WeightEstimator.NormAccu
        if self.Norm is None:
            self.Norm=_WeightEstimator.Norm
        else:
            Assert(self.Norm==_WeightEstimator.Norm, "Norm have to be the same to merge statistics")
    
    def GetNewOrderAccepted(self, Name, ErrorThreshold, OrderAccepted):
        self.OrderWeight=self.WeightAccu/self.NormAccu*self.Norm
        MaxTauBin=self.__Map.MaxTauBin
        x=range(0, MaxTauBin)
        Shape=self.OrderWeight.shape
        for orderindex in range(OrderAccepted, Shape[0]):
            # Order Index is equal to real Order-1
            RelativeError=0
            for sp in range(Shape[1]):
                for sub in range(Shape[2]):
                    for vol in range(Shape[3]):
                        y=self.OrderWeight[orderindex,sp,sub,vol, :]
                        if not np.allclose(y, 0.0, 1e-10): 
                            smooth, sigma=Smooth(x, y)
                            relative=abs(sigma)/abs(np.average(smooth))
                            if abs(relative)>RelativeError:
                                RelativeError=relative
                                error=sigma
                                Original=y.copy()
                                Smoothed=smooth.copy()
                                Position=(orderindex,sp,sub,vol)
                            self.OrderWeight[orderindex,sp,sub,vol, :]=smooth
            try:
                State="Accepted with relative error {0} (Threshold {1})".format(RelativeError, ErrorThreshold)
                if RelativeError>=ErrorThreshold:
                    State="NOT "+State
                self.__Plot(Name, x, Original, Smoothed, error, Position, State)
            except:
                raise
                log.info("failed to plot")
            log.info("Maximum at Order {0} is {1}".format(orderindex, RelativeError))
            if RelativeError>=ErrorThreshold:
                break
        NewOrderAccepted=orderindex
        log.info("OrderAccepted={0}".format(NewOrderAccepted))
        return NewOrderAccepted

    def GetWeight(self, OrderAccepted):
        OrderIndex=OrderAccepted-1
        Dict={self.__Weight.Name: np.sum(self.OrderWeight[:OrderIndex+1], axis=0)}
        self.__Weight.FromDict(Dict)
        return self.__Weight

    def __Plot(self, Name, x, y, smooth, sigma, Position, State):
        path=os.path.join(workspace, "status")
        os.system("mkdir "+path)
        import matplotlib.pyplot as plt
        mid=len(x)/2
        Order=Position[0]+1
        plt.subplot(2, 1, 1)
        plt.plot(x, y.real, 'ro', x, smooth.real, 'b-')
        plt.errorbar(x[mid], smooth[mid].real, yerr=sigma.real,
                label="Error {0:.2g}".format(sigma.imag))
        plt.title("Order {0} at Spin:{1}, Sublat:{2}, Coordi:{3}\n{4}".format(Order, 
            self.__Map.IndexToSpin4(Position[1]), 
            self.__Map.IndexToSublat(Position[2]),
            self.__Map.IndexToCoordi(Position[3]), State))
        plt.legend().get_frame().set_alpha(0.5)
        plt.subplot(2, 1, 2)
        plt.plot(x, y.imag, 'ro', x, smooth.imag, 'b-')
        plt.errorbar(x[mid], smooth[mid].imag, yerr=sigma.imag, 
                label="Error {0:.2g}".format(sigma.imag))
        plt.legend().get_frame().set_alpha(0.5)
        plt.xlabel("Tau")
        plt.savefig(os.path.join(path, "{0}_Smoothed_Order{1}.jpg".format(Name, Order)))
        #plt.show()
        plt.clf()
        plt.cla()
        plt.close()

    def FromDict(self, data):
        datamat=data[self.__Weight.Name]
        self.WeightAccu=datamat['WeightAccu']
        self.NormAccu=datamat['NormAccu']
        self.Norm=datamat['Norm']
        self.OrderWeight=self.WeightAccu*self.NormAccu/self.Norm
        self.__AssertShape(self.Shape, self.WeightAccu.shape)

    def ToDict(self):
        datatmp={}
        datatmp["WeightAccu"]=self.WeightAccu
        datatmp["NormAccu"]=self.NormAccu
        datatmp["Norm"]=self.Norm
        return {self.__Weight.Name: datatmp}

    def __AssertShape(self, shape1, shape2):
        Assert(tuple(shape1)==tuple(shape2), \
                "Shape {0} is expected instead of shape {1}!".format(shape1, shape2))

def GetFileList():
    FileList = [f for f in os.listdir(workspace) if os.path.isfile(os.path.join(workspace,f))]
    StatisFileList=[os.path.join(workspace, f) for f in FileList if f.find(StatisFilePattern) is not -1]
    return StatisFileList

def CollectStatis(_map, _order):
    Sigma=weight.Weight("SmoothT", _map, "TwoSpins", "AntiSymmetric")
    SigmaSmoothT=WeightEstimator(Sigma, _order)
    Polar=weight.Weight("SmoothT", _map, "FourSpins", "Symmetric")
    PolarSmoothT=WeightEstimator(Polar, _order)
    SigmaTemp=SigmaSmoothT.Copy()
    PolarTemp=PolarSmoothT.Copy()
    _FileList=GetFileList()
    if len(_FileList)==0:
        Abort("No statistics files to read!") 
    log.info("Collect statistics from {0}".format(_FileList))
    for f in _FileList:
        log.info("Merging {0} ...".format(f));
        Dict=IO.LoadBigDict(f)
        SigmaTemp.FromDict(Dict['Sigma']['Histogram'])
        PolarTemp.FromDict(Dict['Polar']['Histogram'])
        SigmaSmoothT.Merge(SigmaTemp)
        PolarSmoothT.Merge(PolarTemp)
    return (SigmaSmoothT, PolarSmoothT)

def UpdateWeight(SigmaSmoothT, PolarSmoothT, ErrorThreshold, OrderAccepted):
    SigmaOrder=SigmaSmoothT.GetNewOrderAccepted("Sigma", ErrorThreshold, OrderAccepted)
    PolarOrder=PolarSmoothT.GetNewOrderAccepted("Polar", ErrorThreshold, OrderAccepted)
    log.info("Accepted Sigma order : {0}; Accepted Polar order : {1}".
            format(SigmaOrder, PolarOrder))
    Sigma=SigmaSmoothT.GetWeight(SigmaOrder)
    Polar=PolarSmoothT.GetWeight(PolarOrder)
    return Sigma, Polar

if __name__=="__main__":
    WeightPara={"NSublat": 1, "L":[4, 4],
                "Beta": 0.5, "MaxTauBin":128}
    Order=3
    map=weight.IndexMap(**WeightPara)

    SigmaSmoothT, PolarSmoothT=CollectStatis(map, Order)

    data ={}
    data["Sigma"] = {"Histogram": SigmaSmoothT.ToDict()}
    data["Polar"] = {"Histogram": PolarSmoothT.ToDict()}

    IO.SaveBigDict(workspace+"/statis_total", data)
