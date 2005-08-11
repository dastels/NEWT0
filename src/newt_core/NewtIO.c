/*------------------------------------------------------------------------*/
/**
 * @file	NewtIO.c
 * @brief   入出力処理
 *
 * @author M.Nukui
 * @date 2004-06-05
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* ヘッダファイル */
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef __WIN32__
	#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
#endif


#include "NewtCore.h"
#include "NewtIO.h"


/*------------------------------------------------------------------------*/
/** 入出力ストリーム構造体にファイルの情報をセットする
 *
 * @param stream	[out]入出力ストリーム
 * @param f			[in] ファイル
 *
 * @return			なし
 */

void NIOSetFile(newtStream_t * stream, FILE * f)
{
	stream->file = f;

	if (f == stdout)
		stream->obj = NcGetGlobalVar(NSSYM0(_STDOUT_));
	else if (f == stderr)
		stream->obj = NcGetGlobalVar(NSSYM0(_STDERR_));
	else
		stream->obj = kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** printf フォーマットで出力する（不定長）
 *
 * @param stream	[in] 出力ストリーム
 * @param format	[in] フォーマット
 * @param ...		[in] printf 引数
 *
 * @return			printf の戻り値
 *
 * @note			newtStream_t を使用
 */

int NIOFprintf(newtStream_t * stream, const char * format, ...)
{
	va_list	args;
	int		result;

	va_start(args, format);
	result = NIOVfprintf(stream, format, args);
	va_end(args);

	return result;
}


/*------------------------------------------------------------------------*/
/** vprintf フォーマットで出力する
 *
 * @param stream	[in] 出力ストリーム
 * @param format	[in] フォーマット
 * @param ap		[in] vprintf 引数
 *
 * @return			vprintf の戻り値
 *
 * @note			newtStream_t を使用
 *					文字列に追加する場合の制限 NEWT_SNPRINTF_BUFFSIZE (NewtConf.h)
 */

int NIOVfprintf(newtStream_t * stream, const char * format, va_list ap)
{
	int		result = 0;

	if (NewtRefIsString(stream->obj))
	{
		char	wk[NEWT_SNPRINTF_BUFFSIZE];

		result = vsnprintf(wk, sizeof(wk), format, ap);

		if (0 < result)
		{
			if (sizeof(wk) < result)
				wk[sizeof(wk) - 1] = '\0';

			NewtStrCat(stream->obj, wk);
		}
	}
	else
	{
		result = vfprintf(stream->file, format, ap);
	}

	return result;
}


/*------------------------------------------------------------------------*/
/** 文字を出力する
 *
 * @param c			[in] 文字
 * @param stream	[in] 出力ストリーム
 *
 * @return			fputc の戻り値
 *
 * @note			newtStream_t を使用
 */

int NIOFputc(int c, newtStream_t * stream)
{
	int		result = 0;

	if (NewtRefIsString(stream->obj))
	{
		char	wk[4];

		sprintf(wk, "%c", c);
		NewtStrCat(stream->obj, wk);
	}
	else
	{
		result = fputc(c, stream->file);
	}

	return result;
}


/*------------------------------------------------------------------------*/
/** 文字列を出力する
 *
 * @param str		[in] 文字列
 * @param stream	[in] 出力ストリーム
 *
 * @return			fputs の戻り値
 *
 * @note			newtStream_t を使用
 */

int NIOFputs(const char *str, newtStream_t * stream)
{
	int		result = 0;

	if (NewtRefIsString(stream->obj))
	{
		NewtStrCat(stream->obj, (char *)str);
	}
	else
	{
		result = fputs(str, stream->file);
	}

	return result;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** printf フォーマットで出力する（不定長）
 *
 * @param f			[in] 出力ストリーム
 * @param format	[in] フォーマット
 * @param ...		[in] printf 引数
 *
 * @return			printf の戻り値
 */

int NewtFprintf(FILE * f, const char * format, ...)
{
	newtStream_t	stream;
	va_list	args;
	int		result;

	NIOSetFile(&stream, f);

	va_start(args, format);
	result = NIOVfprintf(&stream, format, args);
	va_end(args);

	return result;
}


/*------------------------------------------------------------------------*/
/** 文字の出力
 *
 * @param c			[in] 文字
 * @param f			[in] 出力ストリーム
 *
 * @return			fputc の戻り値
 */

int NewtFputc(int c, FILE * f)
{
	newtStream_t	stream;

	NIOSetFile(&stream, f);
	return NIOFputc(c, &stream);
}


/*------------------------------------------------------------------------*/
/** 文字の出力
 *
 * @param str		[in] 文字列
 * @param f			[in] 出力ストリーム
 *
 * @return			fputs の戻り値
 */

int NewtFputs(const char *str, FILE * f)
{
	newtStream_t	stream;

	NIOSetFile(&stream, f);
	return NIOFputs(str, &stream);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** printf フォーマットで出力する（不定長）
 *
 * @param title		[in] タイトル
 * @param format	[in] フォーマット
 * @param ...		[in] printf 引数
 *
 * @return			printf の戻り値
 */

int NewtDebugMsg(const char * title, const char * format, ...)
{
	newtStream_t	stream;
	va_list	args;
	int		result;

	NIOSetFile(&stream, stderr);

	if (title != NULL)
	{
		NIOFputs("[", &stream);
		NIOFputs(title, &stream);
		NIOFputs("] ", &stream);
	}

	va_start(args, format);
	result = NIOVfprintf(&stream, format, args);
	va_end(args);

	return result;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** 入力ストリームから文字列を取出す
 *
 * @param stream	[in] 入力ストリーム
 *
 * @retval			文字列オブジェクト	入力データが存在する場合
 * @retval			NIL				入力データが存在しない場合
 *
 * @note			制限 NEWT_FGETS_BUFFSIZE (NewtConf.h)
 */

newtRef NewtFgets(FILE * stream)
{
	newtRefVar  result = kNewtRefNIL;
	char	buff[NEWT_FGETS_BUFFSIZE];
	char *  str;
	char	c;
	int		maxsize;
	int		oldlen;
	int		len;

	maxsize = sizeof(buff) - 1;

	while (str = fgets(buff, sizeof(buff), stream))
	{
		len = strlen(str);

		if (result == kNewtRefNIL)
		{	// 文字列オブジェクト作成
			result = NewtMakeString2(str, len, false);

			if (NewtRefIsNIL(result))
			{	// メモリを確保できなかった
				return NewtThrow0(kNErrOutOfObjectMemory);
			}
		}
		else
		{	// 追加
			oldlen = NewtStringLength(result);
			result = NewtStrCat2(result, str, len);

			if (NewtStringLength(result) < oldlen + len)
			{	// メモリを確保できなかった
				return NewtThrow0(kNErrOutOfObjectMemory);
			}
		}

		if (len < maxsize)
			break;

		// 最後の文字をチェック
		c = buff[maxsize - 1];

		if (c == '\n')
			break;

		if (c == '\r')
		{
			// １文字先読み
			c = fgetc(stream);

			if (c != '\n')
			{	// CRLF でない（CR のみ）場合
				// 先読みした文字をストリームに戻す
				ungetc(c, stream);
				break;
			}
		}
	}

    return result;
}


/*------------------------------------------------------------------------*/
/** 標準入力から文字列を取出す
 *
 * @param rcvr		[in] レシーバ
 *
 * @retval			文字列オブジェクト	入力データが存在する場合
 * @retval			NIL				入力データが存在しない場合
 */

newtRef NsGets(newtRefArg rcvr)
{
	return NewtFgets(stdin);
}


/*------------------------------------------------------------------------*/
/** 入力ストリームから文字を取出す
 *
 * @param stream	[in] 入力ストリーム
 *
 * @retval			文字オブジェクト	入力データが存在する場合
 * @retval			NIL				EOF
 */

newtRef NewtFgetc(FILE * stream)
{
	int		c;

	c = fgetc(stream);

	if (c == EOF)
		return kNewtRefNIL;
	else
		return NewtMakeCharacter(c);
}


/*------------------------------------------------------------------------*/
/** 標準入力から文字を取出す
 *
 * @param rcvr		[in] レシーバ
 *
 * @retval			文字オブジェクト	入力データが存在する場合
 * @retval			NIL				入力データが存在しない場合
 */

newtRef NsGetc(newtRefArg rcvr)
{
	return NewtFgetc(stdin);
}


/*------------------------------------------------------------------------*/
/** キーボードから入力文字を１文字取得
 *
 * @param rcvr		[in] レシーバ
 *
 * @retval			文字オブジェクト	入力データが存在する場合
 * @retval			NIL				入力データが存在しない場合
 */

#ifdef __WIN32__

newtRef NsGetch(newtRefArg rcvr)
{
	int		c;

	c = getch();

	if (c)
		return NewtMakeCharacter(c);
	else
		return kNewtRefNIL;
}

#else

newtRef NsGetch(newtRefArg rcvr)
{
	struct termios tios_save;
	struct termios tios;
	int		fd;
	int		c = 0;
	char	buf[1];

	fd = 0;	// STDIN

	if (tcgetattr(fd, &tios_save) == -1)
		return NewtThrow(kNErrSystemError, NewtRefToInteger(errno));

	tios = tios_save;

	tios.c_lflag &= ~ (ICANON | ECHO);
    tios.c_cc[VTIME] = 0;
    tios.c_cc[VMIN] = 1;
	tcsetattr(fd, TCSANOW, &tios);

	if (0 < read(fd, buf, sizeof(buf)))
		c = buf[0];

	tcsetattr(fd, TCSANOW, &tios_save);

	if (c)
		return NewtMakeCharacter(c);
	else
		return kNewtRefNIL;
}

#endif
