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
#include "SettingsComponent.h"

#include <cmath>
#include <exception>

#include <fmt/format.h>

#include "Misc.h"
#include "SettingsManager.h"

namespace {
   constexpr auto kSettingsLeft     {  20 };
   constexpr auto kSettingsMargin   {  10 };
   constexpr auto kSettingsCtrlLeft {  30 };
   constexpr auto kSettingsWidth    { 420 };
   constexpr auto kSettingsHeight   { 360 };
} // namespace

SettingsComponent::SettingsComponent(SettingsManager& settings_manager)
    : ResizableLayout {this}, settings_manager_ {settings_manager}
{
}

void SettingsComponent::Init()
{
   try {
      setSize(kSettingsWidth, kSettingsHeight);
      pickup_group_.setText(juce::translate("Pick up"));
      pickup_group_.setBounds(kSettingsMargin, kSettingsMargin, kSettingsWidth - 2*kSettingsMargin, 120);
      addToLayout(&pickup_group_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(pickup_group_);
      pickup_label_.setFont(juce::Font {16.f, juce::Font::bold});
      pickup_label_.setText(juce::translate("Disabling the pickup mode may be better for "
                                            "touchscreen interfaces and may solve issues with "
                                            "Lightroom not picking up fast fader/knob movements"),
          juce::NotificationType::dontSendNotification);
      pickup_label_.setJustificationType(juce::Justification::topLeft);
      pickup_label_.setBounds(kSettingsLeft, kSettingsMargin+20, kSettingsWidth - 2*kSettingsLeft, 50);
      addToLayout(&pickup_label_, anchorTopLeft, anchorTopRight);
      pickup_label_.setEditable(false);
      addAndMakeVisible(pickup_label_);

      pickup_enabled_.setToggleState(settings_manager_.GetPickupEnabled(), juce::NotificationType::dontSendNotification);
      pickup_enabled_.setBounds(kSettingsLeft, kSettingsMargin+pickup_label_.getHeight()+24, kSettingsWidth - 2*kSettingsLeft, 32); //-V112
      addToLayout(&pickup_enabled_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(pickup_enabled_);
      pickup_enabled_.onClick = [this] {
         const auto pickup_state {pickup_enabled_.getToggleState()};
         settings_manager_.SetPickupEnabled(pickup_state);
         rsj::Log(pickup_state ? "Pickup set to enabled." : "Pickup set to disabled.");
      };

      /* profile */
      int profile_group_top = 2 * kSettingsMargin + pickup_group_.getHeight();
      profile_group_.setText(juce::translate("Profile"));
      profile_group_.setBounds(kSettingsMargin, profile_group_top, kSettingsWidth - 2*kSettingsMargin, 95);
      addToLayout(&profile_group_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(profile_group_);

      profile_location_button_.setBounds(kSettingsCtrlLeft, profile_group_top+25, kSettingsWidth - 2* kSettingsCtrlLeft, 25);
      addToLayout(&profile_location_button_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(profile_location_button_);
      profile_location_button_.onClick = [this] {
         juce::FileChooser chooser {juce::translate("Select Folder"),
             juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "", true};
         if (chooser.browseForDirectory()) {
            const auto profile_location {chooser.getResult().getFullPathName()};
            settings_manager_.SetProfileDirectory(profile_location);
            rsj::Log(fmt::format(
                FMT_STRING("Profile location set to {}."), profile_location.toStdString()));
            profile_location_label_.setText(
                profile_location, juce::NotificationType::dontSendNotification);
         }
      };

      profile_location_label_.setEditable(false);
      profile_location_label_.setBounds(kSettingsLeft, profile_group_top+55, kSettingsWidth - 2*kSettingsLeft, 30);
      addToLayout(&profile_location_label_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(profile_location_label_);
      profile_location_label_.setText(settings_manager_.GetProfileDirectory(), juce::NotificationType::dontSendNotification);

      /* autohide */
      int autohide_group_top = profile_group_top + profile_group_.getHeight() + kSettingsMargin;
      autohide_group_.setText(juce::translate("Auto hide"));
      autohide_group_.setBounds(kSettingsMargin, autohide_group_top, kSettingsWidth - 2*kSettingsMargin, 100);
      addToLayout(&autohide_group_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(autohide_group_);

      autohide_explain_label_.setFont(juce::Font {16.f, juce::Font::bold});
      autohide_explain_label_.setText(juce::translate("Autohide the plugin window in x seconds.\n"
                                                      "Select 0 for disabling autohide."),
          juce::NotificationType::dontSendNotification);
      autohide_explain_label_.setJustificationType(juce::Justification::topLeft);
      autohide_explain_label_.setBounds(kSettingsLeft, autohide_group_top+20, kSettingsWidth - 2*kSettingsLeft, 40);
      addToLayout(&autohide_explain_label_, anchorTopLeft, anchorTopRight);
      autohide_explain_label_.setEditable(false);
      autohide_explain_label_.setFont(juce::Font {16.f, juce::Font::bold});
      addAndMakeVisible(autohide_explain_label_);

      autohide_setting_.setBounds(kSettingsCtrlLeft, autohide_group_top+65, kSettingsWidth - 2*kSettingsCtrlLeft, 20);
      autohide_setting_.setRange(0, 10, 1);
      autohide_setting_.setValue(settings_manager_.GetAutoHideTime(), juce::NotificationType::dontSendNotification);
      addToLayout(&autohide_setting_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(autohide_setting_);
      autohide_setting_.onValueChange = [this] {
         settings_manager_.SetAutoHideTime(std::lrint(autohide_setting_.getValue()));
         rsj::Log(fmt::format(
             FMT_STRING("Autohide time set to {} seconds."), settings_manager_.GetAutoHideTime()));
      };
      /* turn it on */
      activateLayout();
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

