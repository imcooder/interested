#include "TxtParser.h"
#include <jni.h>
#include "../Ext_Type.h"
#include "../HanvonUtil.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
enum
{
  ENCODING_UNKNOWN,
  ENCODING_ASCII,
  ENCODING_LATIN1,
  ENCODING_UTF8,
  ENCODING_UTF16BE,
  ENCODING_UTF16LE,
  ENCODING_UTF32BE,
  ENCODING_UTF32LE,
  ENCODING_UCS2, /* Native byte order assumed */
  ENCODING_UCS4,
/* Native byte order assumed */
};

#define UNKNOWN_ASCII	'?'
#define UNKNOWN_UNICODE	0xFFFD

enum
{
  bit7 = 0x80,
  bit6 = 0x40,
  bit5 = 0x20,
  bit4 = 0x10,
  bit3 = 8,
  bit2 = 4,
  bit1 = 2,
  bit0 = 1
};

long
Hanvoniconv(int from, int to, const unsigned char *inbuf, unsigned int inbytesleft,
    unsigned char *outbuf, unsigned int outbytesleft)
{
  const unsigned char *src = NULL;
  const unsigned char *dst = NULL;
  unsigned int srclen, dstlen;
  int ch = 0;
  unsigned int total = 0;

  if (!inbuf)
    {
      /* Reset the context */
      return 0;
    }
  if (!outbuf || !outbytesleft)
    {
      return 0;
    }
  src = inbuf;
  srclen = inbytesleft;
  dst = outbuf;
  dstlen = outbytesleft;

  total = 0;
  while (srclen > 0)
    {
      /* Decode a character */
      switch (from)
        {
      case ENCODING_ASCII:
        {
          unsigned char *p = (unsigned char *) src;
          ch = (unsigned int) (p[0] & 0x7F);
          ++src;
          --srclen;
        }
        break;
      case ENCODING_LATIN1:
        {
          unsigned char *p = (unsigned char *) src;
          ch = (unsigned int) p[0];
          ++src;
          --srclen;
        }
        break;
      case ENCODING_UTF8: /* RFC 3629 */
        {
          unsigned char *p = (unsigned char *) src;
          unsigned char a = 0, b = 0, c = 0;

          a = *p++;
          src++;
          --srclen;
          if ((a & (bit7 | bit6 | bit5)) == (bit7 | bit6))
            { // 0x000080-0x0007ff
                {
                  b = *p++;
                  src++;
                  --srclen;
                  ch = (unsigned short) (((a & (bit4 | bit3 | bit2)) >> 2)
                      * 0x100);
                  ch += (unsigned short) (((a & (bit1 | bit0)) << 6) | (b
                      & (bit5 | bit4 | bit3 | bit2 | bit1 | bit0)));
                }
            }
          else if ((a & (bit7 | bit6 | bit5 | bit4)) == (bit7 | bit6 | bit5)) // 0x000800-0x00ffff
            {
              b = *p++;
              src++;
              --srclen;
              c = *p++;
              src++;
              --srclen;
              ch = (unsigned short) ((((a & (bit3 | bit2 | bit1 | bit0)) << 4)
                  | ((b & (bit5 | bit4 | bit3 | bit2)) >> 2)) * 0x100);
              ch += (unsigned short) (((b & (bit1 | bit0)) << 6) | (c & (bit5
                  | bit4 | bit3 | bit2 | bit1 | bit0)));
            }
          else
            {
              ch = a;
            }
        }
        break;
      case ENCODING_UTF16BE: /* RFC 2781 */
        {
          unsigned char *p = (unsigned char *) src;
          unsigned short W1, W2;
          if (srclen >= 2)
            {
              W1 = ((unsigned short) p[0] << 8) | (unsigned short) p[1];
              src += 2;
              srclen -= 2;
              if (W1 < 0xD800 || W1 > 0xDFFF)
                {
                  ch = (unsigned int) W1;
                }
              else
                {
                  if (W1 <= 0xDBFF)
                    {
                      if (srclen >= 2)
                        {
                          p = (unsigned char *) src;
                          W2 = ((unsigned short) p[0] << 8)
                              | (unsigned short) p[1];
                          src += 2;
                          srclen -= 2;
                          if (!(W2 < 0xDC00 || W2 > 0xDFFF))
                            {
                              ch = (((unsigned int) (W1 & 0x3FF) << 10)
                                  | (unsigned int) (W2 & 0x3FF)) + 0x10000;
                            }
                          else
                            {
                              ch = UNKNOWN_UNICODE;
                            }

                        }
                    }

                }

            }
        }

        break;
      case ENCODING_UTF16LE: /* RFC 2781 */
        {
          unsigned char *p = (unsigned char *) src;
          unsigned short W1 = 0, W2 = 0;
          if (srclen >= 2)
            {
              W1 = ((unsigned short) p[1] << 8) | (unsigned short) p[0];
              src += 2;
              srclen -= 2;
              if (W1 < 0xD800 || W1 > 0xDFFF)
                {
                  ch = (unsigned int) W1;
                }
              else
                {
                  if (W1 <= 0xDBFF)
                    {
                      if (srclen >= 2)
                        {
                          p = (unsigned char *) src;
                          W2 = ((unsigned short) p[1] << 8)
                              | (unsigned short) p[0];
                          src += 2;
                          srclen -= 2;
                          if ((!W2 < 0xDC00 || W2 > 0xDFFF))
                            {
                              ch = (((unsigned int) (W1 & 0x3FF) << 10)
                                  | (unsigned int) (W2 & 0x3FF)) + 0x10000;
                            }
                        }
                      else
                        {
                          src += srclen;
                          srclen -= srclen;
                        }
                    }
                }
            }
          else
            {
              src += srclen;
              srclen -= srclen;
            }
        }

        break;
      case ENCODING_UTF32BE:
        {
          unsigned char *p = (unsigned char *) src;
          if (srclen >= 4)
            {
              ch = ((unsigned int) p[0] << 24) | ((unsigned int) p[1] << 16)
                  | ((unsigned int) p[2] << 8) | (unsigned int) p[3];
              src += 4;
              srclen -= 4;
            }
          else
            {
              src += srclen;
              srclen -= srclen;
            }
        }
        break;
      case ENCODING_UTF32LE:
        {
          unsigned char *p = (unsigned char *) src;
          if (srclen >= 4)
            {
              ch = ((unsigned int) p[3] << 24) | ((unsigned int) p[2] << 16)
                  | ((unsigned int) p[1] << 8) | (unsigned int) p[0];
              src += 4;
              srclen -= 4;
            }
          else
            {
              src += srclen;
              srclen -= srclen;
            }

        }
        break;
      case ENCODING_UCS2:
        {
          unsigned short *p = (unsigned short *) src;
          if (srclen >= 2)
            {
              ch = *p;
              src += 2;
              srclen -= 2;
            }
          else
            {
              src += srclen;
              srclen -= srclen;
            }
        }
        break;
      case ENCODING_UCS4:
        {
          unsigned int *p = (unsigned int *) src;
          if (srclen >= 4)
            {
              ch = *p;
              src += 4;
              srclen -= 4;
            }
          else
            {
              src += srclen;
              srclen -= srclen;
            }
        }
        break;
        }

      /* Encode a character */
      switch (to)
        {
      case ENCODING_ASCII:
        {
          unsigned char *p = (unsigned char *) dst;
          if (dstlen >= 1)
            {
              if (ch <= 0x7F)
                {
                  *p = (unsigned char) ch;
                }
              else
                {

                }
            }
          ++dst;
          --dstlen;
        }
        break;
      case ENCODING_LATIN1:
        {
          unsigned char *p = (unsigned char *) dst;
          if (dstlen >= 1)
            {
              *p = (unsigned char) ch;
            }
          ++dst;
          --dstlen;
        }
        break;
      case ENCODING_UTF8: /* RFC 3629 */
        {
          unsigned char *p = (unsigned char *) dst;
          if (ch > 0x10FFFF)
            {
              ch = UNKNOWN_UNICODE;
            }
          if (ch <= 0x7F)
            {
              if (dstlen >= 1)
                {
                  *p = (unsigned char) ch;
                  ++dst;
                  --dstlen;
                }
            }
          else if (ch <= 0x7FF)
            {
              if (dstlen >= 2)
                {
                  p[0] = 0xC0 | (unsigned char) ((ch >> 6) & 0x1F);
                  p[1] = 0x80 | (unsigned char) (ch & 0x3F);
                  dst += 2;
                  dstlen -= 2;
                }

            }
          else if (ch <= 0xFFFF)
            {
              if (dstlen >= 3)
                {
                  p[0] = 0xE0 | (unsigned char) ((ch >> 12) & 0x0F);
                  p[1] = 0x80 | (unsigned char) ((ch >> 6) & 0x3F);
                  p[2] = 0x80 | (unsigned char) (ch & 0x3F);
                  dst += 3;
                  dstlen -= 3;
                }

            }
          else if (ch <= 0x1FFFFF)
            {
              if (dstlen >= 4)
                {
                  p[0] = 0xF0 | (unsigned char) ((ch >> 18) & 0x07);
                  p[1] = 0x80 | (unsigned char) ((ch >> 12) & 0x3F);
                  p[2] = 0x80 | (unsigned char) ((ch >> 6) & 0x3F);
                  p[3] = 0x80 | (unsigned char) (ch & 0x3F);
                  dst += 4;
                  dstlen -= 4;
                }
            }
          else if (ch <= 0x3FFFFFF)
            {
              if (dstlen >= 5)
                {
                  p[0] = 0xF8 | (unsigned char) ((ch >> 24) & 0x03);
                  p[1] = 0x80 | (unsigned char) ((ch >> 18) & 0x3F);
                  p[2] = 0x80 | (unsigned char) ((ch >> 12) & 0x3F);
                  p[3] = 0x80 | (unsigned char) ((ch >> 6) & 0x3F);
                  p[4] = 0x80 | (unsigned char) (ch & 0x3F);
                  dst += 5;
                  dstlen -= 5;
                }
            }
          else
            {
              if (dstlen >= 6)
                {
                  p[0] = 0xFC | (unsigned char) ((ch >> 30) & 0x01);
                  p[1] = 0x80 | (unsigned char) ((ch >> 24) & 0x3F);
                  p[2] = 0x80 | (unsigned char) ((ch >> 18) & 0x3F);
                  p[3] = 0x80 | (unsigned char) ((ch >> 12) & 0x3F);
                  p[4] = 0x80 | (unsigned char) ((ch >> 6) & 0x3F);
                  p[5] = 0x80 | (unsigned char) (ch & 0x3F);
                  dst += 6;
                  dstlen -= 6;
                }
            }
        }
        break;
      case ENCODING_UTF16BE: /* RFC 2781 */
        {
          unsigned char *p = (unsigned char *) dst;
          if (ch > 0x10FFFF)
            {
              ch = UNKNOWN_UNICODE;
            }
          if (ch < 0x10000)
            {
              if (dstlen >= 2)
                {
                  p[0] = (unsigned char) (ch >> 8);
                  p[1] = (unsigned char) ch;
                  dst += 2;
                  dstlen -= 2;
                }
            }
          else
            {
              unsigned short W1, W2;
              if (dstlen >= 4)
                {
                  ch = ch - 0x10000;
                  W1 = 0xD800 | (unsigned short) ((ch >> 10) & 0x3FF);
                  W2 = 0xDC00 | (unsigned short) (ch & 0x3FF);
                  p[0] = (unsigned char) (W1 >> 8);
                  p[1] = (unsigned char) W1;
                  p[2] = (unsigned char) (W2 >> 8);
                  p[3] = (unsigned char) W2;
                  dst += 4;
                  dstlen -= 4;
                }
            }
        }
        break;
      case ENCODING_UTF16LE: /* RFC 2781 */
        {
          unsigned char *p = (unsigned char *) dst;
          if (ch > 0x10FFFF)
            {
              ch = UNKNOWN_UNICODE;
            }
          if (ch < 0x10000)
            {
              if (dstlen >= 2)
                {
                  p[1] = (unsigned char) (ch >> 8);
                  p[0] = (unsigned char) ch;
                  dst += 2;
                  dstlen -= 2;
                }
            }
          else
            {
              unsigned short W1, W2;
              if (dstlen >= 4)
                {
                  ch = ch - 0x10000;
                  W1 = 0xD800 | (unsigned short) ((ch >> 10) & 0x3FF);
                  W2 = 0xDC00 | (unsigned short) (ch & 0x3FF);
                  p[1] = (unsigned char) (W1 >> 8);
                  p[0] = (unsigned char) W1;
                  p[3] = (unsigned char) (W2 >> 8);
                  p[2] = (unsigned char) W2;
                  dst += 4;
                  dstlen -= 4;
                }
            }
        }
        break;
      case ENCODING_UTF32BE:
        {
          unsigned char *p = (unsigned char *) dst;
          if (ch > 0x10FFFF)
            {
              ch = UNKNOWN_UNICODE;
            }
          if (dstlen >= 4)
            {
              p[0] = (unsigned char) (ch >> 24);
              p[1] = (unsigned char) (ch >> 16);
              p[2] = (unsigned char) (ch >> 8);
              p[3] = (unsigned char) ch;
              dst += 4;
              dstlen -= 4;
            }
        }
        break;
      case ENCODING_UTF32LE:
        {
          unsigned char *p = (unsigned char *) dst;
          if (ch > 0x10FFFF)
            {
              ch = UNKNOWN_UNICODE;
            }
          if (dstlen >= 4)
            {
              p[3] = (unsigned char) (ch >> 24);
              p[2] = (unsigned char) (ch >> 16);
              p[1] = (unsigned char) (ch >> 8);
              p[0] = (unsigned char) ch;
              dst += 4;
              dstlen -= 4;
            }
        }
        break;
      case ENCODING_UCS2:
        {
          unsigned short *p = (unsigned short *) dst;
          if (ch > 0xFFFF)
            {
              ch = UNKNOWN_UNICODE;
            }
          if (dstlen >= 2)
            {
              *p = (unsigned short) ch;
              dst += 2;
              dstlen -= 2;
            }
        }
        break;
      case ENCODING_UCS4:
        {
          unsigned int *p = (unsigned int *) dst;
          if (ch > 0x7FFFFFFF)
            {
              ch = UNKNOWN_UNICODE;
            }
          if (dstlen >= 4)
            {
              *p = ch;
              dst += 4;
              dstlen -= 4;
            }
        }
        break;
        }

      /* Update state */
      ++total;
    }
  return total;
}

