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

#include    "GpGpu/GBV2_ProgDynOptimiseur.h"

Pt2di Px2Point(int * aPx) { return Pt2di(aPx[0],0); }
bool IsPTest(const Pt2di & aP) {return aP == Pt2di(40,40);}

//  COPIE TEL QUEL DEPUIS /home/mpd/ELISE/applis/Anag/SimpleProgDyn.cpp
/*
    Soit Z dans l'intervalle ouvert I1 [aZ1Min,aZ1Max[,
    on recherche dans l'intervalle ouvert I0 [aZ0Min,aZ0Max[,
    un intervalle ferme, non vide, le plus proche possible
    [aZ+aDzMin,aZ+aDzMax].

    De plus ce calcul doit generer des connexion symetrique.

    Ex  :
        I1 = [10,30[
        I0 = [5,20[

        MaxDeltaZ = 2


        Z = 13 ->    Delta = [-2,2]   // aucune contrainte
        Z = 18 ->    Delta = [-2,1]   // Pour que ca reste dans I0
        Z = 25 ->    Delta = [-6,-6]  //  Pour que l'intersection soit non vide avec I0
        Z = 10 ->    Delta = [-5,-1]  // principe de symetrie, dans l'autre sens                                      // les points [5,9] de I0 devront etre connecte a 10

*/

/// brief Calcul le Z min et max.
static inline void LocComputeIntervaleDelta
(
        INT & aDzMin,
        INT & aDzMax,
        INT aZ,
        INT MaxDeltaZ,
        INT aZ1Min,
        INT aZ1Max,
        INT aZ0Min,
        INT aZ0Max
        )
{
    aDzMin =   aZ0Min-aZ;
    if (aZ != aZ1Min)
        ElSetMax(aDzMin,-MaxDeltaZ);

    aDzMax = aZ0Max-1-aZ;
    if (aZ != aZ1Max-1)
        ElSetMin(aDzMax,MaxDeltaZ);

    // Si les intervalles sont vides, on relie
    // les bornes des intervalles a tous les points
    if (aDzMin > aDzMax)
    {
        if (aDzMax <0)
            aDzMin = aDzMax;
        else
            aDzMax = aDzMin;
    }
}

static cGBV2_CelOptimProgDyn aCelForInit;

cGBV2_ProgDynOptimiseur::cGBV2_ProgDynOptimiseur
(
        cAppliMICMAC&   mAppli,
        cLoadTer&       mLT,
        const cEquiv1D& anEqX,
        const cEquiv1D& anEqY,
        Im2D_INT2       aPxMin,
        Im2D_INT2       aPxMax
) :
    cSurfaceOptimiseur ( mAppli,mLT,1e4,anEqX,anEqY,false,false),
    mXMin       (aPxMin),
    mXMax       (aPxMax),
    mSz         (mXMin.sz()),
    mNbPx       (1),
    mYMin       (mSz.x,mSz.y,0),
    mYMax       (mSz.x,mSz.y,1),
    mMatrCel    (
                    Box2di(Pt2di(0,0),mSz),
                    mXMin.data(),
                    mYMin.data(),
                    mXMax.data(),
                    mYMax.data(),
                    aCelForInit
#if CUDA_ENABLED
                    ,IGpuOpt._poInitCost
#endif
        ),

    mLMR        (mSz)
{
 #if CUDA_ENABLED
    IGpuOpt._preFinalCost1D.ReallocIf(IGpuOpt._poInitCost.Size());
    IGpuOpt._FinalDefCor.ReallocIf(IGpuOpt._poInitCost._dZ.GetDimension());
    IGpuOpt._poInitCost.ReallocData();
    IGpuOpt._poInitCost.fillCostInit(10123);
#endif
}

cGBV2_ProgDynOptimiseur::~cGBV2_ProgDynOptimiseur()
{
#if CUDA_ENABLED
    IGpuOpt.Dealloc();
#endif
}

void cGBV2_ProgDynOptimiseur::Local_SetCout(Pt2di aPTer,int *aPX,REAL aCost,int aLabel)
{

#if CUDA_ENABLED
    Pt2di z     = Px2Point(aPX);
    int3 pt = make_int3(aPTer.x,aPTer.y,z.x);
    IGpuOpt._poInitCost[pt] = cGBV2_TabulCost::CostR2I(aCost);
#else
    mMatrCel[aPTer][Px2Point(aPX)].SetCostInit(cGBV2_TabulCost::CostR2I(aCost));
#endif
}

