// -------------------------------------------------------------------------
// -----                     FairModule source file                    -----
// -----            Created 06/01/04  by M. Al-Turany                  -----
// -------------------------------------------------------------------------
/* Generated by Together */

#include "FairModule.h"

#include "FairVolume.h"
#include "FairVolumeList.h"
#include "FairBaseParSet.h"
#include "FairRun.h"

#include "FairGeoNode.h"
#include "FairRuntimeDb.h"

#include "FairGeoInterface.h"
#include "FairGeoLoader.h"
#include "FairGeoNode.h"
#include "FairGeoRootBuilder.h"
#include "FairGeoMedia.h"


#include "TString.h"
#include "TObjArray.h"
#include "TGeoVolume.h"
#include "TFile.h"
#include "TList.h"
#include "TKey.h"
#include "TGeoManager.h"
#include "TGeoVoxelFinder.h"
#include "TGeoMatrix.h"

#include "TSystem.h"
#include <fstream>
#include <iostream>
using std::cout;              
using std::endl;

TArrayI* FairModule::volNumber=0;
Int_t FairModule::fNbOfVolumes=0;
FairVolumeList*  FairModule::vList=0;
TRefArray*  	FairModule::svList=0;
//__________________________________________________________________________
void FairModule::ConstructGeometry()
{
}
//__________________________________________________________________________
void FairModule::ConstructOpGeometry()
{
}
//__________________________________________________________________________
FairModule::~FairModule()
{

}
//__________________________________________________________________________
FairModule::FairModule(const char * Name, const char *title ,Bool_t Active)
	:TNamed(Name, title),
	 fgeoVer("Not defined"),
     fgeoName("Not defined"),    
	 fModId(-1),
     fActive(Active), 
     fNbOfSensitiveVol(0),
     fVerboseLevel(0),
     flGeoPar(0),
     kGeoSaved(kFALSE)
    
{
    svList=new TRefArray();
    vList=new FairVolumeList();
    
}

//__________________________________________________________________________

FairModule::FairModule()
 : fgeoVer("Not defined"),
   fgeoName("Not defined"),    
   fModId(-1),
   fActive(kFALSE), 
   fNbOfSensitiveVol(0),
   fVerboseLevel(0),
   flGeoPar(0),
   kGeoSaved(kFALSE)

{

}
void FairModule::Streamer(TBuffer& b)
{
  TNamed::Streamer(b);
  if (b.IsReading()) {
    fgeoVer.Streamer(b);
    fgeoName.Streamer(b);
    b >> fActive; 
    b >> fModId;
  }else{
    fgeoVer.Streamer(b);
    fgeoName.Streamer(b);
    b << fActive;
    b << fModId;
   }
  
}

void FairModule::SetGeometryFileName(TString fname, TString geoVer)
{
   fgeoVer=geoVer; 
   TString FileName = fname;
   TString work = getenv("VMCWORKDIR");
   TString userwork = getenv("GEOMPATH");	
   if(userwork != ""){
      fgeoName=userwork;
      if (!fgeoName.EndsWith("/")) fgeoName+="/";
      //fgeoName+=fname;
      if (TString(gSystem->FindFile(fgeoName.Data(),fname)) != TString("")){
	  fgeoName=fname;  
	  cout << "------------------------------------------------------------------"<< endl;  
          cout << "---User path for detector geometry : " << fgeoName.Data() << endl;
      }else{
         cout << "---Detector geometry was not found in user path : " << FileName.Data() << endl;     
         fgeoName=work+"/geometry/";
	 cout << "---Try the standard path : " << fgeoName.Data() << endl;  
         if (TString(gSystem->FindFile(fgeoName.Data(),FileName)) != TString("")){
             fgeoName=FileName;  
             cout << "---Reading detector geometry from : "<<  FileName.Data() << endl;
	 }else{
	     Fatal("FAIRModule::SetGeometryFileName", "Detector geometry not found."); 
	 }
	  cout << "------------------------------------------------------------------"<< endl; 
      }
   }else{
      fgeoName=work+"/geometry/";
      fgeoName+=fname;
   }	
}

