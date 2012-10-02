/*Header-MicMac-eLiSe-25/06/2007

    MicMac : Multi Image Correspondances par Methodes Automatiques de Correlation
    eLiSe  : ELements of an Image Software Environnement

    www.micmac.ign.fr

   
    Copyright : Institut Geographique National
    Author : Marc Pierrot Deseilligny
    Contributors : Gregoire Maillet, Didier Boldo.

[1] M. Pierrot-Deseilligny, N. Paparoditis.
    "A multiresolution and optimization-based image matching approach:
    An application to surface reconstruction from SPOT5-HRS stereo imagery."
    In IAPRS vol XXXVI-1/W41 in ISPRS Workshop On Topographic Mapping From Space
    (With Special Emphasis on Small Satellites), Ankara, Turquie, 02-2006.

[2] M. Pierrot-Deseilligny, "MicMac, un lociel de mise en correspondance
    d'images, adapte au contexte geograhique" to appears in 
    Bulletin d'information de l'Institut Geographique National, 2007.

Francais :

   MicMac est un logiciel de mise en correspondance d'image adapte 
   au contexte de recherche en information geographique. Il s'appuie sur
   la bibliotheque de manipulation d'image eLiSe. Il est distibue sous la
   licences Cecill-B.  Voir en bas de fichier et  http://www.cecill.info.


English :

    MicMac is an open source software specialized in image matching
    for research in geographic information. MicMac is built on the
    eLiSe image library. MicMac is governed by the  "Cecill-B licence".
    See below and http://www.cecill.info.

Header-MicMac-eLiSe-25/06/2007*/


#include "StdAfx.h"


using namespace NS_ParamChantierPhotogram;


/************************************************************************************/
/*                                                                                  */
/*                       Les  Generateurs                                           */
/*                                                                                  */
/************************************************************************************/

bool cGeneratorElem::IsFE() {return false;}


int  cGeneratorElem::DegreOfPolyn(const int * aDegGlob,int aK,eModeControleVarDGen aMode)
{
   // On peut arrive la a cause du mode de controle mixte PP/CD
   if (aMode==eModeContDGCDist)
      return -1;
   if (aMode==eModeContDGDRad)
      return -1;
   if (aMode==eModeContDGDCent)
      return -1;


   ELISE_ASSERT
   (
      aMode==eModeContDGPol,
      "Mode de controle des variable hors Distortion generiqiue stantard"
   );
    return aDegGlob[aK];
}

cCalibDistortion StdToXmlStruct(const cDist_Param_Unif_Gen & aDist) 
{
   cCalibDistortion aRes = ElDistortion22_Gen::XmlDistNoVal();

   cCalibrationInterneUnif aCIU;
   aCIU.TypeModele() = (eModelesCalibUnif) aDist.TypeModele();

   for (int aKv=0 ; aKv<aDist.NbV(); aKv++)
       aCIU.Params().push_back(aDist.KVar(aKv));

   for (int aKs=0 ; aKs<aDist.NbS(); aKs++)
   {
       aCIU.Etats().push_back(aDist.KState(aKs));
   }
   aRes.ModUnif().SetVal(aCIU);
   return aRes;
}
cCalibDistortion cGeneratorElemStd::ToXmlStruct(const cDist_Param_Unif_Gen & aDist,const ElCamera *) 
{
    return StdToXmlStruct(aDist);
}


cCalibrationInterneRadiale  ToXmlDradStrut(const cDist_Param_Unif_Gen & aDist,const ElCamera * aCam)
{
    cCalibrationInterneRadiale aRes;
    int aKMaxNN =4;
    while  ((aKMaxNN>=0) && (aDist.KVar(aKMaxNN)))
        aKMaxNN--;

    for (int aK=0 ; aK<=aKMaxNN ; aK++)
        aRes.CoeffDist().push_back(aDist.KVar(aK));

    aRes.CDist() = aCam->CS()->PP();
    aRes.PPaEqPPs().SetVal(true);

    return aRes;
}


cCalibDistortion cGeneratorState_DRadScaleTr::ToXmlStruct(const cDist_Param_Unif_Gen & aDist,const ElCamera * aCam)
{
   cCalibDistortion aRes = ElDistortion22_Gen::XmlDistNoVal();
   aRes.ModRad().SetVal(ToXmlDradStrut(aDist,aCam));
   return aRes;
}

cTplValGesInit<double>  ValDefDouble(const double & aV)
{
    cTplValGesInit<double> aRes(aV);
    if (aV==0)  
       aRes.SetNoInit();
    else
       aRes.SetVal(aV);

   return aRes;
}

cCalibDistortion cGeneratorState_FraserScaleTr::ToXmlStruct(const cDist_Param_Unif_Gen & aDist,const ElCamera * aCam) 
{
    cCalibDistortion aRes = ElDistortion22_Gen::XmlDistNoVal();
    cCalibrationInternePghrStd aCIPS;

    aCIPS.RadialePart()=ToXmlDradStrut(aDist,aCam);

    aCIPS.P1() = ValDefDouble(aDist.KVar(5));
    aCIPS.P2() = ValDefDouble(aDist.KVar(6));
    aCIPS.b1() = ValDefDouble(aDist.KVar(7));
    aCIPS.b2() = ValDefDouble(aDist.KVar(8));


    aRes.ModPhgrStd().SetVal(aCIPS);
    return aRes;
}


/*
cCalibDistortion cGeneratorState_DRadScaleTr::ToXmlStruct(const cDist_Param_Unif_Gen & aDist,ElCamera * aCam)
{
}
*/




int  cGeneratorDRadFraser::DegreOfPolyn(const int * aDegGlob,int aK,eModeControleVarDGen aMode)
{
    if (aMode==eModeContDGDRad)
    {
        if (aK<5) return aK+1;
        return -1;
    }

    if (aMode==eModeContDGDCent)
    {
        if ((aK>=5) && (aK<7)) return  1;
        return -1;
    }

    return cGeneratorElem::DegreOfPolyn(aDegGlob,aK,aMode);
}

Pt2dr  cGeneratorElem::GuessInv(const Pt2dr & aP,const double *,const double *) 
{
   return aP;
}


void cGeneratorElem::InitClass()
{
}

bool cGeneratorElem::CanExportDistAsGrid()
{
   return true;
}

ElDistortion22_Gen   *  cGeneratorElem::DistPreCond(const double * aVar,const double *aState) 
{
   return 0;
}

bool cGeneratorElem::UseSz() {return false;}
Fonc_Num  cGeneratorElem::NormGradC2M(Pt2d<Fonc_Num> ,Fonc_Num *) {return 0.0;}


    // -----------  cGeneratorNoScaleTr  ----------


bool cGeneratorNoScaleTr::AcceptScalingTranslate()
{
   return false;
}
void cGeneratorNoScaleTr::SetScalingTranslate(const double & F,const Pt2dr & aPP,double * State,double * Vars)
{
   ELISE_ASSERT(false,"cGeneratorNoScaleTr::SetScalingTranslate");
}

    // -----------  cGeneratorState_FPP_ScaleTr  ----------

bool cGeneratorState_FPP_ScaleTr::AcceptScalingTranslate()
{
   return true;
}
void cGeneratorState_FPP_ScaleTr::SetScalingTranslate(const double & F,const Pt2dr & aPP,double * State,double *Vars)
{
   State[0] /= F;
   State[1] = (State[1]-aPP.x) / F;
   State[2] = (State[2]-aPP.y) / F;
}

    // -----------  cGeneratorState_FPP_ScaleTr  ----------

bool  cGeneratorState_DRadScaleTr::AcceptScalingTranslate()
{
   return true;
}
void  cGeneratorState_DRadScaleTr::SetScalingTranslate(const double & aF,const Pt2dr & aPP,double * State,double *Vars)
{
    for (int aK=0 ; aK< 5 ; aK++)
        Vars[aK] *=  pow(aF,2*(1+aK));
}

bool  cGeneratorState_FraserScaleTr::AcceptScalingTranslate()
{
   return true;
}
void  cGeneratorState_FraserScaleTr::SetScalingTranslate(const double & aF,const Pt2dr & aPP,double * State,double *Vars)
{
    cGeneratorState_DRadScaleTr::SetScalingTranslate(aF,aPP,State,Vars);
    for (int aK=5 ; aK< 7 ; aK++)
        Vars[aK] *= aF;
}



/*
   Note sur les modeles de brown et ebner :

     - pas de terme constant : point principal

     - il y a 2 terme affine , car un est rempli par la focale
     et l'autre la rotation autout de l'axe optique

     - il y a 4 terme quadratique, en effet les rotations de basculement
     selon les axe X et Y sont equivalente a des X2 et Y2

     - Dans Brown le terme A14 peut venir d'un modele radiale,

     


*/

