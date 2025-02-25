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

#pragma once
#ifndef HELM_VOICE_HANDLER_H
#define HELM_VOICE_HANDLER_H

#include "mopo.h"
#include "helm_common.h"
#include "helm_module.h"

#include <vector>

namespace mopo {
  class BypassRouter;
  class Delay;
  class Distortion;
  class Envelope;
  class Filter;
  class FormantManager;
  class LinearSlope;
  class Oscillator;
  class SmoothValue;
  class StepGenerator;
  class TriggerCombiner;
  class HelmOscillators;

  // The voice handler duplicates processors to produce polyphony.
  // Everything in the synthesizer we want per-voice instances of must be
  // contained in here.
  class HelmVoiceHandler : public virtual VoiceHandler, public virtual HelmModule {
    public:
      HelmVoiceHandler(Processor* beats_per_second);
      virtual ~HelmVoiceHandler() { } // Should probably delete things.

      void init() override;

      void process() override;
      void noteOn(mopo_float note, mopo_float velocity = 1,
                  int sample = 0) override;
      VoiceEvent noteOff(mopo_float note, int sample = 0) override;
      void setModWheel(mopo_float value);
      void setPitchWheel(mopo_float value);
      Output* note_retrigger() { return &note_retriggered_; }

    private:
      // Create the portamento, legato, amplifier envelope and other processors
      // that effect how voices start and turn into other notes.
      void createArticulation(Output* note, Output* velocity, Output* trigger);

      // Create the oscillators and hook up frequency controls.
      void createOscillators(Output* frequency, Output* reset);

      // Create the LFOs, Step Sequencers, etc.
      void createModulators(Output* reset);

      // Create the filter and filter envelope.
      void createFilter(Output* audio, Output* keytrack, Output* reset, Output* note_event);

      Processor* beats_per_second_;

      Add* note_from_center_;
      SmoothValue* mod_wheel_amount_;
      SmoothValue* pitch_wheel_amount_;
      PortamentoSlope* current_frequency_;
      Envelope* amplitude_envelope_;
      Multiply* amplitude_;
      SimpleDelay* osc_feedback_;

      Value* legato_;
      Distortion* distorted_filter_;
      FormantManager* formant_filter_;
      Envelope* filter_envelope_;
      BypassRouter* formant_container_;
      Output note_retriggered_;

      Multiply* output_;
  };
} // namespace mopo

#endif // HELM_VOICE_HANDLER_H
