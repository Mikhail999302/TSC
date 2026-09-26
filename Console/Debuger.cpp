// Debuger.cpp : Defines the entry point for the console application.
//


//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
#include <iomanip>
#include "HelperFunctions.h"

double avr(doubles& v) {
	double s = 0;
	for (int i = 0; i < v.size(); ++i)
		s += v[i];
	return s / v.size();
}

double disp(doubles& v) {
	double s = 0, average = avr(v);
	for (int i = 0; i < v.size(); ++i)
		s += (v[i] - average) * (v[i] - average);
	return s / v.size();
}

int main()
{
	cin.tie(0);
	cout.tie(0);
	ios_base::sync_with_stdio(0);
	
	/////////////
	doubles pch_simple1, padv_simple1, pch_one1, padv_one1;
	doubles pch_simple, padv_simple, pch_one, padv_one;
	for (int ind = 0; ind < 100; ++ind) {
		TSCGenerator::doubles distrib;
		setDistribution(distrib); //Здесь задаются значения параметров в виде вектора. 
		//Интерпретация параметров - в функции моделирования, а именно 
		int fixNumofInd = 100; //Число респондентов
		long startTime1 = std::clock();
		//long lastStamp=startTime;
		int numOfInd;


		auto_ptr<TSCGenerator::CGenerator_1Pos_mine > generator1(new TSCGenerator::CGenerator_1Pos_mine(distrib, std::clock()));
		generator1->SetSize(fixNumofInd);

		generator1->Generate();
		//edited 10.02
		//cout<<"Generated sample: \n" << generator.get()->GetSample()<<endl;

		doubles cur_sample1 = generator1.get()->GetSample();
		/*cout << "Generated sample: \n" << cur_sample1.size() << endl;
		for (int i = 0; i < cur_sample1.size(); ++i) {
			cout << cur_sample1[i] << " ";
			if ((i + 1) % (int)sqrt(cur_sample1.size()) == 0)
				cout << endl;
		}

		cout << endl;

		cout << "Data: \n";
		for (int i = 0; i < distrib.size() - 2; ++i) {
			cout << "p" << i << " " << distrib[i] << endl;
		}
		cout << "pch " << distrib[distrib.size() - 2] << endl;
		cout << "padv " << distrib[distrib.size() - 1] << endl;*/
		//

		long lastStamp1 = startTime1;
		initDumper();




		MathModels::TLDotCacher ldotCacher1;
		MathModels::TInfoCacher infoCacher1;


		Tsc::TableManager::CTableManager<double>* sampleManager1 =
			Tsc::TableManager::TSampleManagerFactory::CreateFromGenerator(generator1.get());
		try
		{
			auto_ptr<GroupedTSC::CModel> model_1Pos1(GroupedTSC::CModel::CreateCModel(sampleManager1->GetWidth(),
				sampleManager1,
				false,
				new GroupedTSC::CTSCModel_1Pos_mine(sampleManager1->GetWidth(), ETSCModel, ldotCacher1, infoCacher1)//*/GroupedTSC::CTSCTrunkModel()  //sampleManager_in_brand
			));

			model_1Pos1->ProcessInitialCalculation(true);

			/*cout << "\n*******\n 1Positive model IN estimation \n*******\n\n";
			cout << "Simple estimates: \n";
			cout << "estim: " << model_1Pos1->GetEstimateCount() << endl;
			for (int i = 0; i < model_1Pos1->GetEstimateCount(); ++i) {
				std::cout << model_1Pos1->GetEstimateAt(i).first << " " << model_1Pos1->GetEstimateAt(i).second.Value << std::endl;
			}*/

			pch_simple1.push_back(model_1Pos1->GetEstimateAt(2).second.Value);
			padv_simple1.push_back(model_1Pos1->GetEstimateAt(3).second.Value);

			//cout << "\n \nOne-step estimates:\n";
			model_1Pos1->ProcessEstimatesCalculation();
			/*for (int i = 0; i < model_1Pos1->GetEstimateCount(); ++i) {
				std::cout << model_1Pos1->GetEstimateAt(i).first << " " << model_1Pos1->GetEstimateAt(i).second.Value << std::endl;
			}*/
			pch_one1.push_back(model_1Pos1->GetEstimateAt(2).second.Value);
			padv_one1.push_back(model_1Pos1->GetEstimateAt(3).second.Value);
		}
		catch (exception& e)
		{
			cout << "exception!" << e.what() << endl;
		}

	//for (int ind = 0; ind < 1; ++ind) {
		long startTime = std::clock();
		//long lastStamp = startTime;
		//TSCGenerator::doubles distrib;
		//setDistribution(distrib); //Здесь задаются значения параметров в виде вектора. 
		////Интерпретация параметров - в функции моделирования, а именно 
		//int fixNumofInd = 2000; //Число респондентов
		//long startTime = std::clock();
		////long lastStamp=startTime;
		//int numOfInd;

		auto_ptr<TSCGenerator::CGenerator_1Pos > generator(new TSCGenerator::CGenerator_1Pos(distrib, std::clock()));
		generator->SetSize(fixNumofInd);
		//******************* main problem - how to do same sample
		generator->Generate();
		//edited 10.02
		//cout<<"Generated sample: \n" << generator.get()->GetSample()<<endl;

		//doubles cur_sample = generator.get()->GetSample();
		//cout << "Generated sample: \n" << cur_sample.size() << endl;
		//for (int i = 0; i < cur_sample.size(); ++i) {
		//	cout << cur_sample[i] << " ";
		//	if ((i + 1) % (int)sqrt(cur_sample.size()) == 0)
		//		cout << endl;
		//}
		////for file
		//ofstream out("outsample1.txt");
		//for (int i = 0; i < cur_sample.size(); ++i) {
		//	out << cur_sample[i] << " ";
		//	if ((i + 1) % (int)sqrt(cur_sample.size()) == 0)
		//		out << endl;
		//}
		//out.close();
		//
		//cout << endl;

		/*cout << "Data: \n";
		for (int i = 0; i < distrib.size() - 2; ++i) {
			cout << "p" << i << " " << distrib[i] << endl;
		}
		cout << "pch " << distrib[distrib.size() - 2] << endl;
		cout << "padv " << distrib[distrib.size() - 1] << endl;*/
		//
		//auto_ptr<TSCGenerator::CGenerator_1Pos > generator(new TSCGenerator::CGenerator_1Pos(distrib, std::clock()));
		//generator->SetSize(fixNumofInd);

		//generator->Generate();
		////edited 10.02
		////cout<<"Generated sample: \n" << generator.get()->GetSample()<<endl;

		//doubles cur_sample = generator.get()->GetSample();
		//cout << "Generated sample: \n" << cur_sample.size() << endl;
		//for (int i = 0; i < cur_sample.size(); ++i) {
		//	cout << cur_sample[i] << " ";
		//	if ((i + 1) % (int)sqrt(cur_sample.size()) == 0)
		//		cout << endl;
		//}

		//cout << endl;

		//cout << "Data: \n";
		//for (int i = 0; i < distrib.size() - 2; ++i) {
		//	cout << "p" << i << " " << distrib[i] << endl;
		//}
		//cout << "pch " << distrib[distrib.size() - 2] << endl;
		//cout << "padv " << distrib[distrib.size() - 1] << endl;
		////

		long lastStamp = startTime;
		initDumper();




		MathModels::TLDotCacher ldotCacher;
		MathModels::TInfoCacher infoCacher;


		Tsc::TableManager::CTableManager<double>* sampleManager =
			Tsc::TableManager::TSampleManagerFactory::CreateFromGenerator(generator.get());
		try
		{
			auto_ptr<GroupedTSC::CModel> model_1Pos(GroupedTSC::CModel::CreateCModel(sampleManager->GetWidth(),
				sampleManager,
				false,
				new GroupedTSC::CTSCModel_1Pos(sampleManager->GetWidth(), ETSCModel, ldotCacher, infoCacher)//*/GroupedTSC::CTSCTrunkModel()  //sampleManager_in_brand
			));

			model_1Pos->ProcessInitialCalculation(true);
			/*cout << "\n*******\n 1Positive model IN estimation \n*******\n\n";
			cout << "Simple estimates: \n";
			for (int i = 0; i < model_1Pos->GetEstimateCount(); ++i) {
				std::cout << model_1Pos->GetEstimateAt(i).first << " " << model_1Pos->GetEstimateAt(i).second.Value << std::endl;
			}*/

			pch_simple.push_back(model_1Pos->GetEstimateAt(2).second.Value);
			padv_simple.push_back(model_1Pos->GetEstimateAt(3).second.Value);

			//cout << "\n \nOne-step estimates:\n";
			model_1Pos->ProcessEstimatesCalculation();
			/*for (int i = 0; i < model_1Pos->GetEstimateCount(); ++i) {
				std::cout << model_1Pos->GetEstimateAt(i).first << " " << model_1Pos->GetEstimateAt(i).second.Value << std::endl;
			}*/
			pch_one.push_back(model_1Pos->GetEstimateAt(2).second.Value);
			padv_one.push_back(model_1Pos->GetEstimateAt(3).second.Value);

		}
		catch (exception& e)
		{
			cout << "exception!" << e.what() << endl;
		}
		//}
	}

	cout << "for new model" << endl;
	cout << "pch_simple avr: " << avr(pch_simple1) << endl;
	cout << "padv_simple avr: " << avr(padv_simple1) << endl;
	cout << "pch_one avr: " << avr(pch_one1) << endl;
	cout << "padv_one avr: " << avr(padv_one1) << endl;
	cout << "pch_simple disp: " << disp(pch_simple1) << endl;
	cout << "padv_simple disp: " << disp(padv_simple1) << endl;
	cout << "pch_one disp: " << disp(pch_one1) << endl;
	cout << "padv_one disp: " << disp(padv_one1) << endl;
	cout << "for old model" << endl;
	cout << "pch_simple avr: " << avr(pch_simple) << endl;
	cout << "padv_simple avr: " << avr(padv_simple) << endl;
	cout << "pch_one avr: " << avr(pch_one) << endl;
	cout << "padv_one avr: " << avr(padv_one) << endl;
	cout << "pch_simple disp: " << disp(pch_simple) << endl;
	cout << "padv_simple disp: " << disp(padv_simple) << endl;
	cout << "pch_one disp: " << disp(pch_one) << endl;
	cout << "padv_one disp: " << disp(padv_one) << endl;

	return 0;
}

