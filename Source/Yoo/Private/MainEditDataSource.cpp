#include "MainEditDataSource.h"

#include "AppleARKitLiveLinkSourceFactory.h"
#include "ILiveLinkClient.h"
#include "LiveLinkClient.h"
#include "LiveLinkClientReference.h"
#include "LiveLinkSourceCollection.h"
#include "NeuronLiveLink.h"
#include "AppleARKitFaceSupport/Private/AppleARKitLiveLinkSource.h"
#include "Interfaces/IPv4/IPv4Address.h"

#define LOCTEXT_NAMESPACE "EditableObject"

AMainEditDataSource::AMainEditDataSource()
	: Type(EDataSourceType::Unknown)
{
	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetVisibility(true);
	SpriteComponent->CastShadow = false;
	SpriteComponent->bHiddenInGame = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(SpriteComponent);
	SphereComponent->SetSphereRadius(24.0f);
}

ANeuronDataSource::ANeuronDataSource()
{
	LocalEndpoint.Address = FIPv4Address(127, 0, 0, 1);
	LocalEndpoint.Port = 7004;
	
	TypeName = LOCTEXT("ANeuronDataSource_TypeName", "Neuron");
	Type = EDataSourceType::Body | EDataSourceType::Hand;
}

bool ANeuronDataSource::IsLegalIPString(const FString& String)
{
	FString IpString;
	FString PortString;
	if (!String.Split(TEXT(":"), &IpString, &PortString))
		return false;

	if (FIPv4Address Address; !FIPv4Address::Parse(IpString, Address))
		return false;

	if (const int32 Port = FCString::Atoi(*PortString); Port < 0 || Port > MAX_uint16)
		return false;
	
	return true;
}

FString ANeuronDataSource::GetLocalIPString() const
{
	return LocalEndpoint.ToString();
}

bool ANeuronDataSource::SetLocalIPString(const FString& String)
{
	FString IpString;
	FString PortString;
	if (!String.Split(TEXT(":"), &IpString, &PortString))
		return false;
	
	if (!FIPv4Address::Parse(IpString, LocalEndpoint.Address))
		return false;

	const int32 Port = FCString::Atoi(*PortString);
	if (Port < 0 || Port > MAX_uint16)
		return false;

	LocalEndpoint.Port = Port;
	UpdateSource();
	
	return true;
}

TArray<FName> ANeuronDataSource::BP_GetSubjectNames() const
{
	return GetSubjectNames();
}

TArray<FName> ANeuronDataSource::GetSubjectNames() const
{
	return NeuronSource->SubjectNames.Array();
}

float ANeuronDataSource::GetSkeletonHeight(FName SubjectName)
{
	if (SubjectNameToSkeletonHeight.Contains(SubjectName))
	{
		return SubjectNameToSkeletonHeight.FindChecked(SubjectName);
	}

	// Default human skeleton height
	return 160.f;
}

void ANeuronDataSource::SetSkeletonHeight(FName SubjectName, float Height)
{
	SubjectNameToSkeletonHeight.Add(SubjectName, Height);
}

void ANeuronDataSource::BeginPlay()
{
	Super::BeginPlay();

	FNeuronLiveLinkModule::Get().GetLiveLinkSkeletonHeight.BindUObject(this, &ANeuronDataSource::GetSkeletonHeight);
	
	UpdateSource();
}

void ANeuronDataSource::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	DestroySource();
}

void ANeuronDataSource::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << LocalEndpoint;
}

EDataSourceState ANeuronDataSource::CurrentStatus_Implement() const
{
	FString Status = NeuronSource->GetSourceStatus().ToString();
	if (Status.Equals(TEXT("Online"), ESearchCase::IgnoreCase))
	{
		return EDataSourceState::Online;
	}
	if (Status.Equals(TEXT("Offwork"), ESearchCase::IgnoreCase))
	{
		return EDataSourceState::Offline;
	}
	if (Status.Equals(TEXT("Online Connection Error"), ESearchCase::IgnoreCase))
	{
		return EDataSourceState::Error;
	}
	return EDataSourceState::Unknown;
}

void ANeuronDataSource::UpdateSource()
{
	if (NeuronSource && LocalEndpoint == NeuronSource->LocalEndpoint)
		return;
	
	if (IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		FLiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		if (NeuronSource)
		{
			NeuronSource->RequestSourceShutdown();
			LiveLinkClient.RemoveSource(NeuronSource);
		}
		NeuronSource = MakeShared<FNeuronLiveLinkSource>(LocalEndpoint, true, LocalEndpoint, TEXT("YXZ"), LocalEndpoint.Port);
		LiveLinkClient.AddSource(NeuronSource);
	}
}

void ANeuronDataSource::DestroySource()
{
	if (IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		FLiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		if (NeuronSource)
		{
			NeuronSource->RequestSourceShutdown();
			LiveLinkClient.RemoveSource(NeuronSource);
			NeuronSource.Reset();
		}
	}
}

AARKitDataSource::AARKitDataSource()
	: ARKitSource(nullptr)
{
	TypeName = LOCTEXT("AARKitDataSource_TypeName", "ARKit");
	Type = EDataSourceType::Face;
}

TArray<FARKitSubject> AARKitDataSource::GetSubjectAndDeviceNames() const
{
	TArray<FARKitSubject> Result;
	if (ARKitSource)
	{
		for (auto Item : ARKitSource->BlendShapePerDeviceMap)
		{
			FName Device = Item.Key;
			FName Subject = Item.Value.SubjectKey.SubjectName;
			Result.Emplace(Subject, Device);
		}
	}
	return Result;
}

TArray<FName> AARKitDataSource::GetSubjectNames() const
{
	TArray<FName> Result;
	if (ARKitSource)
	{
		for (auto Item : ARKitSource->BlendShapePerDeviceMap)
		{
			Result.Add(Item.Value.SubjectKey.SubjectName);
		}
	}
	return Result;
}

void AARKitDataSource::BeginPlay()
{
	Super::BeginPlay();
	
	if (IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		FLiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		for (FGuid Id : LiveLinkClient.GetSources())
		{
			if (auto Item = LiveLinkClient.GetCollection()->FindSource(Id))
			{
				if (Item->Setting->Factory->IsChildOf(UAppleARKitLiveLinkSourceFactory::StaticClass()))
				{
					ARKitSource = static_cast<FAppleARKitLiveLinkSource*>(Item->Source.Get());
					break;
				}
			}
		}
	}
}

EDataSourceState AARKitDataSource::CurrentStatus_Implement() const
{
	return ARKitSource && ARKitSource->IsSourceStillValid() ? EDataSourceState::Online : EDataSourceState::Offline;
}

#undef LOCTEXT_NAMESPACE
