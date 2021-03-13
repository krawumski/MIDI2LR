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
#include "Profile.h"

#include <algorithm>
#include <exception>
#include <memory>

#include "Misc.h"

void Profile::AddCommandForMessageI(const size_t command, const rsj::MidiMessageId& message)
{
   try {
      if (command < command_set_.CommandAbbrevSize()) {
         const auto& cmd_abbreviation {command_set_.CommandAbbrevAt(command)};
         message_map_[message] = cmd_abbreviation;
         command_string_map_.emplace(cmd_abbreviation, message);
         SortI();
         profile_unsaved_ = true;
      }
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::AddRowMapped(const std::string& command, const rsj::MidiMessageId& message)
{
   try {
      auto guard {std::unique_lock {mutex_}};
      if (!MessageExistsInMapI(message)) {
         if (!command_set_.CommandTextIndex(command)) {
            message_map_[message] = CommandSet::kUnassigned;
            command_string_map_.emplace(CommandSet::kUnassigned, message);
         }
         else {
            message_map_[message] = command;
            command_string_map_.emplace(command, message);
         }
         command_table_.push_back(message);
         SortI();
         profile_unsaved_ = true;
      }
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::AddRowUnmapped(const rsj::MidiMessageId& message)
{
   try {
      auto guard {std::unique_lock {mutex_}};
      if (!MessageExistsInMapI(message)) {
         AddCommandForMessageI(0, message); /* add an entry for 'no command' */
         command_table_.push_back(message);
         SortI();
         profile_unsaved_ = true;
      }
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::FromXml(const juce::XmlElement* root)
{
   /* external use only, but will either use external versions of Profile calls to lock individual
    * accesses or manually lock any internal calls instead of using mutex for entire method */
   try {
      if (!root || root->getTagName().compare("settings") != 0)
         return;
      RemoveAllRows();
      setup_table_.clear();
      const auto* setting {root->getFirstChildElement()};
      while (setting) {
         if (setting->getTagName().compare("setup") == 0) {
            if (setting->hasAttribute("controller")) {
               const rsj::MidiMessageId message {setting->getIntAttribute("channel"),
                   setting->getIntAttribute("controller"), rsj::MessageType::kCc};
               setup_table_.push_back(MidiMsg(message, setting->getIntAttribute("value")));
            }
            if (setting->hasAttribute("note")) {
               const rsj::MidiMessageId note {setting->getIntAttribute("channel"),
                   setting->getIntAttribute("note"), rsj::MessageType::kNoteOn};
               setup_table_.push_back(MidiMsg(note, setting->getIntAttribute("value")));
             }
         }
         else if (setting->hasAttribute("controller")) {
            const rsj::MidiMessageId message {setting->getIntAttribute("channel"),
                setting->getIntAttribute("controller"), rsj::MessageType::kCc};
            AddRowMapped(setting->getStringAttribute("command_string").toStdString(), message);
         }
         else if (setting->hasAttribute("note")) {
            const rsj::MidiMessageId note {setting->getIntAttribute("channel"),
                setting->getIntAttribute("note"), rsj::MessageType::kNoteOn};
            AddRowMapped(setting->getStringAttribute("command_string").toStdString(), note);
         }
         else if (setting->hasAttribute("pitchbend")) {
            const rsj::MidiMessageId pb {
                setting->getIntAttribute("channel"), 0, rsj::MessageType::kPw};
            AddRowMapped(setting->getStringAttribute("command_string").toStdString(), pb);
         }
         setting = setting->getNextElement();
      }
      auto guard {std::unique_lock {mutex_}};
      SortI();
      saved_map_ = message_map_;
      profile_unsaved_ = false;
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

std::vector<rsj::MidiMessageId> Profile::GetMessagesForCommand(const std::string& command) const
{
   try {
      auto guard {std::shared_lock {mutex_}};
      std::vector<rsj::MidiMessageId> mm;
      const auto [begin, end] {command_string_map_.equal_range(command)};
      std::for_each(begin, end, [&mm](auto&& x) { mm.push_back(x.second); });
      return mm;
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::RemoveAllRows()
{
   try {
      auto guard {std::unique_lock {mutex_}};
      command_string_map_.clear();
      command_table_.clear();
      message_map_.clear();
      /* no reason for profile_unsaved_ here. nothing to save */
      profile_unsaved_ = false;
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::RemoveMessage(const rsj::MidiMessageId& message)
{ /* do not erase command table as that will cause disappearing messages when go from Unassigned to
     assigned */
   try {
      auto guard {std::unique_lock {mutex_}};
      command_string_map_.erase(message_map_.at(message));
      message_map_.erase(message);
      profile_unsaved_ = true;
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::RemoveRow(const size_t row)
{
   try {
      auto guard {std::unique_lock {mutex_}};
      const auto msg {GetMessageForNumberI(row)};
      command_string_map_.erase(message_map_.at(msg));
      command_table_.erase(command_table_.cbegin() + row);
      message_map_.erase(msg);
      profile_unsaved_ = true;
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::RemoveUnassignedMessages()
{
   try {
      auto guard {std::unique_lock {mutex_}};
      auto it {command_string_map_.find("Unassigned")};
      if (it != command_string_map_.end()) {
         profile_unsaved_ = true;
         do {
            message_map_.erase(it->second);
            command_table_.erase(
                std::remove(command_table_.begin(), command_table_.end(), it->second),
                command_table_.end());
            command_string_map_.erase(it);
            it = command_string_map_.find("Unassigned");
         } while (it != command_string_map_.end());
      }
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::Resort(const std::pair<int, bool> new_order)
{
   try {
      auto guard {std::unique_lock {mutex_}};
      current_sort_ = new_order;
      SortI();
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::SortI()
{
   try {
      const auto msg_idx {[this](const rsj::MidiMessageId a) {
         return command_set_.CommandTextIndex(GetCommandForMessageI(a));
      }};
      const auto msg_sort {[&msg_idx](const rsj::MidiMessageId a, const rsj::MidiMessageId b) {
         return msg_idx(a) < msg_idx(b);
      }};
      if (current_sort_.first == 1)
         if (current_sort_.second)
            std::sort(command_table_.begin(), command_table_.end());
         else
            std::sort(command_table_.rbegin(), command_table_.rend());
      else if (current_sort_.second)
         std::sort(command_table_.begin(), command_table_.end(), msg_sort);
      else
         std::sort(command_table_.rbegin(), command_table_.rend(), msg_sort);
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void Profile::ToXmlFile(const juce::File& file)
{
   try {
      auto guard {std::shared_lock {mutex_}};
      /* don't bother if map is empty */
      if (!message_map_.empty()) {
         /* save the contents of the command map to an xml file */
         juce::XmlElement root {"settings"};

         /* @jr10feb21 Write the setup definitions. */
         for (const auto& msg : setup_table_) {
            auto setup {std::make_unique<juce::XmlElement>("setup")};
            setup->setAttribute("channel", msg.msgId.channel);
            switch (msg.msgId.msg_id_type) {
            case rsj::MessageType::kNoteOn:
               setup->setAttribute("note", msg.msgId.control_number);
               break;
            case rsj::MessageType::kCc:
               setup->setAttribute("controller", msg.msgId.control_number);
               break;
            }
            setup->setAttribute("value", msg.value);
            root.addChildElement(setup.release());
         }
         for (const auto& [msg_id, cmd_str] : message_map_) {
            auto setting {std::make_unique<juce::XmlElement>("setting")};
            setting->setAttribute("channel", msg_id.channel);
            switch (msg_id.msg_id_type) {
            case rsj::MessageType::kNoteOn:
               setting->setAttribute("note", msg_id.control_number);
               break;
            case rsj::MessageType::kCc:
               setting->setAttribute("controller", msg_id.control_number);
               break;
            case rsj::MessageType::kPw:
               setting->setAttribute("pitchbend", 0);
               break;
            case rsj::MessageType::kChanPressure:
            case rsj::MessageType::kKeyPressure:
            case rsj::MessageType::kNoteOff:
            case rsj::MessageType::kPgmChange:
            case rsj::MessageType::kSystem:
               /* can't handle other types */
               continue;
            }
            setting->setAttribute("command_string", cmd_str);
            root.addChildElement(setting.release());
         }
         if (!root.writeTo(file)) {
            /* Give feedback if file-save doesn't work */
            const auto& p {file.getFullPathName()};
            rsj::LogAndAlertError(
                juce::translate("Unable to save file. Choose a different location and try again.")
                    + ' ' + p,
                "Unable to save file. Choose a different location and try again. " + p);
         }
         saved_map_ = message_map_;
         profile_unsaved_ = false;
      }
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}