// Copyright 2015-2023 MY.GAMES. All Rights Reserved.

#if WITH_CIVET

#include "PsWebServerUtils.h"
#include "PsWebServerDefines.h"

#include "Containers/StringConv.h"

namespace PsWebServerUtils
{

namespace Impl
{

/** Size of the one data chunk */
static constexpr int32 BufferLen = 10240;

/** Iterator for the request connection data */
class ReqeustConnectionDataIterator
{
public:
	ReqeustConnectionDataIterator(mg_connection* InRequestConnection)
		: RequestConnection(InRequestConnection)
	{
		check(InRequestConnection);
		ReadChunk();
	}

	/** Move iterator to the next byte */
	void operator++()
	{
		return Next();
	}

	/** Whether the iterator points to the end of data */
	bool IsEnd() const
	{
		return bEnd;
	}

	/** Get current byte */
	uint8 operator*()
	{
		check(!bEnd);
		return Buffer[CurrentIdx];
	}

private:
	mg_connection* RequestConnection;

	/** Move iterator to the next byte */
	void Next()
	{
		check(!bEnd);
		check(CurrentIdx < ReadCount);

		if (CurrentIdx == ReadCount - 1)
		{
			ReadChunk();
		}
		else
		{
			++CurrentIdx;
		}
	}

	/** Read next data chunk from the connection */
	void ReadChunk()
	{
		ReadCount = mg_read(RequestConnection, Buffer, BufferLen);
		if (ReadCount <= 0)
		{
			if (ReadCount < 0)
			{
				UE_LOG(LogPwsAll, Error, TEXT("%s: cannot read data from the connection"), *PS_FUNC_LINE);
			}

			bEnd = true;
			return;
		}

		CurrentIdx = 0;
	}

	/** Data buffer */
	uint8 Buffer[BufferLen];

	/** Index of the current byte in the buffer */
	int32 CurrentIdx = 0;

	/** Count of the available bytes in the buffer */
	int32 ReadCount = 0;

