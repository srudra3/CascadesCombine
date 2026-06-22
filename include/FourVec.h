#include <cmath>
#include <iostream>
#include <vector>
#include <limits>
#include "PreciseDouble.h"  // Optionally use boost multiprecision for adding 4-vectors to better preserve mass precision
#include "Math/Boost.h"
#include "Math/Vector3D.h"
#include "Math/LorentzVector.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "asymm_mt2_lester_bisect.h"  // Lester-Nachman MT2 implementation

#include "FourVecUtils.h" // Define my own PxPyPzM4D as it seems that the ROOT version is incompatible.

// Use PxPyPzM to store mass precisely
//using LorentzVectorM = ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double>>;
using LorentzVectorM = ROOT::Math::LorentzVector<FourVecUtils::PxPyPzM4D<double>>;
using LorentzVectorE = ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>;

class FourVec {
public:
    int pdgID;
    int flag;
    LorentzVectorM v;

    // Default constructor (zero 4-vector with pdgID = 0, flag = 1)
    FourVec() : pdgID(0), flag(1), v(0.0, 0.0, 0.0, 0.0) {}

    // Basic constructor using (px, py, pz, m)
    explicit FourVec(int id, double px, double py, double pz, double m, int f = 1)
        : pdgID(id), flag(f), v(px, py, pz, m) {}

    // Optional factory method using (pt, eta, phi, m) constructor
    static FourVec FromPtEtaPhiM(int id, double pt, double eta, double phi, double m, int f = 1) {
        double px = pt * std::cos(phi);
        double py = pt * std::sin(phi);
        double pz = pt * std::sinh(eta);
        return FourVec(id, px, py, pz, m, f);
    }

    // Optional factory method for (px, py, pz, E) for backwards compatibility. 
    // (but need to switch to this ...) - preferred is to use the (px, py, pz, m) case directly.
    static FourVec FromPxPyPzE(int id, double px, double py, double pz, double E, int f = 1) {
        double msq = E*E - (px*px + py*py + pz*pz);
        double m  = msq > 0 ? std::sqrt(msq) : 0.0;
        return FourVec(id, px, py, pz, m, f);
    }

    FourVec& operator+=(const FourVec& other) {
        using Precise::pdouble;    // can be double or quadruple precision 

    // Do computation using pdouble representation
    // if boost::multiprecision available can do the mass calculation in quadruple precision

    // Extract components from this and other, using PxPyPzM representation
        const auto& p1 = this->v;
        const auto& p2 = other.v;

    // Get Px, Py, Pz, M for both vectors
        pdouble px1 = p1.Px(), py1 = p1.Py(), pz1 = p1.Pz(), m1 = p1.M();
        pdouble px2 = p2.Px(), py2 = p2.Py(), pz2 = p2.Pz(), m2 = p2.M();

    // Compute momentum magnitudes
        pdouble p2_1 = px1*px1 + py1*py1 + pz1*pz1;
        pdouble p2_2 = px2*px2 + py2*py2 + pz2*pz2;

    // Compute energies using E = sqrt(p^2 + m^2)
        pdouble e1 = sqrt(p2_1 + m1*m1);
        pdouble e2 = sqrt(p2_2 + m2*m2);

    // Sum Px, Py, Pz, and E
        pdouble px = px1 + px2;
        pdouble py = py1 + py2;
        pdouble pz = pz1 + pz2;
        pdouble e  = e1  + e2;

    // Compute invariant mass squared
        pdouble p2_sum = px*px + py*py + pz*pz;
        pdouble msq = e*e - p2_sum;
        pdouble m = (msq > 0) ? sqrt(msq) : 0;

    // Overwrite the vector using PxPyPzM constructor
       this->v = LorentzVectorM(FourVecUtils::PxPyPzM4D<double> (static_cast<double>(px), static_cast<double>(py), static_cast<double>(pz), static_cast<double>(m) ));
       return *this;

}

    // Four-momentum addition (physically correct)
/*    FourVec& operator+=(const FourVec& other) {
        LorentzVectorE v1 = AsPxPyPzE();
        LorentzVectorE v2 = other.AsPxPyPzE();
        LorentzVectorE sum = v1 + v2;
        v = ToPxPyPzM(sum);
        return *this;
    }
*/

    // Non-mutating 4-vector addition
    FourVec operator+(const FourVec& other) const {
        FourVec result = *this;
        result += other;  // Reuse operator+= logic
        return result;
    }

    // Convert to energy-based 4-vector
    LorentzVectorE AsPxPyPzE() const {
        double px = v.Px();
        double py = v.Py();
        double pz = v.Pz();
        double m  = v.M();
        double E  = std::sqrt(px*px + py*py + pz*pz + m*m);
        return LorentzVectorE(px, py, pz, E);
    }

