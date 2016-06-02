// EMART.cpp : implementation file
//

#include "stdafx.h"
#include "EEM-ART.h"
#include "EMART.h"


// CEMART

IMPLEMENT_DYNAMIC(CEMART, CWnd)

CEMART::CEMART()
{

}

CEMART::~CEMART()
{
}


BEGIN_MESSAGE_MAP(CEMART, CWnd)
END_MESSAGE_MAP()



// CEMART message handlers




void CEMART::init(int NumIF, int* NumElemIF)
{
	art1.init(NumIF, NumElemIF);

	int* elementNum = new int[1]; // episode�� channel�� 1
	elementNum[0] = 0; // ���� ������ event category�� �����Ƿ�..
	art2.init(1, elementNum);
	art2.choiceParamDefault = 0; // episode�� 1���� input channel�̰�, choice parameter�� ���� �ʿ� ����.
	art2.choiceParam[0];


	bufferChannel = new double[0];
	outputChannel = new double[0];

	cueLength = 0;

	resetCue();
}


void CEMART::terminate()
{
	art1.terminate();
	art2.terminate();
	delete [] bufferChannel;
	delete [] outputChannel;
}

int CEMART::emARTRoutine(double** x_in)
{
	int routineResult = art1.artRoutine(x_in);
	if(routineResult==CART::categoryAdded) // category�� �߰��� ���
	{
		int* tempNumElemIF = (int*)malloc(sizeof(int)*art2.N_inputField); // art2.N_inputField == 1
		tempNumElemIF[art2.N_inputField-1] = art1.N_categryField; // tempNumElemIF[0] = art1.N_categryField;
		art2.reallocInputField(1, tempNumElemIF);

		bufferChannel = (double*)realloc(bufferChannel, sizeof(double)*art1.N_categryField);
		outputChannel = (double*)realloc(outputChannel, sizeof(double)*art1.N_categryField);
		bufferChannel[art1.N_categryField-1] = 0;
	}

	//TRACE(_T("Output Channel Check\n"));
	//TRACE(_T("Buffer Channel Check\n"));
	cueLength++;
	if(cueLength>25) // ���̴� 25�� ���� �ʵ���... (int�� ���� ����..)
	{
		cueLength = 25;

		/*
		for(int j=0; j<art1.N_categryField; j++)
		{
			bufferChannel[j] = 2*bufferChannel[j];
			if(bufferChannel[j] > 1) bufferChannel[j] = bufferChannel[j] - 1;
		}
		*/
	}
	//TRACE(_T("Cue Length %d\n"), cueLength);
	
	//bufferChannel[art1.activatedIndex] += 1./((double)my2Power(cueLength));
	for(int j=0; j<art1.N_categryField; j++)
	{
		//TRACE(_T("%f\t"), art1.activatedCF[j]);
		bufferChannel[j] = art1.activatedCF[j]/2. + bufferChannel[j]/2.; // activated Category�� 1�� �ƴ� 0.5�� �����ؾ� normailzation�� �ȴ�.
		//outputChannel[j] = bufferChannel[j]/((double)my2Power(cueLength)); // normalization
		outputChannel[j] = bufferChannel[j];

		//TRACE(_T("%f\t"), bufferChannel[j]);
		//TRACE(_T("%f\t"), outputChannel[j]);
	}
	//TRACE(_T("\n\n"));

	art2.anticipationRoutine(&outputChannel, cueLength);

	return routineResult;
}


void CEMART::resetCue(void)
{
	cueLength = 0;
	for(int j=0; j<art1.N_categryField; j++)
	{
		bufferChannel[j] = 0;
	}
}


void CEMART::epiList(CString* list_out)
{
	*list_out = _T("");
	CString tempString = _T("");
	CString tempString2 = _T("");
	for(int j=0; j<art2.N_categryField; j++)
	{
		CString tempString3 = _T("");

		int* sequence = new int[art2.episodeL[j]];

		if(art2.episodeL[j] == getSequence(j, sequence))
		{
			for(int i=0; i<art2.episodeL[j]; i++)
			{
				tempString2.Format(_T("%d"), sequence[art2.episodeL[j]-i-1]);
				tempString3 += tempString2;
			}
		}

		tempString.Format(_T("Episode %d : a = %f, s = %f\t"), j, art2.choiceFunction[j], art2.strength[j]);

		*list_out += tempString + tempString3 + _T("\r\n");
	}
}


int CEMART::getSequence(int epiIndex, int* sequence)
{
	int sequenceL = 0;
	sequenceL = art2.episodeL[epiIndex];

	double** targetEpisode = new double*[art2.N_inputField];
	for(int k=0; k<art2.N_inputField; k++)
	{
		targetEpisode[k] = new double[art2.N_elementInputField[k]];
	}

	if(art2.readOut(targetEpisode, epiIndex)) // target episode redout
	{
		int seqenceIndex = 0;
		while(seqenceIndex < sequenceL)
		{
			for(int i=0; i<art2.N_elementInputField[0]; i++)
			{
				// Ÿ�� ���Ǽҵ� weigths���� �ι辿 �ϴٰ�
				// 1���� ���ų� Ŀ���� ���� ��Ÿ���� �� �ش� index�� �о��
				// seqence�� �����Ѵ�.
				// �Ź� ������ 1���� ���ų� ū ���� �����ϰ�, �ϳ��� �����Ѵ�.
				targetEpisode[0][i] = targetEpisode[0][i]*2;
				if(targetEpisode[0][i] >= 1)
				{
					targetEpisode[0][i] = targetEpisode[0][i] - 1;
					sequence[seqenceIndex] = i;
				}
			}

			seqenceIndex++;
		}

		return sequenceL;
	}

	return 0;
}
