#include <string>
#include <vector>
#include "string2.h"
#include "smalloc.h"
#include "maths.h"
#include "poldata.h"
#include "gmx_simple_comm.h"
#include "molprop.hpp"
      
const char *mpo_name[MPO_NR] = 
{ 
    "potential", "dipole", "quadrupole", "polarizability", "energy" 
};

const char *cs_name[CS_NR] =
{
    "recv_data", "recv_empty", "send_data", "send_empty" 
};

#define GMX_SEND_DATA 19823
#define GMX_SEND_DONE -666
CommunicationStatus gmx_send_data(t_commrec *cr,int dest)
{
    gmx_send_int(cr,dest,GMX_SEND_DATA);
    
    return CS_SEND_DATA;
}

CommunicationStatus gmx_send_done(t_commrec *cr,int dest)
{
    gmx_send_int(cr,dest,GMX_SEND_DONE);
    
    return CS_SEND_DATA;
}

CommunicationStatus gmx_recv_data(t_commrec *cr,int src)
{
    int kk = gmx_recv_int(cr,src);
    
    if (kk == GMX_SEND_DATA)
        return CS_RECV_DATA;
    else { 
        if (kk != GMX_SEND_DONE)
            fprintf(stderr,"Received %d in gmx_recv_data. Was expecting either %d or %d\n.",kk,
                    (int)GMX_SEND_DATA,(int)GMX_SEND_DONE);
        return CS_RECV_EMPTY;
    }
}
#undef GMX_SEND_DATA
#undef GMX_SEND_DONE
    
