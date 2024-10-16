#include "CustomSpoutSender.h"

#include "Windows/AllowWindowsPlatformTypes.h" 
#include <d3d11on12.h>
#include "SpoutCommon.h"
#include "SpoutDirectX.h"
#include "SpoutDX.h"
#include "SpoutDX12.h"
#include "SpoutSenderNames.h"
#include "Windows/HideWindowsPlatformTypes.h"

struct FSpoutSender::FRHIContext
{
	virtual ~FRHIContext() = default;
	
	FRHICOMMAND_MACRO(FRHIInitializeSpoutContext)
	{
		FRHIContext* Context;
		FString Name;
		EPixelFormat Format;

		FORCEINLINE_DEBUGGABLE FRHIInitializeSpoutContext(FRHIContext* InContext, const FString& InName, EPixelFormat InFormat)
			: Context(InContext), Name(InName), Format(InFormat)
		{
			ensure(Context);
		}
		
		SPOUT2_API void Execute(FRHICommandListBase& RHICmdList) const
		{
			Context->RHIInitialize(RHICmdList, Name, Format);
		}
	};
	
	FRHICOMMAND_MACRO(FRHIReleaseSpoutContext)
	{
		FRHIContext* Context;
		FString Name;

		FORCEINLINE_DEBUGGABLE FRHIReleaseSpoutContext(FRHIContext* InContext, const FString& InName)
			: Context(InContext), Name(InName)
		{
			ensure(Context);
		}
		
		SPOUT2_API void Execute(FRHICommandListBase& RHICmdList) const
		{
			Context->RHIRelease(RHICmdList, Name);
			delete Context;
		}
	};
	
	FRHICOMMAND_MACRO(FRHISpoutSendTexture)
	{
		FRHIContext* Context;
		FString Name;
		FTextureRHIRef TextureRHI;

		FORCEINLINE_DEBUGGABLE FRHISpoutSendTexture(FRHIContext* InContext, const FString& InName, FRHITexture* InTexture)
			: Context(InContext), Name(InName), TextureRHI(InTexture)
		{
			ensure(Context);
			ensure(TextureRHI);
		}
		
		SPOUT2_API void Execute(FRHICommandListBase& RHICmdList) const
		{
			Context->RHISendTexture(RHICmdList, Name, TextureRHI);
		}
	};

	void Initialize(FRHICommandList& RHICmdList, const FString& InName, EPixelFormat InFormat)
	{
		if (RHICmdList.Bypass())
		{
			RHIInitialize(RHICmdList, InName, InFormat);
			return;
		}
		ALLOC_COMMAND_CL(RHICmdList, FRHIInitializeSpoutContext)(this, InName, InFormat);
	}

	void Release(FRHICommandList& RHICmdList, const FString& InName)
	{
		if (RHICmdList.Bypass())
		{
			RHIRelease(RHICmdList, InName);
			delete this;
			return;
		}
		ALLOC_COMMAND_CL(RHICmdList, FRHIReleaseSpoutContext)(this, InName);
	}

	void SendTexture(FRHICommandList& RHICmdList, const FString& InName, FRHITexture* InTexture)
	{
		if (RHICmdList.Bypass())
		{
			RHISendTexture(RHICmdList, InName, InTexture);
			return;
		}
		ALLOC_COMMAND_CL(RHICmdList, FRHISpoutSendTexture)(this, InName, InTexture);
	}

	virtual void RHIInitialize(FRHICommandListBase& RHICmdList, const FString& InName, EPixelFormat InFormat) = 0;

	virtual void RHIRelease(FRHICommandListBase& RHICmdList, const FString& InName) = 0;

	virtual void RHISendTexture(FRHICommandListBase& RHICmdList, const FString& InName, FRHITexture* InTexture) = 0;
};

struct FSpoutSender::FRHIContextD3D11 : FRHIContext
{
	spoutDX Sender;
	
	virtual void RHIInitialize(FRHICommandListBase& RHICmdList, const FString& InName, EPixelFormat InFormat) override
	{
		ID3D11Device* D3D11Device = static_cast<ID3D11Device*>(GDynamicRHI->RHIGetNativeDevice());
		
		DXGI_FORMAT RHITexFormat = (DXGI_FORMAT)GPixelFormats[InFormat].PlatformFormat;
		if (RHITexFormat == DXGI_FORMAT_B8G8R8A8_TYPELESS)
		{
			RHITexFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		}

		verify(Sender.OpenDirectX11(D3D11Device));
		
		const FString FixedName = FString::Printf(TEXT("[%s]: %s"), FPlatformProcess::ExecutableName(false), *InName);
		const auto Conversion = StringCast<UTF8CHAR>(*FixedName);
		verify(Sender.SetSenderName(reinterpret_cast<const char*>(Conversion.Get())));
		
		Sender.SetSenderFormat(RHITexFormat);
	}

	virtual void RHIRelease(FRHICommandListBase& RHICmdList, const FString& InName) override
	{
		Sender.ReleaseSender();
		Sender.CloseDirectX11();
	}

