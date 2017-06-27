/* -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*- */
// $Id: MidiClass.h 2056 2014-09-13 23:45:17Z tk $
//
// MidiClass.h - blm_scalar_emulation 
//
// Created by Phil Taylor 2010
//
#ifndef _MIDI_H


class MidiClass : public Component,
                 public ButtonListener
{
public:
	MidiClass();
	~MidiClass();
	void sendBLMLayout(void);
private:


};

#endif /* _BLM_H */
