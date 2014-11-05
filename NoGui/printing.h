#ifndef PRINTING_H
#define	PRINTING_H

#ifdef DEBUG
#define DBP(x) std::cout << x;
#define DBPL(x) std::cout << x << std::endl;
#else
#define DBP(x);
#define DBPL(x);
#endif




#define PRINTLINE(x) std::cout << x << std::endl;
#define PRINT(x) std::cout << x;

#endif	/* PRINTING_H */