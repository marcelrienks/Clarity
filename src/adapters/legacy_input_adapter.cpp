#include "adapters/legacy_input_adapter.h"

LegacyInputAdapter::LegacyInputAdapter(LegacyInputService* legacyService)
    : legacyService_(legacyService)
{
}

std::unique_ptr<IInputAction> LegacyInputAdapter::GetShortPressAction()
{
    if (legacyService_) {
        return std::make_unique<LegacyShortPressAction>(legacyService_);
    }
    return std::make_unique<NoAction>();
}

std::unique_ptr<IInputAction> LegacyInputAdapter::GetLongPressAction()
{
    if (legacyService_) {
        return std::make_unique<LegacyLongPressAction>(legacyService_);
    }
    return std::make_unique<NoAction>();
}

bool LegacyInputAdapter::CanProcessInput() const
{
    return legacyService_ ? legacyService_->CanProcessInput() : false;
}