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
	delete [] N_elementInputField; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��
	delete [] contribution; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��
	delete [] choiceParam; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��
	delete [] vigilance; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��
	delete [] learningRate; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��

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
	if(N_categryField > 0) delete [] weights; // [category ����] x [input filed ����] x [input element ����] ũ�� ��ŭ �Ҵ��ؾ� ��
}

void CART::initInputField(int NumIF, int* NumElemIF)
{
	// input field(+element)�� ũ�� ����
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

		contribution[k] = 1./(double)N_inputField; // default ��
		choiceParam[k] = choiceParamDefault; // default ��
		vigilance[k] = 0.9; // default ��
		learningRate[k] = 0.1; // default ��
		
		resonanceValue[k] = 0;

		inputX[k] = new double[N_elementInputField[k]];
		compX[k] = new double[N_elementInputField[k]*2];
	}

}


void CART::reallocInputField(int NumIF, int* NumElemIF)
{
	// ���� �� ����
	int old_N_inputField = N_inputField;
	if(NumIF < N_inputField) return; // ���� ���� ������ ���� �������� ������ ����
	int* old_N_elementInputField = new int[old_N_inputField];
	for(int k=0; k<old_N_inputField; k++)
	{
		old_N_elementInputField[k] = N_elementInputField[k];
		if(NumElemIF[k] < N_elementInputField[k]) return; // ���� ���� ������ ���� �������� ������ ����
	}

	// �����Ҵ�
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

		contribution[k] = 1./(double)N_inputField; // default ��
		choiceParam[k] = choiceParamDefault; // default ��
		vigilance[k] = 0.9; // default ��
		learningRate[k] = 0.1; // default ��

		
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
			if(k<old_N_inputField) // ������ �ִ� input field�� ���
			{
				weights[j][k] = (double*)realloc(weights[j][k], sizeof(double)*N_elementInputField[k]*2);
				// complement coding�̶� �ι��� ũ��� �Ҵ��ؾ� ��
				
				// �߰��� element ������ 0���� ä���
				for(int i=old_N_elementInputField[k]; i<N_elementInputField[k]; i++) weights[j][k][i] = 0;
			}
			else // ���� ���� input field
			{
				weights[j][k] = new double[N_elementInputField[k]*2];
				for(int i=0; i<N_elementInputField[k]; i++) weights[j][k][i] = 0;
			}

			// complement coding, weights�鵵 complement �ڵ� �� �͵�
			for(int i=0; i<N_elementInputField[k]; i++) weights[j][k][i+N_elementInputField[k]] = 1 - weights[j][k][i];

		}
	}
}



bool CART::complementCoding(int NumVector, double* vector, int NumCompVector, double* compVector)
{
	if(NumCompVector != 2*NumVector) return FALSE; // complement vector�� ũ��� �Է� vector�� ũ�� 2���̾�� �Ѵ�.

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
			// complement coding�̶� �ι��� ũ��� �Ҵ��ؾ� ��

			for(int i=0; i<N_elementInputField[k]*2; i++)
			{
				weights[j][k][i] = 0.; // ù ī�װ��� ��� weights�� 0���� ���´�.
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
			// complement coding�� �迭�� ũ������ ���� ����
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
		double minimumNorm = 1./my2Power(episodeL[j]); // log ���� ������ �� ������ ���� �ʵ��� �ּ� ���� �̸� ���

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
				// norm cue ���
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

			if(fuzzyAndNorm < minimumNorm) fuzzyAndNorm = minimumNorm; // log ���� ������ �� ������ ���� �ʵ���
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
			// anticipation������ complemented coding ���� ���� �ʴ´�.
			{
				tempNorm1 += min(cue[k][i]/tempPower, w_in[j][k][i]);
				tempNorm2 += w_in[j][k][i];
				//tempNorm2 += cue[k][i];
				//tempNorm2 += min(cue[k][i], w_in[j][k][i]);;
			}
			//tempSum += tempPower*(tempNorm1)/(tempNorm2);
			if(tempNorm1 < minimumNorm) tempNorm1 = minimumNorm; // log ���� ������ �� 0�� �ǵ���

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
		T_out[j] = 0; // �ϴ� ��� choice function ��� ���� 0���� ����

		// �ִ밪�� �ش� index ã��
		if(T_in[j] > tempMax)
		{
			tempMax = T_in[j];
			maxIndex = j;
		}
	}

	//T_out[0] = 0;
	if(maxIndex != -1) T_out[maxIndex] = 1; // �ִ밪�� ���� choice function�� 1�� ����

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
			// complement coding�� �迭�� ũ������ ���� ����
		{
			tempNorm1 += min(x_in[k][i], w_in[Activatedindex][k][i]);
			tempNorm2 += x_in[k][i];
		}
		m_out[k] = tempNorm1/tempNorm2;

		if(m_out[k] >= vigilance[k]) tempResonanceCnt++;
	}

	if(tempResonanceCnt == N_inputField) return TRUE; // ��� ä�ο� ���� resonance�� �Ͼ
	return FALSE; // resonance�� �ּ� �� ä�� �̻� �Ͼ�� ����
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
		// episode������ compliment coding�� ������� �ʴ´�.
		{
			tempNorm1 += min(x_in[k][i], w_in[Activatedindex][k][i]);
			tempNorm2 += x_in[k][i];
		}
		m_out[k] = tempNorm1/tempNorm2;

		if(m_out[k] >= vigilance[k]) tempResonanceCnt++;
	}

	if(tempResonanceCnt == N_inputField) return TRUE; // ��� ä�ο� ���� resonance�� �Ͼ
	return FALSE; // resonance�� �ּ� �� ä�� �̻� �Ͼ�� ����
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
			tempVal2 = learningRate[k]*x_in[k][i]; // ������ ���� �ּ�ó���� ��� fuzzy and operation�� ��� �ϴµ� �ϴ� �̰ɷ�
			ww[j][k][i] = tempVal1 + tempVal2;
		}
	}
}


