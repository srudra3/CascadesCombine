#ifndef FOURVECUTILS_H
#define FOURVECUTILS_H

#include <cmath>

namespace FourVecUtils {

// Coordinate system storing (px, py, pz, m), with derived energy
template<class T>
class PxPyPzM4D {
public:
    using Scalar = T;

    PxPyPzM4D() : fPx(0), fPy(0), fPz(0), fM(0) {}
    PxPyPzM4D(T px, T py, T pz, T m) : fPx(px), fPy(py), fPz(pz), fM(m) {}

   // Templated conversion constructor (needed by ROOT::Boost and conversions)
    template <class OtherCoordSystem>
    PxPyPzM4D(const OtherCoordSystem& other)
        : fPx(other.Px()), fPy(other.Py()), fPz(other.Pz()) {
        T e  = other.E();
        T p2 = fPx*fPx + fPy*fPy + fPz*fPz;
        T m2 = e*e - p2;
        fM = (m2 >= 0) ? std::sqrt(m2) : 0;
    }

    // === ROOT-required coordinate accessors ===
    T Px() const { return fPx; }
    T Py() const { return fPy; }
    T Pz() const { return fPz; }
    T E()  const { return std::sqrt(fPx*fPx + fPy*fPy + fPz*fPz + fM*fM); }
    T M()  const { return fM; }

    // === ROOT-required coordinate setters ===
    void SetPx(T px) { fPx = px; }
    void SetPy(T py) { fPy = py; }
    void SetPz(T pz) { fPz = pz; }
    void SetE(T e) {
        T p2 = fPx*fPx + fPy*fPy + fPz*fPz;
        T m2 = e*e - p2;
        fM = (m2 >= 0) ? std::sqrt(m2) : 0;
    }

    void SetPxPyPzE(T px, T py, T pz, T e) {
        fPx = px;
        fPy = py;
        fPz = pz;
        T p2 = px*px + py*py + pz*pz;
        T m2 = e*e - p2;
        fM = (m2 >= 0) ? std::sqrt(m2) : 0;
    }

    // === Derived kinematic quantities ===
    T Pt()    const { return std::sqrt(fPx*fPx + fPy*fPy); }
    T P()     const { return std::sqrt(fPx*fPx + fPy*fPy + fPz*fPz); }
    T R()     const { return P(); } // Needed by ROOT::LorentzVector::P()
    T Theta() const { return std::atan2(Pt(), fPz); }
    T Eta()   const {
        const T pz = fPz;
        const T p  = P();
        const T epsilon = 1e-12;
        return 0.5 * std::log((p + pz + epsilon) / (p - pz + epsilon));
    }
    T Phi()   const { return std::atan2(fPy, fPx); }

    // === Compatibility with ROOT::Boost and coordinate conversions ===
    T x() const { return Px(); }
    T y() const { return Py(); }
    T z() const { return Pz(); }
    T t() const { return E(); }

private:
    T fPx, fPy, fPz, fM;
};

} // namespace FourVecUtils

#endif // FOURVECUTILS_H

