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
#include "CommandMenu.h"

#include <exception>

#include <gsl/gsl>

#include "CCoptions.h"
#include "CommandSet.h"
#include "Misc.h"
#include "PWoptions.h"
#include "Profile.h"

CommandMenu::CommandMenu(
    rsj::MidiMessageId message, const CommandSet& command_set, Profile& profile)
try : TextButtonAligned {
   CommandSet::UnassignedTranslated()
}
, command_set_(command_set), profile_(profile), message_ {message} {}
catch (const std::exception& e)
{
   MIDI2LR_E_RESPONSE;
   throw;
}

void CommandMenu::clicked(const juce::ModifierKeys& modifiers)
{
   try {
      if (modifiers.isPopupMenu()) {
         switch (message_.msg_id_type) {
         case rsj::MessageType::kCc: {
            CCoptions ccopt;
            /* convert 1-based to 0-based */
            ccopt.BindToControl(message_.channel - 1, message_.control_number);
            juce::DialogWindow::showModalDialog(
               juce::translate("Adjust CC dialog"), &ccopt, nullptr, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true);
            break;
         }
         case rsj::MessageType::kPw: {
            PWoptions pwopt;
            /* convert 1-based to 0 based */
            pwopt.BindToControl(message_.channel - 1);
            juce::DialogWindow::showModalDialog(
                juce::translate("Adjust PW dialog"), &pwopt, nullptr, juce::Colours::white, true);
            break;
         }
         case rsj::MessageType::kChanPressure:
         case rsj::MessageType::kKeyPressure:
         case rsj::MessageType::kNoteOff:
         case rsj::MessageType::kNoteOn:
         case rsj::MessageType::kPgmChange:
         case rsj::MessageType::kSystem:
             /* do nothing for other types of controllers */;
         }
      }
      else {
         size_t index {1};
         juce::PopupMenu main_menu;
         main_menu.addItem(gsl::narrow_cast<int>(index), CommandSet::UnassignedTranslated(), true,
             index == selected_item_);
         index++;
         size_t submenu_number {0}; /* to track name for submenu */
         /* add each submenu */
         for (const auto& submenus : command_set_.GetMenuEntries()) {
            auto tick_menu {false}; /* tick when submenu item is currently selected */
            juce::PopupMenu sub_menu;
            for (const auto& command : submenus) {
               /* add each submenu entry, ticking the previously selected entry and marking used
                * entries red */
               if (profile_.CommandHasAssociatedMessage(command_set_.CommandAbbrevAt(index - 1))) {
                  const auto tick_item {index == selected_item_};
                  tick_menu |= tick_item;
                  sub_menu.addColouredItem(
                      gsl::narrow_cast<int>(index), command, juce::Colours::red, true, tick_item);
               }
               else
                  sub_menu.addItem(gsl::narrow_cast<int>(index), command, true, false);
               index++;
            }
            main_menu.addSubMenu(
                command_set_.GetMenus().at(submenu_number++), sub_menu, true, nullptr, tick_menu);
         }
         const auto result {gsl::narrow_cast<size_t>(main_menu.show())};
         if (result) {
            /* user chose a different command, remove previous command mapping associated to this
             * menu */
            if (selected_item_ < std::numeric_limits<decltype(selected_item_)>::max())
               profile_.RemoveMessage(message_);
            if (result - 1 < command_set_.CommandAbbrevSize())
               juce::Button::setButtonText(command_set_.CommandLabelAt(result - 1));
            selected_item_ = result;
            /* Map the selected command to the CC */
            profile_.AddCommandForMessage(result - 1, message_);
         }
      }
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void CommandMenu::paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) {
   g.setColour(findColour(juce::TextButton::textColourOnId));
   g.drawText(getButtonText(), 10, 0, getWidth(), getHeight(), juce::Justification::centredLeft, true);
   g.setOpacity(0.5);
   g.drawRoundedRectangle(0, 3, getWidth(), getHeight() - 6, 6, 1);
}