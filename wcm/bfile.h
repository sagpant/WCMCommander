#ifndef BFILE_H
#define BFILE_H

/*
#include <wal.h>

using namespace wal;

class BFile {
   File _f;
   char _buf[0x20000];
   int _size;
   int _pos;
   bool FillBuf(){
      if (_size < sizeof(_buf)) return false;
      _size = _f.Read(_buf, sizeof(_buf));
      _pos = 0;
      return _size > 0;
   }
public:
   BFile():_size(sizeof(_buf)), _pos(sizeof(_buf)){}
   void Open(sys_char_t *s){ _f.Open(s); FillBuf(); }
   void Close(){ _f.Close(); _size = sizeof(_buf); _pos = sizeof(_buf);}
   int GetC(){ return (_pos < _size || FillBuf()) ? _buf[_pos++] : EOF; }

   char *GetStr(char *b, int bSize)
   {
      char *s = b;
      int n = bSize-1;

      if (_pos >= _size && _size < sizeof(_buf))
         return 0;

      for ( ; n>0 ; n--, s++) {
         int c = GetC();
         if (c == EOF) {
            *s=0;
            return b;
         }
         *s = c;
         if (c == '\n') {
            s++;
            *s = 0;
            return b;
         }
      }
      *s = 0;
      while (true) { int c = GetC(); if (c == '\n' || c==EOF) break; }
      return b;
   }

   ~BFile(){}
};
*/

#endif
