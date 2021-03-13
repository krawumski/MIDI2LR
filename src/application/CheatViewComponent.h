#ifndef MIDI2LR_CHEATVIEWCOMPONENT_H_INCLUDED
#define MIDI2LR_CHEATVIEWCOMPONENT_H_INCLUDED
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

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

class CheatViewComponent final : public juce::Component {
 public:
   explicit CheatViewComponent();
   ~CheatViewComponent() = default; // NOLINT(modernize-use-override)
   CheatViewComponent(const CheatViewComponent& other) = delete;
   CheatViewComponent(CheatViewComponent&& other) = delete;
   CheatViewComponent& operator=(const CheatViewComponent& other) = delete;
   CheatViewComponent& operator=(CheatViewComponent&& other) = delete;

   void ReadImage(const juce::String& profile_dir, const juce::String& profile_name);
   void paint(juce::Graphics& g) override;

 private:
    juce::Image cheat_img_;
    bool cheat_img_found_;
    juce::String current_profile_name_;
};

#endif
