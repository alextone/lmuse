HEADERS = \
	./synti/deicsonze/deicsonzefilter.h \
	./synti/deicsonze/deicsonzeplugin.h \
	./synti/deicsonze/deicsonze.h \
	./synti/deicsonze/common_defs.h \
	./synti/deicsonze/deicsonzegui.h \
	./synti/deicsonze/deicsonzepreset.h \
	./synti/organ/organgui.h \
	./synti/organ/common_defs.h \
	./synti/organ/organ.h \
	./synti/vam/vam.h \
	./synti/vam/vamgui.h \
	./synti/vam/common_defs.h \
	./synti/fluid/fluid.h \
	./synti/fluid/common_defs.h \
	./synti/fluid/fluidgui.h \
	./synti/fluidsynth/fluidsynti.h \
	./synti/fluidsynth/fluidsynthgui.h \
	./synti/fluidsynth/common_defs.h \
	./synti/simpledrums2/simpledrumsgui.h \
	./synti/simpledrums2/ssplugingui.h \
	./synti/simpledrums2/ssplugin.h \
	./synti/simpledrums2/simpledrums.h \
	./synti/simpledrums2/common_defs.h \
	./synti/simpledrums2/common.h \
	./synti/libsynti/poly.h \
	./synti/libsynti/gui.h \
	./synti/libsynti/mono.h \
	./synti/libsynti/mess.h \
	./muse/key.h \
	./muse/cobject.h \
	./muse/mpevent.h \
	./muse/miditransform.h \
	./muse/ticksynth.h \
	./muse/globals.h \
	./muse/mididev.h \
	./muse/helper.h \
	./muse/mixer/amixer.h \
	./muse/mixer/panknob.h \
	./muse/mixer/astrip.h \
	./muse/mixer/mstrip.h \
	./muse/mixer/routedialog.h \
	./muse/mixer/rack.h \
	./muse/mixer/auxknob.h \
	./muse/mixer/strip.h \
	./muse/ctrl/ctrledit.h \
	./muse/ctrl/ctrlcanvas.h \
	./muse/ctrl/ctrlpanel.h \
	./muse/xml.h \
	./muse/mtc.h \
	./muse/master/tscale.h \
	./muse/master/lmaster.h \
	./muse/master/master.h \
	./muse/master/masteredit.h \
	./muse/node.h \
	./muse/remote/pyapi.h \
	./muse/value.h \
	./muse/midiseq.h \
	./muse/gui.h \
	./muse/keyevent.h \
	./muse/wave.h \
	./muse/osc.h \
	./muse/track.h \
	./muse/shortcuts.h \
	./muse/functions.h \
	./muse/midievent.h \
	./muse/ctrl.h \
	./muse/cleftypes.h \
	./muse/midi.h \
	./muse/arranger/tlist.h \
	./muse/arranger/alayout.h \
	./muse/arranger/trackautomationview.h \
	./muse/arranger/arrangerview.h \
	./muse/arranger/pcanvas.h \
	./muse/arranger/arranger.h \
	./muse/undo.h \
	./muse/part.h \
	./muse/midifile.h \
	./muse/debug.h \
	./muse/confmport.h \
	./muse/driver/rtctimer.h \
	./muse/driver/timerdev.h \
	./muse/driver/jackmidi.h \
	./muse/driver/audiodev.h \
	./muse/driver/alsamidi.h \
	./muse/driver/jackaudio.h \
	./muse/driver/alsatimer.h \
	./muse/waveevent.h \
	./muse/marker/marker.h \
	./muse/marker/markerview.h \
	./muse/event.h \
	./muse/ladspa.h \
	./muse/audioprefetch.h \
	./muse/dssihost.h \
	./muse/fastlog.h \
	./muse/pos.h \
	./muse/waveedit/waveview.h \
	./muse/waveedit/editgain.h \
	./muse/waveedit/waveedit.h \
	./muse/controlfifo.h \
	./muse/vst.h \
	./muse/memory.h \
	./muse/instruments/editinstrument.h \
	./muse/instruments/minstrument.h \
	./muse/instruments/midictrledit.h \
	./muse/midictrl.h \
	./muse/dialogs.h \
	./muse/gconfig.h \
	./muse/mplugins/mittranspose.h \
	./muse/mplugins/midiitransform.h \
	./muse/mplugins/mitplugin.h \
	./muse/mplugins/rhythm.h \
	./muse/mplugins/mrconfig.h \
	./muse/mplugins/midifilterimpl.h \
	./muse/mplugins/random.h \
	./muse/route.h \
	./muse/device.h \
	./muse/widgets/genset.h \
	./muse/widgets/mdisettings.h \
	./muse/widgets/mlabel.h \
	./muse/widgets/citem.h \
	./muse/widgets/ttoolbar.h \
	./muse/widgets/ctrlcombo.h \
	./muse/widgets/visibletracks.h \
	./muse/widgets/intlabel.h \
	./muse/widgets/tools.h \
	./muse/widgets/vscale.h \
	./muse/widgets/dentry.h \
	./muse/widgets/mtscale.h \
	./muse/widgets/action.h \
	./muse/widgets/menutitleitem.h \
	./muse/widgets/section.h \
	./muse/widgets/swidget.h \
	./muse/widgets/drange.h \
	./muse/widgets/mmath.h \
	./muse/widgets/utils.h \
	./muse/widgets/siglabel.h \
	./muse/widgets/sigscale.h \
	./muse/widgets/checkbox.h \
	./muse/widgets/mtscale_flo.h \
	./muse/widgets/slider.h \
	./muse/widgets/midisyncimpl.h \
	./muse/widgets/splitter.h \
	./muse/widgets/sclif.h \
	./muse/widgets/function_dialogs/move.h \
	./muse/widgets/function_dialogs/transpose.h \
	./muse/widgets/function_dialogs/deloverlaps.h \
	./muse/widgets/function_dialogs/legato.h \
	./muse/widgets/function_dialogs/remove.h \
	./muse/widgets/function_dialogs/setlen.h \
	./muse/widgets/function_dialogs/gatetime.h \
	./muse/widgets/function_dialogs/quantize.h \
	./muse/widgets/function_dialogs/crescendo.h \
	./muse/widgets/function_dialogs/velocity.h \
	./muse/widgets/ttoolbutton.h \
	./muse/widgets/verticalmeter.h \
	./muse/widgets/noteinfo.h \
	./muse/widgets/spinbox.h \
	./muse/widgets/header.h \
	./muse/widgets/posedit.h \
	./muse/widgets/nentry.h \
	./muse/widgets/pitchedit.h \
	./muse/widgets/scrollscale.h \
	./muse/widgets/didyouknow.h \
	./muse/widgets/doublelabel.h \
	./muse/widgets/knob.h \
	./muse/widgets/metronome.h \
	./muse/widgets/unusedwavefiles.h \
	./muse/widgets/popupmenu.h \
	./muse/widgets/comment.h \
	./muse/widgets/pitchlabel.h \
	./muse/widgets/meter.h \
	./muse/widgets/combobox.h \
	./muse/widgets/view.h \
	./muse/widgets/projectcreateimpl.h \
	./muse/widgets/filedialog.h \
	./muse/widgets/bigtime.h \
	./muse/widgets/routepopup.h \
	./muse/widgets/poslabel.h \
	./muse/widgets/sliderbase.h \
	./muse/widgets/comboQuant.h \
	./muse/widgets/aboutbox_impl.h \
	./muse/widgets/wtscale.h \
	./muse/widgets/mtrackinfo.h \
	./muse/widgets/canvas.h \
	./muse/widgets/shortcutconfig.h \
	./muse/widgets/scldiv.h \
	./muse/widgets/lcombo.h \
	./muse/widgets/hitscale.h \
	./muse/widgets/tempolabel.h \
	./muse/widgets/mixdowndialog.h \
	./muse/widgets/pastedialog.h \
	./muse/widgets/pasteeventsdialog.h \
	./muse/widgets/sigedit.h \
	./muse/widgets/songinfo.h \
	./muse/widgets/scldraw.h \
	./muse/widgets/tb1.h \
	./muse/widgets/spinboxFP.h \
	./muse/widgets/dimap.h \
	./muse/widgets/shortcutcapturedialog.h \
	./muse/midiport.h \
	./muse/thread.h \
	./muse/audioconvert.h \
	./muse/sig.h \
	./muse/midiedit/drumedit.h \
	./muse/midiedit/dcanvas.h \
	./muse/midiedit/prcanvas.h \
	./muse/midiedit/pianoroll.h \
	./muse/midiedit/ecanvas.h \
	./muse/midiedit/piano.h \
	./muse/midiedit/cmd.h \
	./muse/midiedit/dlist.h \
	./muse/midiedit/drummap.h \
	./muse/midiedit/scoreedit.h \
	./muse/steprec.h \
	./muse/midieditor.h \
	./muse/tempo.h \
	./muse/plugin.h \
	./muse/song.h \
	./muse/audio.h \
	./muse/synth.h \
	./muse/icons.h \
	./muse/liste/listedit.h \
	./muse/liste/editevent.h \
	./muse/eventbase.h \
	./muse/cliplist/cliplist.h \
	./muse/default_click.h \
	./muse/stringparam.h \
	./muse/app.h \
	./muse/transport.h \
	./muse/appearance.h \
	./muse/structure.h \
	./muse/sync.h \
	./muse/evdata.h \
	./muse/conf.h \
	./muse/globaldefs.h \
	./al/xml.h \
	./al/sig.h \
	./al/dsp.h \
	./al/al.h \
	./plugins/freeverb/comb.h \
	./plugins/freeverb/denormals.h \
	./plugins/freeverb/revmodel.h \
	./plugins/freeverb/tuning.h \
	./plugins/freeverb/allpass.h \
	./plugins/doublechorus/doublechorusmodel.h \
	./plugins/doublechorus/simplechorusmodel.h \
	./plugins/pandelay/pandelaymodel.h \
	./plugins/pandelay/ladspapandelay.h \
	./awl/aslider.h \
	./awl/tempoedit.h \
	./awl/midivolentry.h \
	./awl/panknob.h \
	./awl/utils.h \
	./awl/awlplugin.h \
	./awl/floatentry.h \
	./awl/checkbox.h \
	./awl/midipanknob.h \
	./awl/slider.h \
	./awl/awl.h \
	./awl/volentry.h \
	./awl/fastlog.h \
	./awl/posedit.h \
	./awl/pitchedit.h \
	./awl/midimeter.h \
	./awl/knob.h \
	./awl/midipanentry.h \
	./awl/pitchlabel.h \
	./awl/combobox.h \
	./awl/volknob.h \
	./awl/poslabel.h \
	./awl/midimslider.h \
	./awl/drawbar.h \
	./awl/panentry.h \
	./awl/mslider.h \
	./awl/tempolabel.h \
	./awl/volslider.h \
	./awl/sigedit.h \
	./awl/tcanvas.h \
	./all.h \
 