void CART::addCategory(double** x_in)
{
	// x_in�� complement coded �� ���̶�� �� ���� ����
	// x_in�� ���� ���� ī�װ��� weights���� �ʱⰪ�� �ǵ���

	N_categryField = N_categryField + 1;
	if(N_categryField > 1000000) return; // ī�װ��� 1000000�������� ���. ���� ���ɼ��� ������;;

	// weights�� �޸� ������ �ø��� ���� ���� �����ؿ�, realloc�� �ǹ�
	weights = (double***)realloc(weights, sizeof(double**)*N_categryField);
	
	// �ٸ� ���� �״���̹Ƿ� �߰��� category ������ ���ؼ��� �޸� �Ҵ��� �ϸ� ��
	int j = N_categryField - 1;
	weights[j] = new double*[N_inputField];
	for(int k=0; k<N_inputField; k++)
	{
		weights[j][k] = new double[N_elementInputField[k]*2];
		// complement coding�̶� �ι��� ũ��� �Ҵ��ؾ� ��

		// �ʱⰪ�� x_in ������ ����
		for(int i=0; i<N_elementInputField[k]; i++)
		{
			weights[j][k][i] = x_in[k][i];
		}
		for(int i=0; i<N_elementInputField[k]; i++)
		{
			weights[j][k][i+N_elementInputField[k]] = 1-x_in[k][i];
		}
	}

	// strength�� �޸� ������ �ø��� ���� ���� ����
	strength = (double*)realloc(strength, sizeof(double)*N_categryField);
	double s_init = 0.5; // default �ʱⰪ. ���� �ʱⰪ ���� ��� ��� �ʿ�
	strength[N_categryField-1] = s_init;

	//choiceFunction = new double[N_categryField];
	choiceFunction = (double*)realloc(choiceFunction, sizeof(double)*N_categryField);
	choiceFunction[N_categryField-1] = 1;

	// �߰��� ī�װ��� activated ī�װ��� �Ǿ�� �Ѵ�.
	activatedCF = (double*)realloc(activatedCF, sizeof(double)*N_categryField);
	if(activatedIndex >= 0) activatedCF[activatedIndex] = 0;
	activatedCF[N_categryField-1] = 1;
	activatedIndex = N_categryField-1;

	episodeL = (int*)realloc(episodeL, sizeof(int)*N_categryField);
	episodeL[N_categryField-1] = 0; // dedault ��. episode category��� �ܺο��� ���� ���� ���� ������Ʈ ��Ŵ
}




