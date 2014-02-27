// File Automatically generated by eLiSe
#include "general/all.h"
#include "private/all.h"


class cEqAppui_X_C2MDCBrown: public cElCompiledFonc
{
   public :

      cEqAppui_X_C2MDCBrown();
      void ComputeVal();
      void ComputeValDeriv();
      void ComputeValDerivHessian();
      double * AdrVarLocFromString(const std::string &);
      void SetDCBrown_State_0_0(double);
      void SetXIm(double);
      void SetXTer(double);
      void SetYIm(double);
      void SetYTer(double);
      void SetZTer(double);


      static cAutoAddEntry  mTheAuto;
      static cElCompiledFonc *  Alloc();
   private :

      double mLocDCBrown_State_0_0;
      double mLocXIm;
      double mLocXTer;
      double mLocYIm;
      double mLocYTer;
      double mLocZTer;
};