namespace alexandria {

void GenericProperty::SetType(std::string type) 
{ 
    if ((_type.size() == 0) && (type.size() > 0))
        _type = type; 
    else
    {
        if (_type.size() == 0)
            fprintf(stderr,"Replacing GenericProperty type '%s' by '%s'\n",_type.c_str(),type.c_str());
    }
}
    
void GenericProperty::SetUnit(std::string unit)
{ 
    if ((_unit.size() == 0) && (unit.size() > 0))
        _unit = unit; 
    else
    {
        if (_unit.size() == 0)
            fprintf(stderr,"Replacing GenericProperty unit '%s' by '%s'\n",_unit.c_str(),unit.c_str());
    }
}
   
void CalcAtom::SetUnit(std::string unit)
{ 
    if ((_unit.size() == 0) && (unit.size() > 0))
        _unit = unit; 
    else
    {
        if (_unit.size() == 0)
            fprintf(stderr,"Replacing CalcAtom unit '%s' by '%s'\n",_unit.c_str(),unit.c_str());
    }
}
   
void MolecularComposition::AddAtom(AtomNum an)
{
    AtomNumIterator mci = SearchAtom(an.GetAtom());
    if (mci == EndAtomNum())
    {
        _atomnum.push_back(an); 
    }
    else
    {
        mci->SetNumber(mci->GetNumber()+an.GetNumber());
    }
}

void MolProp::AddBond(Bond b)
{
    BondIterator bi;
    bool bFound = false;
    
    for(bi=BeginBond(); !bFound && (bi<EndBond()); bi++)
    {
        bFound = (((bi->GetAi() == b.GetAi()) && (bi->GetAj() == b.GetAj())) ||
                  ((bi->GetAi() == b.GetAj()) && (bi->GetAj() == b.GetAi())));
        if (bFound)
            break;
    }
    if (!bFound)
    {
        _bond.push_back(b); 
    }
    else if ((NULL != debug) && (bi->GetBondOrder() != b.GetBondOrder()))
    {
        fprintf(debug,"Different bond orders in molecule %s\n",GetMolname().c_str());
    }
}

void MolecularComposition::DeleteAtom(std::string catom)
{
    AtomNumIterator ani;
    
    if ((ani = SearchAtom(catom)) != EndAtomNum())
    {
        _atomnum.erase(ani);
    }
}
      
AtomNumIterator MolecularComposition::SearchAtom(std::string an)
{
    AtomNumIterator ani;
    
    for(ani=BeginAtomNum(); (ani<EndAtomNum()); ani++)
    {
        if (ani->GetAtom() == an)
            return ani;
    }
    return EndAtomNum();
}
  
void MolecularComposition::ReplaceAtom(std::string oldatom,std::string newatom)
{
    AtomNumIterator i;
    
    for(i=BeginAtomNum(); (i<EndAtomNum()); i++) 
    {
        if (oldatom == i->GetAtom())
        {
            i->SetAtom(newatom);
            break;
        }
    }
}
  
int MolecularComposition::CountAtoms(std::string atom)
{
    AtomNumIterator i;
    
    for(i=BeginAtomNum(); (i<EndAtomNum()); i++) 
    {
        if (atom == i->GetAtom())
            return i->GetNumber();
    }
    return 0;
}
  
int MolecularComposition::CountAtoms(const char *atom)
{
    std::string str(atom);
    
    return CountAtoms(str);
}
  
int MolecularComposition::CountAtoms()
{
    int nat = 0;
    AtomNumIterator i;
    
    for(i=BeginAtomNum(); (i<EndAtomNum()); i++) 
    {
        nat += i->GetNumber();
    }
    return nat;
}

void MolProp::CheckConsistency() 
{
}
  
bool MolProp::SearchCategory(std::string catname)
{
    std::vector<std::string>::iterator i;
    
    for(i=BeginCategory(); (i < EndCategory()); i++)
    {
        if (*i == catname)
            return true;
    }
    return false;
}
  
void MolProp::DeleteComposition(std::string compname)
{
    MolecularCompositionIterator i;
    
    for(i=BeginMolecularComposition(); (i<EndMolecularComposition()); i++) 
    {
        if (i->GetCompName() == compname) 
        {
            break;
        }
    }
    if (i < EndMolecularComposition())
        _mol_comp.erase(i);
}

void Experiment::Dump(FILE *fp)
{
    if (NULL != fp)
    {
        fprintf(fp,"Experiment %d polar %d dipole\n",
                NPolar(),NDipole());
        fprintf(fp,"reference    = %s\n",_reference.c_str());
        fprintf(fp,"conformation = %s\n",_conformation.c_str());
    }
}

void Calculation::Dump(FILE *fp)
{
    Experiment::Dump(fp);
    if (NULL != fp)
    {
        fprintf(fp,"Calculation %d polar %d dipole\n",
                NPolar(),NDipole());
        fprintf(fp,"program    = %s\n",_program.c_str());
        fprintf(fp,"method     = %s\n",_method.c_str());
        fprintf(fp,"basisset   = %s\n",_basisset.c_str());
        fprintf(fp,"datafile   = %s\n",_datafile.c_str());
    }
}

void Experiment::Merge(Experiment& src)
{
    alexandria::MolecularEnergyIterator mei;
    alexandria::MolecularDipPolarIterator dpi;
    alexandria::MolecularQuadrupoleIterator mqi;
    
    for(mei=src.BeginEnergy(); (mei<src.EndEnergy()); mei++)
    {
        alexandria::MolecularEnergy me(mei->GetType(),mei->GetUnit(),mei->GetValue(),mei->GetError());
        AddEnergy(me);
    }

    for(dpi=src.BeginDipole(); (dpi<src.EndDipole()); dpi++) 
    {
        alexandria::MolecularDipPolar dp(dpi->GetType(),dpi->GetUnit(),
                                         dpi->GetX(),dpi->GetY(),dpi->GetZ(),
                                         dpi->GetAver(),dpi->GetError());
        AddDipole(dp);
    }
      
    for(dpi=src.BeginPolar(); (dpi<src.EndPolar()); dpi++) 
    {
        alexandria::MolecularDipPolar dp(dpi->GetType(),dpi->GetUnit(),
                                         dpi->GetX(),dpi->GetY(),dpi->GetZ(),
                                         dpi->GetAver(),dpi->GetError());
        AddPolar(dp);
    }

    for(mqi=src.BeginQuadrupole(); (mqi<src.EndQuadrupole()); mqi++)
    {
        alexandria::MolecularQuadrupole mq(mqi->GetType(),mqi->GetUnit(),
                                           mqi->GetXX(),mqi->GetYY(),mqi->GetZZ(),
                                           mqi->GetXY(),mqi->GetXZ(),mqi->GetYZ());
        AddQuadrupole(mq);
    }
}

void Calculation::Merge(Calculation& src)
{
    Experiment::Merge(src);
    
    for(CalcAtomIterator cai=src.BeginAtom(); (cai<src.EndAtom()); cai++)
    {
        double x,y,z;
        CalcAtom caa(cai->GetName(),cai->GetObtype(),cai->GetAtomid());
        
        cai->GetCoords(&x,&y,&z);
        caa.SetCoords(x,y,z);
        caa.SetUnit(cai->GetUnit());
        
        for(AtomicChargeIterator aci=cai->BeginQ(); (aci<cai->EndQ()); aci++)
        {
            AtomicCharge aq(aci->GetType(),aci->GetUnit(),aci->GetQ());
            caa.AddCharge(aq);
        }
        AddAtom(caa);
    }

    for(ElectrostaticPotentialIterator mep=src.BeginPotential(); (mep<src.EndPotential()); mep++)
    {
        alexandria::ElectrostaticPotential ep(mep->GetXYZunit(),mep->GetVunit(),mep->GetEspid(),
                                              mep->GetX(),mep->GetY(),mep->GetZ(),mep->GetV());
        AddPotential(ep);
    }
}

void CalcAtom::AddCharge(AtomicCharge q)
{
    AtomicChargeIterator aci;

    for(aci=BeginQ(); (aci<EndQ()); aci++)
    {
        if ((aci->GetType() == q.GetType()) &&
            (aci->GetUnit() == q.GetUnit()) &&
            (aci->GetQ() == q.GetQ()))
            break;
    }
    if (aci == EndQ())
        _q.push_back(q);
}

bool CalcAtom::Equal(CalcAtom ca)
{
    if ((_name != ca.GetName()) ||
        (_obtype != ca.GetObtype()) ||
        (_x != ca.GetX()) ||
        (_y != ca.GetY()) ||
        (_z != ca.GetZ()) ||
        (_atomid != ca.GetAtomid()))
        return false;
    return true;
}

CalcAtomIterator Calculation::SearchAtom(CalcAtom ca)
{
    CalcAtomIterator cai;
    for(cai=BeginAtom(); (cai<EndAtom()); cai++)
    {
        if (cai->Equal(ca))
            break;
    }
    return cai;
}

void Calculation::AddAtom(CalcAtom ca)
{
    CalcAtomIterator cai;
    
    if ((cai = SearchAtom(ca)) == EndAtom())
    { 
        _catom.push_back(ca); 
    }
    else
    {
        printf("Trying to add identical atom %s (%s) twice. N = %d\n",
               ca.GetName().c_str(),ca.GetObtype().c_str(),(int)_catom.size());
    }
}

void MolProp::AddComposition(MolecularComposition mc)
{
    MolecularCompositionIterator mci = SearchMolecularComposition(mc.GetCompName());
    if (mci == EndMolecularComposition())
    {
        _mol_comp.push_back(mc);
    }
}

bool MolProp::BondExists(Bond b)
{
    for(alexandria::BondIterator bi=BeginBond(); (bi<EndBond()); bi++) 
    {
        if (((bi->GetAi() == b.GetAi()) && (bi->GetAj() == b.GetAj())) ||
            ((bi->GetAi() == b.GetAj()) && (bi->GetAj() == b.GetAi())))
        {
            return true;
        }
    }
    return false;
}

void MolProp::Merge(MolProp& src)
{
    double q,sq;
    std::string stmp,dtmp;
    
    for(std::vector<std::string>::iterator si=src.BeginCategory(); (si<src.EndCategory()); si++) 
    {
        AddCategory(*si);
    }
    SetFormula(src.GetFormula());
    SetMass(src.GetMass());
    if (GetMultiplicity() <= 1)
    {
        SetMultiplicity(src.GetMultiplicity());
    }
    else 
    {
        int smult = src.GetMultiplicity();
        if ((NULL != debug) && (smult != GetMultiplicity()))
            fprintf(debug,"Not overriding multiplicity to %d when merging since it is %d (%s)\n",
                    smult,GetMultiplicity(),src.GetMolname().c_str());
    }
    q = GetCharge();
    if (q == 0)
        SetCharge(src.GetCharge());
    else
    {
        sq = src.GetCharge();
        if ((NULL != debug) && (sq != q))
            fprintf(debug,"Not overriding charge to %g when merging since it is %g (%s)\n",
                    sq,q,GetMolname().c_str());
    }
  
    stmp = src.GetMolname();
    if ((GetMolname().size() == 0) && (stmp.size() != 0))
        SetMolname(stmp);

    stmp = src.GetIupac();
    if ((GetIupac().size() == 0) && (stmp.size() != 0))
        SetIupac(stmp);
  
    stmp = src.GetCas();
    if ((GetCas().size() == 0) && (stmp.size() != 0))
        SetCas(stmp);
      
    stmp = src.GetCid();
    if ((GetCid().size() == 0) && (stmp.size() != 0))
        SetCid(stmp);
      
    stmp = src.GetInchi();
    if ((GetInchi().size() == 0) && (stmp.size() != 0))
        SetInchi(stmp);
  
    if (NBond() == 0)
    {
        for(alexandria::BondIterator bi=src.BeginBond(); (bi<src.EndBond()); bi++) 
        {
            alexandria::Bond bb(bi->GetAi(),bi->GetAj(),bi->GetBondOrder());
            AddBond(bb);
        }
    }
    else
    {
        for(alexandria::BondIterator bi=src.BeginBond(); (bi<src.EndBond()); bi++) 
        {
            alexandria::Bond bb(bi->GetAi(),bi->GetAj(),bi->GetBondOrder());
            if (!BondExists(bb))
            {
                fprintf(stderr,"WARNING bond %d-%d not present in %s\n",
                        bi->GetAi(),bi->GetAj(),GetMolname().c_str());
            }
        }
    }

    for(alexandria::ExperimentIterator ei=src.BeginExperiment(); (ei<src.EndExperiment()); ei++)
    {
        Experiment ex(ei->GetReference(),ei->GetConformation());
        
        ex.Merge(*ei);
        AddExperiment(ex);
    }

    for(alexandria::CalculationIterator ci=src.BeginCalculation(); (ci<src.EndCalculation()); ci++)
    {    
        Calculation ca(ci->GetProgram(),ci->GetMethod(),
                       ci->GetBasisset(),ci->GetReference(),
                       ci->GetConformation(),ci->GetDatafile());
        ca.Merge(*ci);
      
        AddCalculation(ca);
    }
    
    for(alexandria::MolecularCompositionIterator mci=src.BeginMolecularComposition(); 
        (mci<src.EndMolecularComposition()); mci++) 
    {
        for(alexandria::AtomNumIterator ani=mci->BeginAtomNum(); (ani<mci->EndAtomNum()); ani++) 
        {
            AtomNum an(ani->GetAtom(),ani->GetNumber());
            mci->AddAtom(an);
        }
        AddComposition(*mci);
    }
}

MolecularCompositionIterator MolProp::SearchMolecularComposition(std::string str)
{
    MolecularCompositionIterator i;
    
    for(i=BeginMolecularComposition(); (i<EndMolecularComposition()); i++) 
    {
        if (i->GetCompName() == str) 
            return i;
    }
    return i;
}

void MolProp::Dump(FILE *fp)
{
    std::vector<std::string>::iterator si;
    ExperimentIterator ei;
    CalculationIterator ci;
    
    if (fp) {
        fprintf(fp,"formula:      %s\n",GetFormula().c_str());
        fprintf(fp,"molname:      %s\n",GetMolname().c_str());
        fprintf(fp,"iupac:        %s\n",GetIupac().c_str());
        fprintf(fp,"CAS:          %s\n",GetCas().c_str());
        fprintf(fp,"cis:          %s\n",GetCid().c_str());
        fprintf(fp,"InChi:        %s\n",GetInchi().c_str());
        fprintf(fp,"mass:         %g\n",GetMass());
        fprintf(fp,"charge:       %d\n",GetCharge());
        fprintf(fp,"multiplicity: %d\n",GetMultiplicity());
        fprintf(fp,"category:    ");
        for(si=BeginCategory(); (si<EndCategory()); si++)
        {
            fprintf(fp," '%s'",si->c_str());
        }
        fprintf(fp,"\n");
        for(ei=BeginExperiment(); (ei<EndExperiment()); ei++)
        {
            ei->Dump(fp);
        }
        for(ci=BeginCalculation(); (ci<EndCalculation()); ci++)
        {
            ci->Dump(fp);
        }
    }
}

bool MolProp::GenerateComposition(gmx_poldata_t pd)
{
    CalculationIterator ci;
    CalcAtomIterator cai;
    MolecularComposition mci_bosque("bosque"),mci_spoel("spoel"),mci_miller("miller");
    AtomNumIterator ani;
    
    for(ci=BeginCalculation(); (mci_spoel.CountAtoms() <= 0) && (ci<EndCalculation()); ci++) 
    {
        /* This assumes we have either all atoms or none. 
         * A consistency check could be
         * to compare the number of atoms to the formula */
        for(cai=ci->BeginAtom(); (cai<ci->EndAtom()); cai++)
        {
            AtomNum anb(cai->GetName(),1),ans(cai->GetObtype(),1);
            mci_bosque.AddAtom(anb);
            mci_spoel.AddAtom(ans);
            
            const char *ptype = gmx_poldata_atype_to_ptype(pd,cai->GetObtype().c_str());
            if (NULL != ptype)
            {
                const char *miller = gmx_poldata_ptype_to_miller(pd,ptype);
                                                 
                if (NULL != miller)
                {
                    AtomNum anm(miller,1);
                    mci_miller.AddAtom(anm);
                }
            }
        }
    }
    
    if (mci_bosque.CountAtoms() > 0)
    {
        AddComposition(mci_bosque);
    }
    if (mci_miller.CountAtoms() > 0)
    {
        AddComposition(mci_miller);
    }
    if (mci_spoel.CountAtoms() > 0)
    {
        AddComposition(mci_spoel);
        if (NULL != debug) 
        {
            fprintf(debug,"LO_COMP: ");
            for(ani = mci_spoel.BeginAtomNum(); (ani < mci_spoel.EndAtomNum()); ani++)
            {
                fprintf(debug," %s:%d",ani->GetAtom().c_str(),ani->GetNumber());
            }
            fprintf(debug,"\n");
        }
        return true;
    }
    return false;
}

bool MolProp::GenerateFormula(gmx_atomprop_t ap)
{
    int  j,cnumber,an;
    char formula[1280],number[32];
    int  *ncomp;
    real value;
    std::string compname,catom,mform;
    alexandria::MolPropIterator mpi;
    alexandria::MolecularCompositionIterator mci;
    alexandria::AtomNumIterator ani;
    
    snew(ncomp,110);  
    formula[0] = '\0';
    mci = SearchMolecularComposition("bosque");
    if (mci != EndMolecularComposition()) 
    {
        for(ani=mci->BeginAtomNum(); (ani<mci->EndAtomNum()); ani++)
        {
            catom = ani->GetAtom();
            cnumber = ani->GetNumber();
            if (gmx_atomprop_query(ap,epropElement,"???",catom.c_str(),&value))
            {
                an = gmx_nint(value);
                range_check(an,0,110);
                if (an > 0)
                    ncomp[an] += cnumber;
            }
        }
    }
    if (ncomp[6] > 0) 
    {
        strcat(formula,"C");
        if (ncomp[6] > 1) 
        {
            sprintf(number,"%d",ncomp[6]);
            strcat(formula,number);
        }
        ncomp[6] = 0;
        if (ncomp[1] > 0) 
        {
            strcat(formula,"H");
            if (ncomp[1] > 1) 
            {
                sprintf(number,"%d",ncomp[1]);
                strcat(formula,number);
            }
            ncomp[1] = 0;
        }
    }
    for(j=110; (j>=1); j--) 
    {
        if (ncomp[j] > 0)
        {
            strcat(formula,gmx_atomprop_element(ap,j));
            if (ncomp[j] > 1) 
            {
                sprintf(number,"%d",ncomp[j]);
                strcat(formula,number);
            }
        }
    }
    mform = GetFormula();
    if (strlen(formula) > 0) 
    {
        if (debug) 
        {
            if ((mform.size() > 0) && (strcasecmp(formula,mform.c_str()) != 0))
                fprintf(debug,"Formula '%s' does match '%s' based on composition for %s.\n",
                        mform.c_str(),formula,GetMolname().c_str());
        }
        SetFormula(formula);
    }
    else if ((mform.size() == 0) && debug)
        fprintf(debug,"Empty composition and formula for %s\n",
                GetMolname().c_str());
    sfree(ncomp);
    
    return (strlen(formula) > 0);
}
  
bool MolProp::HasComposition(std::string composition)
{
    bool bComp  = false;
    MolecularCompositionIterator mci;
    
    if (composition.size() > 0)
    {    
        for(mci=BeginMolecularComposition(); !bComp && (mci<EndMolecularComposition()); mci++)
        {
            if (mci->GetCompName() == composition)
                bComp = true;
        }
    }
    if (debug && !bComp)
        fprintf(debug,"No composition %s for molecule %s\n",composition.c_str(),
                GetMolname().c_str());
              
    return bComp;   
}

bool Experiment::GetVal(const char *type,MolPropObservable mpo,
                        double *value,double *error,double vec[3],
                        tensor quadrupole) 
{
    bool   done = false;
    double x,y,z;
                
    switch (mpo) 
    {
    case MPO_ENERGY:
        for(MolecularEnergyIterator mei=BeginEnergy(); !done && (mei<EndEnergy()); mei++) 
        {
            if ((NULL == type) || (strcasecmp(mei->GetType().c_str(),type) == 0))
            {
                mei->Get(value,error);
                done = true;
            }
        }
        break;
    case MPO_DIPOLE:
        for(MolecularDipPolarIterator mdp=BeginDipole(); !done && (mdp<EndDipole()); mdp++)
        {
            if ((NULL == type) || (strcasecmp(mdp->GetType().c_str(),type) == 0))
            {
                mdp->Get(&x,&y,&z,value,error);
                vec[XX] = x;
                vec[YY] = y;
                vec[ZZ] = z;
                done = true;
            }
        }
        break;
    case MPO_POLARIZABILITY:
        for(MolecularDipPolarIterator mdp=BeginPolar(); !done && (mdp<EndPolar()); mdp++)
        {
            if ((NULL == type) || (strcasecmp(mdp->GetType().c_str(),type) == 0))
            {
                mdp->Get(&x,&y,&z,value,error);
                vec[XX] = x;
                vec[YY] = y;
                vec[ZZ] = z;
                done = true;
            }
        }
        break;
    case MPO_QUADRUPOLE:
        for(MolecularQuadrupoleIterator mqi=BeginQuadrupole(); !done && (mqi<EndQuadrupole()); mqi++)
        {
            if ((NULL == type) || (strcasecmp(mqi->GetType().c_str(),type) == 0))
            {
                double xx,yy,zz,xy,xz,yz;
                mqi->Get(&xx,&yy,&zz,&xy,&xz,&yz);
                quadrupole[XX][XX] = xx;
                quadrupole[XX][YY] = xy;
                quadrupole[XX][ZZ] = xz;
                quadrupole[YY][XX] = 0;
                quadrupole[YY][YY] = yy;
                quadrupole[YY][ZZ] = yz;
                quadrupole[ZZ][XX] = 0;
                quadrupole[ZZ][YY] = 0;
                quadrupole[ZZ][ZZ] = zz;
                done = true;
            }
        }
        break;
    default:
        break;
    }
    return done;
}

bool MolProp::GetPropRef(MolPropObservable mpo,iqmType iQM,char *lot,
                         const char *conf,const char *type,double *value,double *error,
                         char **ref,char **mylot,
                         double vec[3],tensor quadrupole)
{
    alexandria::ExperimentIterator ei;
    alexandria::CalculationIterator ci;
    std::string reference,method,name,basisset,program,conformation,expconf;
    
    int  k;
    bool done=false;
    char **ll;
    
    if ((iQM == iqmExp) || (iQM == iqmBoth)) 
    {
        for(ei=BeginExperiment(); (ei<EndExperiment()); ei++)
        {
            reference = ei->GetReference();
            expconf   = ei->GetConformation();
            
            if ((NULL == conf) || (strcasecmp(conf,expconf.c_str()) == 0))
                done = ei->GetVal(type,mpo,value,error,vec,quadrupole);
            if (done)
                break;
        }
        if (!done) {
            if (NULL != ref)
                *ref = strdup(reference.c_str());
            if (NULL != mylot)
                *mylot = strdup("Experiment");
        }
    }
    
    if ((!done) && ((iQM == iqmBoth) || (iQM == iqmQM)))
    {
        if (NULL != lot) 
        {
            ci = GetLotPropType(lot,mpo,type);
            if (ci != EndCalculation())
            {
                basisset     = ci->GetBasisset();
                method       = ci->GetMethod();
                reference    = ci->GetReference();
                conformation = ci->GetConformation();
                      
                if ((NULL == conf) || (strcasecmp(conf,conformation.c_str()) == 0))
                {
                    done = ci->GetVal(type,mpo,value,error,vec,quadrupole);
                }
                if (done && (NULL != ref))
                {
                    *ref = strdup(reference.c_str());
                }
                if (done && (NULL != mylot))
                {
                    *mylot = strdup(lot);
                }
            }
        }
    }
    return done;
}

bool MolProp::GetProp(MolPropObservable mpo,iqmType iQM,char *lot,
                      char *conf,char *type,double *value)
{
    double error,vec[3];
    tensor quad;

    return GetPropRef(mpo,iQM,lot,conf,type,value,&error,NULL,NULL,vec,quad);
}                


CalculationIterator MolProp::GetLotPropType(const char *lot,
                                            MolPropObservable mpo,
                                            const char *type)
{
    char **ll;
    CalculationIterator ci;
    
    ll = split('/',lot);
    if ((NULL != ll[0]) && (NULL != ll[1])) 
    {
        for(ci=BeginCalculation(); (ci < EndCalculation()); ci++)
        {
            if ((strcasecmp(ci->GetMethod().c_str(),ll[0]) == 0) &&
                (strcasecmp(ci->GetBasisset().c_str(),ll[1]) == 0))
            {
                bool done = false;
                switch (mpo) {
                case MPO_POTENTIAL:
                    done = ci->NPotential() > 0;
                    break;
                case MPO_DIPOLE:
                    for (MolecularDipPolarIterator mdp = ci->BeginDipole(); !done && (mdp < ci->EndDipole()); mdp++)
                    { 
                        done =  ((NULL == type) ||
                                 (strcasecmp(type,mdp->GetType().c_str()) == 0));
                    }
                    break;
                case MPO_QUADRUPOLE:
                    for (MolecularQuadrupoleIterator mdp = ci->BeginQuadrupole(); !done && (mdp < ci->EndQuadrupole()); mdp++)
                    { 
                        done =  ((NULL == type) ||
                                 (strcasecmp(type,mdp->GetType().c_str()) == 0));
                    }
                    break;
                case MPO_POLARIZABILITY:
                    for (MolecularDipPolarIterator mdp = ci->BeginPolar(); !done && (mdp < ci->EndPolar()); mdp++)
                    { 
                        done =  ((NULL == type) ||
                                 (strcasecmp(type,mdp->GetType().c_str()) == 0));
                    }
                    break;
                case MPO_ENERGY:
                    for (MolecularEnergyIterator mdp = ci->BeginEnergy(); !done && (mdp < ci->EndEnergy()); mdp++)
                    { 
                        done =  ((NULL == type) ||
                                 (strcasecmp(type,mdp->GetType().c_str()) == 0));
                    }
                    break;
                default:
                    break;
                }
                if (done)
                    break;
            }
        }
        int k = 0;
        while (ll[k] != NULL)
        {
            sfree(ll[k]);
            k++;
        }
        sfree(ll);
        
        return ci;
    }
    else
    {
        return EndCalculation();
    }
}

CalculationIterator MolProp::GetLot(const char *lot)
{
    char **ll;
    CalculationIterator ci;
    
    ll = split('/',lot);
    if ((NULL != ll[0]) && (NULL != ll[1])) 
    {
        bool done = false;
        for(ci=BeginCalculation(); (!done) && (ci < EndCalculation()); ci++)
        {
            done = ((strcasecmp(ci->GetMethod().c_str(),ll[0]) == 0) &&
                    (strcasecmp(ci->GetBasisset().c_str(),ll[1]) == 0));
            if (done)
                break;
        }
        int k = 0;
        while (ll[k] != NULL)
        {
            sfree(ll[k]);
            k++;
        }
        sfree(ll);
        
        return ci;
    }
    else
    {
        return EndCalculation();
    }
}

CommunicationStatus GenericProperty::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    
    cs = gmx_send_data(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        gmx_send_str(cr,dest,_type.c_str());
        gmx_send_str(cr,dest,_unit.c_str());
    }
    return cs;
}

