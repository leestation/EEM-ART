#include "StdAfx.h"
#include "ART.h"

#include <math.h>


CART::CART(void)
{
}


CART::~CART(void)
{
}


void CART::init(int NumIF, int* NumElemIF)
{
	choiceParamDefault = 0.1;
	initInputField(NumIF, NumElemIF);
	initCategory();
	routineCnt = 0;
}


void CART::terminate(void)
{
	delete [] N_elementInputField; // input field 개수 만큼 배열 크기가 할당되어야 함
	delete [] contribution; // input field 개수 만큼 배열 크기가 할당되어야 함
	delete [] choiceParam; // input field 개수 만큼 배열 크기가 할당되어야 함
	delete [] vigilance; // input field 개수 만큼 배열 크기가 할당되어야 함
	delete [] learningRate; // input field 개수 만큼 배열 크기가 할당되어야 함

	for(int k=0; k<N_inputField; k++)
	{
		delete [] inputX[k];
		delete [] compX[k];
	}
	delete [] inputX;
	delete [] compX;

	delete [] choiceFunction;
	delete [] activatedCF;
	delete [] resonanceValue;
	delete [] strength;

	delete [] episodeL;

	for(int j=0; j<N_categryField; j++)
	{
		for(int k=0; k<N_inputField; k++)
		{
			delete [] weights[j][k];
		}
		delete [] weights[j];
	}
	if(N_categryField > 0) delete [] weights; // [category 개수] x [input filed 개수] x [input element 개수] 크기 만큼 할당해야 함
}

void CART::initInputField(int NumIF, int* NumElemIF)
{
	// input field(+element)의 크기 설정
	N_inputField = NumIF;
	N_elementInputField = new int[N_inputField];

	contribution = new double[N_inputField];
	choiceParam = new double[N_inputField];
	vigilance = new double[N_inputField];

	inputX = new double*[N_inputField];
	compX = new double*[N_inputField];

	resonanceValue = new double[N_inputField];
	resonanceOccured = FALSE;

	learningRate = new double[N_inputField];

	for(int k=0; k<N_inputField; k++)
	{
		N_elementInputField[k] = NumElemIF[k];

		contribution[k] = 1./(double)N_inputField; // default 값
		choiceParam[k] = choiceParamDefault; // default 값
		vigilance[k] = 0.9; // default 값
		learningRate[k] = 0.1; // default 값
		
		resonanceValue[k] = 0;

		inputX[k] = new double[N_elementInputField[k]];
		compX[k] = new double[N_elementInputField[k]*2];
	}

}


