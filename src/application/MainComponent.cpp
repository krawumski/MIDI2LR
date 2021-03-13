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
#include "MainComponent.h"

#include <exception>
#include <functional>
#include <string>

#include <fmt/format.h>
#include <gsl/gsl>

#include "LR_IPC_Out.h"
#include "MIDIReceiver.h"
#include "MIDISender.h"
#include "MidiUtilities.h"
#include "Misc.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "SettingsComponent.h"
#include "SettingsManager.h"
#include "CheatViewComponent.h"


namespace {
   constexpr int kMainWidth {560}; /* equals CommandTable columns total width plus 60 */
   constexpr int kMainHeight {730};
   constexpr int kMainLeft {20};
   constexpr int kBottomSectionHeight {105};
   constexpr int kTopGroupHeight{ 80 };
   constexpr int kSpaceBetweenButton {10};
   constexpr int kStandardHeight {25};
   constexpr int kFullWidth {kMainWidth - kMainLeft * 2};
   constexpr int kButtonWidth{ static_cast<int>((kFullWidth - kSpaceBetweenButton * 2) / 3.2 ) };
   constexpr int kButtonXIncrement {kButtonWidth + kSpaceBetweenButton};
   constexpr int kConnectionLabelWidth = {kMainWidth - kMainLeft - 200};
   constexpr int kTopButtonY {65};
   constexpr int kFirstButtonX {kMainLeft};
   constexpr int kSecondButtonX {kMainLeft + kButtonXIncrement};
   constexpr int kThirdButtonX {kMainLeft + kFullWidth - kButtonWidth};
   constexpr int kCommandTableY{ 140 };
   constexpr int kCommandTableHeight {kMainHeight - kCommandTableY - kBottomSectionHeight};
   constexpr int kLabelWidth {kFullWidth / 2};
   constexpr int kProfileNameY {kMainHeight - kBottomSectionHeight + 7};
   constexpr int kCommandLabelX {kMainLeft + kLabelWidth};
   constexpr int kCommandLabelY {kProfileNameY};
   constexpr int kBottomButtonY {kMainHeight - kBottomSectionHeight + 35};
   constexpr int kBottomButtonY2 {kMainHeight - kBottomSectionHeight + 70};
   constexpr auto kDefaultsFile { MIDI2LR_UC_LITERAL("default.xml") };
} // namespace

MainContentComponent::MainContentComponent(const CommandSet& command_set, Profile& profile,
    ProfileManager& profile_manager, SettingsManager& settings_manager, LrIpcOut& lr_ipc_out,
    MidiReceiver& midi_receiver, MidiSender& midi_sender)
try : ResizableLayout {
   this
}
, command_table_model_(command_set, profile), lr_ipc_out_ {lr_ipc_out},
    midi_receiver_ {midi_receiver}, midi_sender_ {midi_sender}, profile_(profile),
    profile_manager_(profile_manager), settings_manager_(settings_manager)
{
   setSize(kMainWidth, kMainHeight);
}
catch (const std::exception& e)
{
   MIDI2LR_E_RESPONSE;
   throw;
}

MainContentComponent::~MainContentComponent()
{
   settings_manager_.SetDefaultProfile(profile_name_label_.getText());
}

