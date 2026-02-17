#include "EffectChain.h"

//==============================================================================
EffectChain::EffectChain()
{
}

//==============================================================================
void EffectChain::prepare (double sampleRate, int maxBlockSize, int numChannels)
{
    currentSampleRate = sampleRate;
    currentMaxBlockSize = maxBlockSize;
    currentNumChannels = numChannels;

    // Prepare all effects
    for (auto& slot : effectSlots)
    {
        if (slot.effect)
            slot.effect->prepare (sampleRate, maxBlockSize, numChannels);
    }
}

void EffectChain::reset()
{
    for (auto& slot : effectSlots)
    {
        if (slot.effect)
            slot.effect->reset();
    }
}

//==============================================================================
void EffectChain::process (juce::AudioBuffer<float>& buffer,
                           const juce::AudioBuffer<float>* sidechainBuffer)
{
    // Process through each effect in order
    for (auto& slot : effectSlots)
    {
        if (slot.effect && !slot.bypassed && !slot.effect->isBypassed())
        {
            slot.effect->process (buffer, sidechainBuffer);
        }
    }
}

//==============================================================================
void EffectChain::addEffect (std::unique_ptr<EffectModule> effect)
{
    if (effect)
    {
        effect->prepare (currentSampleRate, currentMaxBlockSize, currentNumChannels);
        effectSlots.emplace_back (std::move (effect), generateSlotId());
    }
}

void EffectChain::insertEffect (int position, std::unique_ptr<EffectModule> effect)
{
    if (effect && position >= 0 && position <= getNumEffects())
    {
        effect->prepare (currentSampleRate, currentMaxBlockSize, currentNumChannels);
        effectSlots.insert (effectSlots.begin() + position,
                           Slot (std::move (effect), generateSlotId()));
    }
}

void EffectChain::removeEffect (int position)
{
    if (position >= 0 && position < getNumEffects())
    {
        effectSlots.erase (effectSlots.begin() + position);
    }
}

void EffectChain::moveEffect (int fromPosition, int toPosition)
{
    if (fromPosition >= 0 && fromPosition < getNumEffects() &&
        toPosition >= 0 && toPosition < getNumEffects())
    {
        auto slot = std::move (effectSlots[fromPosition]);
        effectSlots.erase (effectSlots.begin() + fromPosition);
        effectSlots.insert (effectSlots.begin() + toPosition, std::move (slot));
    }
}

void EffectChain::clearChain()
{
    effectSlots.clear();
}

//==============================================================================
EffectModule* EffectChain::getEffect (int position) const
{
    if (position >= 0 && position < getNumEffects())
        return effectSlots[position].effect.get();
    return nullptr;
}

EffectChain::Slot* EffectChain::getSlot (int position)
{
    if (position >= 0 && position < getNumEffects())
        return &effectSlots[position];
    return nullptr;
}

//==============================================================================
void EffectChain::setEffectBypassed (int position, bool shouldBeypass)
{
    if (auto* slot = getSlot (position))
        slot->bypassed = shouldBeypass;
}

bool EffectChain::isEffectBypassed (int position) const
{
    if (position >= 0 && position < getNumEffects())
        return effectSlots[position].bypassed;
    return false;
}

//==============================================================================
juce::ValueTree EffectChain::saveState() const
{
    juce::ValueTree chainState ("EffectChain");

    for (const auto& slot : effectSlots)
    {
        if (slot.effect)
        {
            juce::ValueTree slotState ("Slot");
            slotState.setProperty ("id", slot.slotId, nullptr);
            slotState.setProperty ("bypassed", slot.bypassed, nullptr);
            slotState.setProperty ("type", static_cast<int> (slot.effect->getType()), nullptr);

            // Save effect state
            slotState.appendChild (slot.effect->saveState(), nullptr);

            chainState.appendChild (slotState, nullptr);
        }
    }

    return chainState;
}

void EffectChain::loadState (const juce::ValueTree& state)
{
    if (!state.hasType ("EffectChain"))
        return;

    clearChain();

    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto slotState = state.getChild (i);

        if (slotState.hasType ("Slot"))
        {
            auto type = static_cast<EffectModule::Type> (static_cast<int> (slotState.getProperty ("type", 0)));
            auto effect = EffectModuleFactory::createEffect (type);

            if (effect)
            {
                // Load effect state
                if (slotState.getNumChildren() > 0)
                    effect->loadState (slotState.getChild (0));

                // Add to chain
                addEffect (std::move (effect));

                // Set bypass state
                effectSlots.back().bypassed = slotState.getProperty ("bypassed", false);
                effectSlots.back().slotId = slotState.getProperty ("id", generateSlotId());
            }
        }
    }
}

//==============================================================================
int EffectChain::getTotalLatencySamples() const
{
    int totalLatency = 0;

    for (const auto& slot : effectSlots)
    {
        if (slot.effect && !slot.bypassed)
            totalLatency += slot.effect->getLatencySamples();
    }

    return totalLatency;
}

juce::String EffectChain::generateSlotId()
{
    return "slot_" + juce::String (nextSlotId++);
}