void cGBV2_ProgDynOptimiseur::BalayageOneSens
(
        const std::vector<Pt2di> &   aVPt,     // vecteur de points
        cGBV2_CelOptimProgDyn::eSens aSens,    // sens du parcourt
        int                          anIndInit,// premier point
        int                          aDelta,   // delta incremenation de progression
        int                          aLimite   // Limite de progression
        )
{

    // Initialisation des couts sur les premieres valeurs
    {
        // Matrice des cellules
        tCGBV2_tMatrCelPDyn &  aMat0 = mMatrCel[aVPt[anIndInit]];

        // Le rectangle
        const Box2di & aBox0 = aMat0.Box();

        Pt2di aP0;

        std::cout << aBox0.P0() << " " << aBox0.P1() << "\n";
        getchar();

        for (aP0.y = aBox0._p0.y ;  aP0.y<aBox0._p1.y; aP0.y++)

            for (aP0.x = aBox0._p0.x ; aP0.x<aBox0._p1.x;aP0.x++)

                aMat0[aP0].SetBeginCumul(aSens);
    }

    // Propagation
    int anI0 = anIndInit;
    while ((anI0+ aDelta)!= aLimite)
    {
        //
        int anI1 = anI0+aDelta;

        tCGBV2_tMatrCelPDyn &  aMat1 = mMatrCel[aVPt[anI1]];
        const Box2di & aBox1 = aMat1.Box();
        Pt2di aP1;

        // Met un cout infini aux successeurs
        for (aP1.y = aBox1._p0.y ; aP1.y<aBox1._p1.y;aP1.y++)
        {
            for (aP1.x = aBox1._p0.x ; aP1.x<aBox1._p1.x ; aP1.x++)
            {
                aMat1[aP1].SetCumulInitial(aSens);
            }
        }

        // Propagation
        tCGBV2_tMatrCelPDyn &  aMat0 = mMatrCel[aVPt[anI0]];
        const Box2di & aBox0 = aMat0.Box();
        Pt2di aP0;
        for (aP0.y=aBox0._p0.y ; aP0.y<aBox0._p1.y ; aP0.y++)
        {
            int aDyMin,aDyMax;
            // Calcul du delta sur Y
            LocComputeIntervaleDelta
                    (
                        aDyMin,
                        aDyMax,
                        aP0.y,
                        mMaxEc[1],
                    aBox0._p0.y,
                    aBox0._p1.y,
                    aBox1._p0.y,
                    aBox1._p1.y
                    );
            for (aP0.x=aBox0._p0.x ;  aP0.x<aBox0._p1.x ; aP0.x++)
            {
                int aDxMin,aDxMax;
                // Calcul du delta sur X
                LocComputeIntervaleDelta
                        (
                            aDxMin,
                            aDxMax,
                            aP0.x,
                            mMaxEc[0],
                        aBox0._p0.x,
                        aBox0._p1.x,
                        aBox1._p0.x,
                        aBox1._p1.x
                        );

                // Cellule courante
                cGBV2_CelOptimProgDyn & aCel0 = aMat0[aP0];

                // Parcours des cellules dans l'intervalle des Deltas
                for (int aDy=aDyMin ; aDy<=aDyMax; aDy++)
                {
                    for (int aDx=aDxMin ; aDx<=aDxMax; aDx++)
                    {

                        aMat1[aP0+Pt2di(aDx,aDy)].UpdateCost                       // cellule colonne suivante
                                (
                                    aCel0,                                               // cellule colonne courante
                                    aSens,                                               // Sens de parcours
                                    mTabCost[0].Cost(aDx) + mTabCost[1].Cost(aDy)        // Tabulation des p�nalit�s ou cout de transition
                                );                                                                                 // mCostActu[0]*ElAbs(aDx)+mCostActu[1]*ElAbs(aDy)
                    }
                }
            }
        }
        anI0 = anI1;
    }
}

void cGBV2_ProgDynOptimiseur::BalayageOneSensGpu(const std::vector<Pt2di> &aVPt, cGBV2_CelOptimProgDyn::eSens aSens, int anIndInit, int aDelta, int aLimite)
{
    // Initialisation des couts sur les premieres valeurs -------------------

    // Matrice des cellules
    tCGBV2_tMatrCelPDyn &  aMat0 = mMatrCel[aVPt[anIndInit]];

    // Le rectangle
    const Box2di & aBox0 = aMat0.Box();
    Pt2di aP0;

    for(aP0.x = aBox0._p0.x ; aP0.x<aBox0._p1.x;aP0.x++)
        aMat0[aP0].SetBeginCumul(aSens);

    // Propagation ------------------------------------------------------------
    int anI0 = anIndInit;
    while ((anI0+ aDelta)!= aLimite)
    {
        int anI1 = anI0+aDelta;

        tCGBV2_tMatrCelPDyn& aMat1 = mMatrCel[aVPt[anI1]];
        const Box2di&        aBox1 = aMat1.Box();
        Pt2di aP1;

        // Met un cout infini aux successeurs
        for (aP1.x = aBox1._p0.x ; aP1.x<aBox1._p1.x ; aP1.x++)
            aMat1[aP1].SetCumulInitial(aSens);

        // Propagation
        tCGBV2_tMatrCelPDyn& aMat0 = mMatrCel[aVPt[anI0]];
        const Box2di &       aBox0 = aMat0.Box();
        Pt2di aP0;

        //std::cout << "[" << aBox0._p0.x << "," << aBox0._p1.x << "] ";
        for (aP0.x=aBox0._p0.x ;  aP0.x<aBox0._p1.x ; aP0.x++)
        {
            int aDxMin,aDxMax;
            // Calcul du delta sur X
            LocComputeIntervaleDelta(aDxMin,aDxMax,aP0.x, mMaxEc[0],aBox0._p0.x, aBox0._p1.x, aBox1._p0.x,aBox1._p1.x);

            // Cellule courante
            cGBV2_CelOptimProgDyn & aCel0 = aMat0[aP0];

            // Parcours des cellules dans l'intervalle des Deltas
            for (int aDx=aDxMin ; aDx<=aDxMax; aDx++)
                aMat1[aP0+Pt2di(aDx,0)].UpdateCost(aCel0, aSens, mTabCost[0].Cost(aDx));

        }

        anI0 = anI1;
    }
}