void CART::reallocInputField(int NumIF, int* NumElemIF)
{
	// 예전 값 저장
	int old_N_inputField = N_inputField;
	if(NumIF < N_inputField) return; // 새로 생길 공간이 이전 공간보다 작으면 리턴
	int* old_N_elementInputField = new int[old_N_inputField];
	for(int k=0; k<old_N_inputField; k++)
	{
		old_N_elementInputField[k] = N_elementInputField[k];
		if(NumElemIF[k] < N_elementInputField[k]) return; // 새로 생길 공간이 이전 공간보다 작으면 리턴
	}

	// 공간할당
	N_inputField = NumIF;
	N_elementInputField = (int*)realloc(N_elementInputField, sizeof(int)*N_inputField);

	contribution = (double*)realloc(contribution, sizeof(double)*N_inputField);
	choiceParam = (double*)realloc(choiceParam, sizeof(double)*N_inputField);
	vigilance = (double*)realloc(vigilance, sizeof(double)*N_inputField);

	//inputX = new double*[N_inputField];
	inputX = (double**)realloc(inputX, sizeof(double*)*N_inputField);
	//compX = new double*[N_inputField];
	compX = (double**)realloc(compX, sizeof(double*)*N_inputField);

	//resonanceValue = new double[N_inputField];
	resonanceValue = (double*)realloc(resonanceValue, sizeof(double)*N_inputField);
	resonanceOccured = FALSE;

	//learningRate = new double[N_inputField];
	learningRate = (double*)realloc(learningRate, sizeof(double)*N_inputField);

	for(int k=0; k<N_inputField; k++)
	{
		N_elementInputField[k] = NumElemIF[k];

		contribution[k] = 1./(double)N_inputField; // default 값
		choiceParam[k] = choiceParamDefault; // default 값
		vigilance[k] = 0.9; // default 값
		learningRate[k] = 0.1; // default 값

		
		if(k<old_N_inputField)
		{
			inputX[k] = (double*)realloc(inputX[k], sizeof(double)*N_elementInputField[k]);
			compX[k] = (double*)realloc(compX[k], sizeof(double)*N_elementInputField[k]*2);
		}
		else
		{
			inputX[k] = new double[N_elementInputField[k]];
			compX[k] = new double[N_elementInputField[k]*2];
		}
	}

	// reallocate weights
	//weights = (double***)realloc(weights, sizeof(double**)*N_categryField);
	for(int j=0; j<N_categryField; j++)
	{
		//weights[j] = new double*[N_inputField];
		weights[j] = (double**)realloc(weights[j], sizeof(double*)*N_inputField);
		for(int k=0; k<N_inputField; k++)
		{
			if(k<old_N_inputField) // 기존에 있던 input field인 경우
			{
				weights[j][k] = (double*)realloc(weights[j][k], sizeof(double)*N_elementInputField[k]*2);
				// complement coding이라 두배의 크기로 할당해야 함
				
				// 추가될 element 공간만 0으로 채운다
				for(int i=old_N_elementInputField[k]; i<N_elementInputField[k]; i++) weights[j][k][i] = 0;
			}
			else // 새로 생긴 input field
			{
				weights[j][k] = new double[N_elementInputField[k]*2];
				for(int i=0; i<N_elementInputField[k]; i++) weights[j][k][i] = 0;
			}

			// complement coding, weights들도 complement 코딩 된 것들
			for(int i=0; i<N_elementInputField[k]; i++) weights[j][k][i+N_elementInputField[k]] = 1 - weights[j][k][i];

		}
	}
}



bool CART::complementCoding(int NumVector, double* vector, int NumCompVector, double* compVector)
{
	if(NumCompVector != 2*NumVector) return FALSE; // complement vector의 크기는 입력 vector의 크기 2배이어야 한다.

	for(int i=0; i<NumVector; i++) compVector[i] = vector[i];
	for(int i=0; i<NumVector; i++) compVector[i+NumVector] = 1 - vector[i];

	return TRUE;
}


void CART::initCategory()
{
	N_categryField = 0;
	weights = new double**[N_categryField];
	for(int j=0; j<1; j++)
	{
		weights[j] = new double*[N_inputField];
		for(int k=0; k<N_inputField; k++)
		{
			weights[j][k] = new double[N_elementInputField[k]*2];
			// complement coding이라 두배의 크기로 할당해야 함

			for(int i=0; i<N_elementInputField[k]*2; i++)
			{
				weights[j][k][i] = 0.; // 첫 카테고리는 모든 weights를 0으로 갖는다.
			}
		}
	}

	choiceFunction = new double[N_categryField];

	activatedCF = new double[N_categryField];
	activatedIndex = -1;

	strength = new double[N_categryField];

	forgettedIndex = -1;
	allowForgetting = FALSE;

	episodeL = new int[N_categryField];
}


void CART::codeActivation(double* T_out, double** x_in, double*** w_in)
{
	double tempSum, tempNorm1, tempNorm2;
	// T_out[0] = 0;
	for(int j=0; j<N_categryField; j++)
	{
		tempSum = 0;
		for(int k=0; k<N_inputField; k++)
		{
			tempNorm1 = 0;
			tempNorm2 = 0;
			for(int i=0; i<(N_elementInputField[k]*2); i++)
			//for(int i=0; i<(N_elementInputField[k]); i++)
			// complement coding된 배열의 크기임을 잊지 말자
			{
				tempNorm1 += min(x_in[k][i], w_in[j][k][i]);
				tempNorm2 += w_in[j][k][i];
			}
			tempSum += contribution[k]*tempNorm1/(choiceParam[k] + tempNorm2);
		}
		T_out[j] = tempSum;
	}
}

