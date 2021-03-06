// File Automatically generated by eLiSe
#include "StdAfx.h"
#include "cREgDistVal_CamBilin.h"


cREgDistVal_CamBilin::cREgDistVal_CamBilin():
    cElCompiledFonc(2)
{
   AddIntRef (cIncIntervale("Intr",0,11));
   Close(false);
}



void cREgDistVal_CamBilin::ComputeVal()
{
   double tmp0_ = mLocPts1_x - mLocRegDistValP1_x;
   double tmp1_ = mLocPts1_x - mLocPts0_x;
   double tmp2_ = (tmp0_) / (tmp1_);
   double tmp3_ = mLocPts2_y - mLocRegDistValP1_y;
   double tmp4_ = mLocPts2_y - mLocPts0_y;
   double tmp5_ = (tmp3_) / (tmp4_);
   double tmp6_ = 1 - tmp2_;
   double tmp7_ = 1 - tmp5_;
   double tmp8_ = (tmp2_) * (tmp5_);
   double tmp9_ = (tmp6_) * (tmp5_);
   double tmp10_ = (tmp2_) * (tmp7_);
   double tmp11_ = (tmp6_) * (tmp7_);

  mVal[0] = (mCompCoord[3] * tmp8_ + mCompCoord[5] * tmp9_ + mCompCoord[7] * tmp10_ + mCompCoord[9] * tmp11_) - mLocRegDistKnownVal_x;

  mVal[1] = (mCompCoord[4] * tmp8_ + mCompCoord[6] * tmp9_ + mCompCoord[8] * tmp10_ + mCompCoord[10] * tmp11_) - mLocRegDistKnownVal_y;

}


void cREgDistVal_CamBilin::ComputeValDeriv()
{
   double tmp0_ = mLocPts1_x - mLocRegDistValP1_x;
   double tmp1_ = mLocPts1_x - mLocPts0_x;
   double tmp2_ = (tmp0_) / (tmp1_);
   double tmp3_ = mLocPts2_y - mLocRegDistValP1_y;
   double tmp4_ = mLocPts2_y - mLocPts0_y;
   double tmp5_ = (tmp3_) / (tmp4_);
   double tmp6_ = 1 - tmp2_;
   double tmp7_ = 1 - tmp5_;
   double tmp8_ = (tmp2_) * (tmp5_);
   double tmp9_ = (tmp6_) * (tmp5_);
   double tmp10_ = (tmp2_) * (tmp7_);
   double tmp11_ = (tmp6_) * (tmp7_);

  mVal[0] = (mCompCoord[3] * tmp8_ + mCompCoord[5] * tmp9_ + mCompCoord[7] * tmp10_ + mCompCoord[9] * tmp11_) - mLocRegDistKnownVal_x;

  mCompDer[0][0] = 0;
  mCompDer[0][1] = 0;
  mCompDer[0][2] = 0;
  mCompDer[0][3] = tmp8_;
  mCompDer[0][4] = 0;
  mCompDer[0][5] = tmp9_;
  mCompDer[0][6] = 0;
  mCompDer[0][7] = tmp10_;
  mCompDer[0][8] = 0;
  mCompDer[0][9] = tmp11_;
  mCompDer[0][10] = 0;
  mVal[1] = (mCompCoord[4] * tmp8_ + mCompCoord[6] * tmp9_ + mCompCoord[8] * tmp10_ + mCompCoord[10] * tmp11_) - mLocRegDistKnownVal_y;

  mCompDer[1][0] = 0;
  mCompDer[1][1] = 0;
  mCompDer[1][2] = 0;
  mCompDer[1][3] = 0;
  mCompDer[1][4] = tmp8_;
  mCompDer[1][5] = 0;
  mCompDer[1][6] = tmp9_;
  mCompDer[1][7] = 0;
  mCompDer[1][8] = tmp10_;
  mCompDer[1][9] = 0;
  mCompDer[1][10] = tmp11_;
}


void cREgDistVal_CamBilin::ComputeValDerivHessian()
{
  ELISE_ASSERT(false,"Foncteur cREgDistVal_CamBilin Has no Der Sec");
}

void cREgDistVal_CamBilin::SetPts0_x(double aVal){ mLocPts0_x = aVal;}
void cREgDistVal_CamBilin::SetPts0_y(double aVal){ mLocPts0_y = aVal;}
void cREgDistVal_CamBilin::SetPts1_x(double aVal){ mLocPts1_x = aVal;}
void cREgDistVal_CamBilin::SetPts2_y(double aVal){ mLocPts2_y = aVal;}
void cREgDistVal_CamBilin::SetRegDistKnownVal_x(double aVal){ mLocRegDistKnownVal_x = aVal;}
void cREgDistVal_CamBilin::SetRegDistKnownVal_y(double aVal){ mLocRegDistKnownVal_y = aVal;}
void cREgDistVal_CamBilin::SetRegDistValP1_x(double aVal){ mLocRegDistValP1_x = aVal;}
void cREgDistVal_CamBilin::SetRegDistValP1_y(double aVal){ mLocRegDistValP1_y = aVal;}



double * cREgDistVal_CamBilin::AdrVarLocFromString(const std::string & aName)
{
   if (aName == "Pts0_x") return & mLocPts0_x;
   if (aName == "Pts0_y") return & mLocPts0_y;
   if (aName == "Pts1_x") return & mLocPts1_x;
   if (aName == "Pts2_y") return & mLocPts2_y;
   if (aName == "RegDistKnownVal_x") return & mLocRegDistKnownVal_x;
   if (aName == "RegDistKnownVal_y") return & mLocRegDistKnownVal_y;
   if (aName == "RegDistValP1_x") return & mLocRegDistValP1_x;
   if (aName == "RegDistValP1_y") return & mLocRegDistValP1_y;
   return 0;
}


cElCompiledFonc::cAutoAddEntry cREgDistVal_CamBilin::mTheAuto("cREgDistVal_CamBilin",cREgDistVal_CamBilin::Alloc);


cElCompiledFonc *  cREgDistVal_CamBilin::Alloc()
{  return new cREgDistVal_CamBilin();
}


