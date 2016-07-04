// Wagner-Fischer algorithm: https://en.wikipedia.org/wiki/Wagner%E2%80%93Fischer_algorithm
// Code from: http://www.sanfoundry.com/cpp-program-implement-levenshtein-distance-computing-algorithm/

int lev(char* s, char* t) // WagnerFischer algorithm
{
	int i,j,m,n,temp,tracker;
	m = strlen(s);
	n = strlen(t);

	// int d[100][100];
	int** d = new int*[n+1];
	for(int k=0; k<n+1; k++)
	{
		d[k] = new int[m+1];
	}


	for(i=0;i<=m;i++)
		d[0][i] = i;
	for(j=0;j<=n;j++)
		d[j][0] = j;

	for (j=1;j<=m;j++)
	{
		for(i=1;i<=n;i++)
		{
			if(s[i-1] == t[j-1])
			{
				tracker = 0;
			}
			else
			{
				tracker = 1;
			}
			temp = min((d[i-1][j]+1),(d[i][j-1]+1));
			d[i][j] = min(temp,(d[i-1][j-1]+tracker));
		}
	}

	return d[n][m];
}