CommunicationStatus GenericProperty::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    
    cs = gmx_recv_data(cr,src);
    if (CS_RECV_DATA == cs)
    {
        _type.assign(gmx_recv_str(cr,src));
        _unit.assign(gmx_recv_str(cr,src));
    }
    return cs;
}

CommunicationStatus MolecularQuadrupole::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    
    cs = GenericProperty::Send(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        cs = gmx_send_data(cr,dest);
    }
    if (CS_SEND_DATA == cs) 
    {
        gmx_send_double(cr,dest,_xx);
        gmx_send_double(cr,dest,_yy);
        gmx_send_double(cr,dest,_zz);
        gmx_send_double(cr,dest,_xy);
        gmx_send_double(cr,dest,_xz);
        gmx_send_double(cr,dest,_yz);
    }
    return cs;
}

CommunicationStatus MolecularQuadrupole::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;

    cs = GenericProperty::Receive(cr,src);
    if (CS_RECV_DATA == cs)
    {
        cs = gmx_recv_data(cr,src);    
    }
    if (CS_RECV_DATA == cs)
    {
        _xx    = gmx_recv_double(cr,src);
        _yy    = gmx_recv_double(cr,src);
        _zz    = gmx_recv_double(cr,src);
        _xy    = gmx_recv_double(cr,src);
        _xz    = gmx_recv_double(cr,src);
        _yz    = gmx_recv_double(cr,src);
    }
    return cs;
}

