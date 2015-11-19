/* Ovladac pro informacni displej puvodne pro Kit s HCS08GB60
 * upraveny pro kit s Kinetis KL25Z.
 * POZOR: chovani parametru "zobraz/invert" je asi opacne, pro 1 zhasne led a pro 0 rozsviti.
 * */

void DispInit(void);
char SetBit(int radek, int sloupec, char zobraz);
void ZobrazZnak(char znak, int radek, int sloupec, char invert);
void ZobrazText(char *text, int radek, int sloupec,char invert);
void Clear(void);
void Fill(void);