SOURCES = \
	./synti/deicsonze/deicsonzeplugin.cpp \
	./synti/deicsonze/deicsonze.cpp \
	./synti/deicsonze/deicsonzegui.cpp \
	./synti/deicsonze/deicsonzepreset.cpp \
	./synti/deicsonze/deicsonzefilter.cpp \
	./synti/organ/organ.cpp \
	./synti/organ/organgui.cpp \
	./synti/vam/vam.cpp \
	./synti/vam/vamgui.cpp \
	./synti/fluid/fluidgui.cpp \
	./synti/fluid/fluid.cpp \
	./synti/s1/s1.cpp \
	./synti/fluidsynth/fluidsynti.cpp \
	./synti/fluidsynth/fluidsynthgui.cpp \
	./synti/simpledrums2/simpledrums.cpp \
	./synti/simpledrums2/ssplugin.cpp \
	./synti/simpledrums2/ssplugingui.cpp \
	./synti/simpledrums2/simpledrumsgui.cpp \
	./synti/libsynti/mono.cpp \
	./synti/libsynti/gui.cpp \
	./synti/libsynti/mess.cpp \
	./synti/libsynti/poly.cpp \
	./muse/appearance.cpp \
	./muse/helper.cpp \
	./muse/audiotrack.cpp \
	./muse/eventlist.cpp \
	./muse/mixer/astrip.cpp \
	./muse/mixer/panknob.cpp \
	./muse/mixer/amixer.cpp \
	./muse/mixer/auxknob.cpp \
	./muse/mixer/routedialog.cpp \
	./muse/mixer/mstrip.cpp \
	./muse/mixer/rack.cpp \
	./muse/mixer/strip.cpp \
	./muse/ctrl/ctrledit.cpp \
	./muse/ctrl/ctrlcanvas.cpp \
	./muse/ctrl/ctrlpanel.cpp \
	./muse/xml.cpp \
	./muse/midievent.cpp \
	./muse/exportmidi.cpp \
	./muse/value.cpp \
	./muse/master/lmaster.cpp \
	./muse/master/masteredit.cpp \
	./muse/master/master.cpp \
	./muse/master/tscale.cpp \
	./muse/remote/pyapi.cpp \
	./muse/audio.cpp \
	./muse/main.cpp \
	./muse/dialogs.cpp \
	./muse/route.cpp \
	./muse/dssihost.cpp \
	./muse/midifile.cpp \
	./muse/ctrl.cpp \
	./muse/wave.cpp \
	./muse/ticksynth.cpp \
	./muse/arranger/trackautomationview.cpp \
	./muse/arranger/arranger.cpp \
	./muse/arranger/arrangerview.cpp \
	./muse/arranger/pcanvas.cpp \
	./muse/arranger/tlist.cpp \
	./muse/arranger/alayout.cpp \
	./muse/confmport.cpp \
	./muse/seqmsg.cpp \
	./muse/stringparam.cpp \
	./muse/gconfig.cpp \
	./muse/miditransform.cpp \
	./muse/driver/rtctimer.cpp \
	./muse/driver/dummyaudio.cpp \
	./muse/driver/jackmidi.cpp \
	./muse/driver/alsatimer.cpp \
	./muse/driver/alsamidi.cpp \
	./muse/driver/jack.cpp \
	./muse/importmidi.cpp \
	./muse/marker/markerview.cpp \
	./muse/marker/marker.cpp \
	./muse/memory.cpp \
	./muse/vst.cpp \
	./muse/transport.cpp \
	./muse/steprec.cpp \
	./muse/sync.cpp \
	./muse/plugin.cpp \
	./muse/midictrl.cpp \
	./muse/structure.cpp \
	./muse/sig.cpp \
	./muse/wavetrack.cpp \
	./muse/midieditor.cpp \
	./muse/icons.cpp \
	./muse/audioprefetch.cpp \
	./muse/midiport.cpp \
	./muse/waveedit/waveedit.cpp \
	./muse/waveedit/waveview.cpp \
	./muse/waveedit/editgain.cpp \
	./muse/conf.cpp \
	./muse/synth.cpp \
	./muse/audioconvert.cpp \
	./muse/mpevent.cpp \
	./muse/track.cpp \
	./muse/instruments/minstrument.cpp \
	./muse/instruments/editinstrument.cpp \
	./muse/instruments/midictrledit.cpp \
	./muse/controlfifo.cpp \
	./muse/mplugins/midiitransform.cpp \
	./muse/mplugins/midifilterimpl.cpp \
	./muse/mplugins/random.cpp \
	./muse/mplugins/rhythm.cpp \
	./muse/mplugins/mrconfig.cpp \
	./muse/mplugins/mitplugin.cpp \
	./muse/mplugins/mittranspose.cpp \
	./muse/functions.cpp \
	./muse/widgets/aboutbox_impl.cpp \
	./muse/widgets/metronome.cpp \
	./muse/widgets/nentry.cpp \
	./muse/widgets/sclif.cpp \
	./muse/widgets/pitchedit.cpp \
	./muse/widgets/vscale.cpp \
	./muse/widgets/sigscale.cpp \
	./muse/widgets/intlabel.cpp \
	./muse/widgets/midisyncimpl.cpp \
	./muse/widgets/spinbox.cpp \
	./muse/widgets/lcombo.cpp \
	./muse/widgets/menutitleitem.cpp \
	./muse/widgets/moc_ttoolbar.cpp \
	./muse/widgets/drange.cpp \
	./muse/widgets/posedit.cpp \
	./muse/widgets/dentry.cpp \
	./muse/widgets/shortcutconfig.cpp \
	./muse/widgets/dimap.cpp \
	./muse/widgets/genset.cpp \
	./muse/widgets/mtrackinfo.cpp \
	./muse/widgets/unusedwavefiles.cpp \
	./muse/widgets/splitter.cpp \
	./muse/widgets/pastedialog.cpp \
	./muse/widgets/pasteeventsdialog.cpp \
	./muse/widgets/ctrlcombo.cpp \
	./muse/widgets/siglabel.cpp \
	./muse/widgets/projectcreateimpl.cpp \
	./muse/widgets/canvas.cpp \
	./muse/widgets/citem.cpp \
	./muse/widgets/comboQuant.cpp \
	./muse/widgets/utils.cpp \
	./muse/widgets/hitscale.cpp \
	./muse/widgets/checkbox.cpp \
	./muse/widgets/combobox.cpp \
	./muse/widgets/musewidgetsplug.cpp \
	./muse/widgets/spinboxFP.cpp \
	./muse/widgets/tb1.cpp \
	./muse/widgets/meter.cpp \
	./muse/widgets/function_dialogs/deloverlaps.cpp \
	./muse/widgets/function_dialogs/quantize.cpp \
	./muse/widgets/function_dialogs/crescendo.cpp \
	./muse/widgets/function_dialogs/move.cpp \
	./muse/widgets/function_dialogs/velocity.cpp \
	./muse/widgets/function_dialogs/setlen.cpp \
	./muse/widgets/function_dialogs/remove.cpp \
	./muse/widgets/function_dialogs/legato.cpp \
	./muse/widgets/function_dialogs/transpose.cpp \
	./muse/widgets/function_dialogs/gatetime.cpp \
	./muse/widgets/visibletracks.cpp \
	./muse/widgets/sigedit.cpp \
	./muse/widgets/view.cpp \
	./muse/widgets/noteinfo.cpp \
	./muse/widgets/poslabel.cpp \
	./muse/widgets/mdisettings.cpp \
	./muse/widgets/popupmenu.cpp \
	./muse/widgets/mixdowndialog.cpp \
	./muse/widgets/bigtime.cpp \
	./muse/widgets/swidget.cpp \
	./muse/widgets/mtscale.cpp \
	./muse/widgets/sliderbase.cpp \
	./muse/widgets/mtscale_flo.cpp \
	./muse/widgets/shortcutcapturedialog.cpp \
	./muse/widgets/mmath.cpp \
	./muse/widgets/knob.cpp \
	./muse/widgets/ttoolbutton.cpp \
	./muse/widgets/doublelabel.cpp \
	./muse/widgets/scrollscale.cpp \
	./muse/widgets/header.cpp \
	./muse/widgets/comment.cpp \
	./muse/widgets/verticalmeter.cpp \
	./muse/widgets/ttoolbar.cpp \
	./muse/widgets/slider.cpp \
	./muse/widgets/scldiv.cpp \
	./muse/widgets/routepopup.cpp \
	./muse/widgets/filedialog.cpp \
	./muse/widgets/scldraw.cpp \
	./muse/widgets/tools.cpp \
	./muse/widgets/tempolabel.cpp \
	./muse/widgets/mlabel.cpp \
	./muse/widgets/wtscale.cpp \
	./muse/widgets/pitchlabel.cpp \
	./muse/shortcuts.cpp \
	./muse/event.cpp \
	./muse/cobject.cpp \
	./muse/part.cpp \
	./muse/midiedit/dlist.cpp \
	./muse/midiedit/dcanvas.cpp \
	./muse/midiedit/piano.cpp \
	./muse/midiedit/pianoroll.cpp \
	./muse/midiedit/prcanvas.cpp \
	./muse/midiedit/drummap.cpp \
	./muse/midiedit/scoreedit.cpp \
	./muse/midiedit/ecanvas.cpp \
	./muse/midiedit/drumedit.cpp \
	./muse/thread.cpp \
	./muse/help.cpp \
	./muse/undo.cpp \
	./muse/app.cpp \
	./muse/midiseq.cpp \
	./muse/osc.cpp \
	./muse/keyevent.cpp \
	./muse/waveevent.cpp \
	./muse/liste/editevent.cpp \
	./muse/liste/listedit.cpp \
	./muse/mididev.cpp \
	./muse/mtc.cpp \
	./muse/tempo.cpp \
	./muse/cliplist/cliplist.cpp \
	./muse/pos.cpp \
	./muse/song.cpp \
	./muse/key.cpp \
	./muse/node.cpp \
	./muse/globals.cpp \
	./muse/midi.cpp \
	./muse/songfile.cpp \
	./al/xml.cpp \
	./al/sig.cpp \
	./al/dsp.cpp \
	./al/dspSSE.cpp \
	./al/al.cpp \
	./al/dspXMM.cpp \
	./plugins/freeverb/revmodel.cpp \
	./plugins/freeverb/freeverb.cpp \
	./plugins/doublechorus/simplechorusmodel.cpp \
	./plugins/doublechorus/doublechorusmodel.cpp \
	./plugins/doublechorus/doublechorus.cpp \
	./plugins/pandelay/pandelay.cpp \
	./plugins/pandelay/pandelaymodel.cpp \
	./plugins/pandelay/ladspapandelay.cpp \
	./awl/pitchedit.cpp \
	./awl/floatentry.cpp \
	./awl/posedit.cpp \
	./awl/tempoedit.cpp \
	./awl/midimslider.cpp \
	./awl/utils.cpp \
	./awl/checkbox.cpp \
	./awl/midimeter.cpp \
	./awl/combobox.cpp \
	./awl/panknob.cpp \
	./awl/aslider.cpp \
	./awl/sigedit.cpp \
	./awl/midivolentry.cpp \
	./awl/awlplugin.cpp \
	./awl/mslider.cpp \
	./awl/poslabel.cpp \
	./awl/drawbar.cpp \
	./awl/awl.cpp \
	./awl/volknob.cpp \
	./awl/ltest.cpp \
	./awl/panentry.cpp \
	./awl/knob.cpp \
	./awl/slider.cpp \
	./awl/volentry.cpp \
	./awl/tcanvas.cpp \
	./awl/tempolabel.cpp \
	./awl/midipanentry.cpp \
	./awl/midipanknob.cpp \
	./awl/volslider.cpp \
	./awl/pitchlabel.cpp \
	./grepmidi/grepmidi.cpp \
 