void CART::anticipation(double* T_out, double** cue, int cueLength, double*** w_in)
{
	double tempSum, tempNorm1, tempNorm2;
	// T_out[0] = 0;
	for(int j=0; j<N_categryField; j++)
	{
		double actLevel = 0;
		double significancy = 0;
		double similarity = 0;

		double lengthWeight = 0;
		lengthWeight = (double)cueLength / (double)episodeL[j];
		//lengthWeight = 0;
		if(cueLength >= episodeL[j]) lengthWeight = 1;
		lengthWeight = lengthWeight;//*lengthWeight*lengthWeight;
		TRACE(_T("lengthWeight : %f\n\n"), lengthWeight);
		
		int diffPower = 1;
		if(episodeL[j] > cueLength) diffPower = my2Power(episodeL[j] - cueLength);
		double minimumNorm = 1./my2Power(episodeL[j]); // log 값을 취했을 때 음수가 되지 않도록 최소 값을 미리 계산

		double** normCue = new double*[N_categryField];
		for(int k=0; k<N_inputField; k++)
		{
			normCue[k] = new double[N_elementInputField[k]];

			double fuzzyAndNorm = 0;
			double fuzzyOrNorm = 0;
			double logFuzzyAndNorm = 0;
			double wNorm = 0;
			double ncNorm = 0;
			double inner = 0;
			double wsNorm = 0;
			double ncsNorm = 0;
			double logNcNorm = 0;
			TRACE(_T("Normalized Cue\n"));
			for(int i=0; i<N_elementInputField[k]; i++)
			{
				// norm cue 계산
				normCue[k][i] = cue[k][i]/(double)diffPower;
				//normCue[k][i] = cue[k][i];
				TRACE(_T("%f\t"), normCue[k][i]);

				fuzzyAndNorm += min(normCue[k][i], w_in[j][k][i]);
				fuzzyOrNorm += max(normCue[k][i], w_in[j][k][i]);

				//logFuzzyAndNorm += min(log(normCue[k][i]*my2Power(episodeL[j]+1)), log(w_in[j][k][i]*my2Power(episodeL[j]+1)));

				wNorm += w_in[j][k][i];
				ncNorm += normCue[k][i];

				inner += normCue[k][i]*w_in[j][k][i];
				//if(normCue != 0) wsNorm += w_in[j][k][i]*w_in[j][k][i];
				wsNorm += w_in[j][k][i]*w_in[j][k][i];
				ncsNorm += normCue[k][i]*normCue[k][i];
				logNcNorm += log(normCue[k][i]*my2Power(episodeL[j]+1))*log(normCue[k][i]*my2Power(episodeL[j]+1));
			}
			TRACE(_T("\n\n"));

			if(fuzzyAndNorm < minimumNorm) fuzzyAndNorm = minimumNorm; // log 값을 취했을 때 음수가 되지 않도록
			significancy = log(fuzzyAndNorm * my2Power(episodeL[j]+1))/log(wNorm * my2Power(episodeL[j]+1));
//			significancy = fuzzyAndNorm/wNorm;

			wsNorm = sqrt(wsNorm);
			ncsNorm = sqrt(ncsNorm);

			double angleSim = inner/(wsNorm*ncsNorm);
			double sclaeSim = fuzzyAndNorm/fuzzyOrNorm;
			//similarity = min(angleSim, sclaeSim);
			//similarity = min(fuzzyAndNorm/ncNorm, similarity);
			//similarity = fuzzyAndNorm/ncNorm;
			//similarity = (angleSim+similarity)/2;
			//similarity = logFuzzyAndNorm/logNcNorm;

			//similarity = (angleSim+sclaeSim+similarity)/3;

			TRACE(_T("similarity : %f\n\n"), similarity);
			TRACE(_T("angleSim : %f\n\n"), angleSim);
			TRACE(_T("sclaeSim : %f\n\n"), sclaeSim);

			delete [] normCue[k];
		}
		delete [] normCue;
		T_out[j] = significancy*(1-lengthWeight*(1-similarity));
		//T_out[j] = significancy;


		/*
		int tempEpiLen = episodeL[j];
		double minimumNorm = 1./my2Power(tempEpiLen);

		tempSum = 0;
		for(int k=0; k<N_inputField; k++)
		{
			tempNorm1 = 0;
			tempNorm2 = 0;
			int tempPower = 1;
			if(tempEpiLen > cueLength) tempPower = my2Power(tempEpiLen - cueLength);
			//tempNorm2 = cueLength;
			for(int i=0; i<(N_elementInputField[k]); i++)
			// anticipation에서는 complemented coding 쪽은 쓰지 않는다.
			{
				tempNorm1 += min(cue[k][i]/tempPower, w_in[j][k][i]);
				tempNorm2 += w_in[j][k][i];
				//tempNorm2 += cue[k][i];
				//tempNorm2 += min(cue[k][i], w_in[j][k][i]);;
			}
			//tempSum += tempPower*(tempNorm1)/(tempNorm2);
			if(tempNorm1 < minimumNorm) tempNorm1 = minimumNorm; // log 값을 취했을 때 0이 되도록

			double similarity = 0;

			tempSum += log(tempNorm1 * my2Power(tempEpiLen+1))/log(tempNorm2 * my2Power(tempEpiLen+1));
		}
		T_out[j] = tempSum;
		*/
	}
}