CommunicationStatus MolecularEnergy::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;

    cs = GenericProperty::Receive(cr,src);
    if (CS_RECV_DATA == cs)
    {
        cs = gmx_recv_data(cr,src);    
    }
    if (CS_RECV_DATA == cs)
    {
        _value = gmx_recv_double(cr,src);
        _error = gmx_recv_double(cr,src);
    }
    return cs;
}

CommunicationStatus MolecularEnergy::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    
    cs = GenericProperty::Send(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        cs = gmx_send_data(cr,dest);
    }
    if (CS_SEND_DATA == cs)
    {
        gmx_send_double(cr,dest,_value);
        gmx_send_double(cr,dest,_error);
    }
    return cs;
}

CommunicationStatus Bond::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    
    cs = gmx_send_data(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        gmx_send_int(cr,dest,_ai);
        gmx_send_int(cr,dest,_aj);
        gmx_send_int(cr,dest,_bondorder);
    }
    return cs;
}

CommunicationStatus Bond::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    
    cs = gmx_recv_data(cr,src);
    if (CS_RECV_DATA == cs)
    {
        _ai = gmx_recv_int(cr,src);
        _aj = gmx_recv_int(cr,src);
        _bondorder = gmx_recv_int(cr,src);
    }
    return cs;
}

CommunicationStatus MolecularDipPolar::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    
    cs = GenericProperty::Send(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        cs = gmx_send_data(cr,dest);
    }
    if (CS_SEND_DATA == cs)
    {
        gmx_send_double(cr,dest,_x);
        gmx_send_double(cr,dest,_y);
        gmx_send_double(cr,dest,_z);
        gmx_send_double(cr,dest,_aver);
        gmx_send_double(cr,dest,_error);
    }
    return cs;
}

