//
//  index_map.h
//  Feynman_Simulator
//
//  Created by Kun Chen on 11/24/14.
//  Copyright (c) 2014 Kun Chen. All rights reserved.
//

#ifndef __Feynman_Simulator__index_map__
#define __Feynman_Simulator__index_map__

#include "utility/convention.h"
#include "lattice/lattice.h"
#include <vector>

namespace weight {
enum TauSymmetry {
    TauSymmetric = 1,
    TauAntiSymmetric = -1
};

const uint DELTA_T_SIZE = 3;
const uint SMOOTH_T_SIZE = 4;

enum SPIN4Filter { UpUp2UpUp,
                   UpDown2UpDown,
                   UpDown2DownUp };

class IndexMap {
public:
    IndexMap(real Beta, uint MaxTauBin, const Lattice& Lat, TauSymmetry Symmetry);
    int GetTauSymmetryFactor(real t_in, real t_out) const;
    const uint* GetShape() const; //the shape of internal weight array
    real Beta;
    Lattice Lat;
    uint MaxTauBin;
    TauSymmetry Symmetry;
    int TauIndex(real tau) const;
    int TauIndex(real t_in, real t_out) const;
    real IndexToTau(int TauIndex) const;

protected:
    vector<uint> _Shape;
    int _TauSymmetryFactor;
    real _dBeta;
    real _dBetaInverse;
};

class IndexMapSPIN2 : public IndexMap {
public:
    IndexMapSPIN2(real Beta, uint MaxTauBin, const Lattice& Lat, TauSymmetry Symmetry)
        : IndexMap(Beta, MaxTauBin, Lat, Symmetry)
    {
        _Shape = std::vector<uint>({ 4, (uint)Lat.SublatVol2, (uint)Lat.Vol, MaxTauBin });
    }
    static bool IsSameSpin(int spindex);
    void Map(uint* result, spin in, spin out,
             const Site& rin, const Site& rout,
             real tin, real tout) const;
    void MapDeltaT(uint* result, spin in, spin out,
                   const Site& rin, const Site& rout) const;

private:
    static int SpinIndex(spin SpinIn, spin SpinOut);
};

class IndexMapSPIN4 : public IndexMap {
public:
    IndexMapSPIN4(real Beta, uint MaxTauBin, const Lattice& Lat, TauSymmetry Symmetry)
        : IndexMap(Beta, MaxTauBin, Lat, Symmetry)
    {
        _Shape = std::vector<uint>({ 16, (uint)Lat.SublatVol2, (uint)Lat.Vol, MaxTauBin });
    }
    //First In/Out: direction of WLine; Second In/Out: direction of Vertex
    static std::vector<int> GetSpinIndexVector(SPIN4Filter filter);
    void Map(uint* result, const spin* in, const spin* out,
             const Site& rin, const Site& rout,
             real tin, real tout) const;
    void MapDeltaT(uint* result, const spin* in, const spin* out,
                   const Site& rin, const Site& rout) const;

private:
    static int SpinIndex(spin SpinInIn, spin SpinInOut, spin SpinOutIn, spin SpinOutOut);
    static int SpinIndex(const spin* TwoSpinIn, const spin* TwoSpinOut);
};
}

#endif /* defined(__Feynman_Simulator__index_map__) */