int CART::codeCompetition(double* T_in, double* T_out)
{
	double tempMax = -1;
	int maxIndex = -1;
	for(int j=0; j<N_categryField; j++)
	{
		T_out[j] = 0; // 일단 모든 choice function 출력 값은 0으로 세팅

		// 최대값과 해당 index 찾기
		if(T_in[j] > tempMax)
		{
			tempMax = T_in[j];
			maxIndex = j;
		}
	}

	//T_out[0] = 0;
	if(maxIndex != -1) T_out[maxIndex] = 1; // 최대값을 갖는 choice function만 1로 세팅

	return maxIndex; // choiced category index
}


bool CART::templateMatching(double* m_out, double** x_in, double*** w_in, int Activatedindex)
{
	double tempNorm1, tempNorm2;
	int tempResonanceCnt = 0;
	for(int k=0; k<N_inputField; k++)
	{
		tempNorm1 = 0;
		tempNorm2 = 0;
		for(int i=0; i<N_elementInputField[k]*2; i++)
			// complement coding된 배열의 크기임을 잊지 말자
		{
			tempNorm1 += min(x_in[k][i], w_in[Activatedindex][k][i]);
			tempNorm2 += x_in[k][i];
		}
		m_out[k] = tempNorm1/tempNorm2;

		if(m_out[k] >= vigilance[k]) tempResonanceCnt++;
	}

	if(tempResonanceCnt == N_inputField) return TRUE; // 모든 채널에 대해 resonance가 일어남
	return FALSE; // resonance가 최소 한 채널 이상 일어나지 않음
}

bool CART::recallMatching(double* m_out, double** x_in, double*** w_in, int Activatedindex)
{
	double tempNorm1, tempNorm2;
	int tempResonanceCnt = 0;
	for(int k=0; k<N_inputField; k++)
	{
		tempNorm1 = 0;
		tempNorm2 = 0;
		for(int i=0; i<N_elementInputField[k]; i++)
		// episode에서는 compliment coding을 사용하지 않는다.
		{
			tempNorm1 += min(x_in[k][i], w_in[Activatedindex][k][i]);
			tempNorm2 += x_in[k][i];
		}
		m_out[k] = tempNorm1/tempNorm2;

		if(m_out[k] >= vigilance[k]) tempResonanceCnt++;
	}

	if(tempResonanceCnt == N_inputField) return TRUE; // 모든 채널에 대해 resonance가 일어남
	return FALSE; // resonance가 최소 한 채널 이상 일어나지 않음
}