CommunicationStatus MolecularDipPolar::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    
    cs = GenericProperty::Receive(cr,src);
    if (CS_RECV_DATA == cs)
    {
        cs = gmx_recv_data(cr,src);    
    }
    if (CS_RECV_DATA == cs)
    {
        _x     = gmx_recv_double(cr,src);
        _y     = gmx_recv_double(cr,src);
        _z     = gmx_recv_double(cr,src);
        _aver  = gmx_recv_double(cr,src);
        _error = gmx_recv_double(cr,src);
    }
    return cs;
}

CommunicationStatus Experiment::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    
    cs = gmx_recv_data(cr,src);    
    
    if (CS_RECV_DATA == cs)
    {
        //! Receive literature reference
        _reference    = gmx_recv_str(cr,src);
        
        //! Receive conformation
        _conformation = gmx_recv_str(cr,src);

        //! Receive polarizabilities
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {
            MolecularDipPolar mdp;
            
            cs = mdp.Receive(cr,src);
            if (CS_RECV_DATA == cs)
                AddPolar(mdp);
        }
        
        //! Receive dipoles
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {
            MolecularDipPolar mdp;
            
            cs = mdp.Receive(cr,src);
            if (CS_RECV_DATA == cs)
                AddDipole(mdp);
        }

        //! Receive energies     
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {
            MolecularEnergy me;
            
            cs = me.Receive(cr,src);
            if  (CS_RECV_DATA == cs)
                AddEnergy(me);
        }
    }
    return cs;
}

