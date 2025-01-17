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

#include "volume_section.h"
#include "synth_slider.h"

VolumeSection::VolumeSection(String name) : SynthSection(name) {
  addSlider(volume_ = new SynthSlider("volume"));
  volume_->setSliderStyle(Slider::LinearBar);
}

VolumeSection::~VolumeSection() {
  volume_ = nullptr;
}

void VolumeSection::resized() {
  volume_->setBounds(0, 20, getWidth(), getHeight() - 20);

  SynthSection::resized();
}
