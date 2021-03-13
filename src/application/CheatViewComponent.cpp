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
#include "CheatViewComponent.h"

#include <cmath>
#include <exception>

#include "Misc.h"

CheatViewComponent::CheatViewComponent() : cheat_img_found_(false)
{
}

void CheatViewComponent::ReadImage(const juce::String& profile_dir, const juce::String& profile_name)
{
   try {
      current_profile_name_ = profile_name;
      juce::File profile_location(profile_dir);
      juce::String file_path = (profile_location.getChildFile(profile_name)).getFullPathName();

      cheat_img_found_ = false;
      juce::File imgFile(file_path + ".png");
      if (imgFile.existsAsFile()) cheat_img_found_ = true;
      
      if (!cheat_img_found_) {
         imgFile = profile_location.getChildFile("default.png");
         if (imgFile.existsAsFile()) cheat_img_found_ = true;
      }

      if (cheat_img_found_) cheat_img_ = juce::PNGImageFormat::loadFrom(imgFile);


      if (!cheat_img_found_) {
         imgFile = file_path + ".jpg";
         if (imgFile.existsAsFile()) cheat_img_found_ = true;
      }
      if (!cheat_img_found_) {
         imgFile = profile_location.getChildFile("default.jpg");
         if (imgFile.existsAsFile()) cheat_img_found_ = true;
      }

      if (cheat_img_found_) cheat_img_ = juce::JPEGImageFormat::loadFrom(imgFile);

      if (cheat_img_found_) setSize(cheat_img_.getWidth(), cheat_img_.getHeight());
      else setSize(500, 250);

      repaint();
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void CheatViewComponent::paint(juce::Graphics& g)
{
   if (cheat_img_found_) {
      g.drawImageAt(cheat_img_, 0, 0);
   }
   else {
      juce::String helpMessage;
      g.setColour(getLookAndFeel().findColour(juce::Label::textColourId));
      g.setFont(16.0f);

      helpMessage = juce::translate("Nothing to show (cannot find '");
      helpMessage += current_profile_name_ + ".png', '";
      helpMessage += current_profile_name_ + ".jpg', '";
      helpMessage += juce::translate("'default.png' or 'default.jpg').");
      helpMessage += "\n\n";
      helpMessage += juce::translate("The cheat window can be used to show the key mappings "
         "of the connected MIDI device for the current profile. "
         "If an image file in .png or .jpg format with the same "
         "name as the profile can be found in the profile directory "
         "it will be loaded and displayed in this window. If there "
         "is no image file with the same name as the profile the app "
         "will look for an image file named 'default.png' or "
         "'default.jpg'.");
      g.drawFittedText(helpMessage, 30, 20, getWidth()-60, getHeight()-40, juce::Justification::topLeft, 20);
   }
}

