// ---------------------------------------------------------------------------
#ifndef __SCANNER_H
#define __SCANNER_H
// ---------------------------------------------------------------------------
#include "config.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <list>
using namespace std;

typedef basic_string<char> mString;

class ParserStream {
	bool ReadNewChar;
	char LastChar;
      protected:
	int LineNumber;
	char NewLineChar;
	virtual char ReadChar() = 0;
	virtual void Rewind() = 0;
      public:
	  ParserStream():LineNumber(1), NewLineChar('\n'), ReadNewChar(true) {
	};
	virtual ~ ParserStream() {
	};
	virtual char GetChar() {
		if (ReadNewChar) {
			if (Eof() || Bad())
				return 0;
			LastChar = ReadChar();
			if (LastChar == NewLineChar)
				NewLine();
		}
		ReadNewChar = true;
		return LastChar;
	}
	virtual int Restart() {
		LastChar = 0;
		ReadNewChar = true;
		Rewind();
	}
	virtual bool Eof() = 0;
	virtual bool Bad() = 0;
	virtual mString GetName() {
		return "stream";
	};
	void NewLine() {
		LineNumber++;
	};
	void FirstLine() {
		LineNumber = 1;
	};
	int GetLine() {
		return LineNumber;
	};
	void SetNewLineChar(char c) {
		NewLineChar = c;
	};
	void PushBackChar() {
		ReadNewChar = false;
	};
};

class StringParserStream:public ParserStream {
	int charPtr;
	mString Data;
	mString Name;
      public:
	  StringParserStream(const mString & AS, const mString & StreamName) {
		Data = AS;
		Name = StreamName;
		charPtr = 0;
	} virtual char ReadChar() {
		return Data[charPtr++];
	}
	virtual bool Eof() {
		return ((unsigned) charPtr == Data.length());
	}
	virtual bool Bad() {
		return 0;
	}
	virtual void Rewind() {
		charPtr = 0;
	}
	virtual mString GetName() {
		return Name;
	};
};

class istreamParserStream:public ParserStream {
	istream *Data;
      public:
	istreamParserStream(istream *f):Data(f) {
	};
	virtual char ReadChar() {
		return Data->get();
	}
	virtual bool Eof() {
		return Data->eof();
	}
	virtual void Rewind() {
//		istream->
	}
	virtual bool Bad() {
		return !Data || Data->bad();
	}
	virtual mString GetName() {
		char data[16];
		sprintf(data, "0x%08x", (unsigned long) Data);
		return mString(data);
	}
};

class FstrucParserStream:public ParserStream {
	int charPtr;
	FILE *Data;
      public:
	  FstrucParserStream(FILE * f):Data(f) {
	};
	virtual char ReadChar() {
		fgetc(Data);
	}
	virtual bool Eof() {
		return feof(Data);
	}
	virtual void Rewind() {
		rewind(Data);
	}
	virtual bool Bad() {
		return !Data;
	}
	virtual mString GetName() {
		char data[16];
		sprintf(data, "0x%08x", (unsigned long) Data);
		return mString(data);
	}
};

class FileParserStream:public FstrucParserStream {
	mString Filename;
      public:
	FileParserStream(const char *fname):
		FstrucParserStream(fopen(fname, "rt")),
		Filename(fname) {
	};
	~FileParserStream() {
	}
	virtual mString GetName() {
		return Filename;
	}
};

#define TOKEN_UNKNOWN -1
#define TOKEN_STRING -2
// 0x.*, +n, -n, integer
#define TOKEN_INT -3
// +n.m, -n.m
#define TOKEN_FLOAT -4
#define TOKEN_EOF -5
#define TOKEN_NULL -5
#define TOKEN_OP -6
// &time
#define TOKEN_NUMBER -7
#define TOKEN_IDENTIFIER -8

class TokenMap;
class Scanner {
      public:
	static mString inttos(int L);
	mString escape(const mString &);
	static mString literal(const mString & STR);
	  Scanner();
	  virtual ~ Scanner();
	void AddSeparator(char sep, char second = 0);
	void AddBlank(char blank);
	int BeginParsing(ParserStream * P);
	int EndParsing();
	int Empty();
	void AddToken(const mString & symbol, int token);
	void SetStringDelimiters(char, char);
	inline int GetNextToken() {
		return Token(GetNextWord());
	};
	bool Eof();
	virtual mString GetNextWord();
	mString GetLastString();
	mString GetLastWord() {
		return LastWord;
	};
	void PushBackWord() {
		ReadNewWord = false;
	};
	void PushBackChar() {
		if (PStream)
			PStream->PushBackChar();
	};
	int GetLineNumber() {
		if (PStream)
			return PStream->GetLine();
		else
			return 0;
	};
	mString GetStreamName();

	ParserStream *FindStream(const mString & name);
      protected:
	virtual int Token(const mString & Word) const;
	virtual char GetNextChar();
	bool IsSeparator(char, char second = 0);
	bool BeginsSeparator(char);
	bool IsBlank(char);
	virtual bool IsIdentifier(const mString & Word) const;
	mString GetSymbol(int token);
      protected:
	mString LastWord;
	bool ReadNewWord;
      private:
	short int NSep;
	unsigned short int Separators[1024];
	short int NBlanks;
	char Blanks[256];
	TokenMap *Map;
	char LastChar;
	ParserStream *PStream;
	list < ParserStream * >streams;
	char StringDelimiterBegin;
	char StringDelimiterEnd;
};

#endif