void CART::templateLearning(double*** ww, double** x_in, int categoryIndex)
{
	double tempVal1, tempVal2;
	int j = categoryIndex;
	for(int k=0; k<N_inputField; k++)
	{
		for(int i=0; i<(N_elementInputField[k]*2); i++)
		{
			tempVal1 = (1-learningRate[k])*ww[j][k][i];
			//tempVal2 = learningRate[k]*min(x_in[k][i], ww[j][k][i]);
			tempVal2 = learningRate[k]*x_in[k][i]; // 원래는 위에 주석처리된 대로 fuzzy and operation을 써야 하는데 일단 이걸로
			ww[j][k][i] = tempVal1 + tempVal2;
		}
	}
}


void CART::addCategory(double** x_in)
{
	// x_in은 complement coded 된 것이라는 것 잊지 말자
	// x_in이 새로 생긴 카테고리의 weights들의 초기값이 되도록

	N_categryField = N_categryField + 1;
	if(N_categryField > 1000000) return; // 카테고리는 1000000개까지만 허용. 넘을 가능성은 없지만;;

	// weights의 메모리 공간을 늘리고 기존 값을 복사해옴, realloc의 의미
	weights = (double***)realloc(weights, sizeof(double**)*N_categryField);
	
	// 다른 값은 그대로이므로 추가된 category 영역에 대해서만 메모리 할당을 하면 됨
	int j = N_categryField - 1;
	weights[j] = new double*[N_inputField];
	for(int k=0; k<N_inputField; k++)
	{
		weights[j][k] = new double[N_elementInputField[k]*2];
		// complement coding이라 두배의 크기로 할당해야 함

		// 초기값은 x_in 값으로 설정
		for(int i=0; i<N_elementInputField[k]; i++)
		{
			weights[j][k][i] = x_in[k][i];
		}
		for(int i=0; i<N_elementInputField[k]; i++)
		{
			weights[j][k][i+N_elementInputField[k]] = 1-x_in[k][i];
		}
	}

	// strength도 메모리 공간을 늘리고 기존 값은 유지
	strength = (double*)realloc(strength, sizeof(double)*N_categryField);
	double s_init = 0.5; // default 초기값. 추후 초기값 설정 방법 고민 필요
	strength[N_categryField-1] = s_init;

	//choiceFunction = new double[N_categryField];
	choiceFunction = (double*)realloc(choiceFunction, sizeof(double)*N_categryField);
	choiceFunction[N_categryField-1] = 1;

	// 추가된 카테고리가 activated 카테고리가 되어야 한다.
	activatedCF = (double*)realloc(activatedCF, sizeof(double)*N_categryField);
	if(activatedIndex >= 0) activatedCF[activatedIndex] = 0;
	activatedCF[N_categryField-1] = 1;
	activatedIndex = N_categryField-1;

	episodeL = (int*)realloc(episodeL, sizeof(int)*N_categryField);
	episodeL[N_categryField-1] = 0; // dedault 값. episode category라면 외부에서 길이 값을 직접 업데이트 시킴
}




