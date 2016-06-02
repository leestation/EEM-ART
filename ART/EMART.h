#pragma once

#include "ART.h"

// CEMART

class CEMART : public CWnd
{
	DECLARE_DYNAMIC(CEMART)

public:
	CEMART();
	virtual ~CEMART();

protected:
	DECLARE_MESSAGE_MAP()
public:
	void init(int NumIF, int* NumElemIF);
	void terminate();

	CART art1; // for event categorization
	CART art2; // for episode categorization
	int emARTRoutine(double** x_in);

	double* bufferChannel;
	double* outputChannel;
	int cueLength;

	void resetCue(void);
	void epiList(CString* list_out);
	int getSequence(int epiIndex, int* sequence);
};