    // Convert from E-based to M-based
    static LorentzVectorM ToPxPyPzM(const LorentzVectorE& ve) {
        double px = ve.Px();
        double py = ve.Py();
        double pz = ve.Pz();
        double E  = ve.E();
        double m2 = E*E - (px*px + py*py + pz*pz);
        double m  = m2 > 0 ? std::sqrt(m2) : 0.0;
        return LorentzVectorM(px, py, pz, m);
    }

   // Accessors
    double Px() const { return v.Px(); }
    double Py() const { return v.Py(); }
    double Pz() const { return v.Pz(); }
    double M()  const { return v.M(); }
    double P()  const { return std::sqrt(Px()*Px() + Py()*Py() + Pz()*Pz()); }
    double E()  const { return std::sqrt(Px()*Px() + Py()*Py() + Pz()*Pz() + M()*M()); }
    double Pt() const { return std::sqrt(Px()*Px() + Py()*Py()); }
    double Eta() const { return v.Eta(); }
    double Phi() const { return v.Phi(); }
    double Beta() const { return P()/E(); }
    double Betaz() const { return Pz()/E(); }

    void Print() const {
        std::cout << "4-vector has pdgID " << pdgID << ", Flag=" << flag 
                  << " (" << Px() <<", "<< Py()<<", "<< Pz()<<", "<< E()<<", " << M() 
                  << ", pt=" << Pt() << " costh=" << CosTh() << " qsinth="<<QSinth()
                  << std::endl;
    }

    double Theta() const {
        return v.Theta();
    }

    double eta() const {
        constexpr double epsilon = 1e-10;  // Small cutoff to avoid singularities
        double theta_val = Theta();

        // Avoid division by zero or log(0)
        if (theta_val < epsilon) {
            // theta ~ 0 → eta → +∞
            return std::numeric_limits<double>::infinity();
        } else if (M_PI - theta_val < epsilon) {
            // theta ~ π → eta → -∞
            return -std::numeric_limits<double>::infinity();
        }

        return -std::log(std::tan(0.5 * theta_val));
    }

    double Charge() const {
        if (pdgID==11)  return -1.;
        if (pdgID==-11) return +1.;
        return 0.;
    }

    double mtwo(const FourVec& other) const {
    // Add other 4-vector to self 4-vector and return mass
        auto sumVec = *this + other;  // assuming operator+ is defined for FourVec
        return sumVec.Mass();            // M() returns the invariant mass
    }

    int lcharge() const {
        if (abs(pdgID)==11 || abs(pdgID)==13)
            return pdgID<0 ? +1 : -1; 
        return 0;
    }

    int lflavor() const {
        if (abs(pdgID)==11) return 1;
        if (abs(pdgID)==13) return 2;
        return 0;
    }

   // Determine if two leptons are OSSF (opposite-sign same-flavor)
    int ossf(const FourVec& o) const {
        int flav1 = this->lflavor();
        int flav2 = o.lflavor();
        int charge1 = this->lcharge();
        int charge2 = o.lcharge();
        int ossf = 0;
        if (charge1 * charge2 == -1 && flav1 == flav2 && flav1 != 0) {
            ossf = 1;
        }
        return ossf;
    }

// Electron flavor only
    int eflavor() const {
        int flavor = 0;
        if (abs(pdgID) == 11) {
            flavor = 1;
        }
        return flavor;
    }

    // Muon flavor only
    int mflavor() const {
        int flavor = 0;
        if (abs(pdgID) == 13) {
            flavor = 1;
        }
        return flavor;
    }

// Charge-flavor product
    int qf() const {
        return lcharge()*lflavor();
    }

    bool osdil(const FourVec& o) const {
 // Here require that a dilepton pair is OSSF.
        return lflavor()==o.lflavor()
            && lcharge()*o.lcharge()==-1;
    }
    // ... (repeat OS/SS methods similarly) ...

    bool osdiel(const FourVec& other) const {
        // Boolean function on whether the di-lepton candidate is an OS di-electron
        bool osdiel = false;
        int flav1 = this->lflavor();
        int flav2 = other.lflavor();
        int charge1 = this->lcharge();
        int charge2 = other.lcharge();
        if (flav1 == flav2 && flav1 == 1) {
            if (charge1 * charge2 == -1) {
                osdiel = true;
            }
        }
        return osdiel;
    }

    bool ssdiel(const FourVec& other) const {
        // Boolean function on whether the di-lepton candidate is a SS di-electron
        bool ssdiel = false;
        int flav1 = this->lflavor();
        int flav2 = other.lflavor();
        int charge1 = this->lcharge();
        int charge2 = other.lcharge();
        if (flav1 == flav2 && flav1 == 1) {
            if (charge1 * charge2 == 1) {
                ssdiel = true;
            }
        }
        return ssdiel;
    }