template <class Type> 
         Pt2d<Type> cEbnersModel_Generator<Type>::DistElem(bool UsePC,const Pt2d<Type> &aP,const Type * V,const Type * States,const Type & aFoc,const Pt2d<Type> & aPP)
{
   const Type & B = States[0];
   const Type & x = aP.x;
   const Type & y = aP.y;

   Type xy = x * y;
   Type B2N = B * B * (2.0/3.0);

   Type x2N = x * x - B2N;
   Type y2N = y * y - B2N;


   V--; // Pour avoir les indices de l'article initial

   Type aXOut =  (1+ V[ 1]) * x
                 + V[ 2] * y
		 - V[ 3] * 2* x2N
		 + V[ 4] * xy
		 + V[ 5] * y2N
		 + V[ 7] * x* y2N
		 + V[ 9] * y* x2N
		 + V[11] * x2N * y2N;

    Type aYOut =  (1- V[ 1]) * y
                  + V[ 2] * x
		  + V[ 3] * xy
		  - V[ 4] * 2 *y2N
		  + V[ 6] * x2N
		  + V[ 8] * y * x2N
		  + V[10] * x * y2N
		  + V[12] * x2N * y2N;

   return Pt2d<Type>(aXOut,aYOut);
}




template <class Type> 
         Pt2d<Type> cDCBrownModel_Generator<Type>::DistElem(bool UsePC,const Pt2d<Type> & aP,const Type * V,const Type * States,const Type & aFoc,const Pt2d<Type> & aPP)
{
   const Type & F = States[0];
   const Type & x = aP.x;
   const Type & y = aP.y;

   Type xy = x * y;
   Type x2 = x  *x ;
   Type y2 = y * y ;
   Type xy2 = x * y2;
   Type x2y = x2 * y;
   Type x2y2 = x2 * y2;
   Type x2_P_y2 = x2+y2;


   V--; // Pour avoir les indices de l'article initial

   Type aXOut =  (1+V[ 1]) * x
                + V[ 2] * y
                + V[ 3] * xy
		+ V[ 4] * y2
		+ V[ 5] * x2y
		+ V[ 6] * xy2
		+ V[ 7] * x2y2
		+ V[13] * (x/F) * x2y2
		+ V[14] * x * x2_P_y2;

   Type aYOut =   y
                + V[ 8] * xy
                + V[ 9] * x2
		+ V[10] * x2y
		+ V[11] * xy2
		+ V[12] * x2y2
		+ V[13] * (y/F) * x2y2
		+ V[14] * y * x2_P_y2;

   return Pt2d<Type>(aXOut,aYOut);
}

template <class Type> 
         Pt2d<Type> DistElemDRad
         (
               bool UsePC,
               const Pt2d<Type> &aP,
               const Type * V,
               const Type * States,
               const Type & aFoc,
               const Pt2d<Type> & aPP, 
               bool  WithFraser
        )
{
     Pt2d<Type>  aDec = aP - aPP;
     Type aDX2 = ElSquare(aDec.x) ;
     Type aDY2 = ElSquare(aDec.y) ;
     Type aRho2 =  aDX2 + aDY2 ;
     Type aRho2N = 1;
     Type aSomRho = 0;

     for (int aK= 0 ; aK < 5 ; aK++)
     {
          aRho2N = aRho2N * aRho2;
          aSomRho = aSomRho + V[aK] * aRho2N;
     }
    
     Pt2d<Type>  aRes =  aP +  Pt2d<Type>(aDec.x * aSomRho ,aDec.y*aSomRho);
     if (WithFraser)
     {
           Type aDXY = aDec.x * aDec.y;
           aRes.x =  aRes.x + (2*aDX2+aRho2)*V[5] + 2*aDXY*V[6] + V[7]*aDec.x + V[8]*aDec.y; 
           aRes.y =  aRes.y + (2*aDY2+aRho2)*V[6] + 2*aDXY*V[5];
     }

     return aRes;
}

template <class Type> 
         Pt2d<Type> cDRadModel_Generator<Type>::DistElem
         (
               bool UsePC,
               const Pt2d<Type> &aP,
               const Type * V,
               const Type * States,
               const Type & aFoc,
               const Pt2d<Type> & aPP
        )
{
   return DistElemDRad(UsePC,aP,V,States,aFoc,aPP,false);
}


template <class Type> 
         Pt2d<Type> cFraserModel_Generator<Type>::DistElem
         (
               bool UsePC,
               const Pt2d<Type> &aP,
               const Type * V,
               const Type * States,
               const Type & aFoc,
               const Pt2d<Type> & aPP
        )
{
   return DistElemDRad(UsePC,aP,V,States,aFoc,aPP,true);
}





template <class Type> 
         Pt2d<Type> cDistGen_Deg2_Generator<Type>::DistElem(bool UsePC,const Pt2d<Type> &aP,const Type * V,const Type * States,const Type & aFoc,const Pt2d<Type> & aPP)
{
// std::cout << aP.x << aP.y << "\n";
   Type  x = (aP.x - States[1]) / States[0];
   Type  y = (aP.y - States[2]) / States[0];

// std::cout << "S0 " <<  States[0] << " " << x << " " << y << "\n";

   Type xy = x * y;
   Type x2N = x * x ;
   Type y2N = y * y ;


   Type aXOut =  (1+ V[ 0]) * x
                 + V[ 1] * y
		 - V[ 2] * 2* x2N
		 + V[ 3] * xy
		 + V[ 4] * y2N ;


    Type aYOut =  (1- V[ 0]) * y
                  + V[ 1] * x
		  + V[ 2] * xy
		  - V[ 3] * 2 *y2N
		  + V[ 5] * x2N ;

   return Pt2d<Type>(States[1]+aXOut*States[0],States[2]+aYOut*States[0]);
}

template <class Type>   Pt2d<Type>  DistDegN(bool UsePC,const Pt2d<Type> &aP,const Type * V,const Type * States,int aDegN)
{
   Type  x = (aP.x - States[1]) / States[0];
   Type  y = (aP.y - States[2]) / States[0];


   Pt2d<Type> aResN (0,0);

   std::vector<Type> aVXY;
   aVXY.push_back(1);
   int aInd=6;
   for (int aD=1 ; aD<= aDegN ; aD++)
   {
       aVXY.push_back(y*aVXY.back());
       for (int aK=0 ; aK< aD ; aK++)
          aVXY[aK] =  aVXY[aK] * x;
       if (aD >=3)
       {
            for (int aK=0 ; aK<= aD ; aK++)
                aResN.x = aResN.x+ V[aInd++] *aVXY[aK];
            for (int aK=0 ; aK<= aD ; aK++)
                aResN.y = aResN.y+ V[aInd++] *aVXY[aK];
       }
   }
   Pt2d<Type> aRes2 =  cDistGen_Deg2_Generator<Type>::DistElem(UsePC,aP,V,States,0,Pt2d<Type>(0,0));
   
    return Pt2d<Type>
           (
              aRes2.x + aResN.x*States[0],
              aRes2.y + aResN.y*States[0]
           );
   //return   cDistGen_Deg2_Generator<Type>::Dist(aP,V,States) + aResN *States[0];
}


template <class Type> 
         Pt2d<Type> cDistGen_Deg3_Generator<Type>::DistElem(bool UsePC,const Pt2d<Type> &aP,const Type * V,const Type * States,const Type & aFoc,const Pt2d<Type> & aPP)
{
  return DistDegN(UsePC,aP,V,States,3);
/*
   Type  x = aP.x / States[0];
   Type  y = aP.y / States[0];

   Type x2 = x * x;
   Type y2 = y * y;

   Type x3 = x2 * x;
   Type y3 = y2 * y;

   Type x2y = x2 * y;
   Type xy2 = x * y2;


   return     cDistGen_Deg2_Generator<Type>::Dist(aP,V,States)
           +  Pt2d<Type>
	      (
	          (x3*V[6] + x2y *V[7] + xy2 * V[8] + y3 * V [9]) *States[0],
	          (x3*V[10] + x2y *V[11] + xy2 * V[12] + y3 * V [13]) *States[0]
	      );
*/
}

template <class Type> 
         Pt2d<Type> cDistGen_Deg4_Generator<Type>::DistElem(bool UsePC,const Pt2d<Type> &aP,const Type * V,const Type * States,const Type & aFoc,const Pt2d<Type> & aPP)
{
    return DistDegN(UsePC,aP,V,States,4);
/*
   Type  x = aP.x / States[0];
   Type  y = aP.y / States[0];

   Type x2 = x * x;
   Type y2 = y * y;

   Type x3 = x2 * x;
   Type y3 = y2 * y;

   Type x4 = x3 * x;
   Type y4 = y3 * y;


   Type x3y = x3 * y;
   Type x2y2 = x2 * y2;
   Type xy3 = x * y3;


   return     cDistGen_Deg3_Generator<Type>::Dist(aP,V,States)
           +  Pt2d<Type>
	      (
	          (x4*V[14] + x3y *V[15] + x2y2 * V[16] + xy3 * V [17] + y4 * V[18]) * States[0],
	          (x4*V[19] + x3y *V[20] + x2y2 * V[21] + xy3 * V [22] + y4 * V[23]) * States[0]
	      );
*/
}

/*
void CmpPt(Pt2d<Fonc_Num>,Pt2d<Fonc_Num>)
{
}
void CmpPt(Pt2d<double> aP1,Pt2d<double> aP2)
{
   std::cout << euclid(aP1,aP2) << aP1 << "\n";
   ELISE_ASSERT(euclid(aP1,aP2) <1e-5,"CmpPt");
}
*/