void cGBV2_ProgDynOptimiseur::BalayageOneLine(const std::vector<Pt2di> & aVPt)
{
    // 1er Parcour dans un sens
    // aVPt         : ensemble des points
    // eAvant       : Sens de parcours
    // 0            : Premier point
    // 1            : delta incrementation
    // aVPt.size()  : limite du parcours
    BalayageOneSens(aVPt,cGBV2_CelOptimProgDyn::eAvant,0,1,(int) aVPt.size());

    // 2eme Parcour dans un sens inverse
    // aVPt         : ensemble des points
    // eArriere     : Sens de parcours
    // aVPt.size()-1: on part du dernier point
    // -1           : delta incrementation invers�
    // -1           : limite du parcours
    BalayageOneSens(aVPt,cGBV2_CelOptimProgDyn::eArriere,(int) (aVPt.size())-1,-1,-1);

    // on parcours la ligne
    for (int aK=0 ; aK<int(aVPt.size()) ; aK++)
    {
        // Matrice des cellules
        tCGBV2_tMatrCelPDyn &  aMat = mMatrCel[aVPt[aK]];
        // rectancle
        const Box2di &  aBox = aMat.Box();
        Pt2di aP;

        // Cout infini
        tCost aCoutMin = tCost(1e9);

        //recherche du cout minimum dans le le rectangle
        for (aP.y = aBox._p0.y ; aP.y<aBox._p1.y; aP.y++)
            for (aP.x = aBox._p0.x ; aP.x<aBox._p1.x ; aP.x++)
                ElSetMin(aCoutMin,aMat[aP].CostPassageForce());


        for (aP.y = aBox._p0.y ; aP.y<aBox._p1.y; aP.y++)
        {
            for (aP.x = aBox._p0.x ; aP.x<aBox._p1.x ; aP.x++)
            {
                tCost  aNewCost = aMat[aP].CostPassageForce()-aCoutMin;
                tCost & aCF = aMat[aP].CostFinal();
                if (mModeAgr==ePrgDAgrSomme) // Mode somme
                {

                    aCF += aNewCost;
                }
                else if (mModeAgr==ePrgDAgrMax) // Mode max
                {

                    ElSetMax(aCF,aNewCost);
                }
                else if (mModeAgr==ePrgDAgrProgressif) // Mode max
                {

                    aCF= aNewCost;
                    aMat[aP].SetCostInit
                            (
                                round_ni
                                (
                                    mPdsProgr*aCF
                                    + (1-mPdsProgr)* aMat[aP].GetCostInit()
                                    )
                                );
                }
                else  // Mode reinjection
                {

                    aCF = aNewCost;
                    aMat[aP].SetCostInit(aCF);
                }

            }
        }

    }
}

void cGBV2_ProgDynOptimiseur::BalayageOneLineGpu(const std::vector<Pt2di> &aVPt)
{

    BalayageOneSensGpu(aVPt,cGBV2_CelOptimProgDyn::eAvant,0,1,(int) aVPt.size());
    BalayageOneSensGpu(aVPt,cGBV2_CelOptimProgDyn::eArriere,(int) (aVPt.size())-1,-1,-1);

    // on parcours la ligne
    for (int aK=0 ; aK<int(aVPt.size()) ; aK++)
    {
        // Matrice des cellules
        tCGBV2_tMatrCelPDyn &  aMat = mMatrCel[aVPt[aK]];
        // rectancle
        const Box2di &  aBox = aMat.Box();
        Pt2di aP;
        // Cout infini
        tCost aCoutMin = tCost(1e9);

        //recherche du cout minimum dans le le rectangle
        for (aP.x = aBox._p0.x ; aP.x<aBox._p1.x ; aP.x++)
            ElSetMin(aCoutMin,aMat[aP].CostPassageForce());

        for (aP.x = aBox._p0.x ; aP.x<aBox._p1.x ; aP.x++)
        {
            tCost  aNewCost = aMat[aP].CostPassageForce()-aCoutMin;
            tCost & aCF = aMat[aP].CostFinal();
            aCF += aNewCost;
        }
    }
}

void cGBV2_ProgDynOptimiseur::BalayageOneDirection(Pt2dr aDirR)
{
    Pt2di aDirI = Pt2di(vunit(aDirR) * 20.0);
    mLMR.Init(aDirI,Pt2di(0,0),mSz);

    const std::vector<Pt2di> * aVPt;
    //printf("Optimisation CPU\n");
     while ((aVPt=mLMR.Next()))
        BalayageOneLineGpu(*aVPt);

}

