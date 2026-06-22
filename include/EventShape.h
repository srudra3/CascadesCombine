#ifndef EVENTSHAPE_H
#define EVENTSHAPE_H

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include "Math/GenVector/Boost.h"

namespace EventShape {

//----------------------------
// Thrust calculation
//----------------------------
struct ThrustResult {
    double thrust;
    ROOT::Math::XYZVector axis;
};

// ------------------------------------------------------------------------------
// Thrust hemispheres oriented along CS z-axis filled by ComputeThrustHemispheres 
// ------------------------------------------------------------------------------
struct ThrustHemispheres {

    int nPlus;                     // number of leptons with p·n >= 0
    int nMinus;                    // number of leptons with p·n < 0

    std::vector<size_t> idxPlus;      // indices in + hemisphere
    std::vector<size_t> idxMinus;     // indices in - hemisphere

    double massPlus;               // invariant mass of + hemisphere
    double massMinus;              // invariant mass of - hemisphere

    FourVec p4Plus;                // summed four-vector (+)
    FourVec p4Minus;               // summed four-vector (-)

    ROOT::Math::XYZVector thrustAxis; // oriented thrust axis used
    double thrustValue;               // value of thrust

};

struct ThrustHemisphereResult {
    int nPlus;                     // number of leptons with p·n >= 0
    int nMinus;                    // number of leptons with p·n < 0

    std::vector<size_t> idxPlus;   // indices in + hemisphere
    std::vector<size_t> idxMinus;  // indices in - hemisphere

    double massPlus;               // invariant mass of + hemisphere
    double massMinus;              // invariant mass of - hemisphere

