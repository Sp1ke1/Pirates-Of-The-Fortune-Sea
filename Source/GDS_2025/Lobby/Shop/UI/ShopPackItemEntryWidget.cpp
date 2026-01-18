#include "GDS_2025/Lobby/Shop/UI/ShopPackItemEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UShopPackItemEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemButton)
	{
		ItemButton->OnClicked.AddDynamic(this, &UShopPackItemEntryWidget::OnItemButtonClicked);
	}
}

void UShopPackItemEntryWidget::InitializeItem(const FShopItemData& InItem, int32 InItemIndex)
{
	Item = InItem;
	ItemIndex = InItemIndex;

	if (ItemNameText)
	{
		ItemNameText->SetText(Item.DisplayName);
	}

	// Load preview image
	if (ItemImage && Item.PreviewImage.IsValid())
	{
		// Check if already loaded
		if (UTexture2D* AlreadyLoaded = Item.PreviewImage.Get())
		{
			ItemImage->SetBrushFromTexture(AlreadyLoaded);
		}
		else
		{
			// Request async load
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			const FSoftObjectPath ImagePath = Item.PreviewImage.ToSoftObjectPath();
			if (ImagePath.IsValid())
			{
				Streamable.RequestAsyncLoad(ImagePath, FStreamableDelegate::CreateLambda([this, ImagePath]() {
					if (UObject* LoadedObject = ImagePath.ResolveObject())
					{
						if (UTexture2D* LoadedTexture = Cast<UTexture2D>(LoadedObject))
						{
							if (ItemImage)
							{
								ItemImage->SetBrushFromTexture(LoadedTexture);
							}
						}
					}
				}));
			}
		}
	}
}

void UShopPackItemEntryWidget::OnItemButtonClicked()
{
	OnItemClicked.Broadcast(ItemIndex);
}