void cGBV2_ProgDynOptimiseur::SolveOneEtape(int aNbDir)
{
    mModeAgr = ePrgDAgrSomme;
    mNbDir = aNbDir;


    for (int aKP=0 ; aKP<theDimPxMax ; aKP++)
    {
        // mCostActu[aKP] =0;
        mTabCost[aKP].Reset(0,0);
    }

#if CUDA_ENABLED
        SolveAllDirectionGpu(aNbDir);
#else
    // Parcours dans toutes les directions
    for (int aKDir=0 ; aKDir<mNbDir ; aKDir++)
    {
        mPdsProgr = (1.0+aKDir) / mNbDir;
        double aTeta =   (aKDir*PI)/mNbDir;

        Pt2dr aP = Pt2dr::FromPolar(100.0,aTeta);
        // On le met la parce que en toute rigueur ca depend de la
        // direction, mais pour l'instant on ne gere pas cette dependance
        // Tabulation des couts de transition
        for (int aKP=0 ; aKP<mNbPx ; aKP++)
        {
            mTabCost[aKP].Reset
                    (
                        mCostRegul[aKP],
                        mCostRegul_Quad[aKP]
                        );
        }
        // Balayage dans une direction aP
        BalayageOneDirection(aP);
    }
#endif

    //GpGpuTools::NvtxR_Push("Agregation",0x330000AA);
    Pt2di aPTer;

    // --> TODO Cette etape peut etre  mise dans la partie cuda

    for (aPTer.y=0 ; aPTer.y<mSz.y ; aPTer.y++)
    {
        for (aPTer.x=0 ; aPTer.x<mSz.x ; aPTer.x++)
        {
            tCGBV2_tMatrCelPDyn &  aMat = mMatrCel[aPTer];
            const Box2di &  aBox = aMat.Box();
            Pt2di aPRX;

                for (aPRX.x=aBox._p0.x ;aPRX.x<aBox._p1.x; aPRX.x++)
                {

                    tCost & aCF = aMat[aPRX].CostFinal();
                    //if (mModeAgr==ePrgDAgrSomme) // Mode somme
                    {
                        aCF /= mNbDir;

                    }
                    aMat[aPRX].SetCostInit(aCF);
                    aCF = 0;
                }

                #ifdef CUDA_ENABLED
                         if(mHasMaskAuto)
                              IGpuOpt._FinalDefCor[make_uint2(aPTer.x,aPTer.y)] /= mNbDir;
                #endif
        }
    }
    //nvtxRangePop();

}
#if CUDA_ENABLED

void cGBV2_ProgDynOptimiseur::copyCells_Mat2Stream(Pt2di aDirI, Data2Optimiz<CuHostData3D,2>  &d2Opt, sMatrixCellCost<ushort> &mCellCost, uint idBuf)
{

	GpGpuTools::NvtxR_Push(__FUNCTION__,0xFFAAFF33);

    mLMR.Init(aDirI,Pt2di(0,0),mSz);
    const std::vector<Pt2di>* aVPt;
    uint idLine = 0;
    while ((aVPt = mLMR.Next()))
    {
        uint    pitStrm = 0;
        uint    lLine   = aVPt->size();
        short3* index   = d2Opt.s_Index().pData() + d2Opt.param(idBuf)[idLine].y;

        for (uint aK= 0 ; aK < lLine; aK++)
        {
            Pt2di ptTer = (Pt2di)(*aVPt)[aK];

#ifndef CLAMPDZ
            ushort dZ   = mCellCost.DZ(ptTer);
            index[aK]   = mCellCost.PtZ(ptTer);
#else
            ushort dZ   = min(costInit1D.DZ(ptTer),costInit1D._maxDz);

            if(dZ == costInit1D._maxDz )
                index[aK]   = make_short2(costInit1D.PtZ(ptTer).x,costInit1D.PtZ(ptTer).x + costInit1D._maxDz);
            else
                index[aK]   = costInit1D.PtZ(ptTer);
#endif

            uint idStrm = d2Opt.param(idBuf)[idLine].x + pitStrm;

            ushort* desrCostInit = d2Opt.s_InitCostVol().pData()+idStrm;

            memcpy(desrCostInit,mCellCost[ptTer],dZ * sizeof(ushort));

            pitStrm += dZ;
        }

        idLine++;
    }

    //nvtxRangePop();
}

//#define OUTPUTDEFCOR

