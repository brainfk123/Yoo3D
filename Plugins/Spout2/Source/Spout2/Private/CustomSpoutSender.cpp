#include "CustomSpoutSender.h"

#include "Windows/AllowWindowsPlatformTypes.h" 
#include <d3d11on12.h>
#include "Spout.h"
#include "Windows/HideWindowsPlatformTypes.h"

struct FSpoutSender::FRHIContext
{
	TRefCountPtr<ID3D11Device> D3D11Device;
	HANDLE SharedSendingHandle = nullptr;
	TRefCountPtr<ID3D11Texture2D> SendingTexture;
	TRefCountPtr<ID3D11DeviceContext> DeviceContext;
	
	spoutSenderNames Senders;
	spoutDirectX Sdx;
	
	virtual ~FRHIContext() = default;
	
	FRHICOMMAND_MACRO(FRHIInitializeSpoutContext)
	{
		FRHIContext* Context;
		FString Name;
		int32 Width, Height;
		EPixelFormat Format;

		FORCEINLINE_DEBUGGABLE FRHIInitializeSpoutContext(FRHIContext* InContext, const FString& InName, int32 InWidth, int32 InHeight, EPixelFormat InFormat)
			: Context(InContext), Name(InName), Width(InWidth), Height(InHeight), Format(InFormat)
		{
			ensure(Context);
		}
		
		SPOUT2_API void Execute(FRHICommandListBase& RHICmdList) const
		{
			Context->RHIInitialize(RHICmdList, Name, Width, Height, Format);
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

	void Initialize(FRHICommandList& RHICmdList, const FString& InName, int32 InWidth, int32 InHeight, EPixelFormat InFormat)
	{
		if (RHICmdList.Bypass())
		{
			RHIInitialize(RHICmdList, InName, InWidth, InHeight, InFormat);
			return;
		}
		ALLOC_COMMAND_CL(RHICmdList, FRHIInitializeSpoutContext)(this, InName, InWidth, InHeight, InFormat);
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

	virtual void RHIInitialize(FRHICommandListBase& RHICmdList, const FString& InName, int32 InWidth, int32 InHeight, EPixelFormat InFormat) = 0;

	virtual void RHIRelease(FRHICommandListBase& RHICmdList, const FString& InName) = 0;

	virtual void RHISendTexture(FRHICommandListBase& RHICmdList, const FString& InName, FRHITexture* InTexture) = 0;
};

struct FSpoutSender::FRHIContextD3D11 : FRHIContext
{
	virtual void RHIInitialize(FRHICommandListBase& RHICmdList, const FString& InName, int32 InWidth, int32 InHeight, EPixelFormat InFormat) override
	{
		D3D11Device = static_cast<ID3D11Device*>(GDynamicRHI->RHIGetNativeDevice());
		D3D11Device->GetImmediateContext(DeviceContext.GetInitReference());
		
		DXGI_FORMAT RHITexFormat = (DXGI_FORMAT)GPixelFormats[InFormat].PlatformFormat;
		if (RHITexFormat == DXGI_FORMAT_B8G8R8A8_TYPELESS)
		{
			RHITexFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		}
		
		verify(Sdx.CreateSharedDX11Texture(D3D11Device, InWidth, InHeight, RHITexFormat, SendingTexture.GetInitReference(), SharedSendingHandle));
		verify(Senders.CreateSender(StringCast<ANSICHAR>(*InName).Get(), InWidth, InHeight, SharedSendingHandle, RHITexFormat));
	}

	virtual void RHIRelease(FRHICommandListBase& RHICmdList, const FString& InName) override
	{
		Senders.ReleaseSenderName(StringCast<ANSICHAR>(*InName).Get());
		SendingTexture.SafeRelease();
		DeviceContext.SafeRelease();
		D3D11Device.SafeRelease();
	}

	virtual void RHISendTexture(FRHICommandListBase& RHICmdList, const FString& InName, FRHITexture* InTexture) override
	{
		ID3D11Texture2D* NativeTex = (ID3D11Texture2D*)InTexture->GetNativeResource();
		FIntPoint Size = InTexture->GetDesc().Extent;

		DeviceContext->CopyResource(SendingTexture, NativeTex);
		DeviceContext->Flush();

		verify(Senders.UpdateSender(StringCast<ANSICHAR>(*InName).Get(), Size.X, Size.Y, SharedSendingHandle));
	}
};

struct FSpoutSender::FRHIContextD3D12 : FRHIContext
{
	TRefCountPtr<ID3D11On12Device> D3D11on12Device;
	ID3D11Resource* WrappedDX11Resource = nullptr;
	ID3D11Texture2D* NativeTex = nullptr;
	
	virtual void RHIInitialize(FRHICommandListBase& RHICmdList, const FString& InName, int32 InWidth, int32 InHeight, EPixelFormat InFormat) override
	{
		ID3D12Device* Device12 = static_cast<ID3D12Device*>(GDynamicRHI->RHIGetNativeDevice());
		UINT DeviceFlags11 = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		verify(D3D11On12CreateDevice(
			Device12,
			DeviceFlags11,
			nullptr,
			0,
			nullptr,
			0,
			0,
			D3D11Device.GetInitReference(),
			DeviceContext.GetInitReference(),
			nullptr
		) == S_OK);

		verify(D3D11Device->QueryInterface(__uuidof(ID3D11On12Device), (void**)D3D11on12Device.GetInitReference()) == S_OK);
		
		DXGI_FORMAT RHITexFormat = (DXGI_FORMAT)GPixelFormats[InFormat].PlatformFormat;
		if (RHITexFormat == DXGI_FORMAT_B8G8R8A8_TYPELESS)
		{
			RHITexFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		}
		
		verify(Sdx.CreateSharedDX11Texture(D3D11Device, InWidth, InHeight, RHITexFormat, SendingTexture.GetInitReference(), SharedSendingHandle));
		verify(Senders.CreateSender(StringCast<ANSICHAR>(*InName).Get(), InWidth, InHeight, SharedSendingHandle, RHITexFormat));
	}

	virtual void RHIRelease(FRHICommandListBase& RHICmdList, const FString& InName) override
	{
		NativeTex = nullptr;
		WrappedDX11Resource = nullptr;
		Senders.ReleaseSenderName(StringCast<ANSICHAR>(*InName).Get());
		SendingTexture.SafeRelease();
		DeviceContext.SafeRelease();
		D3D11on12Device.SafeRelease();
		D3D11Device.SafeRelease();
	}

	virtual void RHISendTexture(FRHICommandListBase& RHICmdList, const FString& InName, FRHITexture* InTexture) override
	{
		ID3D11Texture2D* InNativeTex = (ID3D11Texture2D*)InTexture->GetNativeResource();
		FIntPoint Size = InTexture->GetDesc().Extent;

		if (NativeTex != InNativeTex)
		{
			NativeTex = InNativeTex;
			D3D11_RESOURCE_FLAGS rf11 = {};
			verify(D3D11on12Device->CreateWrappedResource(
				NativeTex, &rf11,
				D3D12_RESOURCE_STATE_COPY_SOURCE,
				D3D12_RESOURCE_STATE_PRESENT, __uuidof(ID3D11Resource),
				(void**)&WrappedDX11Resource) == S_OK);
		}

		ID3D11Resource* Resources[] = { WrappedDX11Resource };
		D3D11on12Device->AcquireWrappedResources(Resources, 1);
		DeviceContext->CopyResource(SendingTexture, WrappedDX11Resource);
		D3D11on12Device->ReleaseWrappedResources(Resources, 1);
		DeviceContext->Flush();

		verify(Senders.UpdateSender(StringCast<ANSICHAR>(*InName).Get(), Size.X, Size.Y, SharedSendingHandle));
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
		[InContext = Context, InName, InWidth, InHeight, InFormat](FRHICommandListImmediate& RHICmdList)
		{
			check(InContext);
			InContext->Initialize(RHICmdList, InName, InWidth, InHeight, InFormat);
		});
}

FSpoutSender::~FSpoutSender()
{
	ENQUEUE_RENDER_COMMAND(ReleaseSpoutSenderCommand)(
		[InContext = Context, InName = Name](FRHICommandListImmediate& RHICmdList)
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
			Context->SendTexture(RHICmdList, Name, InTexture->GetResource()->TextureRHI);
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