	/** Data end flag */
	bool bEnd = false;
};

/** Get current byte and move iterator to the next one */
uint32 GetOctet(ReqeustConnectionDataIterator& Itr)
{
	const uint32 Ret = static_cast<uint32>(*Itr);
	++Itr;
	return Ret;
}

/** How many bytes are necessary for read UTF-8 symbol with given first byte
 *  @See FUTF8ToTCHAR_Convert::CodepointFromUtf8
 */
uint8 Utf8SymbolBytesCount(uint8 FirstByte)
{
	const uint32 Octet = static_cast<int32>(FirstByte);

	if (Octet < 192) // one octet
	{
		return 1;
	}
	else if (Octet < 224) // two octets
	{
		return 2;
	}
	else if (Octet < 240) // three octets
	{
		return 3;
	}
	else if (Octet < 248) // four octets
	{
		return 4;
	}
	else if (Octet < 252) // five octets
	{
		return 5;
	}
	else // six octets
	{
		return 6;
	}
}

// clang-format off

/** Just a copy of unreal CodepointFromUtf8 function
 *  @See FUTF8ToTCHAR_Convert::CodepointFromUtf8
 */
uint32 CodepointFromUtf8(const ANSICHAR*& SourceString, const uint32 SourceLengthRemaining)
{
	checkSlow(SourceLengthRemaining > 0)

	const ANSICHAR* OctetPtr = SourceString;

	uint32 Codepoint = 0;
	uint32 Octet = (uint32) ((uint8) *SourceString);
	uint32 Octet2, Octet3, Octet4;

	if (Octet < 128)  // one octet char: 0 to 127
	{
		++SourceString;  // skip to next possible start of codepoint.
		return Octet;
	}
	else if (Octet < 192)  // bad (starts with 10xxxxxx).
	{
		// Apparently each of these is supposed to be flagged as a bogus
		//  char, instead of just resyncing to the next valid codepoint.
		++SourceString;  // skip to next possible start of codepoint.
		return UNICODE_BOGUS_CHAR_CODEPOINT;
	}
	else if (Octet < 224)  // two octets
	{
		// Ensure our string has enough characters to read from
		if (SourceLengthRemaining < 2)
		{
			// Skip to end and write out a single char (we always have room for at least 1 char)
			SourceString += SourceLengthRemaining;
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet -= (128+64);
		Octet2 = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet2 & (128 + 64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Codepoint = ((Octet << 6) | (Octet2 - 128));
		if ((Codepoint >= 0x80) && (Codepoint <= 0x7FF))
		{
			SourceString += 2;  // skip to next possible start of codepoint.
			return Codepoint;
		}
	}
	else if (Octet < 240)  // three octets
	{
		// Ensure our string has enough characters to read from
		if (SourceLengthRemaining < 3)
		{
			// Skip to end and write out a single char (we always have room for at least 1 char)
			SourceString += SourceLengthRemaining;
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet -= (128+64+32);
		Octet2 = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet2 & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet3 = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet3 & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Codepoint = ( ((Octet << 12)) | ((Octet2-128) << 6) | ((Octet3-128)) );

		// UTF-8 characters cannot be in the UTF-16 surrogates range
		if (StringConv::IsHighSurrogate(Codepoint) || StringConv::IsLowSurrogate(Codepoint))
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		// 0xFFFE and 0xFFFF are illegal, too, so we check them at the edge.
		if ((Codepoint >= 0x800) && (Codepoint <= 0xFFFD))
		{
			SourceString += 3;  // skip to next possible start of codepoint.
			return Codepoint;
		}
	}
	else if (Octet < 248)  // four octets
	{
		// Ensure our string has enough characters to read from
		if (SourceLengthRemaining < 4)
		{
			// Skip to end and write out a single char (we always have room for at least 1 char)
			SourceString += SourceLengthRemaining;
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet -= (128+64+32+16);
		Octet2 = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet2 & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet3 = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet3 & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet4 = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet4 & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Codepoint = ( ((Octet << 18)) | ((Octet2 - 128) << 12) |
					 ((Octet3 - 128) << 6) | ((Octet4 - 128)) );
		if ((Codepoint >= 0x10000) && (Codepoint <= 0x10FFFF))
		{
			SourceString += 4;  // skip to next possible start of codepoint.
			return Codepoint;
		}
	}
	// Five and six octet sequences became illegal in rfc3629.
	//  We throw the codepoint away, but parse them to make sure we move
	//  ahead the right number of bytes and don't overflow the buffer.
	else if (Octet < 252)  // five octets
	{
		// Ensure our string has enough characters to read from
		if (SourceLengthRemaining < 5)
		{
			// Skip to end and write out a single char (we always have room for at least 1 char)
			SourceString += SourceLengthRemaining;
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		SourceString += 5;  // skip to next possible start of codepoint.
		return UNICODE_BOGUS_CHAR_CODEPOINT;
	}

	else  // six octets
	{
		// Ensure our string has enough characters to read from
		if (SourceLengthRemaining < 6)
		{
			// Skip to end and write out a single char (we always have room for at least 1 char)
			SourceString += SourceLengthRemaining;
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		Octet = (uint32) ((uint8) *(++OctetPtr));
		if ((Octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
		{
			++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
			return UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		SourceString += 6;  // skip to next possible start of codepoint.
		return UNICODE_BOGUS_CHAR_CODEPOINT;
	}

	++SourceString;  // Sequence was not valid UTF-8. Skip the first byte and continue.
	return UNICODE_BOGUS_CHAR_CODEPOINT;  // catch everything else.
}

// clang-format on

} // namespace Impl

FString GetPostData(mg_connection* RequestConnection)
{
	mg_lock_connection(RequestConnection);

	FString Data;
	Data.Reserve(Impl::BufferLen);

	Impl::ReqeustConnectionDataIterator Itr(RequestConnection);

	const uint8 MaxBytesCount = 6;

	// Buffer to convert
	ANSICHAR Buffer[MaxBytesCount];

	// Buffer to dynamic window of remaining unhandled bytes
	// It is necessary in case of invalaid UTF-8 data
	ANSICHAR RemainingBuffer[MaxBytesCount];

	uint8 BytesInRemainingBuffer = 0;

	while (!Itr.IsEnd() || BytesInRemainingBuffer > 0)
	{
		// Get first byte
		uint8 FirstByte;
		if (BytesInRemainingBuffer > 0)
		{
			FirstByte = RemainingBuffer[0];
		}
		else
		{
			check(!Itr.IsEnd());
			FirstByte = *Itr;
		}

		// Calculate necessary bytes count
		const uint8 BytesCount = FMath::Max(Impl::Utf8SymbolBytesCount(FirstByte), BytesInRemainingBuffer);
		uint8 BytesToGet = BytesCount;

		uint8 Index = 0;

		// Get bytes from the remaining buffer
		if (BytesInRemainingBuffer > 0)
		{
			memcpy(Buffer, RemainingBuffer, BytesInRemainingBuffer);
			Index = BytesInRemainingBuffer;
			BytesToGet -= BytesInRemainingBuffer;
			BytesInRemainingBuffer = 0;
		}

		// Get bytes from the connection
		bool bEnd = false;
		for (auto i = 0; i < BytesToGet; ++i)
		{
			if (Itr.IsEnd())
			{
				bEnd = true;
				break;
			}

			Buffer[Index + i] = *Itr;
			++Itr;
		}

		if (bEnd)
		{
			// There is no enought data from connection
			Data.AppendChar(UNICODE_BOGUS_CHAR_CODEPOINT);
			break;
		}

		const ANSICHAR* Ptr = Buffer;
		const TCHAR Char = Impl::CodepointFromUtf8(Ptr, BytesCount);

		// Calculate unhandled bytes count
		const uint8 HandledBytesCount = Ptr - Buffer;
		const uint8 RemainingBytesCount = BytesCount - HandledBytesCount;

		// Put remaining unhandled bytes in the special buffer
		// It is possible in case of invalaid UTF-8 data
		if (RemainingBytesCount > 0)
		{
			check(RemainingBytesCount <= MaxBytesCount);

			BytesInRemainingBuffer = RemainingBytesCount;
			memcpy(RemainingBuffer, Ptr, RemainingBytesCount);
		}

		Data.AppendChar(Char);
	}

	mg_unlock_connection(RequestConnection);
	return Data;
}

} // namespace PsWebServerUtils

#endif // WITH_CIVET
