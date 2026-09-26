#include <iostream>
#include "..\TSCPlevel\Typedefs.h"
#include "..\ECalc\DiscreteModel.h"
#include "..\ECalc\DataProcessing.h"
using namespace MathModels;
using namespace std;
void TestEcalc()
{
	ModelPtr model=CreateModel(CBaseDiscreteModel::mt2D);
	ModelPtr altModel=CreateModel(CBaseDiscreteModel::mt2D);
	CExpressions states;
	CExpressions altStates;
	CMathParameterInfos paramInfos;
	CMathParameterEstimates estimates;
	DataProcessing::ReadEstimates("a=10;\nb=5;",estimates);
	DataProcessing::ReadExpressions("p[0][0]=a;",states,"p");
	DataProcessing::ReadExpressions("p[0][0]=b;",altStates,"p");
	paramInfos.Construct(estimates);
	//estimates[0].setValue(10);
	//model->Init(states,paramInfos,DataProcessing::CMathOperationIndicator(),false);
	//altModel->Init(altStates,paramInfos,DataProcessing::CMathOperationIndicator(),false);
	CMathParameterValues results;
	//model->CalcSimpleEstimates(PLevel::doubles(1,0), 0, estimates,results);
	results.Construct(estimates);
	results.SetContextValues();
	for(int i=0; i<model->GetStatesCount();++i)
	{
		cout<<model->GetState(i).Eval()<<" ";
		cout<<altModel->GetState(i).Eval()<<endl;
	}
	//cout<<model->GetState(0).Eval();
	exit(0);
}