int mainx()
{

	TSCGenerator::doubles distrib;
	setDistribution(distrib); //Здесь задаются значения параметров в виде вектора. 
	//Интерпретация параметров - в функции моделирования, а именно 
	int fixNumofInd = 2000; //Число респондентов
	long startTime = std::clock();
	//long lastStamp=startTime;
	int numOfInd;


	auto_ptr<TSCGenerator::CGenerator_1Pos > generator(new TSCGenerator::CGenerator_1Pos(distrib, std::clock()));
	generator->SetSize(fixNumofInd);

	generator->Generate();
	cout << "Generated sample: \n" << generator.get()->GetSample() << endl;


	long lastStamp = startTime;
	initDumper();




	MathModels::TLDotCacher ldotCacher;
	MathModels::TInfoCacher infoCacher;


	Tsc::TableManager::CTableManager<double>* sampleManager =
		Tsc::TableManager::TSampleManagerFactory::CreateFromGenerator(generator.get());
	try
	{
		auto_ptr<GroupedTSC::CModel> model_1Pos(GroupedTSC::CModel::CreateCModel(sampleManager->GetWidth(),
			sampleManager,
			false,
			new GroupedTSC::CTSCModel_1Pos(sampleManager->GetWidth(), ETSCModel, ldotCacher, infoCacher)//*/GroupedTSC::CTSCTrunkModel()  //sampleManager_in_brand
		));

		model_1Pos->ProcessInitialCalculation(true);

		cout << "\n*******\n 1Positive model IN estimation \n*******\n\n";
		cout << "Simple estimates: \n";
		std::cout <<  setprecision(10);
		for (int i = 0; i < model_1Pos->GetEstimateCount(); ++i) {
			std::cout << model_1Pos->GetEstimateAt(i).first << " " << model_1Pos->GetEstimateAt(i).second.Value << std::endl;
		}


		cout << "\n \nOne-step estimates:\n";
		model_1Pos->ProcessEstimatesCalculation();
		for (int i = 0; i < model_1Pos->GetEstimateCount(); ++i) {
			std::cout << model_1Pos->GetEstimateAt(i).first << " " << model_1Pos->GetEstimateAt(i).second.Value << std::endl;
		}


	}
	catch (exception& e)
	{
		cout << "exception!" << e.what() << endl;
	}
	return 0;
}

