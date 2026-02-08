#include "EffectModule.h"
#include "CompressorModule.h"
#include "DelayModule.h"
#include "SaturationModule.h"

//==============================================================================
std::unique_ptr<EffectModule> EffectModuleFactory::createEffect (EffectModule::Type type)
{
    switch (type)
    {
        case EffectModule::Type::Compressor:
            return std::make_unique<CompressorModule>();

        case EffectModule::Type::Delay:
            return std::make_unique<DelayModule>();

        case EffectModule::Type::Distortion:
            return std::make_unique<SaturationModule>();

        // Add more effects here as you build them:
        // case EffectModule::Type::EQ:
        //     return std::make_unique<EQModule>();
        // case EffectModule::Type::Reverb:
        //     return std::make_unique<ReverbModule>();

        default:
            return nullptr;
    }
}

std::vector<EffectModule::Type> EffectModuleFactory::getAvailableEffects()
{
    return {
        EffectModule::Type::Compressor,
        EffectModule::Type::Delay,
        EffectModule::Type::Distortion,
    };
}

juce::String EffectModuleFactory::getEffectName (EffectModule::Type type)
{
    switch (type)
    {
        case EffectModule::Type::Compressor:  return "Compressor";
        case EffectModule::Type::EQ:          return "Equalizer";
        case EffectModule::Type::Distortion:  return "Distortion";
        case EffectModule::Type::Reverb:      return "Reverb";
        case EffectModule::Type::Delay:       return "Delay";
        case EffectModule::Type::Chorus:      return "Chorus";
        case EffectModule::Type::Filter:      return "Filter";
        default:                              return "Unknown";
    }
}