void cGBV2_ProgDynOptimiseur::copyCells_Stream2Mat(Pt2di aDirI, Data2Optimiz<CuHostData3D,2>  &d2Opt, sMatrixCellCost<ushort> &mCellCost, CuHostData3D<uint> &costFinal1D,CuHostData3D<uint> &FinalDefCor, uint idBuf)
{
    //GpGpuTools::NvtxR_Push(__FUNCTION__,0xFFAA0033);

    //nvtxMarkA("Start INIT");
    mLMR.Init(aDirI,Pt2di(0,0),mSz);

    const std::vector<Pt2di>* aVPt;
    uint idLine = 0;
#ifdef OUTPUTDEFCOR
    uint iii = 40;
#endif

    while ((aVPt = mLMR.Next()))
    {
        uint    pitStrm         = 0;
        uint    lenghtLine      = aVPt->size();
        uint    piTStream_Alti  = d2Opt.param(idBuf)[idLine].y; // Position dans le stream des altitudes/defCor

#ifdef OUTPUTDEFCOR
        if(idLine == iii)
            DUMP_UINT(lenghtLine)
#endif

        for (uint aK= 0 ; aK < lenghtLine; aK++)
        {

            Pt2di ptTer = (Pt2di)(*aVPt)[aK];
            #ifndef CLAMPDZ
            ushort dZ   = mCellCost.DZ(ptTer);
            #else
            ushort dZ   =  min(costInit1D.DZ(ptTer),costInit1D._maxDz);
            #endif

            // position dans le stream cout force....
            uint idStrm = d2Opt.param(idBuf)[idLine].x + pitStrm;
            uint *forCo = d2Opt.s_ForceCostVol(idBuf).pData() + idStrm; // TODO A verifier car devrait deja calculer dans les parametres...
            uint *finCo = costFinal1D.pData() + mCellCost.Pit(ptTer);


            uint defCorr = (d2Opt.s_DefCor(idBuf).pData()[piTStream_Alti + aK]);

            FinalDefCor[make_uint2(ptTer.x,ptTer.y)] += defCorr;

#ifdef OUTPUTDEFCOR
            if(idLine == iii)
            {
                //uint minPrevCost = d2Opt.s_DefCor(idBuf).pData()[piTStream_Alti + aK]/ mCostDefMasked;

                //uint minPrevCost = (minCost == 0) ? 2 : minCost / mCostDefMasked;
                DUMP_UINT(minCost)
            }
#endif


            for ( int aPx = 0 ; aPx < dZ ; aPx++)
                finCo[aPx] += forCo[aPx];

            pitStrm += dZ;

        }

        idLine++;
    }
#ifdef OUTPUTDEFCOR
   DUMP_LINE
#endif

    //nvtxRangePop();

}

Pt2di cGBV2_ProgDynOptimiseur::direction(int aNbDir, int aKDir)
{
    double teta = (double)((double)aKDir*PI)/(double)aNbDir;

//    BUG cos double retourne NaN en Jp2 avec GCC pas avec Clang!!!!!!!!!!!!!!!!
//    double c    = std::cos(teta);
//    double s    = std::sin(teta);
    double c    = std::cos((float)teta);
    double s    = std::sin((float)teta);
    Pt2dr d2    = Pt2dr(ElStdTypeScal<double>::RtoT(c*100.f),ElStdTypeScal<double>::RtoT(s*100.f));

    return Pt2di( vunit(d2) * 20.0);
}

void cGBV2_ProgDynOptimiseur::SolveAllDirectionGpu(int aNbDir)
{
    const std::vector<Pt2di> * aVPt;

    ushort aPenteMax = (ushort)mEtape.EtapeMEC().ModulationProgDyn().Val().Px1PenteMax().Val();

    float penteMax = (float) aPenteMax/mEtape.KPx(0).ComputedPas();
    IGpuOpt.Prepare(mSz.x,mSz.y,aPenteMax,aNbDir,mCostRegul[0],mCostRegul[1],mCostDefMasked,mCostTransMaskNoMask,mHasMaskAuto);

    TIm2DBits<1> aTMask(mLTCur->ImMasqTer());

    Pt2di aPTer;
    sMatrixCellCost<ushort> &mCellCost = IGpuOpt._poInitCost;

    for (aPTer.y=0 ; aPTer.y<mSz.y ; aPTer.y++)
        for (aPTer.x=0 ; aPTer.x<mSz.x ; aPTer.x++)
            if(aTMask.get(aPTer))
                mCellCost.setDefCor(make_uint2(aPTer.x,aPTer.y),mCostDefMasked);

    int     aKDir       = 0;
    int     aKPreDir    = 0;
    bool    idPreCo     = false;

    IGpuOpt.SetCompute(true);

    //direction(aNbDir, 0);
    //GpGpuTools::OutputInfoGpuMemory();

    while (aKDir < aNbDir)
    {

        if( aKPreDir <= aKDir + 1 && aKPreDir < aNbDir &&  IGpuOpt.GetPreComp() )
        {

            Pt2di aDirI = direction(aNbDir, aKPreDir);

            uint idLine = 0, sizeStreamLine, pitStream = IGpuOpt._poInitCost._maxDz, pitIdStream = WARPSIZE ;

            mLMR.Init(aDirI,Pt2di(0,0),mSz);

            //GpGpuTools::NvtxR_Push("Prepa",0xFF0000FF);

            while ((aVPt = mLMR.Next()))
            {
                uint lenghtLine = (uint)(aVPt->size());

                IGpuOpt.HData2Opt().SetParamLine(idLine,pitStream,pitIdStream,lenghtLine,idPreCo);

                sizeStreamLine = 0;

                for (uint aK = 0 ; aK < lenghtLine; aK++)
                {
                    #ifndef CLAMPDZ
                        sizeStreamLine += IGpuOpt._poInitCost.DZ((Pt2di)(*aVPt)[aK]);
                    #else
//                        if(IGpuOpt._poInitCost.DZ((Pt2di)(*aVPt)[aK]) > IGpuOpt._poInitCost._maxDz)
//                                printf("AAAAAAAAAAAAAA : %d\n",IGpuOpt._poInitCost.DZ((Pt2di)(*aVPt)[aK]));
                        sizeStreamLine += min(IGpuOpt._poInitCost.DZ((Pt2di)(*aVPt)[aK]),IGpuOpt._poInitCost._maxDz);
                    #endif
                }

                // PREDEFCOR
                pitIdStream += iDivUp32(lenghtLine)     << 5;
                pitStream   += iDivUp32(sizeStreamLine) << 5;

                idLine++;
            }

            //nvtxRangePop();

            IGpuOpt.HData2Opt().SetNbLine(idLine);

            double  mDMoyDir = average_euclid_line_seed(aDirI);

            int aJump    = round_ni(penteMax*mDMoyDir);
            int mMaxJumpDir  = ElMax(1,aJump);

            IGpuOpt.HData2Opt().setPenteMax(mMaxJumpDir);

            IGpuOpt.HData2Opt().ReallocInputIf(pitStream + IGpuOpt._poInitCost._maxDz,pitIdStream + WARPSIZE);

            copyCells_Mat2Stream(aDirI, IGpuOpt.HData2Opt(),IGpuOpt._poInitCost,idPreCo);

            //IGpuOpt.SetCompute(true);
            IGpuOpt.SetPreComp(false);
            IGpuOpt.simpleJob();

            aKPreDir++;
            idPreCo = !idPreCo;
        }

        if(IGpuOpt.GetDataToCopy())
        {
            copyCells_Stream2Mat(direction(aNbDir,aKDir),IGpuOpt.HData2Opt(),IGpuOpt._poInitCost,IGpuOpt._preFinalCost1D,IGpuOpt._FinalDefCor,!IGpuOpt.GetIdBuf());
            IGpuOpt.SetDataToCopy(false);
            aKDir++;
        }
    }

    IGpuOpt.freezeCompute();

    //GpGpuTools::NvtxR_Push("FinalCost",0xFF883300);
    for (uint line = 0 ; line < (uint)mSz.y; line++)
        for (uint aK = 0 ; aK < (uint)mSz.x; aK++)
        {
            Pt2di ptd(aK,line);
            uint2 ptTer     = make_uint2(aK,line);

            tCGBV2_tMatrCelPDyn &  aMat = mMatrCel[ptd];
            short3  pDe      = IGpuOpt._poInitCost.PtZ(ptTer);
            short2 ptZ      = make_short2(pDe.x,pDe.y);
            uint* finalCost = IGpuOpt._preFinalCost1D.pData() + IGpuOpt._poInitCost.Pit(ptTer) - ptZ.x ;

            for ( int aPx = ptZ.x ; aPx < ptZ.y ; aPx++)
                 aMat[Pt2di(aPx,0)].SetCostFinal(finalCost[aPx]);
        }

    ////nvtxRangePop();

}
#endif

