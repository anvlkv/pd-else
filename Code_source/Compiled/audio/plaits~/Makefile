
lib.name = plaits~

common.sources = stmlib/dsp/units.cc \
                 stmlib/utils/random.cc \
                 stmlib/dsp/atan.cc \
                 plaits/dsp/voice.cc \
                 plaits/dsp/engine/additive_engine.cc \
                 plaits/dsp/engine/bass_drum_engine.cc \
                 plaits/dsp/engine/chord_engine.cc \
                 plaits/dsp/engine/fm_engine.cc \
                 plaits/dsp/engine/grain_engine.cc \
                 plaits/dsp/engine/hi_hat_engine.cc \
                 plaits/dsp/engine/modal_engine.cc \
                 plaits/dsp/engine/noise_engine.cc \
                 plaits/dsp/engine/particle_engine.cc \
                 plaits/dsp/engine/snare_drum_engine.cc \
                 plaits/dsp/engine/speech_engine.cc \
                 plaits/dsp/engine/string_engine.cc \
                 plaits/dsp/engine/swarm_engine.cc \
                 plaits/dsp/engine/virtual_analog_engine.cc \
                 plaits/dsp/engine/waveshaping_engine.cc \
                 plaits/dsp/engine/wavetable_engine.cc \
                 plaits/dsp/speech/lpc_speech_synth.cc \
                 plaits/dsp/speech/lpc_speech_synth_controller.cc \
                 plaits/dsp/speech/lpc_speech_synth_phonemes.cc \
                 plaits/dsp/speech/lpc_speech_synth_words.cc \
                 plaits/dsp/speech/naive_speech_synth.cc \
                 plaits/dsp/speech/sam_speech_synth.cc \
                 plaits/dsp/physical_modelling/modal_voice.cc \
                 plaits/dsp/physical_modelling/resonator.cc \
                 plaits/dsp/physical_modelling/string.cc \
                 plaits/dsp/physical_modelling/string_voice.cc \
				         plaits/dsp/engine2/chiptune_engine.cc \
				         plaits/dsp/engine2/phase_distortion_engine.cc \
				         plaits/dsp/engine2/six_op_engine.cc \
				         plaits/dsp/engine2/string_machine_engine.cc \
				         plaits/dsp/engine2/virtual_analog_vcf_engine.cc \
				         plaits/dsp/engine2/wave_terrain_engine.cc \
				         plaits/dsp/fm/algorithms.cc \
				         plaits/dsp/fm/dx_units.cc \
				         plaits/dsp/chords/chord_bank.cc \
                 plaits/resources.cc

# input source file (class name == source file basename)
class.sources = plaits~.cpp

cflags = -DTEST -Wno-unused-local-typedefs -I$(shell pwd)

# all extra files to be included in binary distribution of the library
define forDarwin
  cflags += -I /usr/include/c++/v1 -stdlib=libc++ -mmacosx-version-min=10.9
endef

# Change the arch to arm64 if the extension is d_arm64
ifeq ($(extension),d_arm64)
  override arch := arm64
endif

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
