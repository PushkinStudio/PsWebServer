// Copyright 2015-2021 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/Atomic.h"
#include "Templates/SharedPointer.h"

/** Request cancellation token */
class FPsWebCancellationToken
{
public:
	/** Get canceled flag */
	bool IsCanceled() const
	{
		const bool bValue = bCanceled;
		return bValue;
	}

private:
	FPsWebCancellationToken()
		: bCanceled(false)
	{
	}

	/** Set canceled flag */
	void Cancel()
	{
		bCanceled = true;
	}

	/** Canceled flag */
	TAtomic<bool> bCanceled;

	/** Only one source of canceling */
	friend class FPsWebCancellationSource;

	/** Non-copyable and non-movable because of atomic */
	FPsWebCancellationToken(FPsWebCancellationToken&&) = delete;
	FPsWebCancellationToken& operator=(FPsWebCancellationToken&&) = delete;
	FPsWebCancellationToken(const FPsWebCancellationToken&) = delete;
	FPsWebCancellationToken& operator=(const FPsWebCancellationToken&) = delete;
};
/** Cancellation token shared pointer types to share tokens between modules and threads */
using FPsWebCancellationTokenRef = TSharedRef<FPsWebCancellationToken, ESPMode::ThreadSafe>;
using FPsWebCancellationTokenPtr = TSharedPtr<FPsWebCancellationToken, ESPMode::ThreadSafe>;

/** Cancellation source class is using to get tokens and to get the ability for canceling request */
class FPsWebCancellationSource
{
public:
	FPsWebCancellationSource()
		: Token(MakeShareable<FPsWebCancellationToken>(new FPsWebCancellationToken{}))
	{
	}

	FPsWebCancellationSource(FPsWebCancellationSource&&) = default;
	FPsWebCancellationSource& operator=(FPsWebCancellationSource&&) = default;

	~FPsWebCancellationSource()
	{
		if (IsValid())
		{
			Cancel();
		}
	}

	/** Whether the instance is valid. Token may be null in the case of moving */
	bool IsValid() const
	{
		return Token.IsValid();
	}

	/** Get the cancellation token */
	FPsWebCancellationTokenRef GetToken() const
	{
		check(IsValid());
		return Token.ToSharedRef();
	}

	/** Get canceled flag */
	bool IsCanceled() const
	{
		check(IsValid());
		return Token->IsCanceled();
	}

	/** Set canceled flag */
	void Cancel()
	{
		check(IsValid());
		Token->Cancel();
	}

private:
	/** Cancellation token instance pointer */
	FPsWebCancellationTokenPtr Token;

	/** Non-copyable */
	FPsWebCancellationSource(const FPsWebCancellationSource&) = delete;
	FPsWebCancellationSource& operator=(const FPsWebCancellationSource&) = delete;
};