CommunicationStatus Experiment::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    MolecularQuadrupoleIterator qi;
    MolecularDipPolarIterator dpi;
    MolecularEnergyIterator mei;
    
    cs = gmx_send_data(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        //! Send literature reference
        gmx_send_str(cr,dest,_reference.c_str());
        
        //! Send conformation
        gmx_send_str(cr,dest,_conformation.c_str());
    
        //! Send polarizabilities
        for(dpi=BeginPolar(); (CS_SEND_DATA == cs) && (dpi<EndPolar()); dpi++)
        {
            cs = dpi->Send(cr,dest);
        }
        gmx_send_done(cr,dest);
        
        //! Send dipoles
        for(dpi=BeginDipole(); (CS_SEND_DATA == cs) && (dpi<EndDipole()); dpi++)
        {
            cs = dpi->Send(cr,dest);
        }
        gmx_send_done(cr,dest);
        
        //! Send energies
        for(mei=BeginEnergy(); (CS_SEND_DATA == cs) && (mei<EndEnergy()); mei++)
        {    
            cs = mei->Send(cr,dest);
        }
        gmx_send_done(cr,dest);
    }
    return cs;
}

CommunicationStatus ElectrostaticPotential::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    
    cs = gmx_recv_data(cr,src);
    
    if (CS_RECV_DATA == cs)
    {
        _xyz_unit.assign(gmx_recv_str(cr,src));
        _V_unit.assign(gmx_recv_str(cr,src));
        _espid = gmx_recv_int(cr,src);
        _x = gmx_recv_double(cr,src);
        _y = gmx_recv_double(cr,src);
        _z = gmx_recv_double(cr,src);
        _V = gmx_recv_double(cr,src);
    }
    return cs;
}