void MainContentComponent::Init()
{
   try {
      juce::Colour colour_heading(180, 180, 180);

      midi_receiver_.AddCallback(this, &MainContentComponent::MidiCmdCallback);
      lr_ipc_out_.AddCallback(this, &MainContentComponent::LrIpcOutCallback);
      profile_manager_.AddCallback(this, &MainContentComponent::ProfileChanged);

      /* Main title */
      StandardLabelSettings(title_label_);
      title_label_.setFont(juce::Font {36.f, juce::Font::bold});
      // title_label_.setComponentEffect(&titl  e_shadow_);
      title_label_.setBounds(kMainLeft-3, 10, kFullWidth, 30);
      addToLayout(&title_label_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(title_label_);

      /* Version label */
      StandardLabelSettings(version_label_);
      version_label_.setFont(juce::Font{ 14.f, juce::Font::bold });
      version_label_.setBounds(kMainLeft, 40, kFullWidth, 10);
      addToLayout(&version_label_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(version_label_);

      /* Connection status */
      StandardLabelSettings(connection_label_);
      connection_label_.setColour(juce::Label::textColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
      connection_label_.setColour(juce::Label::backgroundColourId, juce::Colours::red);
      connection_label_.setJustificationType(juce::Justification::centred);
      connection_label_.setBounds(200, 15, kConnectionLabelWidth, kStandardHeight);
      addToLayout(&connection_label_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(connection_label_);

      /* Profile Heading */
      profile_load_save_label_.setBounds(kFirstButtonX, kTopButtonY-kSpaceBetweenButton, kButtonWidth, kStandardHeight);
      profile_load_save_label_.setColour(juce::Label::textColourId, colour_heading);
      addToLayout(&profile_load_save_label_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(profile_load_save_label_);

      juce::Font f = profile_load_save_label_.getFont();
      juce::Path path;
      float lineY = kTopButtonY + kSpaceBetweenButton - f.getHeight()/2;
      path.startNewSubPath(juce::Point<float>(kFirstButtonX+static_cast<float>(f.getStringWidth(profile_load_save_label_.getText()))+10,
         lineY));
      path.lineTo(juce::Point<float>(kFirstButtonX+2*kButtonWidth+kSpaceBetweenButton, lineY));
      path.closeSubPath();
      profile_head_line_.setPath(path);
      profile_head_line_.setStrokeThickness(0.3f);
      profile_head_line_.setStrokeFill(juce::FillType(colour_heading));
      addToLayout(&profile_head_line_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(profile_head_line_);

      /* Profile load button */
      load_button_.setBounds(kFirstButtonX, kTopButtonY + static_cast<int>(1.5 * kSpaceBetweenButton), kButtonWidth, kStandardHeight);
      addToLayout(&load_button_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(load_button_);
      load_button_.onClick = [this] {
         if (profile_.ProfileUnsaved()) {
            const auto result {juce::NativeMessageBox::showYesNoBox(juce::AlertWindow::WarningIcon,
                juce::translate("MIDI2LR profiles"),
                juce::translate("Profile changed. Do you want to save your changes? If you "
                                "continue without saving, your changes will be lost."))};
            if (result)
               SaveProfile();
         }
         juce::File profile_directory {settings_manager_.GetProfileDirectory()};
         const auto directory_saved {profile_directory.exists()};
         if (!directory_saved)
            [[unlikely]] profile_directory =
                juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
         juce::FileChooser chooser {
             juce::translate("Open profile"), profile_directory, "*.xml", true};
         if (chooser.browseForFileToOpen()) {
            if (const auto parsed {juce::parseXML(chooser.getResult())}) {
               const auto new_profile {chooser.getResult()};
               lr_ipc_out_.SendCommand(fmt::format(FMT_STRING("ChangedToFullPath {}\n"),
                   new_profile.getFullPathName().toStdString()));
               profile_name_label_.setText(
                   new_profile.getFileName(), juce::NotificationType::dontSendNotification);
               profile_.FromXml(parsed.get());
               command_table_.updateContent();
               command_table_.repaint();
               if (!directory_saved) /* haven't saved a directory yet */
                  [[unlikely]] settings_manager_.SetProfileDirectory(
                      new_profile.getParentDirectory().getFullPathName());
               cheat_view_component_.ReadImage(settings_manager_.GetProfileDirectory(), new_profile.getFileName());
            }
            else {
               rsj::Log(fmt::format(FMT_STRING("Unable to load profile {}."),
                   chooser.getResult().getFullPathName().toStdString()));
            }
         }
      };

      /* Profile save button */
      save_button_.setBounds(kSecondButtonX, kTopButtonY + static_cast<int>(1.5 * kSpaceBetweenButton), kButtonWidth, kStandardHeight);

      addToLayout(&save_button_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(save_button_);
      save_button_.onClick = [this] { SaveProfile(); };

      /* Create cheat view window */
      cheat_window_.reset(new CheatViewWindow(juce::translate("Cheat View")));
      cheat_window_->setContentNonOwned(&cheat_view_component_, true);
      cheat_window_->setVisible(false);

      /* Cheat View Heading */
      cheat_view_label_.setBounds(kThirdButtonX, kTopButtonY-kSpaceBetweenButton, kButtonWidth, kStandardHeight);
      cheat_view_label_.setColour(juce::Label::textColourId, colour_heading);
      addToLayout(&cheat_view_label_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(cheat_view_label_);

      path.clear();
      path.startNewSubPath(juce::Point<float>(kThirdButtonX + static_cast<float>(f.getStringWidth(cheat_view_label_.getText())) + 10,
         lineY));
      path.lineTo(juce::Point<float>(kThirdButtonX + kButtonWidth, lineY));
      path.closeSubPath();
      cheat_head_line_.setPath(path);
      cheat_head_line_.setStrokeThickness(0.3f);
      cheat_head_line_.setStrokeFill(juce::FillType(colour_heading));
      addToLayout(&cheat_head_line_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(cheat_head_line_);


      /* Show cheat button */
      cheat_show_button_.setBounds(kThirdButtonX, kTopButtonY+static_cast<int>(1.5 * kSpaceBetweenButton),
         kButtonWidth, kStandardHeight);
      addToLayout(&cheat_show_button_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(cheat_show_button_);

      cheat_show_button_.onClick = [this] {
         cheat_window_->setVisible(true);
      };

      cheat_raise_enabled_.setToggleState(settings_manager_.GetAutoRaiseCheatEnabled(), juce::NotificationType::dontSendNotification);
      cheat_raise_enabled_.setBounds(kThirdButtonX, kTopButtonY + kStandardHeight + static_cast<int>(1.8*kSpaceBetweenButton), kButtonWidth, kStandardHeight);
      addToLayout(&cheat_raise_enabled_, anchorTopLeft, anchorTopRight);
      addAndMakeVisible(cheat_raise_enabled_);
      cheat_raise_enabled_.onClick = [this] {
         const auto cheat_raise_state{ cheat_raise_enabled_.getToggleState() };
         settings_manager_.SetAutoRaiseCheatEnabled(cheat_raise_state);
         rsj::Log(cheat_raise_state ? "Cheat auto raise set to enabled." : "Cheat auto raise set to disabled.");
      };

      /* Command Table */
      command_table_.setModel(&command_table_model_);
      command_table_.setBounds(kMainLeft, kCommandTableY, kFullWidth, kCommandTableHeight);
      addToLayout(&command_table_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(command_table_);

      /* Profile name label */
      StandardLabelSettings(profile_name_title_);
      int profile_name_title_width = profile_name_title_.getFont().getStringWidth(profile_name_title_.getText()) + 10;
      profile_name_title_.setBounds(kMainLeft, kProfileNameY, profile_name_title_width, kStandardHeight);
      profile_name_title_.setJustificationType(juce::Justification::centredLeft);
      addToLayout(&profile_name_title_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(profile_name_title_);

      StandardLabelSettings(profile_name_label_);
      profile_name_label_.setBounds(kMainLeft+ profile_name_title_width, kProfileNameY, kLabelWidth- profile_name_title_width, kStandardHeight);
      profile_name_label_.setJustificationType(juce::Justification::centredLeft);
      addToLayout(&profile_name_label_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(profile_name_label_);

      /* Last MIDI command */
      StandardLabelSettings(command_label_);
      command_label_.setBounds(kCommandLabelX, kCommandLabelY, kLabelWidth, kStandardHeight);
      addToLayout(&command_label_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(command_label_);

      /* Remove row button */
      remove_row_button_.setBounds(kFirstButtonX, kBottomButtonY, kButtonWidth, kStandardHeight);
      addToLayout(&remove_row_button_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(remove_row_button_);
      remove_row_button_.onClick = [this] {
         if (command_table_.getNumRows() > 0) {
            profile_.RemoveAllRows();
            command_table_.updateContent();
         }
      };

      /* Rescan MIDI button */
      rescan_button_.setBounds(kSecondButtonX, kBottomButtonY, kButtonWidth, kStandardHeight);
      addToLayout(&rescan_button_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(rescan_button_);
      rescan_button_.onClick = [this] {
         /* Re-enumerate MIDI IN and OUT devices */
         midi_receiver_.RescanDevices();
         midi_sender_.RescanDevices();
         /* Send new CC parameters to MIDI Out devices */
         lr_ipc_out_.SendCommand("FullRefresh 1\n");
      };

      /* Disconnect button */
      disconnect_button_.setBounds(kThirdButtonX, kBottomButtonY, kButtonWidth, kStandardHeight);
      disconnect_button_.setClickingTogglesState(true);
      addToLayout(&disconnect_button_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(disconnect_button_);
      disconnect_button_.onClick = [this] {
         if (disconnect_button_.getToggleState()) {
            lr_ipc_out_.SendingStop();
            disconnect_button_.setButtonText(juce::translate("Resume sending to LR"));
            rsj::Log("Sending halted.");
         }
         else {
            lr_ipc_out_.SendingRestart();
            disconnect_button_.setButtonText(juce::translate("Halt sending to LR"));
            rsj::Log("Sending restarted.");
         }
      };

      /* Delete unassigned rows */
      remove_unassigned_button_.setBounds(kFirstButtonX, kBottomButtonY2, kButtonWidth, kStandardHeight);
      addToLayout(&remove_unassigned_button_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(remove_unassigned_button_);
      remove_unassigned_button_.onClick = [this] {
         profile_.RemoveUnassignedMessages();
         command_table_.updateContent();
      };

      /* Settings button */
      settings_button_.setBounds(kThirdButtonX, kBottomButtonY2, kButtonWidth, kStandardHeight);
      addToLayout(&settings_button_, anchorMidLeft, anchorMidRight);
      addAndMakeVisible(settings_button_);
      settings_button_.onClick = [this] {
         SettingsComponent setComp(settings_manager_);
         setComp.Init();
         juce::DialogWindow::showModalDialog(juce::translate("Settings"), &setComp, nullptr, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true);
      };

      /* Try to load a default.xml if the user has not set a profile directory */
      if (settings_manager_.GetProfileDirectory().isEmpty()) {
         const auto filename {rsj::AppDataFilePath(kDefaultsFile)};
         const auto default_profile {juce::File(filename.data())};
         if (const auto parsed {juce::parseXML(default_profile)}) {
            profile_.FromXml(parsed.get());
            command_table_.updateContent();
         }
      }
      else {
         const auto last_prof {settings_manager_.GetDefaultProfile()};
         if (last_prof != juce::String())
            profile_manager_.SwitchToProfile(last_prof);
         else
            /* otherwise use the last profile from the profile directory */
            profile_manager_.SwitchToProfile(0);
      }

      /* turn it on */
      activateLayout();
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void MainContentComponent::SaveProfile() const
{
   juce::File profile_directory {settings_manager_.GetProfileDirectory()};
   if (!profile_directory.exists())
      profile_directory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
   juce::FileChooser chooser {juce::translate("Save profile"), profile_directory, "*.xml", true};
   if (chooser.browseForFileToSave(true)) {
      const auto selected_file {chooser.getResult().withFileExtension("xml")};
      profile_.ToXmlFile(selected_file);
   }
}

void MainContentComponent::MidiCmdCallback(const rsj::MidiMessage& mm)
{
   try {
      /* @jr10feb21 Handle note off message. */
      if (mm.message_type_byte == rsj::MessageType::kNoteOff) {
         for (const auto& msg : profile_.setup_table_) {
            if (msg.msgId.channel == (mm.channel+1) 
               && msg.msgId.control_number == mm.control_number
               && msg.msgId.msg_id_type == rsj::MessageType::kNoteOn
               && msg.value == 1) {
               /* jr10feb21 Send note again if the key is set to be lit (in case the controller turned it off on the note off event). */
               midi_sender_.Send(msg.msgId, msg.value);
            }
         }
         if (cheat_window_ && cheat_window_->isVisible() && settings_manager_.GetAutoRaiseCheatEnabled()) {
            cheat_window_->setVisible(false);
            cheat_window_->toBehind(getParentComponent());
         }
         /* @jr20feb21 Note off events can't be assigned to anything, so no further actions. */
         return;
      }

      if (mm.message_type_byte == rsj::MessageType::kNoteOn) {
         if (mm.channel == profile_msg_.channel &&
            mm.control_number == profile_msg_.control_number &&
            settings_manager_.GetAutoRaiseCheatEnabled() == true &&
            cheat_window_) {
            /* @jr26feb21 The same key that triggered the last profile change was pressed -> raise the cheat view. */
            cheat_window_->setVisible(true);
            cheat_window_->toFront(true);
         }

         /* @jr26feb21 Remember the last key press (for bringing the cheat window back to front later). */
         last_msg_ = mm;
      }

      /* Display the MIDI parameters and add/highlight row in table corresponding to the message msg
       * is 1-based for channel, which display expects */
      const rsj::MidiMessageId msg {mm};
      last_command_ = fmt::format(FMT_STRING("{}: {}{} [{}]"), msg.channel, mm.message_type_byte,
          msg.control_number, mm.value);
      profile_.AddRowUnmapped(msg);
      row_to_select_ = gsl::narrow_cast<size_t>(profile_.GetRowForMessage(msg));
      triggerAsyncUpdate();
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void MainContentComponent::LrIpcOutCallback(const bool connected, const bool sending_blocked)
{
   try {
      const juce::MessageManagerLock mm_lock; /* as not called in message loop */
      if (connected) {
         if (sending_blocked) {
            connection_label_.setText(
                juce::translate("Sending halted"), juce::NotificationType::dontSendNotification);
            connection_label_.setColour(juce::Label::backgroundColourId, juce::Colours::yellow);
         }
         else {
            connection_label_.setText(juce::translate("Connected to Lightroom"),
                juce::NotificationType::dontSendNotification);
            connection_label_.setColour(
                juce::Label::backgroundColourId, juce::Colours::greenyellow);
         }
      }
      else {
         connection_label_.setText(juce::translate("Not connected to Lightroom"),
             juce::NotificationType::dontSendNotification);
         connection_label_.setColour(juce::Label::backgroundColourId, juce::Colours::red);
      }
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

#pragma warning(suppress : 26461) /* must not change function signature, used as callback */
void MainContentComponent::ProfileChanged(
    juce::XmlElement* xml_element, const juce::String& file_name)
{ //-V2009 overridden method
   try {
      {
         const juce::MessageManagerLock mm_lock;
         profile_.FromXml(xml_element);

         command_table_.updateContent();
         command_table_.repaint();
         profile_name_label_.setText(file_name, juce::NotificationType::dontSendNotification);
      }

      /* Send new CC parameters to MIDI Out devices */
      lr_ipc_out_.SendCommand("FullRefresh 1\n");

      /* @jr09feb21 Send config messages to MIDI Out device */
      for (const auto& msg : profile_.setup_table_) {
         midi_sender_.Send(msg.msgId, msg.value);
      }

      /* @jr13feb21 Update cheat view */
      cheat_view_component_.ReadImage(settings_manager_.GetProfileDirectory(), file_name.replace(".xml", "", true));
      if (cheat_window_ && settings_manager_.GetAutoRaiseCheatEnabled()) {
         if (!cheat_window_->isVisible()) cheat_window_->setVisible(true);
         cheat_window_->toFront(true);
      }
      profile_msg_ = last_msg_;
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void MainContentComponent::StandardLabelSettings(juce::Label& label_to_set)
{
   try {
      label_to_set.setFont(juce::Font {16.f, juce::Font::bold});
      label_to_set.setEditable(false);
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void MainContentComponent::handleAsyncUpdate()
{
   try {
      /* Update the last command label and set its color to green */
      command_label_.setText(last_command_, juce::NotificationType::dontSendNotification);
      command_label_.setColour(juce::Label::backgroundColourId, juce::Colours::greenyellow);
      command_label_.setColour(juce::Label::textColourId, juce::Colour(juce::Colours::darkgrey));
      startTimer(1000);
      /* Update the command table to add and/or select row corresponding to midi command */
      command_table_.updateContent();
      command_table_.selectRow(gsl::narrow_cast<int>(row_to_select_));
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}

void MainContentComponent::timerCallback()
{
   try {
      /* reset the command label's background */
      command_label_.setColour(juce::Label::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
      command_label_.setColour(juce::Label::textColourId, getLookAndFeel().findColour(juce::Label::textColourId));
      juce::Timer::stopTimer();
   }
   catch (const std::exception& e) {
      MIDI2LR_E_RESPONSE;
      throw;
   }
}