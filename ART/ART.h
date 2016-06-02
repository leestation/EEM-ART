#pragma once
class CART
{
public:
	CART(void);
	~CART(void);
	
	int N_inputField;
	int* N_elementInputField; // input field 개수 만큼 배열 크기가 할당되어야 함
	double* contribution; // input field 개수 만큼 배열 크기가 할당되어야 함
	double* choiceParam; // input field 개수 만큼 배열 크기가 할당되어야 함
	double choiceParamDefault;
	double* vigilance; // input field 개수 만큼 배열 크기가 할당되어야 함
	double* learningRate; // input field 개수 만큼 배열 크기가 할당되어야 함

	int N_categryField;
	double*** weights; // [category 개수] x [input filed 개수] x [input element 개수] 크기 만큼 할당해야 함
	double* strength;

	void init(int NumIF, int* NumElemIF);

	double** inputX;
	double** compX;
	void initInputField(int NumIF, int* NumElemIF);
	void reallocInputField(int NumIF, int* NumElemIF);
	bool complementCoding(int NumVector, double* vector, int NumCompVector, double* compVector);

	void initCategory();
	void addCategory(double** x_in);
	int* episodeL; // episode의 길이를 기억하게 하기 위함. episode layer가 아닌 경우 0이 기본 값
	
	double* choiceFunction;
	double* activatedCF;
	int activatedIndex;
	void codeActivation(double* T_out, double** x_in, double*** w_in);
	int codeCompetition(double* T_in, double* T_out);

	double* resonanceValue;
	bool resonanceOccured;
	bool templateMatching(double* m_out, double** x_in, double*** w_in, int index);
	bool recallMatching(double* m_out, double** x_in, double*** w_in, int Activatedindex);

	void templateLearning(double*** ww, double** x_in, int categoryIndex);
	void terminate(void);

	enum routineResult 
	{
		categoryUpdated,
		categoryAdded,
		categoryDeleted,
	};

	int artRoutine(double** x_in);
	int routineCnt;
	
	void anticipationRoutine(double** x_in, int cueLength);
	void anticipation(double* T_out, double** cue, int cueLength, double*** w_in);

	bool readOut(double** x_out, int categoryIndex);
	bool allowForgetting;
	int forgetting(int activatedCategoryIndex);
	void deleteCategory(int categoryIndex);
	int forgettedIndex;

	int my2Power(int multiplier);
};