int CART::artRoutine(double** x_in)
{
	// 당연한 말이지만 initialization이 되지 않은 상태에서 이 함수가 먼저 불리면 안된다.

	int routineResult;

	routineCnt++;
	TRACE(_T("\n\n\n\n############ Routine #%d ###############\n\n\n\n"), routineCnt);

	//TRACE(_T("Input Copy and Complement Coding\n\n"));
	for(int k=0; k<N_inputField; k++)
	{
		// input 복사
		//TRACE(_T("Input\n"));
		memmove(inputX[k], x_in[k], sizeof(double)*N_elementInputField[k]);
		for(int i=0; i<N_elementInputField[k]; i++)
		{
			//TRACE(_T("%f\t"), x_in[k][i]);
		}
		//TRACE(_T("\n"));

		// complement coding
		complementCoding(N_elementInputField[k], inputX[k], N_elementInputField[k]*2, compX[k]);
	}
	//TRACE(_T("\n"));

	//TRACE(_T("Code Activation\n"));
	codeActivation(choiceFunction, compX, weights);

	for(int j=0; j<N_categryField; j++)
	{
		//TRACE(_T("%f\t"), choiceFunction[j]);
	}
	//TRACE(_T("\n\n"));

	//TRACE(_T("Code Competetion\n"));
	activatedIndex = codeCompetition(choiceFunction, activatedCF);

	for(int j=0; j<N_categryField; j++)
	{
		//TRACE(_T("%f\t"), activatedCF[j]);
	}
	//TRACE(_T("\n\n"));

	resonanceOccured = FALSE;

	if(activatedIndex >= 0)	
	{
		//TRACE(_T("Template Matching\n"));
		resonanceOccured = templateMatching(resonanceValue, compX, weights, activatedIndex);

		for(int k=0; k<N_inputField; k++)
		{
			//TRACE(_T("%f\t"), resonanceValue[k]);
		}
		//TRACE(_T("\n\n"));
	}

	if(resonanceOccured) // update
	{
		//TRACE(_T("Template Learning\n\n"));
		templateLearning(weights, compX, activatedIndex);

		routineResult = categoryUpdated;

		//TRACE(_T("Forgetting or Reinforcing\n\n"));

		forgettedIndex = forgetting(activatedIndex);
		if(forgettedIndex!=0) // 카테고리가 forget 되었다면
		{
			//TRACE(_T("Forget Occured\n"));
			//TRACE(_T("%d\n\n"), forgettedIndex);
			routineResult = categoryDeleted;
			forgettedIndex = 0; // 초기화
		}
	}
	else // category 추가
	{
		//TRACE(_T("Add Category\n\n"));
		addCategory(compX);
		// activatedIndex = N_categryField - 1; // add category 안에 포함되어 있음
		routineResult = categoryAdded;
	}

	/*
	TRACE(_T("Check Weights\n"));
	//TRACE(_T("%f"), min(a,b));
	for(int j=0; j<N_categryField; j++)
	{
		for(int k=0; k<N_inputField; k++)
		{
			for(int i=0; i<N_elementInputField[k]; i++)
			{
				TRACE(_T("%f\t"), weights[j][k][i]);
			}
			TRACE(_T("\n"));
		}
		TRACE(_T("\n"));
	}
	*/

	return routineResult;
}


bool CART::readOut(double** x_out, int categoryIndex)
{
	// read out 하고자 하는 category 번호 범위를 넘어서는 경우
	if((categoryIndex > N_categryField-1)||(categoryIndex < 0)) return FALSE;

	for(int k=0; k<N_inputField; k++)
	{
		memmove(x_out[k], weights[categoryIndex][k], sizeof(double)*N_elementInputField[k]);
		// complement code까지 전달하지는 않는다.
	}

	return TRUE;
}


int CART::forgetting(int activatedCategoryIndex)
{
	int forgettedCategoryIndex = 0;

	double rRate = 0.3; // default 이 값 설정 방법 고민 필요
	double dRate = 0.1; // default 이 값 설정 방법 고민 필요
	double strengthThreshold = 0.1; // default

	for(int j=0; j<N_categryField; j++) // 첫번째 카테고리는 의미 없음
	{
		// reinforcing
		if(j==activatedCategoryIndex)
		{
			strength[j] += (1 - strength[j])*rRate;
		}

		// forgetting
		if(allowForgetting) strength[j] = strength[j]*(1-dRate);

		// 일단 strength는 0이 되더라도 카테고리는 없어지지 않는 것으로
		/*
		// category 제거
		if(	(strength[j] > 0) // 이미 forget 된 카테고리는 strength == 0으로 제거 대상이 아님
			&&(strength[j] < strengthThreshold)
			&&(forgettedCategoryIndex == 0)) // forgetting 함수 한번에 한 카테고리만 제거
		{
			// 카테고리를 제거하고 뒤쪽 인덱스의 카테고리들을 앞당기는 방법도 있지만...
			// 실제로 카테고리를 제거하기보다는 해당 카테고리의 weights를 빈공간으로 만들자
			// 그래야 외부에서 ART에 접근할 때 category index를 수정할 필요가 없다.
			// 1) 해당 카테고리의 weigths들을 모두 0으로 만든다.
			// 2) 카테고리가 제거되었다는 알림을 출력으로 낸다.

			forgettedCategoryIndex = j;
			deleteCategory(forgettedCategoryIndex);
		}
		*/
	}

	return forgettedCategoryIndex;
}


