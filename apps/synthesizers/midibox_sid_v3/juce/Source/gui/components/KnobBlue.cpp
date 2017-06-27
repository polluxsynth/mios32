/* -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*- */
// $Id: KnobBlue.cpp 2070 2014-09-27 23:35:11Z tk $

#include "KnobBlue.h"

//==============================================================================
KnobBlue::KnobBlue(const String &name)
    : Knob(name, 48, 48, ImageCache::getFromMemory(BinaryData::knob_blue_png, BinaryData::knob_blue_pngSize))
{
}

KnobBlue::~KnobBlue()
{
}