    FourVec p4Plus;                // summed four-vector (+)
    FourVec p4Minus;               // summed four-vector (-)
};

// Enhanced thrust calculation with random sampling and iterative refinement
inline ThrustResult ComputeThrustOld(const std::vector<FourVec>& parts, bool boostToRestFrame = true,
                                  int nRandomDirections = 500, int nRefineSteps = 5) {
    std::vector<FourVec> vecs = parts;
    if(boostToRestFrame) {
        FourVec tot;
        for(const auto& p: vecs) tot += p;
        ROOT::Math::XYZVector beta = -tot.BoostVector();
        for(auto& p: vecs) p = p.Boost(beta);
    }

    // Initial best axis guess along x
    ROOT::Math::XYZVector bestAxis(1.0, 0.0, 0.0);
    double bestT = 0;

    // Function to compute thrust along a given axis
    auto thrustAlong = [&](const ROOT::Math::XYZVector& axis) -> double {
        double num=0, den=0;
        for(const auto& p: vecs){
            auto v = p.Vect();
            double comp[3] = {v.X(), v.Y(), v.Z()};
            double dot = axis.X()*comp[0] + axis.Y()*comp[1] + axis.Z()*comp[2];
            num += std::abs(dot);
            den += p.P();
        }
        return (den>0) ? num/den : 0;
    };

    // Random sampling of directions (simple deterministic "pseudo-random" for reproducibility)
    for(int i=0;i<nRandomDirections;i++){
        double theta = M_PI*std::abs(std::sin(i*0.37));
        double phi   = 2*M_PI*std::abs(std::cos(i*0.29));
        ROOT::Math::XYZVector axis(std::sin(theta)*std::cos(phi),
                                   std::sin(theta)*std::sin(phi),
                                   std::cos(theta));
        axis = axis.Unit();
        double T = thrustAlong(axis);
        if(T > bestT){
            bestT = T;
            bestAxis = axis;
        }
    }

    // Iterative refinement around the best axis
    for(int iter=0; iter<nRefineSteps; iter++){
        double dtheta = 0.1/(iter+1);
        for(int dx=-1; dx<=1; dx++){
            for(int dy=-1; dy<=1; dy++){
                for(int dz=-1; dz<=1; dz++){
                    if(dx==0 && dy==0 && dz==0) continue;
                    ROOT::Math::XYZVector axis = bestAxis + ROOT::Math::XYZVector(dx*dtheta, dy*dtheta, dz*dtheta);
                    axis = axis.Unit();
                    double T = thrustAlong(axis);
                    if(T > bestT){
                        bestT = T;
                        bestAxis = axis;
                    }
                }
            }
        }
    }

    return {bestT, bestAxis};
}

inline ThrustResult ComputeThrust(const std::vector<FourVec>& parts, bool boostToRestFrame = true,
                                  int maxIter = 100, double tol = 1e-8) {
    std::vector<FourVec> vecs = parts;
    if(boostToRestFrame) {
        FourVec tot;
        for(const auto& p: vecs) tot += p;
        ROOT::Math::XYZVector beta = -tot.BoostVector();
        for(auto& p: vecs) p = p.Boost(beta);
    }

    // Initialize axis along highest-momentum particle
    ROOT::Math::XYZVector axis(1.0, 0.0, 0.0);
    if(!vecs.empty()) {
        auto it = std::max_element(vecs.begin(), vecs.end(),
            [](const FourVec& a, const FourVec& b){ return a.Vect().R() < b.Vect().R(); });
        axis = it->Vect().Unit();
    }

    auto computeThrust = [&](const ROOT::Math::XYZVector& n) -> double {
        double num = 0, den = 0;
        for(const auto& p: vecs){
            auto v = p.Vect();
            num += std::abs(v.Dot(n));
            den += v.R();
        }
        return (den>0) ? num/den : 0;
    };

    // Gradient ascent loop
    for(int iter=0; iter<maxIter; iter++){
        // Compute gradient numerically
        double delta = 1e-5;
        ROOT::Math::XYZVector grad(0,0,0);
        for(int i=0;i<3;i++){
            ROOT::Math::XYZVector nplus = axis;
            if(i==0) nplus.SetX(axis.X() + delta);
            if(i==1) nplus.SetY(axis.Y() + delta);
            if(i==2) nplus.SetZ(axis.Z() + delta);
            nplus = nplus.Unit();

            ROOT::Math::XYZVector nminus = axis;
            if(i==0) nminus.SetX(axis.X() - delta);
            if(i==1) nminus.SetY(axis.Y() - delta);
            if(i==2) nminus.SetZ(axis.Z() - delta);
            nminus = nminus.Unit();

            double fplus = computeThrust(nplus);
            double fminus = computeThrust(nminus);
            if(i==0) grad.SetX((fplus - fminus)/(2*delta));
            if(i==1) grad.SetY((fplus - fminus)/(2*delta));
            if(i==2) grad.SetZ((fplus - fminus)/(2*delta));
        }

        if(grad.R() < tol) break;

        // Update axis along gradient
        axis += 0.1 * grad.Unit(); // step size 0.1
        axis = axis.Unit();
    }

    double thrust = computeThrust(axis);
    return {thrust, axis};
}

inline ThrustHemisphereResult
ComputeThrustHemispheresOld(const std::vector<FourVec>& leptons,
                          bool boostToRestFrame = true)
{
    ThrustHemisphereResult out;
    out.nPlus = 0;
    out.nMinus = 0;
    out.massPlus = 0.0;
    out.massMinus = 0.0;
    out.p4Plus = FourVec();
    out.p4Minus = FourVec();
    out.idxPlus.clear();
    out.idxMinus.clear();

    const size_t N = leptons.size();
    if (N == 0) return out;

    // ------------------------------------------------------------
    // 1) Copy leptons and (optionally) boost to system rest frame
    // ------------------------------------------------------------
    std::vector<FourVec> vecs = leptons;
    if (boostToRestFrame) {
        FourVec Q;
        for (const auto& p : vecs) Q += p;
        ROOT::Math::XYZVector beta = -Q.BoostVector();
        for (auto& p : vecs) p = p.Boost(beta);
    }

    // ------------------------------------------------------------
    // 2) Compute thrust axis in this frame
    // ------------------------------------------------------------
    ThrustResult thr = ComputeThrust(vecs,
                                     /*boostToRestFrame=*/false,
                                     /*maxIter=*/200,
                                     /*tol=*/1e-8);
    ROOT::Math::XYZVector n = thr.axis;
    if (n.R() > 0) n = n.Unit();

    // ------------------------------------------------------------
    // 3) Assign hemispheres
    // ------------------------------------------------------------
    for (size_t i = 0; i < N; ++i) {
        const auto& p = vecs[i];
        double dot = p.Vect().Dot(n);

        if (dot >= 0.0) {
            out.idxPlus.push_back(i);
            out.p4Plus += p;
            out.nPlus++;
        } else {
            out.idxMinus.push_back(i);
            out.p4Minus += p;
            out.nMinus++;
        }
    }

    // ------------------------------------------------------------
    // 4) Invariant masses
    // ------------------------------------------------------------
    if (out.nPlus > 0)  out.massPlus  = out.p4Plus.M();
    if (out.nMinus > 0) out.massMinus = out.p4Minus.M();

    return out;
}

// Compute hemispheres using thrust axis oriented along given CS z-axis
inline ThrustHemispheres ComputeThrustHemispheres(
        const std::vector<FourVec>& parts,
        const ROOT::Math::XYZVector& zCS,
        bool boostToRestFrame = true)
{
    ThrustHemispheres out;
    if (parts.empty()) return out;

    // copy and boost if requested
    std::vector<FourVec> vecs = parts;
    if (boostToRestFrame) {
        FourVec tot;
        for (const auto &p : vecs) tot += p;
        ROOT::Math::XYZVector beta = -tot.BoostVector();
        for (auto &p : vecs) p = p.Boost(beta);
    }

    // 1) compute thrust
    ThrustResult thr = ComputeThrust(vecs, false);
    ROOT::Math::XYZVector n = thr.axis.Unit();
    out.thrustValue = thr.thrust;

    // 2) orient thrust axis along CS z
    if (n.Dot(zCS) < 0) n *= -1.0;
    out.thrustAxis = n;

    // 3) assign particles to hemispheres
    out.p4Plus  = FourVec(0,0,0,0,0);
    out.p4Minus = FourVec(0,0,0,0,0);

    for (size_t i = 0; i < vecs.size(); i++) {
        double proj = vecs[i].Vect().Dot(n);
        if (proj >= 0) {
            out.idxPlus.push_back(i);
            out.p4Plus += vecs[i];
        } else {
            out.idxMinus.push_back(i);
            out.p4Minus += vecs[i];
        }
    }

    out.nPlus = out.idxPlus.size();
    out.nMinus = out.idxMinus.size();
    out.massPlus = out.p4Plus.M();
    out.massMinus = out.p4Minus.M();

    return out;
}


//----------------------------
// Sphericity, Aplanarity, C-parameter
//----------------------------
struct TensorResult {
    double lambda[3];   // sorted descending
    double sphericity;
    double aplanarity;
    double Cparameter;
};

inline void Eigenvalues3x3(double M[3][3], double lambda[3]) {
    double p1 = M[0][1]*M[0][1] + M[0][2]*M[0][2] + M[1][2]*M[1][2];
    if(p1==0){
        lambda[0] = M[0][0];
        lambda[1] = M[1][1];
        lambda[2] = M[2][2];
    } else {
        double q = (M[0][0]+M[1][1]+M[2][2])/3.0;
        double p2 = (M[0][0]-q)*(M[0][0]-q) + (M[1][1]-q)*(M[1][1]-q) + (M[2][2]-q)*(M[2][2]-q) + 2*p1;
        double p = std::sqrt(p2/6.0);
        double B[3][3];
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                B[i][j] = (1.0/p)*( (i==j?M[i][i]-q: M[i][j]) );
        double r = (B[0][0]*B[1][1]*B[2][2] + 2*B[0][1]*B[0][2]*B[1][2]
                    - B[0][2]*B[0][2]*B[1][1] - B[1][2]*B[1][2]*B[0][0] - B[0][1]*B[0][1]*B[2][2] )/2.0;
        double phi = (r <= -1 ? M_PI/3 : (r >= 1 ? 0 : std::acos(r)/3.0));
        lambda[0] = q + 2*p*std::cos(phi);
        lambda[2] = q + 2*p*std::cos(phi + (2*M_PI/3));
        lambda[1] = 3*q - lambda[0] - lambda[2];
    }
    std::sort(lambda, lambda+3, std::greater<double>());
}

inline TensorResult ComputeTensorObservables(const std::vector<FourVec>& parts, bool boostToRestFrame = true) {
    std::vector<FourVec> vecs = parts;
    if(boostToRestFrame) {
        FourVec tot;
        for(const auto& p: vecs) tot += p;
        ROOT::Math::XYZVector beta = -tot.BoostVector();
        for(auto& p: vecs) p = p.Boost(beta);
    }

    double M[3][3]={{0,0,0},{0,0,0},{0,0,0}};
    double sumP2=0;
    for(const auto& p: vecs){
        auto v = p.Vect();
        double comp[3] = {v.X(), v.Y(), v.Z()};
        sumP2 += v.Mag2();
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                M[i][j] += comp[i]*comp[j];
    }
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            M[i][j] /= sumP2;

    double lambda[3];
    Eigenvalues3x3(M, lambda);
    double sphericity = 1.5*(lambda[1]+lambda[2]);
    double aplanarity = 1.5*lambda[2];
    double Cparameter = 3*(lambda[0]*lambda[1] + lambda[0]*lambda[2] + lambda[1]*lambda[2]);

    return {{lambda[0],lambda[1],lambda[2]}, sphericity, aplanarity, Cparameter};
}

//----------------------------
// Fox-Wolfram moments
//----------------------------
inline std::vector<double> ComputeFoxWolfram(const std::vector<FourVec>& parts, int maxL = 4, bool boostToRestFrame = true) {
    std::vector<FourVec> vecs = parts;
    if(boostToRestFrame) {
        FourVec tot;
        for(const auto& p: vecs) tot += p;
        ROOT::Math::XYZVector beta = -tot.BoostVector();
        for(auto& p: vecs) p = p.Boost(beta);
    }

    std::vector<double> moments(maxL+1,0.0);
    double H0=0.0;
    for(size_t i=0;i<vecs.size();i++){
        for(size_t j=0;j<vecs.size();j++){
            double pij = vecs[i].P()*vecs[j].P();
            double cosTheta = vecs[i].Vect().Dot(vecs[j].Vect()) / (vecs[i].P()*vecs[j].P());
            for(int l=0;l<=maxL;l++){
                double Pl = std::legendre(l, cosTheta);
                moments[l] += pij*Pl;
            }
            H0 += pij;
        }
    }
    for(int l=0;l<=maxL;l++) moments[l]/=H0;
    return moments;
}

//----------------------------
// Print boosted vectors and check sums
//----------------------------
inline void PrintBoostedVectors(std::vector<FourVec> parts, bool boostToRestFrame = true, int precision = 4){
    if(boostToRestFrame){
        FourVec tot;
        for(const auto& p: parts) tot += p;
        ROOT::Math::XYZVector beta = -tot.BoostVector();
        for(auto& p: parts) p = p.Boost(beta);
    }

    std::cout.setf(std::ios::fixed);
    double sumPx=0, sumPy=0, sumPz=0, sumE=0;
    std::cout << "Boosted 4-vectors (px, py, pz, E) and |p|:\n";
    for(size_t i=0;i<parts.size();i++){
        const auto& p = parts[i];
        double pmag = p.P();
        std::cout << std::setprecision(precision)
                  << " ["<<i<<"] px="<<p.Px()<<", py="<<p.Py()
                  << ", pz="<<p.Pz()<<", E="<<p.E()<<", |p|="<<pmag<<"\n";
        sumPx += p.Px(); sumPy += p.Py(); sumPz += p.Pz(); sumE += p.E();
    }
    std::cout << std::setprecision(precision)
              << "Sum: Σpx="<<sumPx<<", Σpy="<<sumPy
              <<", Σpz="<<sumPz<<", ΣE="<<sumE<<"\n\n";
}

// ----------------------------
// Collins-Soper angles for thrust axis
// ----------------------------
struct ThrustCSResult {
    double Qmass;                // invariant mass of the n-lepton system
    double QT;                   // transverse momentum of the system
    double y_lab;                // rapidity of the system in the lab
    double y_CS;                 // rapidity of the system in the CS rest frame (≈0)
    double cosTheta_CS;          // cos(theta_CS) of the thrust axis (in rest frame)
    double phi_CS;               // phi_CS of the thrust axis (range -pi..pi)
    ROOT::Math::XYZVector xCS;   // CS x-axis (unit, rest frame)
    ROOT::Math::XYZVector yCS;   // CS y-axis (unit, rest frame)
    ROOT::Math::XYZVector zCS;   // CS z-axis (unit, rest frame)
    ROOT::Math::XYZVector thrustAxis_rf; // thrust axis unit vector in rest frame
};

// Compute Collins–Soper axes from lab beams (±z) and return angles of the THRUST axis
// leptons : input four-vectors (FourVec), not modified
// Ebeam   : beam energy for constructing massless beam vectors (only direction matters for CS axes)
// Returns: ThrustCSResult with axes and angles
inline ThrustCSResult ComputeThrustAxisInCS(const std::vector<FourVec>& leptons,
                                            double Ebeam = 6800.0)
{
    ThrustCSResult out;
    out.Qmass = 0.0; out.QT = 0.0; out.y_lab = 0.0; out.y_CS = 0.0;
    out.cosTheta_CS = 0.0; out.phi_CS = 0.0;
    out.xCS = ROOT::Math::XYZVector(1.,0.,0.);
    out.yCS = ROOT::Math::XYZVector(0.,1.,0.);
    out.zCS = ROOT::Math::XYZVector(0.,0.,1.);
    out.thrustAxis_rf = ROOT::Math::XYZVector(0.,0.,1.);

    const size_t N = leptons.size();
    if (N == 0) return out;

    // 1) total system Q
    FourVec Q;
    for (const auto &f : leptons) Q += f;
    out.Qmass = Q.M();
    out.QT = std::sqrt(Q.Px()*Q.Px() + Q.Py()*Q.Py());
    out.y_lab = Q.rapidity();

    // 2) construct massless beam four-vectors in lab along ±z
    //    use (px,py,pz,E) with pz = ±Ebeam, E = Ebeam (massless)
    FourVec beam1(0, 0.0, 0.0, +Ebeam, 0.0); // +z
    FourVec beam2(0, 0.0, 0.0, -Ebeam, 0.0); // -z

    // 3) boost leptons and beams to Q rest frame
    ROOT::Math::XYZVector beta = -Q.BoostVector(); // boost vector to send Q -> rest
    std::vector<FourVec> leptons_rf; leptons_rf.reserve(N);
    for (const auto &p : leptons) leptons_rf.push_back(p.Boost(beta));
    FourVec beam1_rf = beam1.Boost(beta);
    FourVec beam2_rf = beam2.Boost(beta);

    // Recompute Q_rf and its rapidity (should be ~0)
    FourVec Q_rf;
    for (const auto &p : leptons_rf) Q_rf += p;
    out.y_CS = Q_rf.rapidity();

    // 4) build unit vectors for boosted beams
    ROOT::Math::XYZVector p1 = beam1_rf.Vect();
    ROOT::Math::XYZVector p2 = beam2_rf.Vect();
    // normalize to unit direction where possible
    if (p1.R() > 0) p1 = p1.Unit();
    if (p2.R() > 0) p2 = p2.Unit();

    // 5) Collins–Soper axes in RF: z_CS = (p1 - p2).unit()
    ROOT::Math::XYZVector zcs = p1 - p2;
    const double eps = 1e-12;
    if (zcs.R() < eps) {
        // Degenerate case (beam vectors nearly identical in RF) -> fallback to lab z in RF
        // use boosted lab z: p_labz = boost of (0,0,1)
        ROOT::Math::XYZVector labz(0.,0.,1.);
        // build a 4-vector-like object to boost? simpler: rotate labz by boost's spatial transform is involved.
        // fallback to using p1 (if available) or canonical z
        if (p1.R() > eps) zcs = p1;
        else zcs = ROOT::Math::XYZVector(0.,0.,1.);
    }
    zcs = zcs.Unit();

    // x_temp = (p1 + p2)
    ROOT::Math::XYZVector x_temp = p1 + p2;

    // ycs = zcs cross x_temp
    ROOT::Math::XYZVector ycs = ROOT::Math::XYZVector(
        zcs.Y()*x_temp.Z() - zcs.Z()*x_temp.Y(),
        zcs.Z()*x_temp.X() - zcs.X()*x_temp.Z(),
        zcs.X()*x_temp.Y() - zcs.Y()*x_temp.X()
    );

    if (ycs.R() < eps) {
        // degeneracy -> choose any vector orthogonal to zcs
        ROOT::Math::XYZVector cand(1.,0.,0.);
        if (std::fabs(zcs.Dot(cand)) > 0.9) cand = ROOT::Math::XYZVector(0.,1.,0.);
        ycs = ROOT::Math::XYZVector(
            zcs.Y()*cand.Z() - zcs.Z()*cand.Y(),
            zcs.Z()*cand.X() - zcs.X()*cand.Z(),
            zcs.X()*cand.Y() - zcs.Y()*cand.X()
        );
    }
    ycs = ycs.Unit();

    // xcs = ycs cross zcs
    ROOT::Math::XYZVector xcs = ROOT::Math::XYZVector(
        ycs.Y()*zcs.Z() - ycs.Z()*zcs.Y(),
        ycs.Z()*zcs.X() - ycs.X()*zcs.Z(),
        ycs.X()*zcs.Y() - ycs.Y()*zcs.X()
    );
    xcs = xcs.Unit();

    out.xCS = xcs; out.yCS = ycs; out.zCS = zcs;

// 6) compute thrust axis in RF (we already are in the RF)
    ThrustResult thr = ComputeThrust(leptons_rf, /*boostToRestFrame=*/false, /*maxIter*/200, /*tol*/1e-8);
    ROOT::Math::XYZVector n_thrust = thr.axis;
    if (n_thrust.R() > 0) n_thrust = n_thrust.Unit();

// --- FIX SIGN CONVENTION ---
// Enforce thrust axis to point in the +z_CS hemisphere
// This removes the ± ambiguity and makes CS angles physical
    if (n_thrust.Dot(zcs) < 0.0) {
        n_thrust *= -1.0;
    }

    out.thrustAxis_rf = n_thrust;

    // 7) compute cosTheta_CS and phi_CS for the thrust axis
    double cosTheta = n_thrust.Dot(zcs);
    if (cosTheta > 1.0) cosTheta = 1.0;
    if (cosTheta < -1.0) cosTheta = -1.0;
    double kdoty = n_thrust.Dot(ycs);
    double kdotx = n_thrust.Dot(xcs);
    double phi = std::atan2(kdoty, kdotx); // (-pi, pi]
    out.cosTheta_CS = cosTheta;
    out.phi_CS = phi;

    return out;
}

inline void PrintThrustHemispheres(const EventShape::ThrustHemispheres& hemi, int precision = 4)
{
    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(precision);

    std::cout << "Thrust hemispheres:\n";
    std::cout << "  + hemisphere: N=" << hemi.nPlus
              << "  mass=" << hemi.massPlus << " GeV\n";
    std::cout << "  - hemisphere: N=" << hemi.nMinus
              << "  mass=" << hemi.massMinus << " GeV\n";

    std::cout << "  + indices: ";
    for (auto i : hemi.idxPlus) std::cout << i << " ";
    std::cout << "\n";

    std::cout << "  - indices: ";
    for (auto i : hemi.idxMinus) std::cout << i << " ";
    std::cout << "\n";

    std::cout << "Thrust value: " << hemi.thrustValue << "\n";
    std::cout << "Thrust axis (unit vector): ("
              << hemi.thrustAxis.X() << ", "
              << hemi.thrustAxis.Y() << ", "
              << hemi.thrustAxis.Z() << ")\n";
}

} // namespace EventShape

#endif // EVENTSHAPE_H

