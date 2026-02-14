// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Async/Future.h"
#include "SLRPCResponse.generated.h"

/**
 * RPC 파라미터 (Key-Value Pair)
 */
USTRUCT(BlueprintType)
struct STARLINK_API FSLRPCParam
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Key;

	UPROPERTY(BlueprintReadWrite)
	FString Value;

	FSLRPCParam()
	{}

	FSLRPCParam(const FString& InKey, const FString& InValue)
		: Key(InKey)
		, Value(InValue)
	{}
};

/**
 * RPC 응답 상태
 */
UENUM(BlueprintType)
enum class ESLRPCResponseStatus : uint8
{
	Pending,		// 대기 중
	Success,		// 성공
	Failed			// 실패
};

/**
 * RPC 응답 데이터
 */
USTRUCT(BlueprintType)
struct STARLINK_API FSLRPCResponseData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	ESLRPCResponseStatus Status = ESLRPCResponseStatus::Pending;

	UPROPERTY(BlueprintReadWrite)
	FString Message;

	UPROPERTY(BlueprintReadWrite)
	TArray<FSLRPCParam> Data;

	UPROPERTY(BlueprintReadWrite)
	int32 ErrorCode = 0;

	FSLRPCResponseData()
		: Status(ESLRPCResponseStatus::Pending)
		, ErrorCode(0)
	{}

	FSLRPCResponseData(ESLRPCResponseStatus InStatus, const FString& InMessage = TEXT(""))
		: Status(InStatus)
		, Message(InMessage)
		, ErrorCode(0)
	{}

	void SetSuccess(const FString& InMessage = TEXT(""))
	{
		Status = ESLRPCResponseStatus::Success;
		Message = InMessage;
	}

	void SetFailed(const FString& InMessage = TEXT(""), int32 InErrorCode = -1)
	{
		Status = ESLRPCResponseStatus::Failed;
		Message = InMessage;
		ErrorCode = InErrorCode;
	}

	bool IsSuccess() const { return Status == ESLRPCResponseStatus::Success; }
	bool IsPending() const { return Status == ESLRPCResponseStatus::Pending; }
	bool IsFailed() const { return Status == ESLRPCResponseStatus::Failed; }

	// Array를 Map으로 변환하는 헬퍼 (값 반환)
	TMap<FString, FString> GetDataMap() const
	{
		TMap<FString, FString> Map;
		for (const FSLRPCParam& Param : Data)
		{
			Map.Add(Param.Key, Param.Value);
		}
		return Map;
	}
};

/**
 * RPC 응답 Promise Wrapper
 */
class STARLINK_API FSLRPCResponse
{
public:
	FSLRPCResponse()
		: Promise(MakeShared<TPromise<FSLRPCResponseData>>())
	{}

	// Future를 반환 (응답을 기다리는 측에서 사용)
	TFuture<FSLRPCResponseData> GetFuture()
	{
		return Promise->GetFuture();
	}

	// Promise에 값을 설정 (응답을 보내는 측에서 사용)
	void SetValue(const FSLRPCResponseData& ResponseData)
	{
		if (Promise.IsValid())
		{
			Promise->SetValue(ResponseData);
		}
	}

	// 성공 응답 간편 설정
	void SetSuccess(const FString& Message = TEXT(""))
	{
		SetValue(FSLRPCResponseData(ESLRPCResponseStatus::Success, Message));
	}

	// 실패 응답 간편 설정
	void SetFailed(const FString& Message = TEXT(""), int32 ErrorCode = -1)
	{
		FSLRPCResponseData ResponseData(ESLRPCResponseStatus::Failed, Message);
		ResponseData.ErrorCode = ErrorCode;
		SetValue(ResponseData);
	}

	// 응답 완료 여부 확인
	bool IsReady() const
	{
		return Promise.IsValid() && Promise->GetFuture().IsReady();
	}

private:
	TSharedPtr<TPromise<FSLRPCResponseData>> Promise;
};