void FairModule::ProcessNodes(TList *aList){

  TListIter iter(aList);
  FairGeoNode* node   = NULL;
  FairGeoNode* MotherNode =NULL;
  FairVolume*  volume = NULL;
  FairRuntimeDb *rtdb= FairRun::Instance()->GetRuntimeDb();
  FairBaseParSet* par=(FairBaseParSet*)(rtdb->getContainer("FairBaseParSet"));
  TObjArray *fNodes = par->GetGeoNodes();
  while( (node = (FairGeoNode*)iter.Next()) ) {

    node->calcLabTransform();
 //      cout << "-I ProcessNodes() Node: " << node->GetName() << " copyNo: " << node->getCopyNo()
 //       << " LabTransform: " << node->getLabTransform() <<  endl;
 //      node->Dump();
 //   GetListOfGeoPar()->Add( node );
    MotherNode=node->getMotherNode();
    volume = new FairVolume( node->getTruncName(), fNbOfVolumes++);
    volume->setRealName(node->GetName());
    vList->addVolume(volume);
    volume->setGeoNode(node);
    volume->setCopyNo(  node->getCopyNo());

    if(MotherNode!=0){
 	volume->setMotherId(node->getMCid());
	volume->setMotherCopyNo(MotherNode->getCopyNo());
    }
    FairGeoVolume *aVol=NULL;

    if ( node->isSensitive() && fActive ) {
      volume->setModId(fModId);
      volume->SetModule(this);
      svList->Add(volume);
      aVol = dynamic_cast<FairGeoVolume*> ( node );
      fNodes->AddLast( aVol );
      fNbOfSensitiveVol++;
    }

  }
/*  cout << " FairModule::ProcessNodes "<< endl;
	 svList->ls();
  cout << " FairModule::ProcessNodes "<< endl;
*/
}

void	FairModule::AddSensitiveVolume(TGeoVolume *v){

//	cout <<"FairModule::AddSensitiveVolume  " << v->GetName() << endl;
  	FairVolume*  volume = NULL;
  	volume = new FairVolume(v->GetName(), fNbOfVolumes++);
  	vList->addVolume(volume);
  	volume->setModId(fModId);
  	volume->SetModule(this);
  	svList->Add(volume);
  	fNbOfSensitiveVol++;

}



FairVolume* FairModule::getFairVolume(FairGeoNode *fN)
{
	FairVolume *fv;
	FairVolume  *fvol=0;
//	cout << " FairModule::getFairVolume " << fN <<  endl;
	for(Int_t i=0; i<vList->getEntries();i++){
		fv=vList->At(i);
//		cout << "" << fv->getGeoNode() << " " << fN <<endl;
		if((fv->getGeoNode())==fN){
//			cout << "Inside " << fv->getGeoNode() << "  " << fN <<endl;
			fvol=fv;
			return fvol;
			break;
		}
    	}
	return fvol;
}

void FairModule::ConstructRootGeometry(){
   
   TGeoManager *OldGeo=gGeoManager;
   TGeoManager *NewGeo=0;
   TGeoVolume *volume=0;;
   TFile *f=new TFile(GetGeometryFileName().Data());
   TList *l= f->GetListOfKeys();
   TKey *key; 
   TIter next( l);
   TGeoNode *n=0;        
   TGeoVolume *v1=0;
   while ((key = (TKey*)next())) {
   if (strcmp(key->GetClassName(),"TGeoManager") != 0) continue;
       gGeoManager=0;
       NewGeo = (TGeoManager*)key->ReadObj();
       break;
   }
   if (NewGeo!=0){
	   NewGeo->cd();
	   volume=(TGeoVolume*)NewGeo->GetNode(0)->GetDaughter(0)->GetVolume();
	   v1=volume->MakeCopyVolume(volume->GetShape());
	   //n=NewGeo->GetTopNode();
	    n=v1->GetNode(0);
	 //  NewGeo=0;
	   
	   delete NewGeo;
	   
   }else{
	   key=(TKey *) l->At(0);
	   volume=(TGeoVolume *)key->ReadObj();
	   n=volume->GetNode(0);        
	   v1=n->GetVolume();
   }

   if(v1==0) {
	   cout << "\033[5m\033[31mFairModule::ConstructRootGeometry(): could not find any geometry in File!!  \033[0m" << GetGeometryFileName().Data() <<  endl; 
	   exit(0);
   }
   gGeoManager=OldGeo;
   gGeoManager->cd();
   TGeoVolume *Cave= gGeoManager->GetTopVolume();
   gGeoManager->AddVolume(v1);
   TGeoVoxelFinder *voxels = v1->GetVoxels();
   if (voxels) voxels->SetNeedRebuild();
   TGeoMatrix *M = n->GetMatrix();
   M->SetDefaultName();
   gGeoManager->GetListOfMatrices()->Remove(M);
   TGeoHMatrix *global = gGeoManager->GetHMatrix();             
   gGeoManager->GetListOfMatrices()->Remove(global); //Remove the Identity matrix 
   Cave->AddNode(v1,0, M);
   ExpandNode(n);
 //  delete NewGeo;
   delete f;
}