void cGBV2_ProgDynOptimiseur::writePoint(FILE* aFP,  Pt3dr            aP,Pt3di           aW)
{
    WriteType(aFP,float(aP.x));
    WriteType(aFP,float(aP.y));
    WriteType(aFP,float(aP.z));

    WriteType(aFP,(U_INT1)(aW.x));
    WriteType(aFP,(U_INT1)(aW.y));
    WriteType(aFP,(U_INT1)(aW.z));
}



void cGBV2_ProgDynOptimiseur::Local_SolveOpt(Im2D_U_INT1 aImCor)
{

    // double aVPentes[theDimPxMax];
    const cModulationProgDyn &  aModul = mEtape.EtapeMEC().ModulationProgDyn().Val();

    // std::cout << " ZRrg " << mCostRegul[0] << " Pente " <<  aModul.Px1PenteMax().Val() << "\n";

   double aPenteMax = aModul.Px1PenteMax().Val();
   double aRegul    =  mCostRegul[0];
   double aRegul_Quad = 0.0;

   mHasMaskAuto = aModul.ArgMaskAuto().IsInit();

   if(mHasMaskAuto)
   {
       const cArgMaskAuto & anAMA  = aModul.ArgMaskAuto().Val();
       //AmplifKL = anAMA.AmplKLPostTr().Val();
       mCostDefMasked = CostR2I(mAppli.CurCorrelToCout(anAMA.ValDefCorrel()));
       mCostTransMaskNoMask = CostR2I(anAMA.CostTrans());
   }
   else
   {
       mCostTransMaskNoMask = 20000;
       mCostDefMasked       = 8000;
   }

    //=================
    double aVPentes[theDimPxMax];

    mCostRegul[0] = aRegul;
    mCostRegul[1] = 0;

    mCostRegul_Quad[0] = aRegul_Quad;
    mCostRegul_Quad[1] = 0 ;

    aVPentes[0] = aPenteMax;
    aVPentes[1] = 10;

    for (int aKP=0 ; aKP<mNbPx ; aKP++)
    {
        double aPente = aVPentes[aKP]; // MODIF  / mEtape.KPx(aKP).Pas();
        mMaxEc[aKP] = ElMax(1,round_ni(aPente));
    }

    int nbDirection = 0;

    for
    (
        std::list<cEtapeProgDyn>::const_iterator itE=aModul.EtapeProgDyn().begin();
        itE!=aModul.EtapeProgDyn().end();
        itE++
    )
    {
        nbDirection = itE->NbDir().Val();
        SolveOneEtape(nbDirection);
    }
//GpGpuTools::NvtxR_Push(__FUNCTION__,0x335A0033);
//Im2D_INT4 aDupRes(mSz.x,mSz.y);

    ////////////////////////////////////////////////////////////////////////////////
    //  write ply file
    //  Mode Ecriture : binaire ou non



#ifdef SAVEPLY
    bool deZoom = mEtape.EtapeMEC().DeZoom() == 128;
    FILE * aFP = NULL;
    bool aBin= true;
    if(deZoom)
    {

        //printf("SAVEEEEEEEEEEEE PLY\n");

        int random_file = rand()%100;


        //int Number = 123;       // number to be converted to a string

        string Result;          // string which will contain the result

        ostringstream convert;   // stream used for the conversion

        convert << random_file;      // insert the textual representation of 'Number' in the characters in the stream

        Result = convert.str(); // set 'Result' to the contents of the stream

        string aNameOut = "defCor_" + Result + ".ply";

        string mode = aBin ? "wb" : "w";
        aFP = FopenNN(aNameOut,mode,"MergePly");

        //Header
        fprintf(aFP,"ply\n");
        string aBinSpec = MSBF_PROCESSOR() ? "binary_big_endian":"binary_little_endian" ;

        fprintf(aFP,"format %s 1.0\n",aBin?aBinSpec.c_str():"ascii");

        fprintf(aFP,"comment author: Gerald\n");
        fprintf(aFP,"comment object: Nappe\n");

        //fprintf(aFP,"element vertex %d\n", mSz.x*mSz.y*3);
        fprintf(aFP,"element vertex %d\n", mSz.x*mSz.y);
        fprintf(aFP,"property float x\n");
        fprintf(aFP,"property float y\n");
        fprintf(aFP,"property float z\n");

        fprintf(aFP,"property uchar red\n");
        fprintf(aFP,"property uchar green\n");
        fprintf(aFP,"property uchar blue\n");

        fprintf(aFP,"element face %d\n",0);
        fprintf(aFP,"property list uchar int vertex_indices\n");
        fprintf(aFP,"end_header\n");
    }

#endif

    //////////////////////////////////////////////////////////////////////

    {
        Pt2di aPTer;

        if(mHasMaskAuto)
        {
            mMaskCalcDone = true;
            mMaskCalc = Im2D_Bits<1>(mSz.x,mSz.y);
        }

        TIm2DBits<1>    aTMask(mMaskCalc);

        for (aPTer.y=0 ; aPTer.y<mSz.y ; aPTer.y++)
        {
            for (aPTer.x=0 ; aPTer.x<mSz.x ; aPTer.x++)
            {
                tCGBV2_tMatrCelPDyn &  aMat = mMatrCel[aPTer];
                const Box2di &  aBox = aMat.Box();
                Pt2di aPRX;
                Pt2di aPRXMin;
                tCost   aCostMin = tCost(1e9);
               // tCost   agreMin  = 0;
                //for (aPRX.y=aBox._p0.y ;aPRX.y<aBox._p1.y; aPRX.y++)
                {
                    for (aPRX.x=aBox._p0.x ;aPRX.x<aBox._p1.x; aPRX.x++)
                    {
                        tCost aCost = aMat[aPRX].GetCostInit();
                        //agreMin += aCost;
                        if (aCost<aCostMin)
                        {

                            aCostMin = aCost;
                            aPRXMin = aPRX;
                        }
                    }
                }

                #ifdef CUDA_ENABLED
                if(mHasMaskAuto)
                {
                    /* CUDA_DEFCOR Officiel*/

                    tCost defCOf = IGpuOpt._FinalDefCor[make_uint2(aPTer.x,aPTer.y)];
                    bool NoVal   = defCOf <  aCostMin; // verifier que je n'ajoute pas 2 fois cost init def cor!!!!!
                    aTMask.oset(aPTer,(!NoVal)  && ( mLTCur->IsInMasq(aPTer)));


                    int aCorI = CostI2CorExport(aCostMin);
                    if(!mLTCur->IsInMasq(aPTer))
                    {
                        aCorI = 0;
                    }

                    if(aImCor.Im2D<U_INT1,INT>::Inside(aPTer))
                        aImCor.SetI(aPTer,aCorI);

                }
                #endif

                mDataImRes[0][aPTer.y][aPTer.x] = aPRXMin.x;

            }
        }

        /* officiel COMBLE TROU */
        if (mHasMaskAuto)
            CombleTrouPrgDyn(aModul,mMaskCalc,mLTCur->ImMasqTer(),mImRes[0]);

/*
        Rect zone(0,0,mSz.x,mSz.y);
        uint2   pTer;
        uint    maxITSPI = 64;

        for (pTer.y=0 ; pTer.y<(uint)mSz.y ; pTer.y++)
        {
            for (pTer.x=0 ; pTer.x<(uint)mSz.x ; pTer.x++)
            {

                int2 curPT          = make_int2(pTer);

                uint minCOR = 10000;

                if(!aTMask.getOK(Pt2di(pTer.x,pTer.y)))
                {
                    int zMin    = 1e9;
                    bool findZ  = false;

                    ushort  iteSpi   = 1;
                    int zMax    = 0;
                    int zMoyen  = 0;
                    int pond    = 0;

                    while((!findZ || iteSpi < maxITSPI) && (iteSpi < maxITSPI) )
                    {
                        bool pair   = (iteSpi % 2) == 0;
                        int vec     = (float)iteSpi/2.f + 0.5f;
                        int sign    = (vec % 2) == 0 ? 1 : -1;
                        int2 tr     = make_int2(sign * pair,sign * !pair);

                        for (int i = 0; i < vec; ++i,curPT += tr)
                        {
                            if(zone.inside(curPT) && IGpuOpt._FinalDefCor[make_uint2(curPT)] >= ((float)minCOR*2.0f))
                            {
                                zMin = min(zMin,mDataImRes[0][curPT.y][curPT.x]);
                                zMax = max(zMax,mDataImRes[0][curPT.y][curPT.x]);
                                zMoyen += mDataImRes[0][curPT.y][curPT.x];
                                pond++;

                                if(!findZ)
                                    findZ  = true;
                            }
                        }

                        iteSpi++;
                    }
                    //mDataImRes[0][pTer.y][pTer.x] = zMin;
                    //IGpuOpt._FinalDefCor[pTer] = 30000;
                    if(findZ) /// TODO!!!! : costinit a defcor si minimum !!!!
                        mDataImRes[0][pTer.y][pTer.x] = zMoyen/pond;

                }

#ifdef SAVEPLY

        //uint2 pTer;

        if(deZoom)
        {
            Pt3dr aP(float(pTer.x),float(pTer.y),float(mDataImRes[0][pTer.y][pTer.x]));
            //Pt3dr aPMax(float(pTer.x),float(pTer.y),float(aBox._p1.x));
            //Pt3dr aPMin(float(pTer.x),float(pTer.y),float(aBox._p0.x));
            Pt3di aW(255,255,255);
            Pt3di aR(255,0,0);
            Pt3di aG(0,255,0);
            Pt3di aB(0,0,255);

            if (aBin)
            {
                //writePoint(aFP, aP, cI > clamp ? Pt3di(255,(float)255.f*(cI-clamp)/(10000-clamp),0) : aG);
                //writePoint(aFP, aP, Pt3di(255,(float)255.f*(finalDefCor/10000 ),0));

                //writePoint(aFP, aP, finalDefCor == 0 ? aG : aR);

                float colorll = (float)256 - (float)256.f*(finalDefCor)/(qualiMAx);

                writePoint(aFP, aP, finalDefCor > minCOR  ? Pt3di(256 - colorll,colorll,0) : aB);


                //writePoint(aFP, aP, finalDefCor > 10000 ? finalDefCor -  : aR);
                //                    writePoint(aFP, aPMax, aR);
                //                    writePoint(aFP, aPMin, aG);
            }
            else
                fprintf(aFP,"%.3f %.3f %.3f %d %d %d\n",aP.x,aP.y,aP.z,aW.x,aW.y,aW.z);
        }
#endif
            }

        }
*/

    }
//nvtxRangePop();

    ////////////////////////////////////
    ///
    ///CLOSE PLY...
#ifdef SAVEPLY
    if(deZoom && aFP !=NULL)
        ElFclose(aFP);
#endif

//    if (0)
//    {

//        Video_Win aW = Video_Win::WStd(mSz,5.0);
//        ELISE_COPY(aW.all_pts(),aDupRes.in()*10,aW.ocirc());
//        getchar();
//    }
}