FORMS = \
	./synti/deicsonze/deicsonzegui.ui \
	./synti/organ/organguibase.ui \
	./synti/vam/vamguibase.ui \
	./synti/fluid/fluidguibase.ui \
	./synti/fluidsynth/fluidsynthguibase.ui \
	./synti/simpledrums2/simpledrumsguibase.ui \
	./synti/simpledrums2/sspluginchooserbase.ui \
	./muse/mixer/routedialogbase.ui \
	./muse/waveedit/editgainbase.ui \
	./muse/instruments/ccontrolbase.ui \
	./muse/instruments/editinstrumentbase.ui \
	./muse/mplugins/rhythmbase.ui \
	./muse/mplugins/midifilter.ui \
	./muse/mplugins/mrconfigbase.ui \
	./muse/widgets/synthconfigbase.ui \
	./muse/widgets/mittransposebase.ui \
	./muse/widgets/fdialogbuttons.ui \
	./muse/widgets/midisync.ui \
	./muse/widgets/mdisettings_base.ui \
	./muse/widgets/mtrackinfobase.ui \
	./muse/widgets/shortcutcapturedialogbase.ui \
	./muse/widgets/appearancebase.ui \
	./muse/widgets/unusedwavefiles.ui \
	./muse/widgets/configmidifilebase.ui \
	./muse/widgets/songinfo.ui \
	./muse/widgets/shortcutconfigbase.ui \
	./muse/widgets/transformbase.ui \
	./muse/widgets/gensetbase.ui \
	./muse/widgets/metronomebase.ui \
	./muse/widgets/mixdowndialogbase.ui \
	./muse/widgets/commentbase.ui \
	./muse/widgets/editnotedialogbase.ui \
	./muse/widgets/function_dialogs/removebase.ui \
	./muse/widgets/function_dialogs/setlenbase.ui \
	./muse/widgets/function_dialogs/transposebase.ui \
	./muse/widgets/function_dialogs/movebase.ui \
	./muse/widgets/function_dialogs/quantbase.ui \
	./muse/widgets/function_dialogs/velocitybase.ui \
	./muse/widgets/function_dialogs/deloverlapsbase.ui \
	./muse/widgets/function_dialogs/crescendobase.ui \
	./muse/widgets/function_dialogs/legatobase.ui \
	./muse/widgets/function_dialogs/gatetimebase.ui \
	./muse/widgets/cliplisteditorbase.ui \
	./muse/widgets/editsysexdialogbase.ui \
	./muse/widgets/projectcreate.ui \
	./muse/widgets/pastedialogbase.ui \
	./muse/widgets/pasteeventsdialogbase.ui \
	./muse/widgets/didyouknow.ui \
	./muse/widgets/itransformbase.ui \
	./muse/widgets/aboutbox.ui \
	./muse/liste/editctrlbase.ui \
	./share/plugins/1050.ui \
	./share/plugins/2142.ui \
 

TRANSLATIONS = \
	./share/locale/muse_sv_SE.ts \
	./share/locale/muse_de.ts \
	./share/locale/muse_ru.ts \
	./share/locale/muse_pl.ts \
	./share/locale/muse_en.ts \
	./share/locale/muse_es.ts \
	./share/locale/muse_fr.ts \
 

