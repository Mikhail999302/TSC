#include <vector>
#include <fstream>
#include <stdio.h>
#include <ctime>
#include <string>
#include <fstream>

#include <boost\shared_ptr.hpp>

#include <Grouping\Entry.h>
#include "..\TSCPLevel\plevel.h"
#include "..\TSCCalc\Model.h"
#include "..\TSCCalc\TSCModel.h"
#include "..\TSCCalc\TSCTrunkModel.h"
#include "..\TSCCalc\GroupFunctions.h"
#include "..\TSCCalc\TableManagerFactory.h"
#include "..\Dumper\Dump.h"
#include "..\TSCGenerator\Generator.h"
#include "..\TSCGenerator\WideGenerator.h"
#include "..\Distributions\SampleHolder.h"
#include "..\Distributions\BoundedSample.h"
#include "..\Distributions\MomentsGroupping.h"
#include "..\Distributions\SampleHolder.h"
#include "..\Distributions\BoundedSample.h"
#include "ModelTypes.h"

using namespace std;
using namespace Distributions;
using namespace Dumper;
using namespace TSCGenerator;
using namespace GroupedTSC;
typedef boost::shared_ptr<Distributions::CBoundedSample> PBoundedSample;
void TestMoments();
void initDumper();
void TestEcalc();
//void TestPLevel();
void TestInfoMatrix();
void TestWideGenerator();
void setDistribution(TSCGenerator::doubles& _distrib);
void setWideDistribution(TSCGenerator::doubles& pbefore_,double& pch_,TSCGenerator::doubles& pafter_);
void setSimleEstimation(GroupedTSC::CModel * _model,
						vector<PBoundedSample>& _sampleEstimates);
void setEstimation(GroupedTSC::CModel*_model, 
				   vector<PBoundedSample>& _sampleEstimates,
				   Distributions::CBoundedSample* _samplePLevel);
void printEstimateResults(const vector<double>& _distrib,
						  const vector<PBoundedSample>& _sampleEstimates,
						  const string &_grouppingName, 
						  ofstream& _out);
void printPLevelResults(const Distributions::CBoundedSample* _samplePLevel,
						const string &_grouppingName, 
						ofstream& _out);
vector<double> getGroupedDistrib(const vector<double>& _distrib,const vector<size_t>&tempGroup);

void printEstimates(vector<double> &distrib, 
					ofstream& out, 
					vector<PBoundedSample >& initEstimates, 
					vector<PBoundedSample >& noneEstimates, 
					vector<PBoundedSample >& standardEstimates, 
					vector<PBoundedSample >& shrinkInitialEstimates, 
					vector<PBoundedSample >& shrinkEstimates );