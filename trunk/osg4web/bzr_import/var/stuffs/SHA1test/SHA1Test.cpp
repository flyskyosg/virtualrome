#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h> // Needed for strlen(...)


#include <Utilities/SHA1.h>


void HashString()
{
	char szString[1024];
	char szReport[1024];
	Utilities::CryptoAlg::SHA1 sha1;

	szString[0] = 0; szReport[0] = 0;

	printf("\r\n Enter the string to hash:\r\n ");

	fflush(stdin);
	gets(szString);

	sha1.Reset();
	sha1.Update((UINT_8*)szString, strlen(szString));
	sha1.Final();

	sha1.ReportHash(szReport, CSHA1::REPORT_HEX);
	printf("\r\n String hashed to:\r\n ");
	printf(szReport);
	printf("\r\n");
}

bool HashFile(char* extfname)
{
	char szFilename[1024];
	char szReport[1024];
	bool bSuccess = false;
	Utilities::CryptoAlg::SHA1 sha1;

	szFilename[0] = 0; szReport[0] = 0;

	sha1.Reset();

	if(extfname == NULL)
	{
		printf("\r\n Filename: ");

		fflush(stdin);
		gets(szFilename);

		bSuccess = sha1.HashFile(szFilename);
		sha1.Final();
	
		sha1.ReportHash(szReport, CSHA1::REPORT_HEX);
	
		if(bSuccess == true)
		{
			printf("\r\n File contents hashed to:\r\n ");
			printf(szReport);
			printf("\r\n");
		}
		else
			printf("\r\n An error occured (does the file really exist?) !\r\n");
	}
	else
	{
		bSuccess = sha1.HashFile(extfname);
		sha1.Final();
	
		sha1.ReportHash(szReport, CSHA1::REPORT_HEX);
	
		if(bSuccess == true)
			printf(szReport);
		else
			printf("\r\n An error occured (does the file really exist?) !\r\n");
	}

	return bSuccess;
}

int main(int argc, char* argv[])
{
	int ch = 0;

	if(argc == 1)
	{
		// Print some information
		printf("\r\n [ Test driver for the CSHA1 class ]\r\n\r\n");

		while(ch != 3)
		{
			printf("\r\n 1) Hash a string\r\n");
			printf(" 2) Hash a file\r\n");
			printf(" 3) Exit\r\n");
			printf("\r\n Enter [1], [2] or [3]: ");
		
			// User's choice
			fflush(stdin);
			ch = getchar();

			// Process user's choice
			if(ch == '3') return(0);
			else if(ch == '1') HashString();
			else if(ch == '2') HashFile(NULL);
			else { printf("\r\n Input error: Enter 1, 2 or 3 plus [Enter]!\r\n"); }
		}

		// Print information and exit
		printf("\r\n Operation successfully completed.\r\n Press any key to exit.\r\n");
		fflush(stdin);
		getchar();
		fflush(stdin);
	}
	else if(argc == 3 && strcmp(argv[1], "-f") == 0)
	{
		if(!HashFile(argv[2]))
			return -1;
	}
	else
	{
		printf("\r\n Reading parameters failed.\r\n Correct syntax: 'sha1pname' [ -f filename ].\r\n");
		return(-1);
	}

	return(0);
}