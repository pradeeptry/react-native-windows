// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <UI.Xaml.Controls.h>

#include <Views/ControlViewManager.h>
#include <Views/ShadowNodeBase.h>

#include <Utils/PropertyUtils.h>

#ifdef USE_WINUI3
#define TAB_INDEX_PROPERTY xaml::UIElement::TabIndexProperty
#else
#define TAB_INDEX_PROPERTY xaml::Controls::Control::TabIndexProperty
#endif

namespace react {
namespace uwp {

ControlViewManager::ControlViewManager(const std::shared_ptr<IReactInstance> &reactInstance) : Super(reactInstance) {}

folly::dynamic ControlViewManager::GetNativeProps() const {
  folly::dynamic props = Super::GetNativeProps();
  props.update(folly::dynamic::object("tabIndex", "number"));
  return props;
}
void ControlViewManager::TransferProperties(const XamlView &oldView, const XamlView &newView) {
  TransferProperty(oldView, newView, xaml::Controls::Control::FontSizeProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::FontFamilyProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::FontWeightProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::FontStyleProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::CharacterSpacingProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::IsTextScaleFactorEnabledProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::BackgroundProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::BorderBrushProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::BorderThicknessProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::PaddingProperty());
  TransferProperty(oldView, newView, xaml::Controls::Control::ForegroundProperty());
  TransferProperty(oldView, newView, TAB_INDEX_PROPERTY());

  // Control.CornerRadius is only supported on >= RS5
  if (oldView.try_as<xaml::Controls::Control>() && newView.try_as<xaml::Controls::Control>()) {
    TransferProperty(oldView, newView, xaml::Controls::Control::CornerRadiusProperty());
  }
  Super::TransferProperties(oldView, newView);
}

bool ControlViewManager::UpdateProperty(
    ShadowNodeBase *nodeToUpdate,
    const std::string &propertyName,
    const folly::dynamic &propertyValue) {
  auto control(nodeToUpdate->GetView().as<xaml::Controls::Control>());

  bool implementsPadding = nodeToUpdate->ImplementsPadding();
  bool finalizeBorderRadius{false};
  bool ret = true;

  if (control != nullptr) {
    if (TryUpdateBackgroundBrush(control, propertyName, propertyValue)) {
    } else if (TryUpdateBorderProperties(nodeToUpdate, control, propertyName, propertyValue)) {
    } else if (TryUpdateForeground(control, propertyName, propertyValue)) {
    } else if (TryUpdateCornerRadiusOnNode(nodeToUpdate, control, propertyName, propertyValue)) {
      finalizeBorderRadius = true;
    } else if (implementsPadding && TryUpdatePadding(nodeToUpdate, control, propertyName, propertyValue)) {
    } else if (propertyName == "tabIndex") {
      if (propertyValue.isNumber()) {
        auto tabIndex = propertyValue.asDouble();
        if (tabIndex == static_cast<int32_t>(tabIndex))
          control.ClearValue(TAB_INDEX_PROPERTY());
        control.TabIndex(static_cast<int32_t>(tabIndex));
      } else if (propertyValue.isNull()) {
        control.ClearValue(TAB_INDEX_PROPERTY());
      }
    } else {
      ret = Super::UpdateProperty(nodeToUpdate, propertyName, propertyValue);
    }
  }

  if (finalizeBorderRadius && control.try_as<xaml::Controls::Control>()) {
    // Control.CornerRadius is only supported on >= RS5, setting borderRadius on Controls have no effect < RS5
    UpdateCornerRadiusOnElement(nodeToUpdate, control);
  }
  return ret;
}

void ControlViewManager::OnViewCreated(XamlView view) {
  // Set the default cornerRadius to 0 for Control: WinUI usually default cornerRadius to 2
  // Only works on >= RS5 becuase Control.CornerRadius is only supported >= RS5
  if (auto control = view.try_as<xaml::Controls::Control>()) {
    control.CornerRadius({0});
  }
}

} // namespace uwp
} // namespace react
