#pragma once
class CART
{
public:
	CART(void);
	~CART(void);
	
	int N_inputField;
	int* N_elementInputField; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��
	double* contribution; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��
	double* choiceParam; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��
	double choiceParamDefault;
	double* vigilance; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��
	double* learningRate; // input field ���� ��ŭ �迭 ũ�Ⱑ �Ҵ�Ǿ�� ��

	int N_categryField;
	double*** weights; // [category ����] x [input filed ����] x [input element ����] ũ�� ��ŭ �Ҵ��ؾ� ��
	double* strength;

	void init(int NumIF, int* NumElemIF);

	double** inputX;
	double** compX;
	void initInputField(int NumIF, int* NumElemIF);
	void reallocInputField(int NumIF, int* NumElemIF);
	bool complementCoding(int NumVector, double* vector, int NumCompVector, double* compVector);

	void initCategory();
	void addCategory(double** x_in);
	int* episodeL; // episode�� ���̸� ����ϰ� �ϱ� ����. episode layer�� �ƴ� ��� 0�� �⺻ ��
	
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