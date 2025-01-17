/* Copyright 2013-2015 Matt Tytel
 *
 * helm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * helm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with helm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "synth_gui_interface.h"
#include "browser_look_and_feel.h"
#include "helm_common.h"
#include "load_save.h"
#include "patch_browser.h"

#define TEXT_PADDING 4.0f
#define LINUX_SYSTEM_PATCH_DIRECTORY "/usr/share/helm/patches"
#define LINUX_USER_PATCH_DIRECTORY "~/.helm/User Patches"
#define BROWSING_HEIGHT 422.0f
#define BROWSE_PADDING 8.0f


namespace {
  Array<File> getSelectedFolders(ListBox* view, FileListBoxModel* model) {
    SparseSet<int> selected_rows = view->getSelectedRows();

    Array<File> selected_folders;
    for (int i = 0; i < selected_rows.size(); ++i)
      selected_folders.add(model->getFileAtRow(selected_rows[i]));

    return selected_folders;
  }

  Array<File> getFoldersToScan(ListBox* view, FileListBoxModel* model) {
    if (view->getSelectedRows().size())
      return getSelectedFolders(view, model);

    return model->getAllFiles();
  }

  void setSelectedRows(ListBox* view, FileListBoxModel* model, Array<File> selected) {
    view->deselectAllRows();

    for (int sel = 0, row = 0; sel < selected.size() && row < model->getNumRows();) {
      String selected_path = selected[sel].getFullPathName();
      int compare = model->getFileAtRow(row).getFullPathName().compare(selected_path);

      if (compare < 0)
        ++row;
      else if (compare > 0)
        ++sel;
      else {
        view->selectRow(row, true, false);
        ++row;
        ++sel;
      }
    }
  }
} // namespace

int FileListBoxModel::getNumRows() {
  return files_.size();
}

void FileListBoxModel::paintListBoxItem(int row_number, Graphics& g,
                                        int width, int height, bool selected) {
  static Font list_font(Typeface::createSystemTypefaceFor(BinaryData::DroidSansMono_ttf,
                                                          BinaryData::DroidSansMono_ttfSize));
  g.fillAll(Colour(0xff323232));
  g.setColour(Colour(0xffdddddd));
  if (selected) {
    g.fillAll(Colour(0xff212121));
    g.setColour(Colour(0xff03a9f4));
  }

  g.setFont(list_font.withPointHeight(12.0f));
  g.drawText(files_[row_number].getFileName(),
             5, 0, width, height,
             Justification::centredLeft, true);

  g.setColour(Colour(0x88000000));
  g.fillRect(0.0f, height - 1.0f, 1.0f * width, 1.0f);
}

void FileListBoxModel::selectedRowsChanged(int last_selected_row) {
  if (listener_)
    listener_->selectedFilesChanged(this);
}

void FileListBoxModel::rescanFiles(const Array<File>& folders,
                                   bool find_files) {
  static const FileSorterAscending file_sorter;
  files_.clear();

  for (File folder : folders) {
    if (folder.isDirectory()) {
      Array<File> child_folders;
      if (find_files) {
        folder.findChildFiles(child_folders, File::findFiles, false,
                              String("*.") + mopo::PATCH_EXTENSION);
      }
      else
        folder.findChildFiles(child_folders, File::findDirectories, false);
      files_.addArray(child_folders);
    }
  }

  files_.sort(file_sorter);
}

PatchBrowser::PatchBrowser() : Component("patch_browser") {
  banks_model_ = new FileListBoxModel();
  banks_model_->setListener(this);
  Array<File> bank_locations;
  File bank_dir = LoadSave::getSystemPatchDirectory();
  bank_locations.add(bank_dir);
  banks_model_->rescanFiles(bank_locations);

  banks_view_ = new ListBox("banks", banks_model_);
  banks_view_->setMultipleSelectionEnabled(true);
  banks_view_->setClickingTogglesRowSelection(true);
  banks_view_->updateContent();
  addAndMakeVisible(banks_view_);

  folders_model_ = new FileListBoxModel();
  folders_model_->setListener(this);

  folders_view_ = new ListBox("folders", folders_model_);
  folders_view_->setMultipleSelectionEnabled(true);
  folders_view_->setClickingTogglesRowSelection(true);
  folders_view_->updateContent();
  addAndMakeVisible(folders_view_);

  patches_model_ = new FileListBoxModel();
  patches_model_->setListener(this);

  patches_view_ = new ListBox("patches", patches_model_);
  patches_view_->updateContent();
  addAndMakeVisible(patches_view_);

  banks_view_->setColour(ListBox::backgroundColourId, Colour(0xff323232));
  folders_view_->setColour(ListBox::backgroundColourId, Colour(0xff323232));
  patches_view_->setColour(ListBox::backgroundColourId, Colour(0xff323232));

  selectedFilesChanged(banks_model_);
  selectedFilesChanged(folders_model_);
}

PatchBrowser::~PatchBrowser() {
}

void PatchBrowser::paint(Graphics& g) {
  static Font info_font(Typeface::createSystemTypefaceFor(BinaryData::RobotoLight_ttf,
                                                          BinaryData::RobotoLight_ttfSize));
  static Font data_font(Typeface::createSystemTypefaceFor(BinaryData::DroidSansMono_ttf,
                                                          BinaryData::DroidSansMono_ttfSize));
  g.fillAll(Colour(0xbb212121));
  g.setColour(Colour(0xff111111));
  g.fillRect(0.0f, 0.0f, 1.0f * getWidth(), BROWSING_HEIGHT);

  g.setColour(Colour(0xff212121));
  g.fillRect(8.0f, 8.0f, 3.0f * getWidth() / 10.0f - BROWSE_PADDING, BROWSING_HEIGHT - 16.0f);

  float width = 3.0f * getWidth() / 10.0f + BROWSE_PADDING;
  float division = 100.0f;
  float buffer = 20.0f;

  g.setFont(info_font.withPointHeight(14.0f));
  g.setColour(Colour(0xff888888));

  g.fillRect(BROWSE_PADDING + division + buffer / 2.0f, BROWSE_PADDING + 30.0f,
             1.0f, 120.0f);

  g.drawText(TRANS("PATCH NAME"),
             BROWSE_PADDING, BROWSE_PADDING + 40.0f, division, 20.0f,
             Justification::centredRight, false);
  g.drawText(TRANS("AUTHOR"),
             BROWSE_PADDING, BROWSE_PADDING + 80.0f, division, 20.0f,
             Justification::centredRight, false);
  g.drawText(TRANS("BANK"),
             BROWSE_PADDING, BROWSE_PADDING + 120.0f, division, 20.0f,
             Justification::centredRight, false);

  if (isPatchSelected()) {
    g.setFont(data_font.withPointHeight(12.0f));
    g.setColour(Colour(0xff03a9f4));

    File selected_patch = getSelectedPatch();
    float data_width = width - division - buffer - 2.0f * BROWSE_PADDING;
    g.drawText(selected_patch.getFileNameWithoutExtension(),
               BROWSE_PADDING + division + buffer, BROWSE_PADDING + 40.0f, data_width, 20.0f,
               Justification::centredLeft, true);
    g.drawText(selected_patch.getParentDirectory().getParentDirectory().getFileName(),
               BROWSE_PADDING + division + buffer, BROWSE_PADDING + 80.0f, data_width, 20.0f,
               Justification::centredLeft, true);
    g.drawText(selected_patch.getParentDirectory().getParentDirectory().getFileName(),
               BROWSE_PADDING + division + buffer, BROWSE_PADDING + 120.0f, data_width, 20.0f,
               Justification::centredLeft, true);
  }
}

void PatchBrowser::resized() {
  float start_x = 3.0f * getWidth() / 10.0f + BROWSE_PADDING;
  float width1 = 2.0f * (getWidth() - start_x) / 7.0f - BROWSE_PADDING;
  float width2 = 3.0f * (getWidth() - start_x) / 7.0f - BROWSE_PADDING;
  float height = BROWSING_HEIGHT - 2.0f * BROWSE_PADDING;
  banks_view_->setBounds(start_x, BROWSE_PADDING, width1, height);
  folders_view_->setBounds(start_x + width1 + BROWSE_PADDING, BROWSE_PADDING, width1, height);
  patches_view_->setBounds(start_x + 2.0f * (width1 + BROWSE_PADDING), BROWSE_PADDING,
                           width2, height);
}

void PatchBrowser::selectedFilesChanged(FileListBoxModel* model) {
  if (model == banks_model_) {
    Array<File> banks = getFoldersToScan(banks_view_, banks_model_);
    Array<File> folders_selected = getSelectedFolders(folders_view_, folders_model_);

    folders_model_->rescanFiles(banks);
    folders_view_->updateContent();
    setSelectedRows(folders_view_, folders_model_, folders_selected);
  }
  if (model == banks_model_ || model == folders_model_) {
    Array<File> folders = getFoldersToScan(folders_view_, folders_model_);
    Array<File> patches_selected = getSelectedFolders(patches_view_, patches_model_);

    patches_model_->rescanFiles(folders, true);
    patches_view_->updateContent();
    setSelectedRows(patches_view_, patches_model_, patches_selected);
  }
  else if (model == patches_model_) {
    SparseSet<int> selected_rows = patches_view_->getSelectedRows();
    if (selected_rows.size()) {
      File patch = patches_model_->getFileAtRow(selected_rows[0]);
      loadFromFile(patch);
    }
    repaint();
  }
}

void PatchBrowser::mouseUp(const MouseEvent& e) {
  if (e.getPosition().y > BROWSING_HEIGHT)
    setVisible(false);
}

File PatchBrowser::getSelectedPatch() {
  SparseSet<int> selected_rows = patches_view_->getSelectedRows();
  if (selected_rows.size())
    return patches_model_->getFileAtRow(selected_rows[0]);
  return File();
}

void PatchBrowser::loadPrevPatch() {
  SparseSet<int> selected_rows = patches_view_->getSelectedRows();
  if (selected_rows.size()) {
    int row = selected_rows[0] - 1;
    if (row < 0)
      row += patches_model_->getNumRows();
    patches_view_->selectRow(row);
  }
  else
    patches_view_->selectRow(0);
}

void PatchBrowser::loadNextPatch() {
  SparseSet<int> selected_rows = patches_view_->getSelectedRows();
  if (selected_rows.size()) {
    int row = selected_rows[0] + 1;
    if (row >= patches_model_->getNumRows())
      row -= patches_model_->getNumRows();
    patches_view_->selectRow(row);
  }
  else
    patches_view_->selectRow(0);
}

void PatchBrowser::loadFromFile(File& patch) {
  var parsed_json_state;
  if (JSON::parse(patch.loadFileAsString(), parsed_json_state).wasOk()) {
    SynthGuiInterface* parent = findParentComponentOfClass<SynthGuiInterface>();
    parent->loadFromVar(parsed_json_state);
  }
}
