#pragma once
#include "CoreMinimal.h"
#include "Engine.h"

class SPOUT2_API FSpoutSender : public FRefCountBase
{
	FString Name;
	int32 Width;
	int32 Height;
	EPixelFormat Format;

	struct FRHIContext;
	struct FRHIContextD3D11;
	struct FRHIContextD3D12;
	FRHIContext* Context;
public:
	FSpoutSender(const FString& InName, int32 InWidth, int32 InHeight, EPixelFormat InFormat);
	virtual ~FSpoutSender() override;

	bool SendTexture(UTexture* InTexture) const;

	bool IsCompatibleWith(const FString& InName, UTexture2D* InTexture) const;
	bool IsCompatibleWith(const FString& InName, UTextureRenderTarget2D* InTexture) const;
};
