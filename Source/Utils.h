#pragma once

#include "IMyComponent.h"

namespace Utils
{
    void InitButton(juce::Component* _parent,
                    juce::Button& _button,
                    juce::String _text,
                    std::function<void()> _callback,
                    juce::Colour _colour,
                    bool _isEnabled);

    void DrawGrid(juce::Graphics& g,
                         unsigned int _width,
                         unsigned int _height,
                         unsigned int _spacingX = 50,
                         unsigned int _spacingY = 50);
    
    void SetComponentOwner(IMyComponent* _owned, IMyComponent* _owner);
}