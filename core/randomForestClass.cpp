#include "ftree.h"
//#include "estimator.h"
//#include "utils.h"
#include "rndforest.h"
//#include "rfUtil.h"
#include "options.h"
#include <stdio.h>
#include "frontend.h"

#include "myclasses.ppp"

#include "errors.hpp" // for raiseError

extern Options *opt ;
extern featureTree *gFT ;

TRandomForestLearner::TRandomForestLearner(const int &seed)
: randomSeed(seed),
  noTrees(100),
  predictClass(FALSE),
  sampleProp(0.0),
  noSelAttr(0),
  kNearestEqual(30)
{}

/*RandomForestLearner::RandomForestLearner(int rfNoTrees){
	this->rfNoTrees=rfNoTrees;
}
*/
PClassifier TRandomForestLearner::operator()(PExampleGenerator egen, const int &weightID)
{
  PRandomGenerator randGen = new TRandomGenerator(randomSeed);
	gFT=NULL;
	opt=NULL;
	Forest = new featureTree();
	options = new Options();
	gFT = Forest;
	// set attributes into opt
	options->rfNoTrees = noTrees;
	options->rfPredictClass = predictClass ? TRUE : FALSE ;
	options->rfSampleProp = sampleProp;
	options->rfNoSelAttr = noSelAttr;
	options->rfkNearestEqual = kNearestEqual;

	opt = options;
	//opt->readConfig("credita.par") ;
	TExampleTable &table = dynamic_cast<TExampleTable &>(toExampleTable(egen).getReference());
	gFT->readDescription(table);
	gFT->readData(table);
	gFT->learnRF = TRUE ;
	gFT->buildForest();
    
	TRandomForest* ranForest = new TRandomForest(egen->domain->classVar, randGen);
	ranForest->options = opt;
	ranForest->Forest = gFT;

	gFT=NULL;
	opt=NULL;
  
  return ranForest;
}


TRandomForest::TRandomForest(PVariable classVar, PRandomGenerator rgen)
: TClassifier(classVar),
  randomGenerator(rgen)
{
	if (classVar->varType != TValue::INTVAR)
    raiseError("MyClassifier cannot work with a non-discrete attribute '%s'", classVar->name.c_str());
  computesProbabilities = true;
}



PDistribution TRandomForest::classDistribution(const TExample &origexam)
{ 
	gFT=Forest;
	opt = options;
	//gFT->clearData();

	int contJ = 0, discJ = 1;
	int i = gFT->DTeach.len();

    for(TExample::const_iterator eei(origexam.begin()), eee(origexam.end()-1); eei != eee; eei++)
      if ((*eei).varType == TValue::FLOATVAR)
		  gFT->ContData.Set(i, contJ++, (*eei).isSpecial() ? NAcont : (*eei).floatV);
      else
        gFT->DiscData.Set(i, discJ++, (*eei).isSpecial() ? NAdisc : (*eei).intV+1);
	//TExample exam = TExample(domain, origexam);
	//gFT->t
	/*
	marray<double> probDist(gFT->NoClasses + 1) ;
	probDist.init(0.0) ;
	//gFT->DTest[1]
	if (opt->rfkNearestEqual>0)
	  gFT->rfNearCheck(gFT->DTest[0], probDist) ;       
	else if (gFT->NoClasses==2 && opt->rfRegType==1)
	  gFT->rfCheckReg(gFT->DTest[0], probDist) ;
	else 
	  gFT->rfCheck(gFT->DTest[0], probDist) ;
	*/
	marray<double> probDist = gFT->getDistribution(gFT->DTest);
	int arrSize=probDist.len();
	double *probDistArr = probDist.toArray();
	//probDist.unWrap(arrSize);
	//float distr[arrSize];

	float *distr=(float*)malloc(arrSize);
	for (i=0;i<arrSize-1;i++){
		distr[i]=(float)probDistArr[i+1];
	}
	TDiscDistribution *distribucija= new TDiscDistribution(distr,arrSize);

	return distribucija;
}