void
Java_org_parser_txt_TxtParser_Parser(JNIEnv *env, jobject thiz,
    jobject thisParser, jstring strPath)
{
  LOGD("Enter TxtParser_Parser\n");
  jmethodID startElementID = NULL;
  jmethodID endElementID = NULL;
  jmethodID characterDataID = NULL;
  jclass clsParser = env->GetObjectClass(thisParser);

  if (clsParser)
    {
      startElementID
          = env->GetMethodID(clsParser, "startElementHandler", "()V");
      if (!startElementID)
        {
          LOGD("GetMethodID startElementHandler failed\n");
        }
      endElementID = env->GetMethodID(clsParser, "endElementHandler", "()V");
      if (!endElementID)
        {
          LOGD("GetMethodID endElementID failed\n");
        }
      characterDataID = env->GetMethodID(clsParser, "characterDataHandler",
          "(Ljava/lang/String;)V");
      if (!characterDataID)
        {
          LOGD("GetMethodID characterDataID failed\n");
        }
    }
  else
    {
      LOGD("GetObjectClass failed\n");
    }
  if (!startElementID || !endElementID || !characterDataID)
    {
      return;
    }
  if (startElementID)
    {
      env->CallVoidMethod(thisParser, startElementID);

    }
  long nFrom = ENCODING_ASCII;
  char szFilePath[MAX_PATH * 2] =
    { 0 };
  long nPathLen = env->GetStringUTFLength(strPath);
  LOGD("Enter 1国家\n");
  env->GetStringUTFRegion(strPath, 0, nPathLen, szFilePath);
  LOGD("Enter 2\n"); LOGD(szFilePath);
  FILE* pFile = NULL;
  long nFileOffset = 0;
  unsigned short ashtUnicodeBuffer[2048] =
    { 0 };
  pFile = fopen(szFilePath, "r");
  if (pFile)
    {
      const long BUFSIZE = 2048;
      unsigned char *buffer = new unsigned char[BUFSIZE];
      //std::string str;
      long nLength = 0;
      if (startElementID)
        {
          env->CallVoidMethod(thisParser, startElementID);
        }
      do
        {
          nLength = fread(buffer, sizeof(char), BUFSIZE, pFile);
          unsigned char *start = buffer;
          if (!nFileOffset)
            {
              if (nLength >= 2 && (buffer[0] == 0xFF && buffer[1] == 0xFE))
                {
                  nFrom = ENCODING_UTF16LE;
                  nFileOffset += 2;
                }
              else if (nLength >= 2 && (buffer[0] == 0xFE && buffer[1] == 0xFF))
                {
                  nFrom = ENCODING_UTF16BE;
                  nFileOffset += 2;
                }
              else if (nLength >= 3 && (buffer[0] == 0xEF && buffer[1] == 0xBB
                  && buffer[2] == 0xBF))
                {
                  nFrom = ENCODING_UTF8;
                  nFileOffset += 3;
                }
              else if (nLength >= 4 && (buffer[0] == 0xFF && buffer[1] == 0xFE
                  && buffer[2] == 0x00 && buffer[3] == 0x00))
                {
                  nFrom = ENCODING_UTF32LE;
                  nFileOffset += 4;
                }
              else if (nLength >= 4 && (buffer[0] == 0x00 && buffer[1] == 0x00
                  && buffer[2] == 0xFE && buffer[3] == 0xFF))
                {
                  nFrom = ENCODING_UTF32BE;
                  nFileOffset += 4;
                }
              start += nFileOffset;
            }
          //LOGD(start);
          const unsigned char *end = buffer + nLength;
          for (unsigned char *ptr = start; ptr != end; ++ptr)
            {
              if (*ptr == '\n' || *ptr == '\r')
                {
                  bool skipNewLine = false;
                  if (*ptr == '\r' && (ptr + 1) != end && *(ptr + 1) == '\n')
                    {
                      skipNewLine = true;
                      *ptr = '\n';
                    }
                  if (start != ptr)
                    {
                      //str.erase();
                      //myConverter->convert(str, start, ptr + 1);
                      //characterDataHandler(str);
                      ashtUnicodeBuffer[0] = 0;
                      ZeroMemory(ashtUnicodeBuffer, sizeof(ashtUnicodeBuffer));
                      long nLen = Hanvoniconv(nFrom, ENCODING_UTF16LE, start, ptr + 1 - start, (unsigned char*)ashtUnicodeBuffer, sizeof(ashtUnicodeBuffer) - 10);
                       if (nLen > 0)
                        {
                          char szCode[500] =
                            { 0 };
                          strcpy(szCode, "guojiadeshiqijie 汉王科技");
                          jstring strCode = env->NewString(ashtUnicodeBuffer,
                              nLen / 2);
                          if (characterDataID)
                            {
                              env->CallVoidMethod(thisParser, characterDataID,
                                  strCode);
                            }
                        }
                      //LOGD(szUnicode);
                    }
                  if (skipNewLine)
                    {
                      ++ptr;
                    }
                  start = ptr + 1;
                  if (endElementID)
                    {
                      env->CallVoidMethod(thisParser, endElementID);
                    }
                  if (startElementID)
                    {
                      env->CallVoidMethod(thisParser, startElementID);
                    }
                  //newLineHandler();
                }
              else if (0/*isspace((unsigned char) *ptr)*/)
                {
                  if (*ptr != '\t')
                    {
                      *ptr = ' ';
                    }
                }
              else
                {
                }
            }
          if (start != end)
            {
              //str.erase();
              //myConverter->convert(str, start, end);
              //characterDataHandler(str);
            }
        }
      while (nLength == BUFSIZE);
      delete[] buffer;
      buffer = NULL;
      //endDocumentHandler();
      fclose(pFile);
      pFile = NULL;
      //stream.close();

    }
  else
    {
      LOGD("TxtParser_Parser:file open failed\n");
    }

  LOGD("Leave TxtParser_Parser\n");
}