void CART::deleteCategory(int categoryIndex)
{
	// 다른 값은 그대로이므로 추가된 category 영역에 대해서만 메모리 할당을 하면 됨
	int j = categoryIndex;
	for(int k=0; k<N_inputField; k++)
	{
		weights[j][k] = new double[N_elementInputField[k]*2];
		// complement coding이라 두배의 크기로 할당해야 함

		// weights 값 0으로 지움
		for(int i=0; i<N_elementInputField[k]*2; i++)
		{
			weights[j][k][i] = 0;
		}
	}

	// 해당 category의 strength도 0으로
	strength[j] = 0;
}



void CART::anticipationRoutine(double** x_in, int cueLength)
{
	//TRACE(_T("Input Copy and Complement Coding\n\n"));
	for(int k=0; k<N_inputField; k++)
	{
		// input 복사
		TRACE(_T("Input\n"));
		memmove(inputX[k], x_in[k], sizeof(double)*N_elementInputField[k]);
		for(int i=0; i<N_elementInputField[k]; i++)
		{
			TRACE(_T("%f\t"), x_in[k][i]);
		}
		TRACE(_T("\n"));
	}
	TRACE(_T("\n"));

	TRACE(_T("Code Anticipation\n"));
	anticipation(choiceFunction, inputX, cueLength, weights);
	for(int j=0; j<N_categryField; j++)
	{
		TRACE(_T("%f\t"), choiceFunction[j]);
	}
	TRACE(_T("\n\n"));

	TRACE(_T("Code Competetion\n"));
	activatedIndex = codeCompetition(choiceFunction, activatedCF);

	for(int j=0; j<N_categryField; j++)
	{
		TRACE(_T("%f\t"), activatedCF[j]);
	}
	TRACE(_T("\n\n"));

	resonanceOccured = FALSE;

	if(activatedIndex >= 0)	
	{
		TRACE(_T("Recall Matching\n"));
		resonanceOccured = recallMatching(resonanceValue, inputX, weights, activatedIndex);

		for(int k=0; k<N_inputField; k++)
		{
			TRACE(_T("%f\t"), resonanceValue[k]);
		}
		TRACE(_T("\n\n"));
	}

	if(resonanceOccured) // anticipation 중 resonance가 일어나면 strength 강화
	{
		// Episode는 weights가 update 되지 않고 strength만 변화한다.

		//TRACE(_T("Template Learning\n\n"));
		//templateLearning(weights, compX, activatedIndex);

		TRACE(_T("Forgetting or Reinforcing\n\n"));
		forgetting(activatedIndex);

		// 일단 이렇게 해놓기는 했는데, 강화되는 것 말고 약화되는 건 resonance가 안 일어나도 진행되어야 한다. 코드 수정 필요
	}

	TRACE(_T("Check Weights\n"));
	//TRACE(_T("%f"), min(a,b));
	for(int j=0; j<N_categryField; j++)
	{
		for(int k=0; k<N_inputField; k++)
		{
			for(int i=0; i<N_elementInputField[k]; i++)
			{
				TRACE(_T("%f\t"), weights[j][k][i]);
			}
			TRACE(_T("\n"));
		}
		TRACE(_T("\n"));
	}
}

int CART::my2Power(int multiplier)
{
	if(multiplier > 30)
	{
		return 1;
	}

	int tempResult = 1;
	for(int i=0; i<multiplier; i++)
	{
		tempResult = tempResult*2;
	}

	return tempResult;
}