	virtual void RHISendTexture(FRHICommandListBase& RHICmdList, const FString& InName, FRHITexture* InTexture) override
	{
		ID3D11Texture2D* NativeTex = (ID3D11Texture2D*)InTexture->GetNativeResource();
		verify(Sender.SendTexture(NativeTex))
	}
};

struct FSpoutSender::FRHIContextD3D12 : FRHIContext
{
	spoutDX12 Sender;
	
	virtual void RHIInitialize(FRHICommandListBase& RHICmdList, const FString& InName, EPixelFormat InFormat) override
	{
		ID3D12Device* Device12 = static_cast<ID3D12Device*>(GDynamicRHI->RHIGetNativeDevice());
		
		DXGI_FORMAT RHITexFormat = (DXGI_FORMAT)GPixelFormats[InFormat].PlatformFormat;
		if (RHITexFormat == DXGI_FORMAT_B8G8R8A8_TYPELESS)
		{
			RHITexFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		}

		verify(Sender.OpenDirectX12(Device12));

		const FString FixedName = FString::Printf(TEXT("[%s]: %s"), FPlatformProcess::ExecutableName(false), *InName);
		const auto Conversion = StringCast<UTF8CHAR>(*FixedName);
		verify(Sender.SetSenderName(reinterpret_cast<const char*>(Conversion.Get())));
		
		Sender.SetSenderFormat(RHITexFormat);
	}

	virtual void RHIRelease(FRHICommandListBase& RHICmdList, const FString& InName) override
	{
		Sender.ReleaseSender();
		Sender.CloseDirectX12();
	}

	virtual void RHISendTexture(FRHICommandListBase& RHICmdList, const FString& InName, FRHITexture* InTexture) override
	{
		ID3D12Resource* InNativeTex = (ID3D12Resource*)InTexture->GetNativeResource();
		ID3D11Resource* WrapedDX11Tex;
		verify(Sender.WrapDX12Resource(InNativeTex, &WrapedDX11Tex, D3D12_RESOURCE_STATE_COPY_SOURCE));
		verify(Sender.SendDX11Resource(WrapedDX11Tex));
	}
};

FSpoutSender::FSpoutSender(const FString& InName, int32 InWidth, int32 InHeight, EPixelFormat InFormat)
	: Name(InName), Width(InWidth), Height(InHeight), Format(InFormat), Context(nullptr)
{
	FString RHIName = GDynamicRHI->GetName();
	if (RHIName == TEXT("D3D11"))
	{
		Context = new FRHIContextD3D11();
	}
	else if (RHIName == TEXT("D3D12"))
	{
		Context = new FRHIContextD3D12();
	}
	else
	{
		checkf(0, TEXT("Unsupported RHI for Spout!"));
	}
	
	ENQUEUE_RENDER_COMMAND(InitializeSpoutSenderCommand)(
		[this, InWidth, InHeight, InContext = Context, InName, InFormat](FRHICommandListImmediate& RHICmdList)
		{
			check(InContext);
			Texture2D = RHICmdList.CreateTexture(FRHITextureCreateDesc::Create2D(TEXT("Spout Sender Texture2D"), {InWidth, InHeight}, InFormat));
			InContext->Initialize(RHICmdList, InName, InFormat);
		});
}

FSpoutSender::~FSpoutSender()
{
	ENQUEUE_RENDER_COMMAND(ReleaseSpoutSenderCommand)(
		[this, InContext = Context, InName = Name](FRHICommandListImmediate& RHICmdList)
		{
			check(InContext);
			InContext->Release(RHICmdList, InName);
		});
	Context = nullptr;
}

bool FSpoutSender::SendTexture(UTexture* InTexture) const
{
	if (!InTexture)
		return false;
	
	ENQUEUE_RENDER_COMMAND(SpoutSendTextureCommand)(
		[this, InTexture](FRHICommandListImmediate& RHICmdList)
		{
			const auto TextureRHI = InTexture->GetResource()->TextureRHI;
			RHICmdList.Transition(FRHITransitionInfo(Texture2D, ERHIAccess::Present, ERHIAccess::CopyDest));
			RHICmdList.CopyTexture(TextureRHI, Texture2D, {});
			RHICmdList.Transition(FRHITransitionInfo(Texture2D, ERHIAccess::CopyDest, ERHIAccess::CopySrc));
			Context->SendTexture(RHICmdList, Name, Texture2D);
		});
	return true;
}

bool FSpoutSender::IsCompatibleWith(const FString& InName, UTexture2D* InTexture) const
{
	return InName.Equals(Name)
		&& InTexture->GetPixelFormat() == Format
		&& InTexture->GetSizeX() == Width
		&& InTexture->GetSizeY() == Height;
}

bool FSpoutSender::IsCompatibleWith(const FString& InName, UTextureRenderTarget2D* InTexture) const
{
	return InName.Equals(Name)
		&& InTexture->GetFormat() == Format
		&& InTexture->SizeX == Width
		&& InTexture->SizeY == Height;
}
