#ifndef MIDI2LR_CHEATVIEWWINDOW_H_INCLUDED
#define MIDI2LR_CHEATVIEWWINDOW_H_INCLUDED
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
#include <memory>

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>

class CheatViewWindow final : public juce::DocumentWindow {
 public:
   CheatViewWindow(const juce::String& name);
   ~CheatViewWindow() = default; // NOLINT(modernize-use-override)
   CheatViewWindow(const CheatViewWindow& other) = delete;
   CheatViewWindow(CheatViewWindow&& other) = delete;
   CheatViewWindow& operator=(const CheatViewWindow& other) = delete;
   CheatViewWindow& operator=(CheatViewWindow&& other) = delete;

 private:
   void closeButtonPressed() override
   {
      setVisible(false);
   }

};

#endif