CommunicationStatus ElectrostaticPotential::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    
    cs = gmx_send_data(cr,dest);
    
    if (CS_SEND_DATA == cs)
    {
        gmx_send_str(cr,dest,_xyz_unit.c_str());
        gmx_send_str(cr,dest,_V_unit.c_str());
        gmx_send_int(cr,dest,_espid);
        gmx_send_double(cr,dest,_x);
        gmx_send_double(cr,dest,_y);
        gmx_send_double(cr,dest,_z);
        gmx_send_double(cr,dest,_V);
    }
    return cs;
}

CommunicationStatus AtomicCharge::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    
    cs = GenericProperty::Receive(cr,src);
    
    if (CS_RECV_DATA == cs)
    {
        cs = gmx_recv_data(cr,src);
    }
    if (CS_RECV_DATA == cs)
    {
        _q = gmx_recv_double(cr,src);
    }
    return cs;
}

CommunicationStatus AtomicCharge::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    
    cs = GenericProperty::Send(cr,dest);
    
    if (CS_SEND_DATA == cs)
    {
        cs = gmx_send_data(cr,dest);
    }
    if (CS_SEND_DATA == cs)
    {
        gmx_send_double(cr,dest,_q);
    }
    return cs;
}

CommunicationStatus CalcAtom::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    
    cs = gmx_recv_data(cr,src);
    if (CS_RECV_DATA == cs)
    {
        _name.assign(gmx_recv_str(cr,src));
        _obtype.assign(gmx_recv_str(cr,src));
        _atomid = gmx_recv_int(cr,src);
        _unit.assign(gmx_recv_str(cr,src));
        _x = gmx_recv_double(cr,src);
        _y = gmx_recv_double(cr,src);
        _z = gmx_recv_double(cr,src);
        
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {
            AtomicCharge aq;
            
            cs = aq.Receive(cr,src);
            if (CS_RECV_DATA == cs)
                AddCharge(aq);
        }
    }
    return cs;
}

CommunicationStatus CalcAtom::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    AtomicChargeIterator qi;
    
    cs = gmx_send_data(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        gmx_send_str(cr,dest,_name.c_str());
        gmx_send_str(cr,dest,_obtype.c_str());
        gmx_send_int(cr,dest,_atomid);
        gmx_send_str(cr,dest,_unit.c_str());
        gmx_send_double(cr,dest,_x);
        gmx_send_double(cr,dest,_y);
        gmx_send_double(cr,dest,_z);
        
        for(qi=BeginQ(); (CS_SEND_DATA == cs) && (qi<EndQ()); qi++)
        {
            cs = qi->Send(cr,dest);
        }
        gmx_send_done(cr,dest);
    }
    return cs;
}

CommunicationStatus Calculation::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    ElectrostaticPotentialIterator epi;
    CalcAtomIterator cai;
    
    cs = Experiment::Receive(cr,src);
    
    if (CS_RECV_DATA == cs)
    {
        cs = gmx_recv_data(cr,src);
    }
    if (CS_RECV_DATA == cs)
    {
        _program.assign(gmx_recv_str(cr,src));
        _method.assign(gmx_recv_str(cr,src));
        _basisset.assign(gmx_recv_str(cr,src));
        _datafile.assign(gmx_recv_str(cr,src));

        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {
            ElectrostaticPotential ep;
            
            cs = ep.Receive(cr,src);
            if (CS_RECV_DATA == cs)
                AddPotential(ep);
        }
        
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {
            CalcAtom ca;
            
            cs = ca.Receive(cr,src);
            if (CS_RECV_DATA == cs)
                AddAtom(ca);
        }
    }
    return cs;
}

CommunicationStatus Calculation::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    ElectrostaticPotentialIterator epi;
    CalcAtomIterator cai;
    
    cs = Experiment::Send(cr,dest);
    
    if (CS_SEND_DATA == cs)
    {
        cs = gmx_send_data(cr,dest);
    }
    if (CS_SEND_DATA == cs)
    {
        gmx_send_str(cr,dest,_program.c_str());
        gmx_send_str(cr,dest,_method.c_str());
        gmx_send_str(cr,dest,_basisset.c_str());
        gmx_send_str(cr,dest,_datafile.c_str());

        for(epi=BeginPotential(); (CS_SEND_DATA == cs) && (epi<EndPotential()); epi++)
        {
            cs = epi->Send(cr,dest);
        }
        gmx_send_done(cr,dest);
        
        for(cai=BeginAtom(); (CS_SEND_DATA == cs) && (cai<EndAtom()); cai++)
        {
            cs = cai->Send(cr,dest);
        }
        gmx_send_done(cr,dest);
    }
    return cs;
}