int CART::artRoutine(double** x_in)
{
	// �翬�� �������� initialization�� ���� ���� ���¿��� �� �Լ��� ���� �Ҹ��� �ȵȴ�.

	int routineResult;

	routineCnt++;
	TRACE(_T("\n\n\n\n############ Routine #%d ###############\n\n\n\n"), routineCnt);

	//TRACE(_T("Input Copy and Complement Coding\n\n"));
	for(int k=0; k<N_inputField; k++)
	{
		// input ����
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
		if(forgettedIndex!=0) // ī�װ��� forget �Ǿ��ٸ�
		{
			//TRACE(_T("Forget Occured\n"));
			//TRACE(_T("%d\n\n"), forgettedIndex);
			routineResult = categoryDeleted;
			forgettedIndex = 0; // �ʱ�ȭ
		}
	}
	else // category �߰�
	{
		//TRACE(_T("Add Category\n\n"));
		addCategory(compX);
		// activatedIndex = N_categryField - 1; // add category �ȿ� ���ԵǾ� ����
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
	// read out �ϰ��� �ϴ� category ��ȣ ������ �Ѿ�� ���
	if((categoryIndex > N_categryField-1)||(categoryIndex < 0)) return FALSE;

	for(int k=0; k<N_inputField; k++)
	{
		memmove(x_out[k], weights[categoryIndex][k], sizeof(double)*N_elementInputField[k]);
		// complement code���� ���������� �ʴ´�.
	}

	return TRUE;
}


int CART::forgetting(int activatedCategoryIndex)
{
	int forgettedCategoryIndex = 0;

	double rRate = 0.3; // default �� �� ���� ��� ��� �ʿ�
	double dRate = 0.1; // default �� �� ���� ��� ��� �ʿ�
	double strengthThreshold = 0.1; // default

	for(int j=0; j<N_categryField; j++) // ù��° ī�װ��� �ǹ� ����
	{
		// reinforcing
		if(j==activatedCategoryIndex)
		{
			strength[j] += (1 - strength[j])*rRate;
		}

		// forgetting
		if(allowForgetting) strength[j] = strength[j]*(1-dRate);

		// �ϴ� strength�� 0�� �Ǵ��� ī�װ��� �������� �ʴ� ������
		/*
		// category ����
		if(	(strength[j] > 0) // �̹� forget �� ī�װ��� strength == 0���� ���� ����� �ƴ�
			&&(strength[j] < strengthThreshold)
			&&(forgettedCategoryIndex == 0)) // forgetting �Լ� �ѹ��� �� ī�װ��� ����
		{
			// ī�װ��� �����ϰ� ���� �ε����� ī�װ����� �մ��� ����� ������...
			// ������ ī�װ��� �����ϱ⺸�ٴ� �ش� ī�װ��� weights�� ��������� ������
			// �׷��� �ܺο��� ART�� ������ �� category index�� ������ �ʿ䰡 ����.
			// 1) �ش� ī�װ��� weigths���� ��� 0���� �����.
			// 2) ī�װ��� ���ŵǾ��ٴ� �˸��� ������� ����.

			forgettedCategoryIndex = j;
			deleteCategory(forgettedCategoryIndex);
		}
		*/
	}

	return forgettedCategoryIndex;
}


void CART::deleteCategory(int categoryIndex)
{
	// �ٸ� ���� �״���̹Ƿ� �߰��� category ������ ���ؼ��� �޸� �Ҵ��� �ϸ� ��
	int j = categoryIndex;
	for(int k=0; k<N_inputField; k++)
	{
		weights[j][k] = new double[N_elementInputField[k]*2];
		// complement coding�̶� �ι��� ũ��� �Ҵ��ؾ� ��

		// weights �� 0���� ����
		for(int i=0; i<N_elementInputField[k]*2; i++)
		{
			weights[j][k][i] = 0;
		}
	}

	// �ش� category�� strength�� 0����
	strength[j] = 0;
}



void CART::anticipationRoutine(double** x_in, int cueLength)
{
	//TRACE(_T("Input Copy and Complement Coding\n\n"));
	for(int k=0; k<N_inputField; k++)
	{
		// input ����
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

	if(resonanceOccured) // anticipation �� resonance�� �Ͼ�� strength ��ȭ
	{
		// Episode�� weights�� update ���� �ʰ� strength�� ��ȭ�Ѵ�.

		//TRACE(_T("Template Learning\n\n"));
		//templateLearning(weights, compX, activatedIndex);

		TRACE(_T("Forgetting or Reinforcing\n\n"));
		forgetting(activatedIndex);

		// �ϴ� �̷��� �س���� �ߴµ�, ��ȭ�Ǵ� �� ���� ��ȭ�Ǵ� �� resonance�� �� �Ͼ�� ����Ǿ�� �Ѵ�. �ڵ� ���� �ʿ�
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