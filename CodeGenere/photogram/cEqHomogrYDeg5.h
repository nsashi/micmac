// File Automatically generated by eLiSe
#include "StdAfx.h"


class cEqHomogrYDeg5: public cElCompiledFonc
{
   public :

      cEqHomogrYDeg5();
      void ComputeVal();
      void ComputeValDeriv();
      void ComputeValDerivHessian();
      double * AdrVarLocFromString(const std::string &);
      void SetXL1(double);
      void SetXL2(double);
      void SetYL1(double);
      void SetYL2(double);


      static cAutoAddEntry  mTheAuto;
      static cElCompiledFonc *  Alloc();
   private :

      double mLocXL1;
      double mLocXL2;
      double mLocYL1;
      double mLocYL2;
};