template <class Type> 
         Pt2d<Type> cDistGen_Deg5_Generator<Type>::DistElem(bool UsePC,const Pt2d<Type> &aP,const Type * V,const Type * States,const Type & aFoc,const Pt2d<Type> & aPP)
{
    return DistDegN(UsePC,aP,V,States,5);
/*
   Type  x = aP.x / States[0];
   Type  y = aP.y / States[0];

   Type x2 = x * x;
   Type y2 = y * y;

   Type x3 = x2 * x;
   Type y3 = y2 * y;

   Type x4 = x3 * x;
   Type y4 = y3 * y;

   Type x5 = x4 * x;
   Type y5 = y4 * y;

   Type x4y = x4 * y;
   Type x3y2 = x3 * y2;
   Type x2y3 = x2 * y3;
   Type xy4 = x * y4;


    Pt2d<Type> aRes =
               cDistGen_Deg4_Generator<Type>::Dist(aP,V,States)
           +  Pt2d<Type>
	      (
	          (x5*V[24] + x4y *V[25] + x3y2 * V[26] + x2y3 * V [27] + xy4 * V[28] + y5 * V[29]) * States[0],
	          (x5*V[30] + x4y *V[31] + x3y2 * V[32] + x2y3 * V [33] + xy4 * V[34] + y5 * V[35]) * States[0]
	      );

    Pt2d<Type> aR2 = DistDegN(aP,V,States,5);
    CmpPt(aRes,aR2);
   return aRes;
*/
}

template <class Type> 
         Pt2d<Type> cDistGen_Deg6_Generator<Type>::DistElem(bool UsePC,const Pt2d<Type> &aP,const Type * V,const Type * States,const Type & aFoc,const Pt2d<Type> & aPP)
{
  return DistDegN(UsePC,aP,V,States,6);
}


template <class Type> 
         Pt2d<Type> cDistGen_Deg7_Generator<Type>::DistElem(bool UsePC,const Pt2d<Type> &aP,const Type * V,const Type * States,const Type & aFoc,const Pt2d<Type> & aPP)
{
  return DistDegN(UsePC,aP,V,States,7);
}



/*********************************************************************/
/*                                                                   */
/*                         cDist_Param_Unif_Gen                      */
/*                                                                   */
/*********************************************************************/

cDist_Param_Unif_Gen::cDist_Param_Unif_Gen(const Pt2dr & aSzIm,CamStenope * aCam) :
   mSzIm (aSzIm),
   mPC   (0),
   mCam  (aCam)
{
}

cDist_Param_Unif_Gen::~cDist_Param_Unif_Gen() 
{
}


const Pt2dr  & cDist_Param_Unif_Gen::SzIm() const
{
   return mSzIm;
}




void  cDist_Param_Unif_Gen::Diff(ElMatrix<REAL> & aMat,Pt2dr aP) const
{
   DiffByDiffFinies(aMat,aP,euclid(mSzIm)/400.0);
}

/*********************************************************************/
/*                                                                   */
/*                         cCamera_Basik_PU                          */
/*                                                                   */
/*********************************************************************/

/*
template <class TDistR,class TDistF,const int NbVar,const int NbState>
    class  cCamera_Basik_PU : public CamStenope
{
     public :
          typedef cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>   tDist;


           cCamera_Basik_PU
           (
                 bool isDistC2M,
                 tDist * aDist
           ) :
              CamStenope(isDistC2M,1.0,Pt2dr(0,0)),
              mDist     (aDist)
           {
                SetSz(Pt2di(1,1));
           }


          // Version generique pour manipulation par CamStenope
           ElDistortion22_Gen   &  Dist() {return *mDist;}
           const ElDistortion22_Gen   &  Dist() const {return *mDist;}

     private  :
            tDist *   mDist;
};
*/



/*********************************************************************/
/*                                                                   */
/*                         cDist_Param_Unif                          */
/*                                                                   */
/*********************************************************************/

