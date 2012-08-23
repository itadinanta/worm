#ifndef __LOG_H
#define __LOG_H

#include <ostream>

namespace model {
	using namespace std;
	typedef enum {LL0, LL1, LL2, LL3, LL4, LL5, LL6, LL7, LL8, LL9} LogLevel;
	const LogLevel LLNone=LL0;
	const LogLevel LLError=LL1;
	const LogLevel LLWarn=LL3;
	const LogLevel LLInfo=LL7;
	const LogLevel LLDebug=LL9;
	class Logger {
		LogLevel maxLevel;
		LogLevel currentLevel;
		ostream *out;
		Logger();
	public:
		Logger(LogLevel level, ostream &outStream): 
			out(&outStream),
			maxLevel(level),
			currentLevel(LL9) {};
		void setStream(ostream &outStream) {
			out=&outStream;
		}
		Logger &operator<<(LogLevel LL) {
			currentlevel(LL);
			return *this;
		}
		inline void maxlevel(LogLevel l) {maxLevel=l;}
		inline LogLevel maxlevel() {return maxLevel;}
		inline void currentlevel(LogLevel l) {currentLevel=l;}
		inline LogLevel currentlevel() {return currentLevel;}
		inline ostream& outStream() {return *out;}
	};

	template <class T>
        Logger &operator << (Logger &L, const T&dato) {
		if (L.currentlevel()<=L.maxlevel()) L.outStream()<<dato;
		return L;
	};

	extern Logger log;
};

#endif