CommunicationStatus AtomNum::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    
    cs = gmx_send_data(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        gmx_send_str(cr,dest,_catom.c_str());
        gmx_send_int(cr,dest,_cnumber);
    }
    return cs;
}

CommunicationStatus AtomNum::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    
    cs = gmx_recv_data(cr,src);
    if (CS_RECV_DATA == cs)
    {
        _catom.assign(gmx_recv_str(cr,src));
        _cnumber = gmx_recv_int(cr,src);
    }
    return cs;
}

CommunicationStatus MolecularComposition::Send(t_commrec *cr,int dest)
{
    AtomNumIterator ani;
    CommunicationStatus cs;
    
    cs = gmx_send_data(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        gmx_send_str(cr,dest,_compname.c_str());
        for(ani=BeginAtomNum(); (CS_SEND_DATA == cs) && (ani<EndAtomNum()); ani++)
        {
            cs = ani->Send(cr,dest);
        }
        gmx_send_done(cr,dest);
    }
    return cs;
}

CommunicationStatus MolecularComposition::Receive(t_commrec *cr,int src)
{
    CommunicationStatus cs;
    
    cs = gmx_recv_data(cr,src);
    if (CS_RECV_DATA == cs)
    {
        _compname.assign(gmx_recv_str(cr,src));
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {
            AtomNum an;
            
            cs = an.Receive(cr,src);
            if (CS_RECV_DATA == cs)
            {
                AddAtom(an);
            }
        }
    }
    return cs;
}

CommunicationStatus MolProp::Send(t_commrec *cr,int dest)
{
    CommunicationStatus cs;
    BondIterator bi;
    MolecularCompositionIterator mci;
    std::vector<std::string>::iterator si;
    ExperimentIterator ei;
    CalculationIterator ci;
    
    /* Generic stuff */
    cs = gmx_send_data(cr,dest);
    if (CS_SEND_DATA == cs)
    {
        gmx_send_double(cr,dest,_mass);
        gmx_send_int(cr,dest,_charge);
        gmx_send_int(cr,dest,_multiplicity);
        gmx_send_str(cr,dest,_formula.c_str());
        gmx_send_str(cr,dest,_molname.c_str());
        gmx_send_str(cr,dest,_iupac.c_str());
        gmx_send_str(cr,dest,_cas.c_str());
        gmx_send_str(cr,dest,_cid.c_str());
        gmx_send_str(cr,dest,_inchi.c_str());
        
        /* Bonds */
        for(bi=BeginBond(); (CS_SEND_DATA == cs) && (bi<EndBond()); bi++)
        {
            cs = bi->Send(cr,dest);
        }
        gmx_send_done(cr,dest);

        /* Composition */
        for(mci=BeginMolecularComposition(); (CS_SEND_DATA == cs) && (mci<EndMolecularComposition()); mci++)
        {
            mci->Send(cr,dest);
        }
        gmx_send_done(cr,dest);

        /* Categories */
        for(si=BeginCategory(); (CS_SEND_DATA == cs) && (si<EndCategory()); si++)
        {
            cs = gmx_send_data(cr,dest);
            if (CS_SEND_DATA == cs)
                gmx_send_str(cr,dest,si->c_str());
        }
        gmx_send_done(cr,dest);

        /* Experiments */
        for(ei=BeginExperiment(); (CS_SEND_DATA == cs) && (ei<EndExperiment()); ei++)
        {
            cs = ei->Send(cr,dest);
        }
        gmx_send_done(cr,dest);

        /* Calculations */
        for(ci=BeginCalculation(); (CS_SEND_DATA == cs) && (ci<EndCalculation()); ci++)
        {
            ci->Send(cr,dest);
        }
        gmx_send_done(cr,dest);
    }
    return cs;   
}

CommunicationStatus MolProp::Receive(t_commrec *cr,int src)
{
    char   *ptr;
    CommunicationStatus cs;
    
    /* Generic stuff */
    cs = gmx_recv_data(cr,src);
    if (CS_RECV_DATA == cs)
    {
        //! Receive mass and more
        _mass = gmx_recv_double(cr,src);
        _charge = gmx_recv_int(cr,src);
        _multiplicity = gmx_recv_int(cr,src);
        _formula.assign(gmx_recv_str(cr,src));
        _molname.assign(gmx_recv_str(cr,src));
        _iupac.assign(gmx_recv_str(cr,src));
        _cas.assign(gmx_recv_str(cr,src));
        _cid.assign(gmx_recv_str(cr,src));
        _inchi.assign(gmx_recv_str(cr,src));
    
        //! Receive Bonds
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {   
            Bond b;
            
            cs = b.Receive(cr,src);
            if (CS_RECV_DATA == cs)
                AddBond(b);
        }
    
        //! Receive Compositions
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {   
            MolecularComposition mc;
            
            cs = mc.Receive(cr,src);
            if (CS_RECV_DATA == cs)
                AddComposition(mc);
        }
        
        //! Receive Categories
        while (NULL != (ptr = gmx_recv_str(cr,src)))
        {
            AddCategory(ptr);
            sfree(ptr);
        }

        //! Receive Experiments
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {   
            Experiment ex;
            
            cs = ex.Receive(cr,src);
            if (CS_RECV_DATA == cs)
                AddExperiment(ex);
        }

        //! Receive Calculations
        cs = gmx_recv_data(cr,src);
        while (CS_RECV_DATA == cs)
        {   
            Calculation cc;
            
            cs = cc.Receive(cr,src);
            if (CS_RECV_DATA == cs)
                AddCalculation(cc);
        }
    }
    if (NULL != debug) 
        fprintf(debug,"Reveived %d calculations from %d for mol %s\n",
                NCalculation(),src,GetMolname().c_str());
    return cs;
}

}


