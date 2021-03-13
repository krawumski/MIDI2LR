/*
 * This file is part of MIDI2LR. Copyright (C) 2015 by Rory Jaffe.
 *
 * MIDI2LR is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * MIDI2LR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with MIDI2LR.  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */
#include "CheatViewWindow.h"

#include <exception>

#include "SettingsManager.h"

CheatViewWindow::CheatViewWindow(const juce::String& name)
try : juce
   ::DocumentWindow {name, juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
       juce::DocumentWindow::closeButton, true}
   {
      juce::TopLevelWindow::setUsingNativeTitleBar(false);
      juce::Component::centreWithSize(getWidth(), getHeight());
      juce::Component::setVisible(true);

   }
catch (const std::exception& e) {
   MIDI2LR_E_RESPONSE;
   throw;
}