    bool osdimu(const FourVec& other) const {
        // Boolean function on whether the di-lepton candidate is an OS di-muon
        bool osdimu = false;
        int flav1 = this->lflavor();
        int flav2 = other.lflavor();
        int charge1 = this->lcharge();
        int charge2 = other.lcharge();
        if (flav1 == flav2 && flav1 == 2) {
            if (charge1 * charge2 == -1) {
                osdimu = true;
            }
        }
        return osdimu;
    }

    bool ssdimu(const FourVec& other) const {
        // Boolean function on whether the di-lepton candidate is a SS di-muon
        bool ssdimu = false;
        int flav1 = this->lflavor();
        int flav2 = other.lflavor();
        int charge1 = this->lcharge();
        int charge2 = other.lcharge();
        if (flav1 == flav2 && flav1 == 2) {
            if (charge1 * charge2 == 1) {
                ssdimu = true;
            }
        }
        return ssdimu;
    }

    bool oselmu(const FourVec& other) const {
        // Boolean function on whether the di-lepton candidate is an OS electron-muon pair
        bool oselmu = false;
        int flav1 = this->lflavor();
        int flav2 = other.lflavor();
        int charge1 = this->lcharge();
        int charge2 = other.lcharge();
        if (flav1 != flav2) {
            if (charge1 * charge2 == -1) {
                oselmu = true;
            }
        }
        return oselmu;
    }

    bool sselmu(const FourVec& other) const {
        // Boolean function on whether the di-lepton candidate is a SS electron-muon pair
        bool sselmu = false;
        int flav1 = this->lflavor();
        int flav2 = other.lflavor();
        int charge1 = this->lcharge();
        int charge2 = other.lcharge();
        if (flav1 != flav2) {
            if (charge1 * charge2 == 1) {
                sselmu = true;
            }
        }
        return sselmu;
    }

    double CosTh() const {
        auto p = v.P();
        return (p>1e-6 ? v.Pz()/p : 0);
    }

    double Mass() const { return v.M(); }   // redundant?



    double QSinth() const {
        double s = Pt()/P();
        return (v.Pz()<0 ? -s : s);
    }

    double DPhiR(const FourVec& o) const {
        double d = o.Phi() - Phi();
        if (d<-M_PI) d+=2*M_PI;
        if (d> M_PI) d-=2*M_PI;
        return d;
    }

    double DEta(const FourVec& o) const {
        double e1 = -std::log(std::tan(0.5*v.Theta()));
        double e2 = -std::log(std::tan(0.5*o.v.Theta()));
        return e2 - e1;
    }
    double DeltaR(const FourVec& o) const {
        return std::hypot(DPhiR(o), DEta(o));
    }

    double DPhi(const FourVec& o) const {
        double dphi  = o.Phi() - Phi();
        double cdphi = std::cos(dphi);
        double d = std::acos(cdphi);    // Should be in the range [0, pi]
        return d;
    }

/////FIXTHIS 
    double CosthStar(const FourVec& o) const {
        double dotp = v.Px()*o.Px() + v.Py()*o.Py() + v.Pz()*o.Pz(); 
        double val = dotp/(v.P()*o.P());
        return val;
    }

    FourVec Add(int newID, const FourVec& o) const {
        auto w = v + o.v;
        return FourVec(newID, w.Px(), w.Py(), w.Pz(), w.M());
    }
    
// Accept electron if |eta| < 2.5 and pt > ptMin
    bool accEl(double ptMin = 2.0) const {
        return std::abs(v.eta()) < 2.5 && v.Pt() > ptMin;
    }

// Accept muon if |eta| < 2.4 and pt > ptMin
    bool accMu(double ptMin = 3.0) const {
        return std::abs(v.eta()) < 2.4 && v.Pt() > ptMin;
    }
 

    double Acop(const FourVec& o) const {
        double d = DPhi(o);
        return M_PI - d;
    }

    double SinThetaStarEta(const FourVec & o) const {
// Use calling order to define first and second lepton
        double costh = std::tanh( ( v.eta() - o.eta() )/2.0 );
        double sinth = std::sqrt(1.0 - costh*costh);
        return sinth;
    }

    double CosThetaStarEta(const FourVec & o) const {
// Use calling order to define first and second lepton
        double costh = std::tanh( ( v.eta() - o.eta() )/2.0 );
        return costh;
    }

    double phistar(const FourVec& o) const {
// Compute the phi*_eta variable following 
// Banfi, Redford, Vesterinen, Waller and Wyatt, EPJC 2011 (71) 1600

        double value = std::tan( Acop(o)/2.0 ) * SinThetaStarEta(o);
        return value;

    }