static void init(double * aVLoc,int aNbLoc,const std::vector<double> * aVInit,const std::string & aName)
{
    for (int aK=0 ; aK<aNbLoc ; aK++)
         aVLoc[aK] = 0;

    if (aVInit)
    {
         int aNbInit = aVInit->size();
         if ((aNbInit!=0) && (aNbInit!=aNbLoc))
	 {
	     std::cout << "Init Incomplete For modele " << aName 
	               << " Nb  ="<< aNbInit << "\n";
             // ELISE_ASSERT(false," Nb Param or State incoherent ");
	 }
	 for (int aK=0 ; aK<aNbInit ; aK++)
	     aVLoc[aK] =  (*aVInit)[aK];
    }
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
     cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::cDist_Param_Unif
     (
           Pt2dr                        aSzIm,
           CamStenope *                 aCam,
           const std::vector<double> *  aVParam,
           const std::vector<double> *  aVEtats
     ) :
       cDist_Param_Unif_Gen  (aSzIm,aCam)
{
    init(mVars,NbVar,aVParam,TheName);
    init(mStates,NbState,aVEtats,TheName);

    TDistR::InitClass();

    mPC = TDistR::DistPreCond(mVars,mStates);
    if (mPC)
    {
        SetParamConvInvDiff(100,1e-3);
    }

}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
     cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::~cDist_Param_Unif()
{
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
const std::string &   cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>:: NameType() const
{
   return TheName;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
Pt2dr   cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::GuessInv(const Pt2dr & aP) const
{
   return TDistR::GuessInv(aP,mVars,mStates);
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
bool cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>:: OwnInverse(Pt2dr & aP) const
{
   if (mPC)
   {
      bool aVUPC = Dist22Gen_UsePreConditionner();
      SetDist22Gen_UsePreConditionner(false);
      aP = ComputeInvFromDirByDiff (aP,aP,false) ;
      SetDist22Gen_UsePreConditionner(aVUPC);
      if (! Dist22Gen_SupressPreCondInInverse())
         aP  = mPC->Inverse(aP);
      return true;
   }
   return false;
   // return TDistR::OwnInverse(aP,mVars,mStates);
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
Pt2dr  cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::Direct(Pt2dr aP) const
{
    return TDistR::DistElem(Dist22Gen_UsePreConditionner(),aP,mVars,mStates,mCam->Focale(),mCam->PP());
    // return mTDist.DistElem(aP,mVars,mStates);
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
double & cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::KVar(int aK)
{
    ELISE_ASSERT((aK>=0)&&(aK<NbVar),"cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::KVar");
    return mVars[aK];
}
template <class TDistR,class TDistF,const int NbVar,const int NbState>
double & cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::KState(int aK)
{
    ELISE_ASSERT((aK>=0)&&(aK<NbState),"cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::KState");
    return mStates[aK];
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
const double &  cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::KVar(int aK) const
{
    ELISE_ASSERT((aK>=0)&&(aK<NbVar),"cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::KVar");
    return mVars[aK];
}
template <class TDistR,class TDistF,const int NbVar,const int NbState>
const double  & cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::KState(int aK) const
{
    ELISE_ASSERT((aK>=0)&&(aK<NbState),"cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::KState");
    return mStates[aK];
}




template <class TDistR,class TDistF,const int NbVar,const int NbState>
int  cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::TypeModele() const
{
   return TheType;
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
int  cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::NbV() const
{
   return NbVar;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
int  cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::NbS() const
{
   return NbState;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
bool  cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::AcceptScaling() const
{
   return TDistR::AcceptScalingTranslate();
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
bool  cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::AcceptTranslate() const
{
   return TDistR::AcceptScalingTranslate();
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
void cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::V_SetScalingTranslate(const double & F,const Pt2dr & aPP)
{
   TDistR::SetScalingTranslate(F,aPP,mStates,mVars);
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
NS_ParamChantierPhotogram::cCalibDistortion cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::ToXmlStruct(const ElCamera * aCam) const
{
   return TDistR::ToXmlStruct(*this,aCam);
}



/*********************************************************************/
/*                                                                   */
/*                     cCamera_Param_Unif_Gen                        */
/*                                                                   */
/*********************************************************************/


cCamera_Param_Unif_Gen::cCamera_Param_Unif_Gen
(
      bool isDistC2M,
      REAL Focale,
      Pt2dr PP,
      const tParamAFocal  & aParam
) :
   CamStenope(isDistC2M,Focale,PP,aParam)
{
}

cCamera_Param_Unif_Gen::~cCamera_Param_Unif_Gen()
{
}

cParamIntrinsequeFormel * cCamera_Param_Unif_Gen::AllocParamInc(bool isDistC2M,cSetEqFormelles & aSet)
{
   return PIF_Gen(isDistC2M,aSet);
}
/*********************************************************************/
/*                                                                   */
/*                         cCamera_Param_Unif                        */
/*                                                                   */
/*********************************************************************/

template <class TDistR,class TDistF,const int NbVar,const int NbState>
cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::cCamera_Param_Unif
(
     bool isDistC2M,
     REAL Focale, 
     Pt2dr PP,
     Pt2dr aSzIm,
     const tParamAFocal  & aParam,
     const std::vector<double> * aVParams,
     const std::vector<double> * aVEtats
) :
   cCamera_Param_Unif_Gen(isDistC2M,Focale,PP,aParam),
   mDist (aSzIm,this,aVParams,aVEtats)
{
   // SetDistInverse();
}

/*
template <class TDistR,class TDistF,const int NbVar,const int NbState> TDistR & cDist_Param_Unif<TDistR,TDistF,NbVar,NbState>::TDist()
{
   return mTDist;
}
*/

template <class TDistR,class TDistF,const int NbVar,const int NbState>
   cDist_Param_Unif<TDistR,TDistF,NbVar,NbState> & 
       cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::DistUnif()
{
    return mDist;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
   const cDist_Param_Unif<TDistR,TDistF,NbVar,NbState> & 
       cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::DistUnif() const
{
    return mDist;
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
   cDist_Param_Unif_Gen & cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::DistUnifGen()
{
    return mDist;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
   const cDist_Param_Unif_Gen & cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::DistUnifGen() const
{
    return mDist;
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
   bool  cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::CanExportDistAsGrid() const
{
    return TDistR::CanExportDistAsGrid();
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
   bool  cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::IsFE() const
{
    return TDistR::IsFE();
}




template <class TDistR,class TDistF,const int NbVar,const int NbState>
   ElDistortion22_Gen & cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::Dist()
{
    return mDist;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
   const ElDistortion22_Gen & cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::Dist() const
{
    return mDist;
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
cPIF_Unif_Gen *  cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::PIF_Gen(bool isDistC2M,cSetEqFormelles & aSet)
{
   return PIF(isDistC2M,aSet);
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
 cPIF_Unif<TDistR,TDistF,NbVar,NbState> *  
    cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>::PIF(bool isDistC2M,cSetEqFormelles & aSet)
{
   return tPIF::Alloc(isDistC2M,this,aSet);
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
 ElDistortion22_Gen   *   cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>:: DistPreCond() const
{
   return TDistR::DistPreCond(&(mDist.KVar(0)),&(mDist.KState(0)));
}



/*********************************************************************/
/*                                                                   */
/*                         cPIF_Unif_Gen                             */
/*                                                                   */
/*********************************************************************/

cPIF_Unif_Gen::cPIF_Unif_Gen
(
    bool                      isC2M,
    cCamera_Param_Unif_Gen *  aCam,
    cSetEqFormelles & aSet
) :
  cParamIntrinsequeFormel(isC2M,aCam,aSet,true)
{
}

cPIF_Unif_Gen::~cPIF_Unif_Gen()
{
}


/*********************************************************************/
/*                                                                   */
/*                         cPIF_Unif                                 */
/*                                                                   */
/*********************************************************************/

template <class TDistR,class TDistF,const int NbVar,const int NbState>
cPIF_Unif<TDistR,TDistF,NbVar,NbState>::cPIF_Unif
(
    bool isDistC2M,
    tCam * aCam,
    cSetEqFormelles & aSet
)  :
   cPIF_Unif_Gen           (isDistC2M,aCam,aSet), 
   mIndInc0                (aSet.Alloc().CurInc()),
   mDistInit               (aCam->DistUnif()),
   mDistCur                (aCam->DistUnif()),
   mCurPIF                 (0)
{
  SetFocFree(true);
  SetPPFree(true);

  for (int aK=0 ; aK< NbVar ; aK++)
  {
      mVarIsFree[aK] = true;
      mVars[aK] =  aSet.Alloc().NewF(&(mDistCur.KVar(aK)));
      mTolCstr[aK] = cContrainteEQF::theContrStricte;
  }

   for (int aKS=0 ; aKS< NbState ; aKS++)
   {
       std::string aName =  mDistCur.NameType() 
                          + std::string("_State_")
			  + ToString(aKS)
			  + std::string("_");
       for (int aKV=0 ; aKV<2 ; aKV++)
       {
           mNameSt[aKV][aKS] = aName + ToString(aKV);
           mStates[aKV][aKS] = cVarSpec(0,mNameSt[aKV][aKS]);
       }
   }

   NV_UpdateCurPIF();

  // CloseEEF();
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
cPIF_Unif<TDistR,TDistF,NbVar,NbState>::~cPIF_Unif()
{
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
std::string cPIF_Unif<TDistR,TDistF,NbVar,NbState>::NameType() const
{
   return mDistCur.NameType();
}




template <class TDistR,class TDistF,const int NbVar,const int NbState>
bool cPIF_Unif<TDistR,TDistF,NbVar,NbState>::IsDistFiged() const
{
    for (int aK=0; aK<NbVar ; aK++)
    {
        if ( mVarIsFree[aK])
           return false;
    }
    return true;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
cMultiContEQF cPIF_Unif<TDistR,TDistF,NbVar,NbState>::StdContraintes()
{
    cMultiContEQF aRes = cParamIntrinsequeFormel::StdContraintes();

    for (int aK=0; aK<NbVar ; aK++)
    {
        if (! mVarIsFree[aK])
	{
	    AddFoncRappInit
	    (
	         aRes,
                 NbInc()-NbVar+aK,
                 NbInc()-NbVar+aK+1,
		 mTolCstr[aK]
	    );
	}
    }

    return aRes;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
Pt2d<Fonc_Num>  cPIF_Unif<TDistR,TDistF,NbVar,NbState>::VDist(Pt2d<Fonc_Num> aP,int aKCam)
{
    return TDistF::DistElem(true,aP,mVars,mStates[aKCam],mFFoc,mFPP);
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
 cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState>  
      cPIF_Unif<TDistR,TDistF,NbVar,NbState>::CurPIFUnif()
{
   double * adrV0 = &mDistCur.KVar(0);
   std::vector<double> aVV(adrV0,adrV0+NbVar);

   double * adrS0 = &mDistCur.KState(0);
   std::vector<double> aVS(adrS0,adrS0+NbState);

  cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState> aRes =
              cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState> 
              (
	          DistIsC2M(),
                  CurFocale(),
                  CurPP(),
		  mDistInit.SzIm(),
                  CurAFoc(),
		  &aVV,&aVS
	      );
   // aRes.SetDistInverse();
   aRes.HeritComplAndSz(*CamInit());

   return aRes;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
 void cPIF_Unif<TDistR,TDistF,NbVar,NbState>::NV_UpdateCurPIF()
{
    mCurPIF =   new cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState> (CurPIFUnif());
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
 void cPIF_Unif<TDistR,TDistF,NbVar,NbState>::UpdateCurPIF()
{
   NV_UpdateCurPIF();
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
 CamStenope * cPIF_Unif<TDistR,TDistF,NbVar,NbState>::CurPIF()
{
   return mCurPIF;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
 CamStenope * cPIF_Unif<TDistR,TDistF,NbVar,NbState>::DupCurPIF()
{
    return   new cCamera_Param_Unif<TDistR,TDistF,NbVar,NbState> (CurPIFUnif());
}




template <class TDistR,class TDistF,const int NbVar,const int NbState>
void cPIF_Unif<TDistR,TDistF,NbVar,NbState>::InitStateOfFoncteur(cElCompiledFonc * aFctr,int aKCam)
{
/*
    for (int aKS=0 ; aKS<NbState ; aKS++)
       *(aFctr->RequireAdrVarLocFromString(mNameSt[aKCam][aKS])) =  mDistInit.KState(aKS);
*/
    for (int aKS=0 ; aKS<NbState ; aKS++)
    {
       double * anAdr = aFctr->AdrVarLocFromString(mNameSt[aKCam][aKS]);
       if (anAdr)
         *anAdr = mDistInit.KState(aKS);
    }
}







template <class TDistR,class TDistF,const int NbVar,const int NbState>
cPIF_Unif<TDistR,TDistF,NbVar,NbState> * 
     cPIF_Unif<TDistR,TDistF,NbVar,NbState>::Alloc
     (
         bool isDistC2M,
         tCam * aCam,
         cSetEqFormelles & aSet
     )
{
    cSetEqFormelles::VerifC2MForPIF(isDistC2M,aCam);
    tPIF * aRes = new  tPIF(isDistC2M,aCam,aSet);
     aRes->CloseEEF();
     aSet.AddObj2Kill(aRes);
    return aRes;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
void  cPIF_Unif<TDistR,TDistF,NbVar,NbState>::VerifIndexVar(int aK)
{
   ELISE_ASSERT((aK>=0)&&(aK<NbVar),"Hors indexe dans cPIF_Unif");
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
void  cPIF_Unif<TDistR,TDistF,NbVar,NbState>::SetFigeKthParam(int aK,double aTol)
{

    VerifIndexVar(aK);
    double aDiag = euclid(mDistInit.SzIm()) / 2.0;

    aTol =  (aTol<=0)                       ?
            cContrainteEQF::theContrStricte :
	    aTol / pow(aDiag,mDegrePolyn[aK]);
    mVarIsFree[aK] = false;
    mTolCstr[aK] = aTol;
}
template <class TDistR,class TDistF,const int NbVar,const int NbState>
void  cPIF_Unif<TDistR,TDistF,NbVar,NbState>::SetFreeKthParam(int aK)
{
    VerifIndexVar(aK);
    mVarIsFree[aK] = true;
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
void  cPIF_Unif<TDistR,TDistF,NbVar,NbState>::FigeIfDegreSup(int aDegre,double aTol,eModeControleVarDGen aModeControl)
{
    for (int aKV=0 ; aKV<NbVar ; aKV++)
    {
        int aDegK = TDistR::DegreOfPolyn(mDegrePolyn,aKV,aModeControl);
        if (aDegK >=0)
        {
             if (aDegK > aDegre)
	         SetFigeKthParam(aKV,aTol);
             else
	         SetFreeKthParam(aKV);
        }
    }
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
void  cPIF_Unif<TDistR,TDistF,NbVar,NbState>::FigeD1_Ou_IfDegreSup(int aDegre,double aTol)
{
    for (int aKV=0 ; aKV<NbVar ; aKV++)
    {
        int aDegK = TDistR::DegreOfPolyn(mDegrePolyn,aKV,eModeContDGPol);
        if (aDegK >=0)
        {
            if (( aDegK> aDegre) ||  (aDegK==1))
	        SetFigeKthParam(aKV,aTol);
            else
	        SetFreeKthParam(aKV);
        }
    }
}


template <class TDistR,class TDistF,const int NbVar,const int NbState>
 Fonc_Num  cPIF_Unif<TDistR,TDistF,NbVar,NbState>::NormGradC2M(Pt2d<Fonc_Num>  aP)
{
   return  TDistF::NormGradC2M(aP,mStates[0]);
}

template <class TDistR,class TDistF,const int NbVar,const int NbState>
 bool  cPIF_Unif<TDistR,TDistF,NbVar,NbState>:: UseSz() const
{
   return TDistF::UseSz() ;
}







/*********************************************************************/
/*                                                                   */
/*                                                                   */
/*********************************************************************/
template <> const std::string  cDist_Ebner::TheName="Ebner";
template <> const int   cDist_Ebner::TheType= (int) eModeleEbner;

template class cDist_Param_Unif<cEbnersModel_Generator<double>,cEbnersModel_Generator<Fonc_Num>,12,1> ;
template class cCamera_Param_Unif<cEbnersModel_Generator<double>,cEbnersModel_Generator<Fonc_Num>,12,1> ;
template class cPIF_Unif<cEbnersModel_Generator<double>,cEbnersModel_Generator<Fonc_Num>,12,1> ;

template <> const int 
   cPIF_Unif<cEbnersModel_Generator<double>,cEbnersModel_Generator<Fonc_Num>,12,1>::mDegrePolyn[12]
          = { 1,1,2,2,2,2,     3,3,3, 3, 4, 4};
         //   1,2,3,4,5,6,     7,8,9,10,11,12};



template <> const std::string  cDist_DRad_PPaEqPPs::TheName="DRad_PPaEqPPs";
template <> const std::string  cDist_Fraser_PPaEqPPs::TheName="Fraser_PPaEqPPs";
template <> const int   cDist_DRad_PPaEqPPs::TheType= (int) eModele_DRad_PPaEqPPs;
template <> const int   cDist_Fraser_PPaEqPPs::TheType= (int) eModele_Fraser_PPaEqPPs;
template class cDist_Param_Unif<cDRadModel_Generator<double>,cDRadModel_Generator<Fonc_Num>,5,1> ;
template class cDist_Param_Unif<cFraserModel_Generator<double>,cFraserModel_Generator<Fonc_Num>,9,1> ;
template class cCamera_Param_Unif<cDRadModel_Generator<double>,cDRadModel_Generator<Fonc_Num>,5,1> ;
template class cCamera_Param_Unif<cFraserModel_Generator<double>,cFraserModel_Generator<Fonc_Num>,9,1> ;
template class cPIF_Unif<cDRadModel_Generator<double>,cDRadModel_Generator<Fonc_Num>,5,1> ;
template class cPIF_Unif<cFraserModel_Generator<double>,cFraserModel_Generator<Fonc_Num>,9,1> ;


template <> const int 
   cPIF_Unif<cDRadModel_Generator<double>,cDRadModel_Generator<Fonc_Num>,5,1>::mDegrePolyn[5]={3,5,7,9,11};
template <> const int 
   cPIF_Unif<cFraserModel_Generator<double>,cFraserModel_Generator<Fonc_Num>,9,1>::mDegrePolyn[9]={3,5,7,9,11,2,2,1,1};



template <> const std::string  cDist_DCBrown::TheName="DCBrown";
template <> const int   cDist_DCBrown::TheType= (int) eModeleDCBrown;

template <> const int 
   cPIF_Unif<cDCBrownModel_Generator<double>,cDCBrownModel_Generator<Fonc_Num>,14,1>::mDegrePolyn[14]
          = { 1,1,2,2,3,3,4,   2,2, 3, 3, 4, 4, 3};
         //   1,2,3,4,5,6,7,   8,9,10,11,12,13,14};  // 13 degre 4 a cause du 1/F

template class cDist_Param_Unif<cDCBrownModel_Generator<double>,cDCBrownModel_Generator<Fonc_Num>,14,1> ;
template class cCamera_Param_Unif<cDCBrownModel_Generator<double>,cDCBrownModel_Generator<Fonc_Num>,14,1> ;
template class cPIF_Unif<cDCBrownModel_Generator<double>,cDCBrownModel_Generator<Fonc_Num>,14,1> ;



template <> const std::string  cDist_Polyn2::TheName="Polyn2";
template <> const int   cDist_Polyn2::TheType= (int) eModelePolyDeg2;

template class cDist_Param_Unif<cDistGen_Deg2_Generator<double>,cDistGen_Deg2_Generator<Fonc_Num>,6,3> ;
template class cCamera_Param_Unif<cDistGen_Deg2_Generator<double>,cDistGen_Deg2_Generator<Fonc_Num>,6,3> ;
template class cPIF_Unif<cDistGen_Deg2_Generator<double>,cDistGen_Deg2_Generator<Fonc_Num>,6,3> ;

template <> const int 
   cPIF_Unif<cDistGen_Deg2_Generator<double>,cDistGen_Deg2_Generator<Fonc_Num>,6,3>::mDegrePolyn[6]
          = { 1,1,2,2,2,2};


template <> const std::string  cDist_Polyn3::TheName="Polyn3";
template <> const int   cDist_Polyn3::TheType= (int) eModelePolyDeg3;

template class cDist_Param_Unif<cDistGen_Deg3_Generator<double>,cDistGen_Deg3_Generator<Fonc_Num>,14,3> ;
template class cCamera_Param_Unif<cDistGen_Deg3_Generator<double>,cDistGen_Deg3_Generator<Fonc_Num>,14,3> ;
template class cPIF_Unif<cDistGen_Deg3_Generator<double>,cDistGen_Deg3_Generator<Fonc_Num>,14,3> ;

template <> const int 
   cPIF_Unif<cDistGen_Deg3_Generator<double>,cDistGen_Deg3_Generator<Fonc_Num>,14,3>::mDegrePolyn[14]
          = { 1,1,2,2,2,2,3,3,3,3,3,3,3,3};



template <> const std::string  cDist_Polyn4::TheName="Polyn4";
template <> const int   cDist_Polyn4::TheType= (int) eModelePolyDeg4;

template class cDist_Param_Unif<cDistGen_Deg4_Generator<double>,cDistGen_Deg4_Generator<Fonc_Num>,24,3> ;
template class cCamera_Param_Unif<cDistGen_Deg4_Generator<double>,cDistGen_Deg4_Generator<Fonc_Num>,24,3> ;
template class cPIF_Unif<cDistGen_Deg4_Generator<double>,cDistGen_Deg4_Generator<Fonc_Num>,24,3> ;

template <> const int 
   cPIF_Unif<cDistGen_Deg4_Generator<double>,cDistGen_Deg4_Generator<Fonc_Num>,24,3>::mDegrePolyn[24]
          = { 1,1,2,2,2,2,3,3,3,3,3,3,3,3 ,4,4,4,4,4  ,4,4,4,4,4};


template <> const std::string  cDist_Polyn5::TheName="Polyn5";
template <> const int   cDist_Polyn5::TheType= (int) eModelePolyDeg5;

template class cDist_Param_Unif<cDistGen_Deg5_Generator<double>,cDistGen_Deg5_Generator<Fonc_Num>,36,3> ;
template class cCamera_Param_Unif<cDistGen_Deg5_Generator<double>,cDistGen_Deg5_Generator<Fonc_Num>,36,3> ;
template class cPIF_Unif<cDistGen_Deg5_Generator<double>,cDistGen_Deg5_Generator<Fonc_Num>,36,3> ;

template <> const int 
   cPIF_Unif<cDistGen_Deg5_Generator<double>,cDistGen_Deg5_Generator<Fonc_Num>,36,3>::mDegrePolyn[36]
          = { 1,1,2,2,2,2,3,3,3,3,3,3,3,3 ,4,4,4,4,4  ,4,4,4,4,4   ,5,5,5,5,5,5   ,5,5,5,5,5,5};



template <> const std::string  cDist_Polyn6::TheName="Polyn6";
template <> const int   cDist_Polyn6::TheType= (int) eModelePolyDeg6;

template class cDist_Param_Unif<cDistGen_Deg6_Generator<double>,cDistGen_Deg6_Generator<Fonc_Num>,50,3> ;
template class cCamera_Param_Unif<cDistGen_Deg6_Generator<double>,cDistGen_Deg6_Generator<Fonc_Num>,50,3> ;
template class cPIF_Unif<cDistGen_Deg6_Generator<double>,cDistGen_Deg6_Generator<Fonc_Num>,50,3> ;

template <> const int 
   cPIF_Unif<cDistGen_Deg6_Generator<double>,cDistGen_Deg6_Generator<Fonc_Num>,50,3>::mDegrePolyn[50]
          = { 1,1,2,2,2,2,3,3,3,3,3,3,3,3 ,4,4,4,4,4  ,4,4,4,4,4   ,5,5,5,5,5,5   ,5,5,5,5,5,5  ,6,6,6,6,6,6,6 ,6,6,6,6,6,6,6 };




template <> const std::string  cDist_Polyn7::TheName="Polyn7";
template <> const int   cDist_Polyn7::TheType= (int) eModelePolyDeg7;

template class cDist_Param_Unif<cDistGen_Deg7_Generator<double>,cDistGen_Deg7_Generator<Fonc_Num>,66,3> ;
template class cCamera_Param_Unif<cDistGen_Deg7_Generator<double>,cDistGen_Deg7_Generator<Fonc_Num>,66,3> ;
template class cPIF_Unif<cDistGen_Deg7_Generator<double>,cDistGen_Deg7_Generator<Fonc_Num>,66,3> ;

template <> const int 
   cPIF_Unif<cDistGen_Deg7_Generator<double>,cDistGen_Deg7_Generator<Fonc_Num>,66,3>::mDegrePolyn[66]
          = { 1,1,2,2,2,2,3,3,3,3,3,3,3,3 ,4,4,4,4,4  ,4,4,4,4,4   ,5,5,5,5,5,5   ,5,5,5,5,5,5  ,6,6,6,6,6,6,6 ,6,6,6,6,6,6,6 
              ,7,7,7,7,7,7,7,7   ,7,7,7,7,7,7,7,7  };



/*******************************************************************/
/*                                                                 */
/*           MODELE GENERAL FISH EYE                               */
/*                                                                 */
/*******************************************************************/

#define InvIsBuged true



      // cFELinear_Precond  

template  <class Type> Fonc_Num cFELinear_Precond<Type>::NormGradC2M(Pt2d<Fonc_Num> aP,Fonc_Num * aState)
{
  aP = Pt2d<Fonc_Num>(aP.x/aState[0],aP.y/aState[0]);

  Fonc_Num aR2 = Square(aP.x)+Square(aP.y);
  return 1 / (1+aR2);
}

template  <class Type>  Type  cFELinear_Precond<Type>::M2CRxSRx(const Type  & aVal)
{
   return AtRxSRx(aVal);
}
template  <class Type>  Type  cFELinear_Precond<Type>::C2MRxSRx(const Type  & aVal)
{
   return TgRxSRx(aVal);
}


template  <class Type>  Type  cFELinear_Precond<Type>::SqM2CRx(const Type  & aVal)
{
   return At2Rx(aVal);
}
template  <class Type> ElDistortion22_Gen * 
    cFELinear_Precond<Type>::DistPreCond(const double &   aVar,const Pt2dr & aP)
{
   return new cDistPrecondAtgt(aVar,aP);
}

    // cFEEquiSolid_Precond 

template  <class Type>  Type  cFEEquiSolid_Precond<Type>::M2CRxSRx(const Type  & aVal)
{
   return f2SAtRxS2SRx(aVal);
}
template  <class Type>  Type  cFEEquiSolid_Precond<Type>::C2MRxSRx(const Type  & aVal)
{
   return Tg2AsRxS2SRx(aVal);
}
template  <class Type>  Type  cFEEquiSolid_Precond<Type>::SqM2CRx(const Type  & aVal)
{
   return f4S2AtRxS2(aVal);
}
template  <class Type> ElDistortion22_Gen * 
    cFEEquiSolid_Precond<Type>::DistPreCond(const double &   aVar,const Pt2dr & aP)
{
   return new cDistPrecond2SinAtgtS2(aVar,aP);
}

template  <class Type> Fonc_Num cFEEquiSolid_Precond<Type>::NormGradC2M(Pt2d<Fonc_Num> aP,Fonc_Num * aState)
{
  // std::cout << "FALSSEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n";
   aP = Pt2d<Fonc_Num>(aP.x/aState[0],aP.y/aState[0]);

  Fonc_Num aR2 = Square(aP.x)+Square(aP.y);
  return cos(atan(sqrt(aR2))/2) / (1+aR2);
}


/*
   Tentative de creation d'un modele de distorsion adapte au fish eye.

   On modelise trois type d'effets :

      - une distorsion radiale, jusqu'a des degre eleves (par exemple 21),
      necessaire pour les coins du fish eye. Soit N le degre de liberte
            N=1 -> R3
            N=10 -> 21
          + deux degres de libertes pour le centre

      - une distorsion decentrique de degre intermediaire, (par exemple 11)
      obtenue par differenciation / au centre de la distorsion radiale selon
      la formule contenue dans la doc MicMac dans le paragraphe "Justification
      des modeles parametriques" (actuellement A.1.2.3) . A partir du deuxieme
      terme (differenciation du R5, la formule n'est plus rigoureuse car les
      termes sont lies entre eux, cependant cela peut modeliser des termes 
      a plus de 2 centres....)
     

      - une distorsion polynomiale de degre "faible"  (par exemple 5 ou 7)
      pour absorber "le reste", on fait attention a ce que les coefficients
      ne soit pas redondan avec 
   

    On donne une variables d'états :Focale . 


     Il s'agit de definir la distorsion polynomiale pour qu'elle forme une base
     libre avec les termes radiaux et decentriques. Pour les termes a partir de R5,
     le centre ayant ete contraint par R3, c'est systematique :

         - le terme radiale  Ak (P-C) |P-C| ^ 2K contient un terme en 
            (X^2K+1,Y^2K+1), donc on fixe dans les terme de degres impair
            un terme globale  (X^2K+1,-Y^2K+1) pour les termes  de degre 2N+1
            de la meme coordonnees
   
        - le terme decentrique contient un term en X^2N pour le terme en X et Y^2N
          pour le terme en Y, comme il y a deux degres de libertes,
          on inhibe ces deux termes


    Le terme R3 joue un role a part, car il donne, en plus du terme radiale habituel,
    4 degre de liberte :

        - 2 terme decentrique
        - 2 coordonnee de centre de distortion

    Comme il y a deja 2 valeur annulee des terme de degre 2 (pour cause de correlation avec
    les homographie de basculement), on annule purement et simplement ces termes. Il y
    a un petit doute formel sur la decorrlation entre ces 6 terme supprimes ?
 

    Inconnue :

        2       pour le centre
        NbRad   pour le coefficients radiaux
        2 *  NbDec pour les coefficient 
        

*/
template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
 bool  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::IsFE()
{
   return true;
}

template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
 cCalibDistortion  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::ToXmlStruct(const cDist_Param_Unif_Gen & aDist, const ElCamera *)

{
    return StdToXmlStruct(aDist);
}
template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
 bool  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::UseSz()
{
   return true;
}

template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
 Fonc_Num  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::NormGradC2M(Pt2d<Fonc_Num> aP,Fonc_Num * aState)
{
    return TPreC::NormGradC2M(aP,aState);
}


template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
         int  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::IndCX()
{
  return 0;
}
template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
         int  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::IndCY()
{
  return 1;
}
template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
         int  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::D0Rad()
{
  return 2;
}
template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
         int  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::D0Dec()
{
  return D0Rad() + NbRad;
}
template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
         int  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::D0Polyn()
{
  return D0Dec() + 2 * NbDec;
}

template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV>
   bool cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::CanExportDistAsGrid() 
{
   return false;
}

template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV>
ElDistortion22_Gen   *  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::DistPreCond(const double * aVar,const double *aState)
{
   // ELISE_ASSERT(false,"FE-A-MODIF");
   // return new cDistPrecondAtgt(aState[0],Pt2dr(aVar[IndCX()],aVar[IndCY()]));
   return TPreC::DistPreCond(aState[0],Pt2dr(aVar[IndCX()],aVar[IndCY()]));
}


 // GESTION DES DEGRES POUR VAR LIBRE
template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
         int  cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::DegreOfPolyn(const int * ,int aK,eModeControleVarDGen aMode)
{
   switch (aMode)
   {
      case eModeContDGPol :
         return mDegrePolyn[aK];
      break;

      case eModeContDGCDist :
         return (  (aK==IndCX()) || (aK==IndCY()) ) ? 1 : -1;
      break;
       
      case eModeContDGDRad :
         return mDegreRad[aK];
      break;
       
      case eModeContDGDCent :
         return mDegreDec[aK];
      break;
   }
   ELISE_ASSERT(false,"cDistGen_FishEye_Generator DegreOfPolyn");
   return -1;
}


void Show(const double * S,const double *V,int aK1,int aK2)
{
   std::cout << "COEF " << V[aK1] << " " << V[aK1+1] << "\n";
   for (double aX = 0 ; aX<8.0 ; aX+=0.1)
   {
      double aR2c = aX*aX;
      double aRN = aR2c;
      double aDistRad = 0.0;
      for (int aK = aK1 ; aK < aK2 ; aK++)
      {
           aDistRad = aDistRad + aRN * V[aK];
           aRN = aRN * aR2c;
      }
      double aRes = aX * (1+aDistRad); 
      std::cout << " X " << aX << " D " << aRes <<  "Atan " << atan(aX) << "\n";
   }
   //getchar();
}

void Show(const Fonc_Num*,const Fonc_Num*,int aK1,int aK2)
{
}

void ShowP(const Pt2d<Fonc_Num> &,const Pt2d<Fonc_Num> &,Fonc_Num)
{
}

void ShowP(const Pt2d<double> &aPin,const Pt2d<double> &aPout,double aF)
{
   std::cout << aPin << " " << aPout <<  " " << aF  << "\n";
}

extern bool BugNanFE;
void ShowD(const Pt2d<Fonc_Num> &,const std::string &,bool) 
{
}
void ShowD(const Pt2d<double> & aV,const std::string & aMes,bool gc) 
{
   if (BugNanFE)
   {
      std::cout << aMes << " " << aV.x << " " << aV.y << "\n";
      if (gc) getchar();
   }
}




template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV>
  Pt2d<typename TPreC::tVal> cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::GuessInv
             (const  Pt2d<tVal> & aP,const   tVal * V,const tVal * aState)
{
if (BugFE)
{
    std::cout << "cDistGen_FishEye_Generator::::GuessInv \n";
}


//  Centre de distorsion
   tVal  aCX = V[IndCX()];
   tVal  aCY = V[IndCY()];

// Ecart au centre de distorsion , normalise
   tVal  aXc =  (aP.x-aCX) / aState[0];
   tVal  aYc =  (aP.y-aCY) / aState[0];


   tVal aR2 = (aXc*aXc+aYc*aYc);
// tVal aR2Cor = TPreC::M2CRxSRx(aR2);
   tVal aR2Cor = TPreC::C2MRxSRx(aR2);
   aXc = aXc * aR2Cor;
   aYc = aYc * aR2Cor;

   // tVal aR = sqrt(aXc*aXc+aYc*aYc);
   // aXc = (aXc/(1e-7+aR)) * tan(aR);
   // aYc = (aYc/(1e-7+aR)) * tan(aR);


   return Pt2d<tVal>(aCX+aXc*aState[0],aCY+aYc*aState[0]);
}


#define ShowIndex 0

template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
         Pt2d<typename TPreC::tVal> cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::DistElem
                    (
                         bool UsePC,
                         const Pt2d<tVal> &aP,
                         const tVal * V,
                         const tVal * States,
                         bool Test
                    )
{

   ShowD(aP," Distelem Input",false);
//  Centre de distorsion
   tVal  aCX = V[IndCX()];
   tVal  aCY = V[IndCY()];

// Ecart au centre de distorsion , normalise
   tVal  aXc =  (aP.x-aCX) / States[0];
   tVal  aYc =  (aP.y-aCY) / States[0];

   tVal aR2CInit = aXc*aXc+aYc*aYc;

   tVal aMul =  UsePC ? TPreC::M2CRxSRx(aR2CInit) : tVal(1) ;
 // std::cout << "USE-PC " << UsePC << "\n";
   aXc = aXc * aMul;
   aYc = aYc * aMul;

//  Coordonnees normalisees

   tVal aResX = aXc;
   tVal aResY = aYc;
  

//  Grandeurs utile

   tVal aX2c =  aXc* aXc;
   tVal aY2c =  aYc* aYc;
   tVal aXYc =  aXc* aYc;
   tVal aR2c =   UsePC ? TPreC::SqM2CRx(aR2CInit) : aR2CInit; 

   int aInd = D0Rad();

// ==============================================
// Calcul de la partie radiale
// ==============================================
   tVal aRN = aR2c;
   tVal aDistRad = 0.0;
   // Type aDistRad = 1.0;
   if (ShowIndex)
   {
       std::cout << "INT FIRST RAD " << aInd << "\n";
   }

   for (int aK = 0 ; aK < NbRad ; aK++)
   {
        mDegreRad[aInd] = aK+1;
        aDistRad = aDistRad + aRN * V[aInd++];
        aRN = aRN * aR2c;
   }

   aResX = aResX +  aXc  * aDistRad;
   aResY = aResY +  aYc  * aDistRad;

   if (Test)
   {
      ELISE_ASSERT(aInd==D0Dec(),"Inc(0) in cDistGen_FishEye_Generator");
   }


// ==============================================
// Calcul de la partie decentrique
// ==============================================
   if (ShowIndex)
   {
       std::cout << "INT FIRST DEC " << aInd << "\n";
   }
   aRN = 1;
   for (int aK=0 ; aK<NbDec ; aK++)
   {
        mDegreDec[aInd] = aK+1;
        tVal aCoefDCX =   V[aInd++];
        mDegreDec[aInd] = aK+1;
        tVal aCoefDCY =   V[aInd++];

         // C'est la derivee du terme  (P-C) |P-C| ^(2*(k+1))

        aResX = aResX + aRN*( aCoefDCX*(aR2c+2*(aK+1)*aX2c)  + aCoefDCY *2*(aK+1) * aXYc);
        aResY = aResY + aRN*( aCoefDCX*2*(aK+1)*aXYc + aCoefDCY *(aR2c+2*(aK+1)*aY2c));

        aRN = aRN * aR2c;
   }

   if (Test)
   {
      ELISE_ASSERT(aInd==D0Polyn(),"Inc(0) in cDistGen_FishEye_Generator");
   }

   // Calcul de la partie polynomiale de degre 1, 
   // Sont deja modelise par la focale et les rotation pure sans
   // basculemment les similtude
   //
   //              aCoef1   aCoef2
   //   A  B       1 0      0 1 
   //   -B A       0 0      1 0

   if (ShowIndex)
   {
       std::cout << "INT FIRST POLY " << aInd << "\n";
   }
   mDegrePolyn[aInd] = 1;
   tVal aCoef1 =  V[aInd++];
   mDegrePolyn[aInd] = 1;
   tVal aCoef2 =  V[aInd++];
   aResX = aResX + aCoef1 * aXc + aCoef2 *  aYc;
   aResY = aResY + aCoef2 * aXc;

// ==============================================
// Calcul de la partie polynomiale
// ==============================================

   // Vecteur des monomes
   static std::vector<tVal> aVXY;
   aVXY.clear();
   aVXY.push_back( aXc * aXc);
   aVXY.push_back( aYc * aXc);
   aVXY.push_back( aYc * aYc);
  
   // Pas de terme de degre 2 :
   //   2 pris par les rotations
   //   2 pris par decentrique
   //   2 ???? pris par centre de dist => A VERIFIERE !!!

   for (int aD=3 ; aD <= NbPolyn ; aD++)
   {
        aVXY.push_back(aYc*aVXY.back());
        for (int aK=0 ; aK<aD ; aK++)
           aVXY[aK] = aVXY[aK] * aXc;

        
        for (int aK=0 ; aK<=aD ; aK++)
        {
             // Dans tous les cas on supprime de terme en XN de X qui peut
             // etre pris en compte par radial et decentrique
             if (aK!=0)
             {
                mDegrePolyn[aInd] = aD;
                aResX = aResX +   V[aInd++] * aVXY[aK];
             }
             // Dans les cas pair, on supprime le terme en YN de Y
             if ((aD%2) || (aK!=aD))
             {
                mDegrePolyn[aInd] = aD;
                aResY = aResY +   V[aInd++] * aVXY[aK];
             }
        }
         
   }

   if (Test)
   {
      std::cout << "NB VAR = " << aInd  
                << " For " << NbRad << " " << NbDec << " " << NbPolyn << "\n";
   }
   else
   {
      ELISE_ASSERT(aInd==NBV,"Inc(end) in cDistGen_FishEye_Generator");
   }
   if (ShowIndex)
   {
       std::cout << "INT LAST POLY " << aInd << "\n";
       exit(-1);
   }
   // Show(States,V,2,D0Dec());

   // ShowP(aP,Pt2d<Type>(aCX+aResX*States[0],aCY+aResY*States[0]),States[0]);
   ShowD(Pt2d<tVal>(aCX+aResX*States[0],aCY+aResY*States[0])," Distelem OutPut ================================== ",true);
   return Pt2d<tVal>(aCX+aResX*States[0],aCY+aResY*States[0]);
}





template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
void          cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::InitClass()
{
   if ( isInit)   return;

   isInit =true;
   int aDef = -1;
   for (int aK=0 ; aK<NBV ; aK++)
   {
       mDegreRad[aK] = aDef;
       mDegreDec[aK] = aDef;
       mDegrePolyn[aK] = aDef;
   }

   // Assure l'initialistaion
   tVal aV[NBV];
   tVal aState[10];
   Pt2d<tVal> aP(0,0);

   DistElem(true,aP,aV,aState,0,Pt2d<tVal>(0,0));
}

#if (0)
#endif




template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
          cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::cDistGen_FishEye_Generator()
{
   // Assure l'initialisation de mDegreRad
}


template <class TPreC,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
         Pt2d<typename TPreC::tVal> cDistGen_FishEye_Generator<TPreC,NbRad,NbDec,NbPolyn,NBV>::DistElem(bool UsePC,const Pt2d<tVal> &aP,const tVal * V,const tVal * States,const tVal & aFoc,const Pt2d<tVal> & aPP)
{
   return DistElem(UsePC,aP,V,States,false);
}

#define INSTANT_ONE_Type_FE(aType,A,B,C,NBV)\
template <> int  cDistGen_FishEye_Generator<aType,A,B,C,NBV>::mDegreRad[NBV]={-1};\
template <> int  cDistGen_FishEye_Generator<aType,A,B,C,NBV>::mDegreDec[NBV]={-1};\
template <> int  cDistGen_FishEye_Generator<aType,A,B,C,NBV>::mDegrePolyn[NBV]={-1};\
template <> bool  cDistGen_FishEye_Generator<aType,A,B,C,NBV>::isInit = false;\
template class cDistGen_FishEye_Generator<aType,A,B,C,NBV>;


#define INSTANT_ONE_Num_FE(PREC,A,B,C,NBV)\
INSTANT_ONE_Type_FE(PREC<double>,A,B,C,NBV)\
INSTANT_ONE_Type_FE(PREC<Fonc_Num>,A,B,C,NBV)\
template class cDist_Param_Unif<cDistGen_FishEye_Generator<PREC<double>,A,B,C,NBV>,cDistGen_FishEye_Generator<PREC<Fonc_Num>,A,B,C,NBV>,NBV,1> ;\
template class cCamera_Param_Unif<cDistGen_FishEye_Generator<PREC<double>,A,B,C,NBV>,cDistGen_FishEye_Generator<PREC<Fonc_Num>,A,B,C,NBV>,NBV,1>;\
template class cPIF_Unif<cDistGen_FishEye_Generator<PREC<double>,A,B,C,NBV>,cDistGen_FishEye_Generator<PREC<Fonc_Num>,A,B,C,NBV>,NBV,1>;\
template <> const int cPIF_Unif<cDistGen_FishEye_Generator<PREC<double>,A,B,C,NBV>,cDistGen_FishEye_Generator<PREC<Fonc_Num>,A,B,C,NBV>,NBV,1>::mDegrePolyn[NBV]={-1};


INSTANT_ONE_Num_FE(cFELinear_Precond,10,5,5,50)
INSTANT_ONE_Num_FE(cFEEquiSolid_Precond,10,5,5,50)

template <> const std::string  cDistLin_FishEye_10_5_5::TheName="FishEye_10_5_5";
template <> const int   cDistLin_FishEye_10_5_5::TheType= (int) eModele_FishEye_10_5_5;


template <> const std::string  cDistEquiSol_FishEye_10_5_5::TheName="EquiSolid_FishEye_10_5_5";
template <> const int   cDistEquiSol_FishEye_10_5_5::TheType= (int) eModele_EquiSolid_FishEye_10_5_5;



// template class cDist_Param_Unif<cDistGen_FishEye_Generator<double>,cDistGen_Deg7_Generator<Fonc_Num>,66,1> ;
// template class cCamera_Param_Unif<cDistGen_Deg7_Generator<double>,cDistGen_Deg7_Generator<Fonc_Num>,66,1> ;
// template class cPIF_Unif<cDistGen_Deg7_Generator<double>,cDistGen_Deg7_Generator<Fonc_Num>,66,1> ;



void TestFishEye()
{
   double aV[200];
   double aState[10];

   cDistGen_FishEye_Generator<cFELinear_Precond<double>,10,5,5,50>::DistElem(true,Pt2dr(0,0),aV,aState,true);
}

/*
template <class Type,const int NbRad,const int NbDec,const int NbPolyn,const int NBV> 
         Pt2d<Type> cDistGen_FishEye_Generator<Type,NbRad,NbDec,NbPolyn,NBV>::DistElem
                    (
                         const Pt2d<Type> &aP,
                         const Type * V,
                         const Type * States,
                         bool Test
                    )
{
   if (! isInit)
   {
       isInit =true;
       int aDef = -1;
       for (int aK=0 ; aK<NBV ; aK++)
       {
            mDegreRad[aK] = aDef;
            mDegreDec[aK] = aDef;
            mDegrePolyn[aK] = aDef;
       }
   }


//  Centre de distorsion
   Type  aCX = V[IndCX()];
   Type  aCY = V[IndCY()];

// Ecart au centre de distorsion , normalise
   Type  aXc =  (aP.x-aCX) / States[0];
   Type  aYc =  (aP.y-aCY) / States[0];


//  Coordonnees normalisees
   Type  aXNorm = aP.x / States[0];
   Type  aYNorm = aP.y / States[0];

   Type aResX = aXNorm;
   Type aResY = aYNorm;
  
   if (1)
   {
        
   }


//  Grandeurs utile
   Type aX2c =  aXc* aXc;
   Type aY2c =  aYc* aYc;
   Type aXYc =  aXc* aYc;
   Type aR2c = aX2c + aY2c;

   int aInd = D0Rad();

// ==============================================
// Calcul de la partie radiale
// ==============================================
   Type aRN = aR2c;
   Type aDistRad = 0.0;
   // Type aDistRad = 1.0;
   for (int aK = 0 ; aK < NbRad ; aK++)
   {
        mDegreRad[aInd] = aK+1;
        aDistRad = aDistRad + aRN * V[aInd++];
        aRN = aRN * aR2c;
   }

   aResX = aResX +  aXc  * aDistRad;
   aResY = aResY +  aYc  * aDistRad;

   if (Test)
   {
      ELISE_ASSERT(aInd==D0Dec(),"Inc(0) in cDistGen_FishEye_Generator");
   }


// ==============================================
// Calcul de la partie decentrique
// ==============================================
   aRN = 1;
   for (int aK=0 ; aK<NbDec ; aK++)
   {
        mDegreDec[aInd] = aK+1;
        Type aCoefDCX =   V[aInd++];
        mDegreDec[aInd] = aK+1;
        Type aCoefDCY =   V[aInd++];

         // C'est la derivee du terme  (P-C) |P-C| ^(2*(k+1))

        aResX = aResX + aRN*( aCoefDCX*(aR2c+2*(aK+1)*aX2c)  + aCoefDCY *2*(aK+1) * aXYc);
        aResY = aResY + aRN*( aCoefDCX*2*(aK+1)*aXYc + aCoefDCY *(aR2c+2*(aK+1)*aY2c));

        aRN = aRN * aR2c;
   }

   if (Test)
   {
      ELISE_ASSERT(aInd==D0Polyn(),"Inc(0) in cDistGen_FishEye_Generator");
   }

   // Calcul de la partie polynomiale de degre 1, 
   // Sont deja modelise par la focale et les rotation pure sans
   // basculemment les similtude
   //
   //              aCoef1   aCoef2
   //   A  B       1 0      0 1 
   //   -B A       0 0      1 0

   mDegrePolyn[aInd] = 1;
   Type aCoef1 =  V[aInd++];
   mDegrePolyn[aInd] = 1;
   Type aCoef2 =  V[aInd++];
   aResX = aResX + aCoef1 * aXNorm + aCoef2 *  aYNorm;
   aResY = aResY + aCoef2 * aXNorm;

// ==============================================
// Calcul de la partie polynomiale
// ==============================================

   // Vecteur des monomes
   static std::vector<Type> aVXY;
   aVXY.clear();
   aVXY.push_back( aXNorm * aXNorm);
   aVXY.push_back( aYNorm * aXNorm);
   aVXY.push_back( aYNorm * aYNorm);
  

   for (int aD=3 ; aD <= NbPolyn ; aD++)
   {
        aVXY.push_back(aYNorm*aVXY.back());
        for (int aK=0 ; aK<aD ; aK++)
           aVXY[aK] = aVXY[aK] * aXNorm;

        
        for (int aK=0 ; aK<=aD ; aK++)
        {
             // Dans tous les cas on supprime de terme en XN de X qui peut
             // etre pris en compte par radial et decentrique
             if (aK!=0)
             {
                mDegrePolyn[aInd] = aD;
                aResX = aResX +   V[aInd++] * aVXY[aK];
             }
             // Dans les cas pair, on supprime le terme en YN de Y
             if ((aD%2) || (aK!=aD))
             {
                mDegrePolyn[aInd] = aD;
                aResY = aResY +   V[aInd++] * aVXY[aK];
             }
        }
         
   }

   if (Test)
   {
      std::cout << "NB VAR = " << aInd  
                << " For " << NbRad << " " << NbDec << " " << NbPolyn << "\n";
   }
   else
   {
      ELISE_ASSERT(aInd==NBV,"Inc(end) in cDistGen_FishEye_Generator");
   }
   Show(States,V,2,D0Dec());

   return Pt2d<Type>(aResX*States[0],aResY*States[0]);
}

*/


#if (0)
#endif






/*Footer-MicMac-eLiSe-25/06/2007

Ce logiciel est un programme informatique servant à la mise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est régi par la licence CeCILL-B soumise au droit français et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffusée par le CEA, le CNRS et l'INRIA 
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilité au code source et des droits de copie,
de modification et de redistribution accordés par cette licence, il n'est
offert aux utilisateurs qu'une garantie limitée.  Pour les mêmes raisons,
seule une responsabilité restreinte pèse sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les concédants successifs.

A cet égard  l'attention de l'utilisateur est attirée sur les risques
associés au chargement,  à l'utilisation,  à la modification et/ou au
développement et à la reproduction du logiciel par l'utilisateur étant 
donné sa spécificité de logiciel libre, qui peut le rendre complexe à 
manipuler et qui le réserve donc à des développeurs et des professionnels
avertis possédant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invités à charger  et  tester  l'adéquation  du
logiciel à leurs besoins dans des conditions permettant d'assurer la
sécurité de leurs systèmes et ou de leurs données et, plus généralement, 
à l'utiliser et l'exploiter dans les mêmes conditions de sécurité. 

Le fait que vous puissiez accéder à cet en-tête signifie que vous avez 
pris connaissance de la licence CeCILL-B, et que vous en avez accepté les
termes.
Footer-MicMac-eLiSe-25/06/2007*/