cSurfaceOptimiseur * cSurfaceOptimiseur::AllocAlgoTestGPU
                     (
                                     cAppliMICMAC &    mAppli,
                                     cLoadTer&         mLT,
                                     const cEquiv1D &        anEqX,
                                     const cEquiv1D &        anEqY
                     )
{
   return new cGBV2_ProgDynOptimiseur
              (
                  mAppli,mLT,
                  anEqX,anEqY,
                  mLT.KthNap(0).mImPxMin,
                  mLT.KthNap(0).mImPxMax
              );
}


/**************************************************/
/*                                                */
/*               cSurfaceOptimiseur               */
/*                                                */
/**************************************************/



/*Footer-MicMac-eLiSe-25/06/2007

Ce logiciel est un programme informatique servant �  la mise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est régi par la licence CeCILL-B soumise au droit français et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffusée par le CEA, le CNRS et l'INRIA
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilité au code source et des droits de copie,
de modification et de redistribution accordés par cette licence, il n'est
offert aux utilisateurs qu'une garantie limitée.  Pour les mêmes raisons,
seule une responsabilité restreinte p�?se sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les concédants successifs.

A cet égard  l'attention de l'utilisateur est attirée sur les risques
associés au chargement,  �  l'utilisation,  �  la modification et/ou au
développement et �  la reproduction du logiciel par l'utilisateur étant
donné sa spécificité de logiciel libre, qui peut le rendre complexe �
manipuler et qui le réserve donc �  des développeurs et des professionnels
avertis possédant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invités �  charger  et  tester  l'adéquation  du
logiciel �  leurs besoins dans des conditions permettant d'assurer la
sécurité de leurs syst�?mes et ou de leurs données et, plus généralement,
�  l'utiliser et l'exploiter dans les mêmes conditions de sécurité.

Le fait que vous puissiez accéder �  cet en-tête signifie que vous avez
pris connaissance de la licence CeCILL-B, et que vous en avez accepté les
termes.
Footer-MicMac-eLiSe-25/06/2007*/