void FairModule::ConstructASCIIGeometry(){
	cout << " FairModule::ConstructASCIIGeometry() : this method has to be implimented in detector class " << endl;
}
Bool_t FairModule::CheckIfSensitive(std::string name){
		
	cout << "\033[5m\033[31m FairModule::CheckIfSensitive(std::string name): this method has to be implimented in detector class  \033[0m\n" << endl;
	return kFALSE;
}
void FairModule::ExpandNode(TGeoNode *fN){
	
   FairGeoLoader*geoLoad = FairGeoLoader::Instance();
   FairGeoInterface *geoFace = geoLoad->getGeoInterface();
   FairGeoMedia *Media =  geoFace->getMedia();
   FairGeoBuilder *geobuild=geoLoad->getGeoBuilder();
   TGeoMatrix *Matrix =fN->GetMatrix();
   if(gGeoManager->GetListOfMatrices()->FindObject(Matrix))gGeoManager->GetListOfMatrices()->Remove(Matrix);
   TGeoVolume *v1=fN->GetVolume();
   TObjArray *NodeList=v1->GetNodes();
   for (Int_t Nod=0; Nod<NodeList->GetEntriesFast();Nod++) {   
      TGeoNode *fNode =(TGeoNode *)NodeList->At(Nod);
      TGeoMatrix *M =fNode->GetMatrix();
      M->SetDefaultName();
      if(fNode->GetNdaughters()>0) ExpandNode(fNode);
      TGeoVolume *v= fNode->GetVolume();
//      Int_t MatId=0;
      TGeoMedium* med1=v->GetMedium();
      if(med1){
         TGeoMaterial*mat1=v->GetMaterial(); 
         TGeoMaterial *newMat = gGeoManager->GetMaterial(mat1->GetName());
         if( newMat==0){
            //std::cout<< "Material " << mat1->GetName() << " is not defined " << std::endl;
            FairGeoMedium *FairMedium=Media->getMedium(mat1->GetName());
            if (!FairMedium) {
               std::cout << "\033[5m\033[31m Material is not defined in ASCII file nor in Root file  \033[0m\n" << std::endl;
               FairMedium=new FairGeoMedium(mat1->GetName());
               Media->addMedium(FairMedium);
            }
            //std::cout << "Create Medium " << mat1->GetName() << std::endl;
            Int_t nmed=geobuild->createMedium(FairMedium);
            v->SetMedium(gGeoManager->GetMedium(nmed));
            gGeoManager->SetAllIndex();
        }else{
           TGeoMedium *med2= gGeoManager->GetMedium(mat1->GetName());
           v->SetMedium(med2);
        }
     }else{ 
    	 if (strcmp(v->ClassName(),"TGeoVolumeAssembly") != 0) {;  	 
    	 //[R.K.-3.3.08]  // When there is NO material defined, set it to avoid conflicts in Geant
        std::cout<<" -E- Error in FairModule::ExpandNode()!  "
                 <<"\tThe volume "<<v->GetName()<<" Has no medium information."<<std::endl;
        abort();
    	 }
     }
     if (!gGeoManager->FindVolumeFast(v->GetName())) {
         if (fVerboseLevel>2)std::cout << "Register Volume : " << v->GetName() << " id "  << std::endl;
         v->RegisterYourself();
     }
     if (CheckIfSensitive(v->GetName())){
        if (fVerboseLevel>2){
              std::cout << "Sensitive Volume : " << v->GetName() << " id "  << std::endl;
        }
        AddSensitiveVolume(v);
     } 
   }
}

//__________________________________________________________________________
ClassImp(FairModule)