    double mtp(const FourVec& o) const {
        double m1 = Mass();
        double m2 = 0.0;     // Assign NO mass to the MET 4-vector in this calculation
        double ET1 = std::sqrt(m1*m1 + Pt()*Pt());
        double ET2 = o.Pt();
        double dpt = Px()*o.Px() + Py()*o.Py();
        return std::sqrt(m1*m1 + 2.0*(ET1*ET2 - dpt));
    } 

    double MTp(const FourVec& o) const {
        double m1 = Mass(), m2 = o.Mass();
        double ET1 = std::sqrt(m1*m1 + Pt()*Pt());
        double ET2 = std::sqrt(m2*m2 + o.Pt()*o.Pt());
        double dpt = Px()*o.Px() + Py()*o.Py();
        return std::sqrt(m1*m1 + m2*m2 + 2.0*(ET1*ET2 - dpt));
    }

    // Compute transverse mass (using eqn from arXiv:2503.13135)
    double mtsimple(const FourVec& o) const {
        double px1 = Px();
        double py1 = Py();
        double pT1 = std::sqrt(px1 * px1 + py1 * py1);

        double px2 = o.Px();
        double py2 = o.Py();

        const double mfloor = 0.511e-3;
        double ET2 = std::sqrt(mfloor * mfloor + px2 * px2 + py2 * py2);

        double cos_term = (px1 * px2 + py1 * py2) / (pT1 * ET2);
        double mtsq = 2.0 * pT1 * ET2 * (1.0 - cos_term);

        return std::sqrt(mtsq);
    }

    // Rapidity
    double rapidity() const {
        const double DEFAULT = (Pz() < 0.0) ? -7.0 : 7.0;

        if (std::abs(E() - Pz()) < 1.0e-6) {
            return DEFAULT;
        } else {
            return 0.5 * std::log((E() + Pz()) / (E() - Pz()));
        }
    }


    FourVec Boost(const ROOT::Math::XYZVector& beta) const {
    // Construct Lorentz boost from the beta vector
        ROOT::Math::Boost boost(beta);

    // Apply the boost to the internal 4-vector
        LorentzVectorM boosted = boost(v);

    // Return a new FourVec with the same pdgID and flag, but boosted 4-vector. 
    // Since it is boosted it should have the same mass as before - so overwrite that part.
        return FourVec(pdgID, boosted.Px(), boosted.Py(), boosted.Pz(), v.M(), flag);
    }

    double MT2(const FourVec& o, const FourVec& vmiss, double invis_massA, double invis_massB, 
               double desiredPrecisionOnMt2 = 0.0001, bool debug=false) const {

// Assemble quantities from the appropriate elements of the 3 specified 4-vectors (this = A, o = B, vmiss = MET)

// Best to adjust the masses according to PDGID

        double mA = Mass();  double pxA = Px();  double pyA = Py();
        double mB = o.Mass();  double pxB = o.Px();  double pyB = o.Py();
        double pxMiss = vmiss.Px();  double pyMiss = vmiss.Py();

// Calculate MT2 using Lester-Nachman asymmetric MT2 bisection implementation (see 1411.4312).
// For this it is allowed to have invis_massA != invis_massB.

        const double ME  = 0.5109989500e-3;
        const double MMU = 0.1056583755;

// Fix up numerical precision issues with 4-vector masses associated with using the pxpypzE 4-vector constructor for leptons

        if( this->lflavor()== 1 ) mA = ME;
        if( this->lflavor()== 2 ) mA = MMU;
        if( o.lflavor()== 1 ) mB = ME;
        if( o.lflavor()== 2 ) mB = MMU;

        double MT2LN = asymm_mt2_lester_bisect::get_mT2(mA, pxA, pyA, mB, pxB, pyB,
                       pxMiss, pyMiss,invis_massA, invis_massB, desiredPrecisionOnMt2);

// TODO Add some debug printing

        if( debug && MT2LN < 0.12 ){
            std::cout << "Verbose output for low MT2 " << std::endl;
            std::cout << "A system " << mA << " " << pxA << " " << pyA << std::endl;
            std::cout << "B system " << mB << " " << pxB << " " << pyB << std::endl;
            std::cout << "pTmiss   " << pxMiss << " " << pyMiss << std::endl;
            std::cout << "Recoil system " << pxA + pxB + pxMiss << " " << pyA + pyB + pyMiss << std::endl;
            std::cout << "MT2 = " << std::setprecision(12) << MT2LN << std::endl;
        }

        return MT2LN;

    }

// Return 3-vector for angular computations
    ROOT::Math::XYZVector Vect() const {
        return ROOT::Math::XYZVector(v.Px(), v.Py(), v.Pz());
    }

// Return boost vector beta for the system
    ROOT::Math::XYZVector BoostVector() const {
        return ROOT::Math::XYZVector(v.Px()/v.E(), v.Py()/v.E(), v.Pz()/v.E());
    }



};
